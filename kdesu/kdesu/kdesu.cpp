/* vi: ts=8 sts=4 sw=4
 * kate: space-indent on; indent-width 2; replace-tabs on; mixed-indent off;
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1998 Pietro Iglio <iglio@fub.it>
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 */

#include <config.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/resource.h>
#if defined(HAVE_SYS_WAIT_H)
#include <sys/wait.h>
#endif

#include <qstring.h>
#include <qfileinfo.h>
#include <qglobal.h>
#include <qfile.h>
#include <qdir.h>

#include <dcopclient.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kuser.h>

#include <kdesu/defaults.h>
#include <kdesu/su.h>
#include <kdesu/client.h>

#include "sudlg.h"

#define ERR strerror(errno)

QCString command;
const char *Version = "1.0";

// NOTE: if you change the position of the -u switch, be sure to adjust it
// at the beginning of main()
static KCmdLineOptions options[] = {
    { "+command", I18N_NOOP("Specifies the command to run"), 0 },
    { "c <command>", I18N_NOOP("Specifies the command to run"), "" },
    { "f <file>", I18N_NOOP("Run command under target uid if <file> is not writable"), "" },
    { "u <user>", I18N_NOOP("Specifies the target uid"), "root" },
    { "n", I18N_NOOP("Do not keep password"), 0 },
    { "s", I18N_NOOP("Stop the daemon (forgets all passwords)"), 0 },
    { "t", I18N_NOOP("Enable terminal output (no password keeping)"), 0 },
    { "p <prio>", I18N_NOOP("Set priority value: 0 <= prio <= 100, 0 is lowest"), "50" },
    { "r", I18N_NOOP("Use realtime scheduling"), 0 },
    { "nonewdcop", I18N_NOOP("Let command use existing dcopserver"), 0 },
    { "noignorebutton", I18N_NOOP("Do not display ignore button"), 0 },
    { "i <icon name>", I18N_NOOP("Specify icon to use in the password dialog"), 0},
    { "d", I18N_NOOP("Do not show the command to be run in the dialog"), 0},
    KCmdLineLastOption
};


QCString dcopNetworkId()
{
    QCString result;
    result.resize(1025);
    QFile file(DCOPClient::dcopServerFile());
    if (!file.open(IO_ReadOnly))
        return "";
    int i = file.readLine(result.data(), 1024);
    if (i <= 0)
        return "";
    result.data()[i-1] = '\0'; // strip newline
    return result;
}

static int startApp();

int main(int argc, char *argv[])
{
    // FIXME: this can be considered a poor man's solution, as it's not
    // directly obvious to a gui user. :)
    // anyway, i vote against removing it even when we have a proper gui
    // implementation.  -- ossi
    const char *duser = ::getenv("ADMIN_ACCOUNT");
    if (duser && duser[0])
        options[3].def = duser;

    KAboutData aboutData("kdesu", I18N_NOOP("KDE su"),
            Version, I18N_NOOP("Runs a program with elevated privileges."),
            KAboutData::License_Artistic,
            "Copyright (c) 1998-2000 Geert Jansen, Pietro Iglio");
    aboutData.addAuthor("Geert Jansen", I18N_NOOP("Maintainer"),
            "jansen@kde.org", "http://www.stack.nl/~geertj/");
    aboutData.addAuthor("Pietro Iglio", I18N_NOOP("Original author"),
            "iglio@fub.it");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication::disableAutoDcopRegistration();
    // kdesu doesn't process SM events, so don't even connect to ksmserver
    QCString session_manager = getenv( "SESSION_MANAGER" );
    unsetenv( "SESSION_MANAGER" );
    KApplication app;
    // but propagate it to the started app
    if (session_manager.data())
    {
        setenv( "SESSION_MANAGER", session_manager.data(), 1 );
    }

    {
        KStartupInfoId id;
        id.initId( kapp->startupId());
        id.setupStartupEnv(); // make DESKTOP_STARTUP_ID env. var. available again
    }

    int result = startApp();

    if (result == 127)
    {
        KMessageBox::sorry(0, i18n("Command '%1' not found.").arg(command));
    }

    return result;
}

static int startApp()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    // Stop daemon and exit?
    if (args->isSet("s"))
    {
        KDEsuClient client;
        if (client.ping() == -1)
        {
            kdError(1206) << "Daemon not running -- nothing to stop\n";
            exit(1);
        }
        if (client.stopServer() != -1)
        {
            kdDebug(1206) << "Daemon stopped\n";
            exit(0);
        }
        kdError(1206) << "Could not stop daemon\n";
        exit(1);
    }

    QString icon;
    if ( args->isSet("i"))
	icon = args->getOption("i");

    bool prompt = true;
    if ( args->isSet("d"))
	prompt = false;

    // Get target uid
    QCString user = args->getOption("u");
    QCString auth_user = user;
    struct passwd *pw = getpwnam(user);
    if (pw == 0L)
    {
        kdError(1206) << "User " << user << " does not exist\n";
        exit(1);
    }
    bool change_uid = (getuid() != pw->pw_uid);

    // If file is writeable, do not change uid
    QString file = QFile::decodeName(args->getOption("f"));
    if (change_uid && !file.isEmpty())
    {
        if (file.at(0) != '/')
        {
            KStandardDirs dirs;
            dirs.addKDEDefaults();
            file = dirs.findResource("config", file);
            if (file.isEmpty())
            {
                kdError(1206) << "Config file not found: " << file << "\n";
                exit(1);
            }
        }
        QFileInfo fi(file);
        if (!fi.exists())
        {
            kdError(1206) << "File does not exist: " << file << "\n";
            exit(1);
        }
        change_uid = !fi.isWritable();
    }

    // Get priority/scheduler
    QCString tmp = args->getOption("p");
    bool ok;
    int priority = tmp.toInt(&ok);
    if (!ok || (priority < 0) || (priority > 100))
    {
        KCmdLineArgs::usage(i18n("Illegal priority: %1").arg(tmp));
        exit(1);
    }
    int scheduler = SuProcess::SchedNormal;
    if (args->isSet("r"))
        scheduler = SuProcess::SchedRealtime;
    if ((priority > 50) || (scheduler != SuProcess::SchedNormal))
    {
        change_uid = true;
        auth_user = "root";
    }

    // Get command
    if (args->isSet("c"))
    {
        command = args->getOption("c");
        for (int i=0; i<args->count(); i++)
        {
            QString arg = QFile::decodeName(args->arg(i));
            KRun::shellQuote(arg);
            command += " ";
            command += QFile::encodeName(arg);
        }
    }
    else
    {
        if( args->count() == 0 )
        {
            KCmdLineArgs::usage(i18n("No command specified."));
            exit(1);
        }
        command = args->arg(0);
        for (int i=1; i<args->count(); i++)
        {
            QString arg = QFile::decodeName(args->arg(i));
            KRun::shellQuote(arg);
            command += " ";
            command += QFile::encodeName(arg);
        }
    }

    // Don't change uid if we're don't need to.
    if (!change_uid)
    {
        int result = system(command);
        result = WEXITSTATUS(result);
        return result;
    }

    // Check for daemon and start if necessary
    bool just_started = false;
    bool have_daemon = true;
    KDEsuClient client;
    if (!client.isServerSGID())
    {
        kdWarning(1206) << "Daemon not safe (not sgid), not using it.\n";
        have_daemon = false;
    }
    else if (client.ping() == -1)
    {
        if (client.startServer() == -1)
        {
            kdWarning(1206) << "Could not start daemon, reduced functionality.\n";
            have_daemon = false;
        }
        just_started = true;
    }

    // Try to exec the command with kdesud.
    bool keep = !args->isSet("n") && have_daemon;
    bool terminal = true;
    bool new_dcop = args->isSet("newdcop");
    bool withIgnoreButton = args->isSet("ignorebutton");

    QCStringList env;
    QCString options;
    env << ( "DESKTOP_STARTUP_ID=" + kapp->startupId());

    if (pw->pw_uid)
    {
       // Only propagate KDEHOME for non-root users,
       // root uses KDEROOTHOME

       // Translate the KDEHOME of this user to the new user.
       QString kdeHome = KGlobal::dirs()->relativeLocation("home", KGlobal::dirs()->localkdedir());
       if (kdeHome[0] != '/')
          kdeHome.prepend("~/");
       else
          kdeHome=QString::null; // Use default

       env << ("KDEHOME="+ QFile::encodeName(kdeHome));
    }

    KUser u;
    env << (QCString) ("KDESU_USER=" + u.loginName().local8Bit());

    if (!new_dcop)
    {
        QCString ksycoca = "KDESYCOCA="+QFile::encodeName(locateLocal("cache", "ksycoca"));
        env << ksycoca;

        options += "xf"; // X-only, dcop forwarding enabled.
    }

    if (keep && !terminal && !just_started)
    {
        client.setPriority(priority);
        client.setScheduler(scheduler);
        int result = client.exec(command, user, options, env);
        if (result == 0)
        {
           result = client.exitCode();
           return result;
        }
    }

    // Set core dump size to 0 because we will have
    // root's password in memory.
    struct rlimit rlim;
    rlim.rlim_cur = rlim.rlim_max = 0;
    if (setrlimit(RLIMIT_CORE, &rlim))
    {
        kdError(1206) << "rlimit(): " << ERR << "\n";
        exit(1);
    }

    // Read configuration
    KConfig *config = KGlobal::config();
    config->setGroup("Passwords");
    int timeout = config->readNumEntry("Timeout", defTimeout);

    // Check if we need a password
    SuProcess proc;
    proc.setUser(auth_user);
    int needpw = proc.checkNeedPassword();
    if (needpw < 0)
    {
        QString err = i18n("Su returned with an error.\n");
        KMessageBox::error(0L, err);
        exit(1);
    }
    if (needpw == 0)
    {
        keep = 0;
        kdDebug() << "Don't need password!!\n";
    }

    // Start the dialog
    QCString password;
    if (needpw)
    {
        KStartupInfoId id;
        id.initId( kapp->startupId());
        KStartupInfoData data;
        data.setSilent( KStartupInfoData::Yes );
        KStartupInfo::sendChange( id, data );
        KDEsuDialog dlg(user, auth_user, keep && !terminal,icon, withIgnoreButton);
	if (prompt)
	    dlg.addLine(i18n("Command:"), command);
        if ((priority != 50) || (scheduler != SuProcess::SchedNormal))
        {
            QString prio;
            if (scheduler == SuProcess::SchedRealtime)
                prio += i18n("realtime: ");
            prio += QString("%1/100").arg(priority);
	    if (prompt)
		dlg.addLine(i18n("Priority:"), prio);
        }
        int ret = dlg.exec();
        if (ret == KDEsuDialog::Rejected)
        {
            KStartupInfo::sendFinish( id );
            exit(0);
        }
        if (ret == KDEsuDialog::AsUser)
            change_uid = false;
        password = dlg.password();
        keep = dlg.keep();
        data.setSilent( KStartupInfoData::No );
        KStartupInfo::sendChange( id, data );
    }

    // Some events may need to be handled (like a button animation)
    kapp->processEvents();

    // Run command
    if (!change_uid)
    {
        int result = system(command);
        result = WEXITSTATUS(result);
        return result;
    }
    else if (keep && have_daemon)
    {
        client.setPass(password, timeout);
        client.setPriority(priority);
        client.setScheduler(scheduler);
        int result = client.exec(command, user, options, env);
        if (result == 0)
        {
            result = client.exitCode();
            return result;
        }
    } else
    {
        SuProcess proc;
        proc.setTerminal(terminal);
        proc.setErase(true);
        proc.setUser(user);
        if (!new_dcop)
        {
            proc.setXOnly(true);
            proc.setDCOPForwarding(true);
        }
        proc.setEnvironment(env);
        proc.setPriority(priority);
        proc.setScheduler(scheduler);
        proc.setCommand(command);
        int result = proc.exec(password);
        return result;
    }
    return -1;
}

