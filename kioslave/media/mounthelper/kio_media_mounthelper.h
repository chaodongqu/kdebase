/* This file is part of the KDE project
   Copyright (c) 2004 K�vin Ottens <ervin ipsquad net>
   Parts of this file are
   Copyright 2003 Waldo Bastian <bastian@kde.org>

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

#ifndef _KIO_MEDIA_MOUNTHELPER_H_
#define _KIO_MEDIA_MOUNTHELPER_H_

#include <kapplication.h>
#include <qstring.h>
#include <kio/job.h>

#include "medium.h"

class Dialog;

class MountHelper : public KApplication
{
        Q_OBJECT
public:
	MountHelper();

private:
	const Medium findMedium(const KURL &url);
	void invokeEject(const QString &device, bool quiet=false);
	QString m_errorStr;
	bool m_isCdrom;
	QString m_mediumId;
	Dialog *dialog;

private slots:
	void slotSendPassword();
	void slotCancel();
	void ejectFinished(KProcess* proc);
	void error();

signals:
	void signalPasswordError(QString errorMsg);
};

#endif
