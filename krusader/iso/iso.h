/***************************************************************************
                                iso.h
                             -------------------
    begin                : Oct 25 2002
    copyright            : (C) 2002 by Szombathelyi Gy�rgy
    email                : gyurco@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

 /* This file is heavily based on tar.h from kdebase
  * (c) David Faure <faure@kde.org>
  */

#ifndef _ISO_H
#define _ISO_H

#include <kio/slavebase.h>
#include <sys/types.h>
#include "kisofile.h"

class KIso;

class kio_isoProtocol : public KIO::SlaveBase
{
public:
    kio_isoProtocol( const QCString &pool, const QCString &app );
    virtual ~kio_isoProtocol();

    virtual void listDir( const KURL & url );
    virtual void stat( const KURL & url );
    virtual void get( const KURL & url );

protected:
    void getFile( const KIsoFile *isoFileEntry, const QString &path );
    void createUDSEntry( const KArchiveEntry * isoEntry, KIO::UDSEntry & entry );
    bool checkNewFile( QString fullPath, QString & path, int startsec );

    KIso * m_isoFile;
    time_t m_mtime;
    int m_mode;
};

#endif
