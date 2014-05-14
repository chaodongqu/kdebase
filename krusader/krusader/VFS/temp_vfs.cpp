/***************************************************************************
                                 temp_vfs.cpp
                             -------------------
    copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
    web site		 : http://krusader.sourceforge.net
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

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// Qt includes
#include <qdir.h>
// KDE includes
#include <kmessagebox.h>
#include <kprocess.h>
// Krusader includes
#include "temp_vfs.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../VFS/krarchandler.h"
#include "../resources.h"
#include "../krservices.h"

temp_vfs::temp_vfs( QString origin, QString type, QWidget* panel, bool ):
          normal_vfs(panel){
  vfs_type=TEMP;
   // first we need to create a temp diretory
  tmpDir = krApp->getTempDir();
  // then we must get the files from the origin to the tmp dir
  if( type == "-arj" || type == "-ace" ) handleAceArj(origin,type);
  else if( type == "-rpm" ) handleRpm(origin);
  else if( type == "-iso" ) handleIso(origin);
  else{
  if (!quietMode) KMessageBox::error(krApp,"Unknown temp_vfs type.");
    return;
  }
}

temp_vfs::~temp_vfs(){
	if( vfs_type == "-iso" ){
		// unmount the ISO image
    KShellProcess umount;
		umount << "umount -f" << tmpDir;
    umount.start(KProcess::Block);
	}
 	// delete the temp dir
 	KShellProcess proc;
 	proc << "rm -rf" << tmpDir;
 	proc.start(KProcess::DontCare);
}

// return the working dir
QString temp_vfs::vfs_workingDir(){
  // get the path inside the archive
  QString path = vfs_origin.path(-1);
  path = path.mid(path.findRev('\\')+1);
  if(path.left(1) != "/") path = "/"+path;
  QDir().mkdir(tmpDir+path);
  return tmpDir+path;
}

bool temp_vfs::vfs_refresh(const KURL& origin){
  KURL backup = vfs_origin;
  vfs_origin = origin;
  vfs_origin.adjustPath(-1);
  // get the directory...
  QString path = origin.path(-1).mid(origin.path(-1).findRev('\\')+1);
  if(path.left(1) =="/") path.remove(0,1);
  if ( !normal_vfs::vfs_refresh(tmpDir+"/"+path) ){
    vfs_origin = backup;
    vfs_origin.adjustPath(-1);
    return false;
  }
  return true;
}

void temp_vfs::handleAceArj(QString origin, QString type){
	// for ace and arj we just unpack to the tmpDir
	if( !KRarcHandler::arcHandled(type) ){
  	if (!quietMode) KMessageBox::error(krApp,"This archive type is NOT supported");
    return;
  }
  else if( !KRarcHandler::unpack(origin,type, QString::null, tmpDir) ){
    return;
  }
}

void temp_vfs::handleRpm(QString origin){
	// then extract the cpio archive from the rpm
	KShellProcess rpm;
  rpm << "rpm2cpio"<<"\""+origin+"\""+" > "+tmpDir+"/contents.cpio";
  rpm.start(KProcess::Block);
	// and write a nice header
	rpm.clearArguments();
	rpm << "rpm -qip"<<"\""+origin+"\""+" > "+tmpDir+"/header.txt";
	rpm.start(KProcess::Block);
	// and a file list
	rpm.clearArguments();
	rpm << "rpm -lpq"<<"\""+origin+"\""+" > "+tmpDir+"/filelist.txt";
	rpm.start(KProcess::Block);
}

void temp_vfs::handleIso(QString origin){
	// mount the ISO image
	KShellProcess mount;
	mount << KrServices::fullPathName( "mount" ) << "-o loop" << origin << tmpDir;
	mount.start(KProcess::Block);
}
