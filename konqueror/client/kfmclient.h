/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>

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

#ifndef __kfmclient_h
#define __kfmclient_h

#include <kapplication.h>
#include <krun.h>

class clientApp : public KApplication
{
  Q_OBJECT
public:
  /** Parse command-line arguments and "do it" */
  static bool doIt();

  /** Make konqueror open a window for @p url */
  static bool createNewWindow(const KURL & url, bool newTab, bool tempFile, const QString & mimetype = QString::null);

  /** Make konqueror open a window for @p profile, @p url and @p mimetype */
  static bool openProfile(const QString & profile, const QString & url, const QString & mimetype = QString::null);

protected slots:
  void slotResult( KIO::Job * );
  void delayedQuit();
  void slotDialogCanceled();

private:
  static void sendASNChange();
  static bool m_ok;
  static QCString startup_id_str;

};

#endif
