// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project
   Copyright (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <qcheckbox.h>
#include <qevent.h>
#include <qgroupbox.h>
#include <qheader.h>
#include <qradiobutton.h>
#include <qvbox.h>

#include <kdialogbase.h>
#include <keditlistbox.h>
#include <kkeydialog.h>
#include <klistview.h>
#include <knuminput.h>

class KGlobalAccel;
class KKeyChooser;
class KListView;
class QPushButton;
class QDialog;
class ConfigDialog;

class ConfigDialog : public KDialogBase
{
    Q_OBJECT

public:
    ConfigDialog(KGlobalAccel *accel, bool isApplet );
   ~ConfigDialog();

    virtual void show();
    void commitShortcuts();

private:
    KKeyChooser *keysWidget;

};

class ListView : public KListView
{
public:
    ListView( ConfigDialog* configWidget, QWidget *parent, const char *name )
	: KListView( parent, name ), _configWidget( configWidget ),
          _regExpEditor(0L) {}
    // QListView has a weird idea of a sizeHint...
    virtual QSize sizeHint () const {
	int w = minimumSizeHint().width();
	int h = header()->height();
	h += viewport()->sizeHint().height();
	h += horizontalScrollBar()->height();

	QListViewItem *item = firstChild();
	while ( item ) {
	    h += item->totalHeight();
	    item = item->nextSibling();
	}

	return QSize( w, h );
    }

protected:
    virtual void rename( QListViewItem* item, int c );
private:
    ConfigDialog* _configWidget;
    QDialog* _regExpEditor;
};

#endif // CONFIGDIALOG_H
