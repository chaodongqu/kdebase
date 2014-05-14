/***************************************************************************
                                main.cpp
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description 
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// KDE includes
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ksplashscreen.h>
#include <unistd.h>
#include <signal.h>
#include <qeventloop.h>

// Krusader includes
#include "krusader.h"
#include "krslots.h"
#include "krusaderapp.h"
#include "defaults.h"
#include <dcopclient.h>
#include <kstartupinfo.h>
#include <stdlib.h>

static const char *description =
	I18N_NOOP("Krusader\nTwin-Panel File Manager for KDE");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE

static KCmdLineOptions options[] =
{
  { "left <path>", I18N_NOOP("Start left panel at <path>"), 0},
  { "right <path>", I18N_NOOP("Start right panel at <path>"), 0},
  { "profile <panel-profile>", I18N_NOOP("Load this profile on startup"), 0},
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

static void sigterm_handler(int i)
{
  fprintf(stderr,"Signal: %d\n",i);

  QApplication::eventLoop()->wakeUp();
  KApplication::exit( - 15 );
}

int main(int argc, char *argv[]) {

// ============ begin icon-stuff ===========
// If the user has no icon specified over the commandline we set up uor own.
// this is acording to the users privileges. The icons are in Krusader::privIcon()

  bool hasIcon = false;
  int i = 0;
  char * myArgv[argc+2];

 // if no --miniicon is given, --icon is used. So we don't need to check for --miniicon separately
  for ( i = 0; i < argc; ++i ) {
    if ( strstr(argv[ i ], "-icon") != 0 ) // important: just one dash because both can appear!
     hasIcon = true;
  }

  static const char* const icon_text="--icon";
  const char* icon_name=Krusader::privIcon();
  char addedParams[strlen(icon_text)+strlen(icon_name)+2];

  if ( ! hasIcon ) {
    for ( i = 0; i < argc; ++i )
      myArgv[ i ] = argv[ i ];

    strcpy(addedParams,icon_text);
    strcpy(addedParams+strlen(icon_text)+1,icon_name);

    myArgv[ argc ] = addedParams;
    myArgv[ ++argc ] = addedParams+strlen(icon_text)+1;
    myArgv[ ++argc ] = 0;

    argv = myArgv;
  }
// ============ end icon-stuff ===========

  // ABOUT data information
#ifdef RELEASE_NAME
  QString versionName = QString("%1 \"%2\"").arg(VERSION).arg(RELEASE_NAME);
#else
  QString versionName = VERSION;
#endif
  KAboutData aboutData( "krusader", ( geteuid() ? I18N_NOOP("Krusader") :
                        I18N_NOOP("Krusader - ROOT PRIVILEGES")),
    versionName.latin1(), description, KAboutData::License_GPL,
    "(c) 2000-2003, Shie Erlich, Rafi Yanai\n(c) 2004-2007, Krusader Krew",
    I18N_NOOP("Feedback\nhttp://www.krusader.org/phpBB/\n\n"
              "IRC\nserver: irc.freenode.net, channel: #krusader"),
    "http://www.krusader.org",
    "krusader@users.sourceforge.net");

  aboutData.addAuthor("Rafi Yanai","Author", "yanai@users.sourceforge.net");
  aboutData.addAuthor("Shie Erlich","Author", "erlich@users.sourceforge.net");
  aboutData.addAuthor("Karai Csaba", "Developer", "ckarai@users.sourceforge.net", 0);
  aboutData.addAuthor("Heiner Eichmann","Developer", "h.eichmann@gmx.de", 0);  
  aboutData.addAuthor("Jonas Baehr", "Developer", "jonas.baehr@web.de", 0);
  aboutData.addAuthor("Dirk Eschler", "Webmaster and i18n coordinator", "deschler@users.sourceforge.net", 0);
  aboutData.addAuthor("Frank Schoolmeesters", "Documentation and marketing coordinator", "frank_schoolmeesters@yahoo.com", 0);
  aboutData.addAuthor("Richard Holt", "Documentation & Proofing", "richard.holt@gmail.com", 0);
  aboutData.addAuthor("Matej Urbancic", " Marketing & Product Research", "matej.urban@gmail.com", 0);
  aboutData.addCredit("Václav Juza", "QA, bug-hunting, patches and general help", "vaclavjuza@gmail.com", 0);
  aboutData.addCredit("Jiri Palecek", "QA, bug-hunting, patches and general help", "jpalecek@web.de", 0);
  aboutData.addCredit("Andrew Neupokoev", "Killer Logo and Icons for Krusader (contest winner)", "doom-blue@yandex.ru", 0);
  aboutData.addCredit("The UsefulArts Organization", "Icon for krusader", "mail@usefularts.rg", 0);
  aboutData.addCredit("Gábor Lehel", "Viewer module for 3rd Hand", "illissius@gmail.com", 0);
  aboutData.addCredit("Mark Eatough", "Handbook Proof-Reader", "markeatough@yahoo.com", 0);
  aboutData.addCredit("Jan Halasa", "The old Bookmark Module", "xhalasa@fi.muni.cz", 0);
  aboutData.addCredit("Hans Loeffler", "Dir history button", 0, 0);
  aboutData.addCredit("Szombathelyi György", "ISO KIO slave", 0, 0);
  aboutData.addCredit("Jan Willem van de Meent (Adios)", "Icons for Krusader", "janwillem@lorentz.leidenuniv.nl", 0);
  aboutData.addCredit("Mikolaj Machowski", "Usability and QA", "<mikmach@wp.pl>", 0);
  aboutData.addCredit("Cristi Dumitrescu","QA, bug-hunting, patches and general help","cristid@chip.ro",0);
  aboutData.addCredit("Aurelien Gateau","patch for KViewer","aurelien.gateau@free.fr",0);
  aboutData.addCredit("Milan Brabec","the first patch ever !","mbrabec@volny.cz",0);
  aboutData.addCredit("Asim Husanovic","Bosnian translation","asim@megatel.ba",0);
  aboutData.addCredit("Doutor Zero","Brazilian Portuguese","doutor.zero@gmail.com",0);
  aboutData.addCredit("Milen Ivanov","Bulgarian translation","milen.ivanov@abv.bg",0);
  aboutData.addCredit("Quim Perez","Catalan translation","noguer@osona.com",0);
  aboutData.addCredit("Jinghua Luo","Chinese Simplified translation","luojinghua@msn.com",0);
  aboutData.addCredit("Mitek","Old Czech translation","mitek@email.cz",0);
  aboutData.addCredit("Martin Sixta","Old Czech translation","lukumo84@seznam.cz",0);
  aboutData.addCredit("Vaclav Juza","Czech translation","VaclavJuza@gmail.com",0);
  aboutData.addCredit("Anders Bruun Olsen", "Old Danish translation", "anders@bruun-olsen.net", 0);
  aboutData.addCredit("Peter H. Sorensen", "Danish translation", "peters@skydebanen.net", 0);
  aboutData.addCredit("Frank Schoolmeesters","Dutch translation","frank_schoolmeesters@yahoo.com",0);
  aboutData.addCredit("Rene-Pierre Lehmann","Old French translation","ripi@lepi.org",0);
  aboutData.addCredit("David Guillerm","French translation","dguillerm@gmail.com",0);
  aboutData.addCredit("Christoph Thielecke","Old German translation","crissi99@gmx.de",0);
  aboutData.addCredit("Dirk Eschler", "German translation", "deschler@users.sourceforge.net", 0);
  aboutData.addCredit("Spiros Georgaras", "Greek translation", "sngeorgaras@gmail.com", 0);
  aboutData.addCredit("Kukk Zoltan","Old Hungarian translation","kukkzoli@freemail.hu",0);
  aboutData.addCredit("Arpad Biro","Hungarian translation","biro_arpad@yahoo.com",0);
  aboutData.addCredit("Giuseppe Bordoni", "Italian translation", "geppo@geppozone.com", 0);
  aboutData.addCredit("Hideki Kimura", "Japanese translation", "hangyo1973@gmail.com", 0);
  aboutData.addCredit("UTUMI Hirosi", "Old Japanese translation", "utuhiro@mx12.freecom.ne.jp", 0);
  aboutData.addCredit("Dovydas Sankauskas", "Lithuanian translation", "laisve@gmail.com", 0);
  aboutData.addCredit("Bruno Queiros", "Portuguese translation", "brunoqueiros@portugalmail.com", 0);
  aboutData.addCredit("Lukasz Janyst","Old Polish translation","ljan@wp.pl",0);
  aboutData.addCredit("Pawel Salawa","Polish translation","boogie@myslenice.one.pl",0);
  aboutData.addCredit("Tomek Grzejszczyk","Polish translation","tgrzej@onet.eu",0);
  aboutData.addCredit("Dmitry A. Bugay","Russian translation","sam@vhnet.ru",0);
  aboutData.addCredit("Dmitry Chernyak","Old Russian translation","chernyak@mail.ru",0);
  aboutData.addCredit("Sasa Tomic","Serbian translation","stomic@gmx.net",0);
  aboutData.addCredit("Zdenko Podobna","Slovak translation","zdpo@mailbox.sk",0);
  aboutData.addCredit("Matej Urbancic","Slovenian translation","matej.urban@gmail.com",0);
  aboutData.addCredit("Rafael Munoz","Old Spanish translation","muror@hotpop.com",0);
  aboutData.addCredit("Alejandro Araiza Alvarado","Spanish translation","mebrelith@gmail.com",0);
  aboutData.addCredit("Erik Johanssen","Old Swedish translation","erre@telia.com",0);
  aboutData.addCredit("Anders Linden","Old Swedish translation","connyosis@gmx.net",0);
  aboutData.addCredit("Peter Landgren","Swedish translation","peter.talken@telia.com",0);
  aboutData.addCredit("Bekir Sonat","Turkish translation","bekirsonat@kde.org.tr",0);
  aboutData.addCredit("Ivan Petrouchtchak","Ukrainian translation","connyosis@gmx.net",0);

  // Command line arguments ...
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  // check for command line arguments

  // create the application
  KrusaderApp app;

  {
    KConfigGroupSaver saver(app.config(), "Look&Feel");
    bool singleInstanceMode = app.config()->readBoolEntry( "Single Instance Mode", _SingleInstanceMode );

    // register with the dcop server
    DCOPClient* client = KApplication::kApplication() ->dcopClient();
    if ( !client->attach() )
       exit( 0 );
    QCString regName = client->registerAs( KApplication::kApplication() ->name(), !singleInstanceMode );
    if( singleInstanceMode && regName != KApplication::kApplication()->name() ) {
      fprintf( stderr, i18n( "Application already running!\n" ).ascii() );

      DCOPClient::mainClient()->send( KApplication::kApplication() ->name(), "Krusader-Interface",
                                    "moveToTop()", QByteArray() );
      KStartupInfo::appStarted();

      return 1;
    }
  }
    
  // splash screen - if the user wants one
  KSplashScreen *splash = 0;
  { // don't remove bracket
  KConfigGroupSaver saver(app.config(), "Look&Feel");
  if (app.config()->readBoolEntry( "Show splashscreen", _ShowSplashScreen )) {
  	QString splashFilename = locate( "data", "krusader/splash.png" );
  	QPixmap pixmap( splashFilename );
  	if (!pixmap.isNull()) {
    		splash = new KSplashScreen( pixmap );
    		splash->show();
  	}
  }
  } // don't remove bracket

  Krusader krusader;
  
  // catching SIGTERM, SIGHUP, SIGQUIT
  signal(SIGTERM,sigterm_handler);
  signal(SIGPIPE,sigterm_handler);
  signal(SIGHUP,sigterm_handler);

  // make sure we receive X's focus in/out events
  QObject::connect(&app, SIGNAL(windowActive()), krusader.slot, SLOT(windowActive()));
  QObject::connect(&app, SIGNAL(windowInactive()), krusader.slot, SLOT(windowInactive()));

  // and set krusader to be the main widget in it
  app.setMainWidget(&krusader);
  
  // hide splashscreen
  if (splash) {
    splash->finish( &krusader );
    delete splash;
  }

  // let's go.
  return app.exec();
}
