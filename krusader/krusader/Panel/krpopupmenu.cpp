/***************************************************************************
                          krpopupmenu.cpp  -  description
                             -------------------
    begin                : Tue Aug 26 2003
    copyright            : (C) 2003 by Shie Erlich & Rafi Yanai
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <klocale.h>
#include <kprocess.h>
#include <kshred.h>
#include <krun.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include "../krservices.h"
#include "../defaults.h"
#include "../MountMan/kmountman.h"
#include "../krslots.h"
#include "krpopupmenu.h"
#include "krview.h"
#include "krviewitem.h"
#include "panelfunc.h"
#include "../krusaderview.h"
#include "../panelmanager.h"

void KrPopupMenu::run(const QPoint &pos, ListPanel *panel) {
	KrPopupMenu menu(panel);
	int result = menu.exec(pos);
	menu.performAction(result);
}

KrPopupMenu::KrPopupMenu(ListPanel *thePanel, QWidget *parent) : KPopupMenu(parent), panel(thePanel), empty(false), 
	multipleSelections(false),actions(0) {
#ifdef __LIBKONQ__
	konqMenu = 0;
#endif
	
   panel->view->getSelectedKrViewItems( &items );
   if ( items.empty() ) {
   	addCreateNewMenu();
   	insertSeparator();
   	addEmptyMenuEntries();
      return;
	} else if ( items.size() > 1 ) multipleSelections = true;

   item = items.first();
   vfile *vf = panel->func->getVFile(item);
   
   // ------------ the OPEN option - open prefered service
   insertItem( i18n("Open/Run"), OPEN_ID );
   if ( !multipleSelections ) { // meaningful only if one file is selected
		changeItem( OPEN_ID, item->icon(), vf->vfile_isExecutable() && !vf->vfile_isDir() ? i18n("Run") : i18n("Open") );
      // open in a new tab (if folder)
      if ( vf->vfile_isDir() ) {
         insertItem( i18n( "Open in New Tab" ), OPEN_TAB_ID );
         changeItem( OPEN_TAB_ID, krLoader->loadIcon( "tab_new", KIcon::Panel ), i18n( "Open in New Tab" ) );
      }
      insertSeparator();
   }

   // ------------- Preview - normal vfs only ?
   if ( panel->func->files()->vfs_getType() == vfs::NORMAL ) {
      // create the preview popup
      QStringList names;
      panel->getSelectedNames( &names );
      preview.setUrls( panel->func->files() ->vfs_getFiles( &names ) );
      insertItem( i18n("Preview"), &preview, PREVIEW_ID );
   }

   // -------------- Open with: try to find-out which apps can open the file
   // this too, is meaningful only if one file is selected or if all the files
   // have the same mimetype !
   QString mime = panel->func->getVFile(item)->vfile_getMime();
   // check if all the list have the same mimetype
   for ( unsigned int i = 1; i < items.size(); ++i ) {
      if ( panel->func->getVFile(( *items.at( i ) )) ->vfile_getMime() != mime ) {
         mime = QString::null;
         break;
      }
   }
   if ( !mime.isEmpty() ) {
      offers = KServiceTypeProfile::offers( mime );
      for ( unsigned int i = 0; i < offers.count(); ++i ) {
         KService::Ptr service = offers[ i ].service();
         if ( service->isValid() && service->type() == "Application" ) {
            openWith.insertItem( service->name(), SERVICE_LIST_ID + i );
            openWith.changeItem( SERVICE_LIST_ID + i, service->pixmap( KIcon::Small ), service->name() );
         }
      }
      openWith.insertSeparator();
      if ( vf->vfile_isDir() )
         openWith.insertItem( krLoader->loadIcon( "konsole", KIcon::Small ), i18n( "Terminal" ), OPEN_TERM_ID );
      openWith.insertItem( i18n( "Other..." ), CHOOSE_ID );
      insertItem( QPixmap(), &openWith, OPEN_WITH_ID );
      changeItem( OPEN_WITH_ID, i18n( "Open With" ) );
      insertSeparator();
   }
	
	// --------------- user actions
   insertItem( i18n("User Actions"), new UserActionPopupMenu( panel->func->files()->vfs_getFile( item->name() ).url() ) );
   _items.setAutoDelete( true );
   for ( KrViewItemList::Iterator it = items.begin(); it != items.end(); ++it ) {
		vfile *file = panel->func->files()->vfs_search(((*it)->name()));
		KURL url = file->vfile_getUrl();
		_items.append( new KFileItem( url,  file->vfile_getMime(), file->vfile_getMode() ) );
   }
   
#ifdef __LIBKONQ__
	// -------------- konqueror menu
   actions = new KActionCollection(this);
	konqMenu = new KonqPopupMenu( KonqBookmarkManager::self(), _items, panel->func->files()->vfs_getOrigin(), *actions, 0, this, 
                           KonqPopupMenu::NoFlags, KParts::BrowserExtension::DefaultPopupItems );
   insertItem( QPixmap(), konqMenu, KONQ_MENU_ID );
   changeItem( KONQ_MENU_ID, i18n( "Konqueror Menu" ) );
#endif
   
	// ------------- 'create new' submenu
   addCreateNewMenu();
	insertSeparator();
   
   // ---------- COPY
   insertItem( i18n( "Copy..." ), COPY_ID );
   if ( panel->func->files() ->vfs_isWritable() ) {
      // ------- MOVE
      insertItem( i18n( "Move..." ), MOVE_ID );
      // ------- RENAME - only one file
      if ( !multipleSelections )
         insertItem( i18n( "Rename" ), RENAME_ID );
  
      // -------- MOVE TO TRASH
      KConfigGroupSaver saver(krConfig, "General");
      bool trash = krConfig->readBoolEntry( "Move To Trash", _MoveToTrash );
      if( trash )
        insertItem( i18n( "Move to Trash" ), TRASH_ID );
      // -------- DELETE
      insertItem( i18n( "Delete" ), DELETE_ID );
      // -------- SHRED - only one file
      if ( panel->func->files() ->vfs_getType() == vfs::NORMAL &&
            !vf->vfile_isDir() && !multipleSelections )
         insertItem( i18n( "Shred" ), SHRED_ID );
   }
   
   // ---------- link handling
   // create new shortcut or redirect links - only on local directories:
   if ( panel->func->files() ->vfs_getType() == vfs::NORMAL ) {
      insertSeparator();
      linkPopup.insertItem( i18n( "New Symlink..." ), NEW_SYMLINK_ID );
      linkPopup.insertItem( i18n( "New Hardlink..." ), NEW_LINK_ID );
      if ( panel->func->getVFile(item)->vfile_isSymLink() )
         linkPopup.insertItem( i18n( "Redirect Link..." ), REDIRECT_LINK_ID);
      insertItem( QPixmap(), &linkPopup, LINK_HANDLING_ID );
      changeItem( LINK_HANDLING_ID, i18n( "Link Handling" ) );
   }
   insertSeparator();

   // ---------- calculate space
   if ( panel->func->files() ->vfs_getType() == vfs::NORMAL && ( vf->vfile_isDir() || multipleSelections ) )
      krCalculate->plug( this );
	
	// ---------- mount/umount/eject
   if ( panel->func->files() ->vfs_getType() == vfs::NORMAL && vf->vfile_isDir() && !multipleSelections ) {
      if ( krMtMan.getStatus( panel->func->files() ->vfs_getFile( item->name() ).path( -1 ) ) == KMountMan::MOUNTED )
         insertItem( i18n( "Unmount" ), UNMOUNT_ID );
      else if ( krMtMan.getStatus( panel->func->files() ->vfs_getFile( item->name() ).path( -1 ) ) == KMountMan::NOT_MOUNTED )
         insertItem( i18n( "Mount" ), MOUNT_ID );
      if ( krMtMan.ejectable( panel->func->files() ->vfs_getFile( item->name() ).path( -1 ) ) )
         insertItem( i18n( "Eject" ), EJECT_ID );
   }
   
   // --------- send by mail
   if ( Krusader::supportedTools().contains( "MAIL" ) && !vf->vfile_isDir() ) {
      insertItem( i18n( "Send by Email" ), SEND_BY_EMAIL_ID );
   }
   
   // --------- synchronize
   if ( panel->view->numSelected() ) {
      insertItem( i18n( "Synchronize Selected Files..." ), SYNC_SELECTED_ID );
   }
   
   // --------- copy/paste
   insertSeparator();
   insertItem( i18n( "Copy to Clipboard" ), COPY_CLIP_ID );
   if ( panel->func->files() ->vfs_isWritable() )
   {
     insertItem( i18n( "Cut to Clipboard" ), MOVE_CLIP_ID );
     insertItem( i18n( "Paste from Clipboard" ), PASTE_CLIP_ID );
   }
   insertSeparator();
   
   // --------- properties
   krProperties->plug( this );
}

KrPopupMenu::~KrPopupMenu() {
	if (actions) delete actions;
#ifdef __LIBKONQ__
	if (konqMenu) delete konqMenu;
#endif	
}

void KrPopupMenu::addEmptyMenuEntries() {
	insertItem( i18n( "Paste from Clipboard" ), PASTE_CLIP_ID );
}

void KrPopupMenu::addCreateNewMenu() {
	createNewPopup.insertItem( krLoader->loadIcon( "folder", KIcon::Small ), i18n("Folder..."), MKDIR_ID);
	createNewPopup.insertItem( krLoader->loadIcon( "txt", KIcon::Small ), i18n("Text File..."), NEW_TEXT_FILE_ID);
	
	insertItem( QPixmap(), &createNewPopup, CREATE_NEW_ID);
	changeItem( CREATE_NEW_ID, i18n( "Create New" ) );
	
}

void KrPopupMenu::performAction(int id) {
   KURL u;
   KURL::List lst;

   switch ( id ) {
         case - 1 : // the user clicked outside of the menu
         return ;
         case OPEN_TAB_ID :
         	// assuming only 1 file is selected (otherwise we won't get here)
         	( ACTIVE_PANEL == LEFT_PANEL ? LEFT_MNG : RIGHT_MNG )->
         		slotNewTab( panel->func->files()->vfs_getFile( item->name() ).url() );
         	break;
         case OPEN_ID :
         	for ( KrViewItemList::Iterator it = items.begin(); it != items.end(); ++it ) {
            	u = panel->func->files()->vfs_getFile( ( *it ) ->name() );
            	KRun::runURL( u, panel->func->getVFile(item)->vfile_getMime() );
         	}
         	break;
         case COPY_ID :
         	panel->func->copyFiles();
         	break;
         case MOVE_ID :
         	panel->func->moveFiles();
         	break;
         case RENAME_ID :
         	SLOTS->rename();
         	break;
         case TRASH_ID :
         	panel->func->deleteFiles( false );
         	break;
         case DELETE_ID :
         	panel->func->deleteFiles( true );
         	break;
         case EJECT_ID :
         	KMountMan::eject( panel->func->files() ->vfs_getFile( item->name() ).path( -1 ) );
         	break;
         case SHRED_ID :
            if ( KMessageBox::warningContinueCancel( krApp,
                 i18n("<qt>Do you really want to shred <b>%1</b>? Once shred, the file is gone forever!</qt>").arg(item->name()),
                 QString::null, KStdGuiItem::cont(), "Shred" ) == KMessageBox::Continue )
               KShred::shred( panel->func->files() ->vfs_getFile( item->name() ).path( -1 ) );
         	break;
         case OPEN_KONQ_ID :
         	kapp->startServiceByDesktopName( "konqueror", panel->func->files() ->vfs_getFile( item->name() ).url() );
         	break;
         case CHOOSE_ID : // open-with dialog
         	u = panel->func->files() ->vfs_getFile( item->name() );
         	lst.append( u );
         	KRun::displayOpenWithDialog( lst );
         	break;
         case MOUNT_ID :
         	krMtMan.mount( panel->func->files() ->vfs_getFile( item->name() ).path( -1 ) );
         	break;
         case NEW_LINK_ID :
         	panel->func->krlink( false );
         	break;
         case NEW_SYMLINK_ID :
         	panel->func->krlink( true );
         	break;
         case REDIRECT_LINK_ID :
         	panel->func->redirectLink();
         	break;
         case UNMOUNT_ID :
         	krMtMan.unmount( panel->func->files() ->vfs_getFile( item->name() ).path( -1 ) );
         	break;
         case COPY_CLIP_ID :
         	panel->func->copyToClipboard();
         	break;
         case MOVE_CLIP_ID :
         	panel->func->copyToClipboard( true );
         	break;
         case PASTE_CLIP_ID :
				panel->func->pasteFromClipboard();
         	break;
         case SEND_BY_EMAIL_ID :
         	SLOTS->sendFileByEmail( panel->func->files() ->vfs_getFile( item->name() ).url() );
         	break;
			case MKDIR_ID :
				SLOTS->mkdir();
				break;
			case NEW_TEXT_FILE_ID:
				SLOTS->editDlg();
				break;
         case SYNC_SELECTED_ID :
         	{
				QStringList selectedNames;
            for ( KrViewItemList::Iterator it = items.begin(); it != items.end(); ++it )
               selectedNames.append( ( *it ) ->name() );
            if( panel->otherPanel->view->numSelected() ) {
               KrViewItemList otherItems;
               panel->otherPanel->view->getSelectedKrViewItems( &otherItems );
               
               for ( KrViewItemList::Iterator it2 = otherItems.begin(); it2 != otherItems.end(); ++it2 ) {
                  QString name = ( *it2 ) ->name();
                  if( !selectedNames.contains( name ) )
                    selectedNames.append( name );
               }
            }
            SLOTS->slotSynchronizeDirs( selectedNames );
         	}
         	break;
         case OPEN_TERM_ID :
         	QString save = getcwd( 0, 0 );
         	chdir( panel->func->files() ->vfs_getFile( item->name() ).path( -1 ).local8Bit() );
				KProcess proc;
				{
				KConfigGroupSaver saver(krConfig, "General");
         	QString term = krConfig->readEntry( "Terminal", _Terminal );
         	proc << KrServices::separateArgs( term );
         	if ( !panel->func->getVFile(item)->vfile_isDir() ) proc << "-e" << item->name();
         	if ( term.contains( "konsole" ) ) {   /* KDE 3.2 bug (konsole is killed by pressing Ctrl+C) */
         	                                      /* Please remove the patch if the bug is corrected */
					proc << "&";
            	proc.setUseShell( true );
         	}
         	if ( !proc.start( KProcess::DontCare ) )
               KMessageBox::sorry( krApp, i18n( "Can't open \"%1\"" ).arg(term) );
				} // group-saver is blown out of scope here
         	chdir( save.local8Bit() );
         	break;
	}
   
   // check if something from the open-with-offered-services was selected
   if ( id >= SERVICE_LIST_ID ) {
      QStringList names;
      panel->getSelectedNames( &names );
      KRun::run( *( offers[ id - SERVICE_LIST_ID ].service() ),
                 *( panel->func->files() ->vfs_getFiles( &names ) ) );
   }
}

#include "krpopupmenu.moc"
