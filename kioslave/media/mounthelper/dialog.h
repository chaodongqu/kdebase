/*  This file is part of the KDE project
 *  Copyright (C) 2007  Jan Kl�tzke <jan kloetzke at freenet de>
 *
 *  Based on kryptomedia- Another KDE cryto media application.
 *  Copyright (C) 2006  Daniel Gollub <dgollub@suse.de>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef DIALOG_H_
#define DIALOG_H_

#include <kmessagebox.h>
#include <klocale.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>

#include <qlineedit.h>
#include <qlabel.h>
#include <qgroupbox.h>

#include "decryptdialog.h"

class KryptoMedia;

class Dialog : public KDialogBase
{

Q_OBJECT

public:
	Dialog(QString url, QString iconName);
	~Dialog();

	QString getPassword();

public slots:
	void slotDialogError(QString errorMsg);
	void slotPasswordChanged(const QString &text);

private:
	DecryptDialog *decryptDialog;
};

#endif // DIALOG_H_


