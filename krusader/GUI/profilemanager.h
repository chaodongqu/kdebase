/***************************************************************************
                       profilemanager.h  -  description
                             -------------------
    copyright            : (C) 2004 + by Csaba Karai
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

#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <qpushbutton.h>
#include <qstring.h>

class ProfileManager : public QPushButton
{
  Q_OBJECT
  
public: 
  ProfileManager( QString profileType, QWidget * parent = 0, const char * name = 0 );
  
  /**
   * @param profileType Type of the profile (sync, search, ...)
   * @return A list of all available profile-names
   */
  static QStringList availableProfiles( QString profileType );

  QStringList getNames();
    
public slots:
  void profilePopup();
  
  void newProfile( QString defaultName = QString::null );
  void deleteProfile( QString name );
  void overwriteProfile( QString name );
  bool loadProfile( QString name );  
  
signals:
  void saveToProfile( QString profileName );
  void loadFromProfile( QString profileName );
  
private:
  QString profileType;
  QStringList profileList;
};

#endif /* PROFILEMANAGER_H */
