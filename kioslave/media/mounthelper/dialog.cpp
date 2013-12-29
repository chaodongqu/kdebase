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

#include "dialog.h"

Dialog::Dialog(QString url, QString iconName) :
	KDialogBase(NULL, "Dialog", true, "Decrypt Storage Device", (Cancel|User1), User1, false, KGuiItem(i18n("Decrypt"), "decrypted" ))
{
	decryptDialog = new DecryptDialog(this);

	decryptDialog->errorBox->hide();
	decryptDialog->descLabel->setText(decryptDialog->descLabel->text().arg(url));
	decryptDialog->descLabel->adjustSize();
	decryptDialog->adjustSize();

	enableButton( User1, false );

	QPixmap pixmap = KGlobal::iconLoader()->loadIcon(iconName, KIcon::NoGroup, KIcon::SizeLarge);
	decryptDialog->encryptedIcon->setPixmap( pixmap );

	connect(decryptDialog->passwordEdit, SIGNAL (textChanged(const QString &)), this, SLOT (slotPasswordChanged(const QString &)));

	setMainWidget(decryptDialog);
}

Dialog::~Dialog()
{
	delete decryptDialog;
}

QString Dialog::getPassword()
{
	return decryptDialog->passwordEdit->text();
}

void Dialog::slotDialogError(QString errorMsg)
{
	kdDebug() << __func__ << "(" << errorMsg << " )" << endl;

	decryptDialog->errorLabel->setText(QString("<b>%1</b>").arg(errorMsg));
	decryptDialog->errorBox->show();
}

void Dialog::slotPasswordChanged(const QString &text)
{
	enableButton( User1, !text.isEmpty() );
}

#include "dialog.moc"

