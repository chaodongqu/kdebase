/* This file is part of the KDE project
   Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <kio/slavebase.h>
#include <kinstance.h>
#include <kdebug.h>
#include <qtextstream.h>
#include <klocale.h>
#include <sys/stat.h>
#include <dcopclient.h>
#include <qdatastream.h>
#include <time.h>
#include <kprocess.h>
#include <kservice.h>
#include <kservicegroup.h>
#include <kstandarddirs.h>

class SettingsProtocol : public KIO::SlaveBase
{
public:
	enum RunMode { SettingsMode, ProgramsMode, ApplicationsMode };
	SettingsProtocol(const QCString &protocol, const QCString &pool, const QCString &app);
	virtual ~SettingsProtocol();
	virtual void get( const KURL& url );
	virtual void stat(const KURL& url);
	virtual void listDir(const KURL& url);
	void listRoot();
	KServiceGroup::Ptr findGroup(const QString &relPath);

private:
	DCOPClient *m_dcopClient;
	RunMode m_runMode;
};

extern "C" {
	KDE_EXPORT int kdemain( int, char **argv )
	{
	  kdDebug() << "kdemain for settings kioslave" << endl;
	  KInstance instance( "kio_settings" );
	  SettingsProtocol slave(argv[1], argv[2], argv[3]);
	  slave.dispatchLoop();
	  return 0;
	}
}


static void addAtom(KIO::UDSEntry& entry, unsigned int ID, long l, const QString& s = QString::null)
{
	KIO::UDSAtom atom;
	atom.m_uds = ID;
	atom.m_long = l;
	atom.m_str = s;
	entry.append(atom);
}

static void createFileEntry(KIO::UDSEntry& entry, const QString& name, const QString& url, const QString& mime, const QString& iconName, const QString& localPath)
{
	entry.clear();
	addAtom(entry, KIO::UDS_NAME, 0, KIO::encodeFileName(name));
	addAtom(entry, KIO::UDS_FILE_TYPE, S_IFREG);
	addAtom(entry, KIO::UDS_URL, 0, url);
	addAtom(entry, KIO::UDS_ACCESS, 0500);
	addAtom(entry, KIO::UDS_MIME_TYPE, 0, mime);
	addAtom(entry, KIO::UDS_SIZE, 0);
	addAtom(entry, KIO::UDS_LOCAL_PATH, 0, localPath);
	addAtom(entry, KIO::UDS_CREATION_TIME, 1);
	addAtom(entry, KIO::UDS_MODIFICATION_TIME, time(0));
	addAtom(entry, KIO::UDS_ICON_NAME, 0, iconName);
}

static void createDirEntry(KIO::UDSEntry& entry, const QString& name, const QString& url, const QString& mime,const QString& iconName)
{
	entry.clear();
	addAtom(entry, KIO::UDS_NAME, 0, name);
	addAtom(entry, KIO::UDS_FILE_TYPE, S_IFDIR);
	addAtom(entry, KIO::UDS_ACCESS, 0500);
	addAtom(entry, KIO::UDS_MIME_TYPE, 0, mime);
	addAtom(entry, KIO::UDS_URL, 0, url);
	addAtom(entry, KIO::UDS_SIZE, 0);
	addAtom(entry, KIO::UDS_ICON_NAME, 0, iconName);
}

SettingsProtocol::SettingsProtocol( const QCString &protocol, const QCString &pool, const QCString &app): SlaveBase( protocol, pool, app )
{
	// Adjusts which part of the K Menu to virtualize.
	if ( protocol == "programs" )
		m_runMode = ProgramsMode;
	else
		if (protocol == "applications")
			m_runMode = ApplicationsMode;
		else
			m_runMode = SettingsMode;

	m_dcopClient = new DCOPClient();
	if (!m_dcopClient->attach())
	{
		kdDebug() << "ERROR WHILE CONNECTING TO DCOPSERVER" << endl;
	}
}

SettingsProtocol::~SettingsProtocol()
{
	delete m_dcopClient;
}

KServiceGroup::Ptr SettingsProtocol::findGroup(const QString &relPath)
{
	QString nextPart;
	QString alreadyFound("Settings/");
	QStringList rest = QStringList::split('/', relPath);

	kdDebug() << "Trying harder to find group " << relPath << endl;
	for (unsigned int i=0; i<rest.count(); i++)
		kdDebug() << "Item (" << *rest.at(i) << ")" << endl;

	while (!rest.isEmpty()) {
		KServiceGroup::Ptr tmp = KServiceGroup::group(alreadyFound);
		if (!tmp || !tmp->isValid())
			return 0;

		KServiceGroup::List list = tmp->entries(true, true);
		KServiceGroup::List::ConstIterator it = list.begin();

		bool found = false;
		for (; it != list.end(); ++it) {
			KSycocaEntry *e = *it;
			if (e->isType(KST_KServiceGroup)) {
			    KServiceGroup::Ptr g(static_cast<KServiceGroup *>(e));
			    if ((g->caption()==rest.front()) || (g->name()==alreadyFound+rest.front())) {
				kdDebug() << "Found group with caption " << g->caption()
					  << " with real name: " << g->name() << endl;
				found = true;
				rest.remove(rest.begin());
				alreadyFound = g->name();
				kdDebug() << "ALREADY FOUND: " << alreadyFound << endl;
				break;
			    }
			}
		}

		if (!found) {
			kdDebug() << "Group with caption " << rest.front() << " not found within "
				  << alreadyFound << endl;
			return 0;
		}

	}
	return KServiceGroup::group(alreadyFound);
}

void SettingsProtocol::get( const KURL & url )
{
	KService::Ptr service = KService::serviceByDesktopName(url.fileName());
	if (service && service->isValid()) {
		KURL redirUrl;
		redirUrl.setPath(locate("apps", service->desktopEntryPath()));
		redirection(redirUrl);
		finished();
	} else {
		error( KIO::ERR_IS_DIRECTORY, url.prettyURL() );
	}
}


void SettingsProtocol::stat(const KURL& url)
{
	KIO::UDSEntry entry;

	QString servicePath( url.path(1) );
	servicePath.remove(0, 1); // remove starting '/'

	if ( m_runMode == SettingsMode)
		servicePath = "Settings/" + servicePath;

	KServiceGroup::Ptr grp = KServiceGroup::group(servicePath);

	if (grp && grp->isValid()) {
		createDirEntry(entry, (m_runMode == SettingsMode) ? i18n("Settings") : ( (m_runMode==ApplicationsMode) ? i18n("Applications") : i18n("Programs")),
			url.url(), "inode/directory",grp->icon() );
	} else {
		KService::Ptr service = KService::serviceByDesktopName( url.fileName() );
		if (service && service->isValid()) {
//			KURL newUrl;
//			newUrl.setPath(locate("apps", service->desktopEntryPath()));
//			createFileEntry(entry, service->name(), newUrl, "application/x-desktop", service->icon());

			createFileEntry(entry, service->name(), url.url(1)+service->desktopEntryName(),
                            "application/x-desktop", service->icon(), locate("apps", service->desktopEntryPath()) );
		} else {
			error(KIO::ERR_SLAVE_DEFINED,i18n("Unknown settings folder"));
			return;
		}
	}

	statEntry(entry);
	finished();
	return;
}


void SettingsProtocol::listDir(const KURL& url)
{
	QString groupPath = url.path(1);
	groupPath.remove(0, 1); // remove starting '/'

	if ( m_runMode == SettingsMode)
		groupPath.prepend("Settings/");

	KServiceGroup::Ptr grp = KServiceGroup::group(groupPath);

	if (!grp || !grp->isValid()) {
		grp = findGroup(groupPath);
		if (!grp || !grp->isValid()) {
		    error(KIO::ERR_SLAVE_DEFINED,i18n("Unknown settings folder"));
		    return;
		}
	}

	unsigned int count = 0;
	KIO::UDSEntry entry;

	KServiceGroup::List list = grp->entries(true, true);
	KServiceGroup::List::ConstIterator it;

	for (it = list.begin(); it != list.end(); ++it) {
		KSycocaEntry * e = *it;

		if (e->isType(KST_KServiceGroup)) {
			KServiceGroup::Ptr g(static_cast<KServiceGroup *>(e));
			QString groupCaption = g->caption();

			// Avoid adding empty groups.
			KServiceGroup::Ptr subMenuRoot = KServiceGroup::group(g->relPath());
			if (subMenuRoot->childCount() == 0)
			    continue;

			// Ignore dotfiles.
			if ((g->name().at(0) == '.'))
			    continue;

			QString relPath = g->relPath();

			// Do not display the "Settings" menu group in Programs Mode.
			if( (m_runMode == ProgramsMode) && relPath.startsWith( "Settings" ) )
			{
				kdDebug() << "SettingsProtocol: SKIPPING entry programs:/" << relPath << endl;
				continue;
			}

			switch( m_runMode )
			{
			  case( SettingsMode ):
				relPath.remove(0, 9); // length("Settings/") ==9
				kdDebug() << "SettingsProtocol: adding entry settings:/" << relPath << endl;
				createDirEntry(entry, groupCaption, "settings:/"+relPath, "inode/directory",g->icon());
				break;
			  case( ProgramsMode ):
				kdDebug() << "SettingsProtocol: adding entry programs:/" << relPath << endl;
				createDirEntry(entry, groupCaption, "programs:/"+relPath, "inode/directory",g->icon());
				break;
			  case( ApplicationsMode ):
				kdDebug() << "SettingsProtocol: adding entry applications:/" << relPath << endl;
				createDirEntry(entry, groupCaption, "applications:/"+relPath, "inode/directory",g->icon());
				break;
		    }

		} else {
			KService::Ptr s(static_cast<KService *>(e));
			kdDebug() << "SettingsProtocol: adding file entry " << url.url(1)+s->name() << endl;
			createFileEntry(entry,s->name(),url.url(1)+s->desktopEntryName(), "application/x-desktop",s->icon(),locate("apps", s->desktopEntryPath()));
		}

		listEntry(entry, false);
		count++;
	}

	totalSize(count);
	listEntry(entry, true);
	finished();
}

// vim: ts=4 sw=4 et
