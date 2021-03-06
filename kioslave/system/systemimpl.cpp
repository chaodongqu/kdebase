/* This file is part of the KDE project
   Copyright (c) 2004 Kevin Ottens <ervin ipsquad net>

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

#include "systemimpl.h"

#include <kdebug.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kdesktopfile.h>

#include <qapplication.h>
#include <qeventloop.h>
#include <qdir.h>

#include <sys/stat.h>

SystemImpl::SystemImpl() : QObject()
{
	KGlobal::dirs()->addResourceType("system_entries",
		KStandardDirs::kde_default("data") + "systemview");
}

bool SystemImpl::listRoot(QValueList<KIO::UDSEntry> &list)
{
	kdDebug() << "SystemImpl::listRoot" << endl;

	QStringList names_found;
	QStringList dirList = KGlobal::dirs()->resourceDirs("system_entries");

	QStringList::ConstIterator dirpath = dirList.begin();
	QStringList::ConstIterator end = dirList.end();
	for(; dirpath!=end; ++dirpath)
	{
		QDir dir = *dirpath;
		if (!dir.exists()) continue;

		QStringList filenames
			= dir.entryList( QDir::Files | QDir::Readable );


		KIO::UDSEntry entry;

		QStringList::ConstIterator filename = filenames.begin();
		QStringList::ConstIterator endf = filenames.end();

		for(; filename!=endf; ++filename)
		{
			if (!names_found.contains(*filename))
			{
				entry.clear();
				createEntry(entry, *dirpath, *filename);
				if ( !entry.isEmpty() )
				{
					list.append(entry);
					names_found.append(*filename);
				}
			}
		}
	}

	return true;
}

bool SystemImpl::parseURL(const KURL &url, QString &name, QString &path) const
{
	QString url_path = url.path();

	int i = url_path.find('/', 1);
	if (i > 0)
	{
		name = url_path.mid(1, i-1);
		path = url_path.mid(i+1);
	}
	else
	{
		name = url_path.mid(1);
		path = QString::null;
	}

	return name != QString::null;
}

bool SystemImpl::realURL(const QString &name, const QString &path,
                         KURL &url) const
{
	url = findBaseURL(name);
	if (!url.isValid())
	{
		return false;
	}

	url.addPath(path);
	return true;
}

bool SystemImpl::statByName(const QString &filename, KIO::UDSEntry& entry)
{
	kdDebug() << "SystemImpl::statByName" << endl;

	QStringList dirList = KGlobal::dirs()->resourceDirs("system_entries");

	QStringList::ConstIterator dirpath = dirList.begin();
	QStringList::ConstIterator end = dirList.end();
	for(; dirpath!=end; ++dirpath)
	{
		QDir dir = *dirpath;
		if (!dir.exists()) continue;

		QStringList filenames
			= dir.entryList( QDir::Files | QDir::Readable );


		QStringList::ConstIterator name = filenames.begin();
		QStringList::ConstIterator endf = filenames.end();

		for(; name!=endf; ++name)
		{
			if (*name==filename+".desktop")
			{
				createEntry(entry, *dirpath, *name);
				return true;
			}
		}
	}

	return false;
}

KURL SystemImpl::findBaseURL(const QString &filename) const
{
	kdDebug() << "SystemImpl::findBaseURL" << endl;

	QStringList dirList = KGlobal::dirs()->resourceDirs("system_entries");

	QStringList::ConstIterator dirpath = dirList.begin();
	QStringList::ConstIterator end = dirList.end();
	for(; dirpath!=end; ++dirpath)
	{
		QDir dir = *dirpath;
		if (!dir.exists()) continue;

		QStringList filenames
			= dir.entryList( QDir::Files | QDir::Readable );


		KIO::UDSEntry entry;

		QStringList::ConstIterator name = filenames.begin();
		QStringList::ConstIterator endf = filenames.end();

		for(; name!=endf; ++name)
		{
			if (*name==filename+".desktop")
			{
				KDesktopFile desktop(*dirpath+filename+".desktop", true);
				if ( desktop.readURL().isEmpty() )
				{
					KURL url;
					url.setPath( desktop.readPath() );
					return url;
				}

				return desktop.readURL();
			}
		}
	}

	return KURL();
}


static void addAtom(KIO::UDSEntry &entry, unsigned int ID, long l,
                    const QString &s = QString::null)
{
	KIO::UDSAtom atom;
	atom.m_uds = ID;
	atom.m_long = l;
	atom.m_str = s;
	entry.append(atom);
}


void SystemImpl::createTopLevelEntry(KIO::UDSEntry &entry) const
{
	entry.clear();
	addAtom(entry, KIO::UDS_NAME, 0, ".");
	addAtom(entry, KIO::UDS_FILE_TYPE, S_IFDIR);
	addAtom(entry, KIO::UDS_ACCESS, 0555);
	addAtom(entry, KIO::UDS_MIME_TYPE, 0, "inode/system_directory");
	addAtom(entry, KIO::UDS_ICON_NAME, 0, "system");
}

void SystemImpl::createEntry(KIO::UDSEntry &entry,
                             const QString &directory,
                             const QString &file)
{
	kdDebug() << "SystemImpl::createEntry" << endl;

	KDesktopFile desktop(directory+file, true);

	kdDebug() << "path = " << directory << file << endl;

	entry.clear();

	// Ensure that we really want this entry to be displayed
	if ( desktop.readURL().isEmpty() && desktop.readPath().isEmpty() )
	{
		return;
	}

	addAtom(entry, KIO::UDS_NAME, 0, desktop.readName());

	QString new_filename = file;
	new_filename.truncate(file.length()-8);

	if ( desktop.readURL().isEmpty() )
	{
		addAtom(entry, KIO::UDS_URL, 0, desktop.readPath());
	}
	else
	{
		addAtom(entry, KIO::UDS_URL, 0, "system:/"+new_filename);
	}

	addAtom(entry, KIO::UDS_FILE_TYPE, S_IFDIR);
	addAtom(entry, KIO::UDS_MIME_TYPE, 0, "inode/directory");

	QString icon = desktop.readIcon();
	QString empty_icon = desktop.readEntry("EmptyIcon");

	if (!empty_icon.isEmpty())
	{
		KURL url = desktop.readURL();

		m_lastListingEmpty = true;

		KIO::ListJob *job = KIO::listDir(url, false, false);
		connect( job, SIGNAL( entries(KIO::Job *,
		                      const KIO::UDSEntryList &) ),
		         this, SLOT( slotEntries(KIO::Job *,
			             const KIO::UDSEntryList &) ) );
		connect( job, SIGNAL( result(KIO::Job *) ),
		         this, SLOT( slotResult(KIO::Job *) ) );
		qApp->eventLoop()->enterLoop();

		if (m_lastListingEmpty) icon = empty_icon;
	}

	addAtom(entry, KIO::UDS_ICON_NAME, 0, icon);
}

void SystemImpl::slotEntries(KIO::Job *job, const KIO::UDSEntryList &list)
{
	if (list.size()>0)
	{
		job->kill(true);
		m_lastListingEmpty = false;
		qApp->eventLoop()->exitLoop();
	}
}

void SystemImpl::slotResult(KIO::Job *)
{
	qApp->eventLoop()->exitLoop();
}


#include "systemimpl.moc"
