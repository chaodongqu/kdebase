/* This file is part of the KDE project
   Copyright (C) 2000 Malte Starostik <malte@kde.org>

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

#include <dcopclient.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kmimetype.h>

#include "konq_faviconmgr.moc"

KonqFavIconMgr::KonqFavIconMgr(QObject *parent, const char *name)
    : QObject(parent, name),
      DCOPObject("KonqFavIconMgr")
{
    connectDCOPSignal("kded", "favicons",
        "iconChanged(bool, QString, QString)",
        "notifyChange(bool, QString, QString)", false);
}

QString KonqFavIconMgr::iconForURL(const QString &url)
{
    return KMimeType::favIconForURL( KURL(url) );
}

void KonqFavIconMgr::setIconForURL(const KURL &url, const KURL &iconURL)
{
    QByteArray data;
    QDataStream str(data, IO_WriteOnly);
    str << url << iconURL;
    kapp->dcopClient()->send("kded", "favicons", "setIconForURL(KURL, KURL)", data);
}

void KonqFavIconMgr::downloadHostIcon(const KURL &url)
{
    QByteArray data;
    QDataStream str(data, IO_WriteOnly);
    str << url;
    kapp->dcopClient()->send("kded", "favicons", "downloadHostIcon(KURL)", data);
}

