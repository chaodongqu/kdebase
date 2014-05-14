/***************************************************************************
                                remoteman.h
                             -------------------
    begin                : Thu May 4 2000
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

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef REMOTEMAN_H
#define REMOTEMAN_H

#include "remotemanbase.h"
#include <qlistview.h>

class remoteMan : public remoteManBase  {
  Q_OBJECT
public:
	remoteMan();
	static QString getHost(); // we call remoteMan mainly with this function
	
protected:
  void config2tree(); // internal
  void tree2config(); // internal
  QListViewItem *findItem(const QString &name, QListViewItem *p);
  	
public slots:
  void addGroup();
  void refreshData()   ;              // called whenver the current session changes
  void updateName(const QString&);    // update the session/group name in real-time
  void updateConnect(const QString&); // enable/disable the connect button in real-time
  void addSession();
  void connection();
  void removeSession();
  void expandDecos(QListViewItem*);
  void collapseDecos(QListViewItem*);

protected slots:
  void accept();
  void reject();

private:
  QListViewItem *currentItem;
  static QString url;
};

#endif
