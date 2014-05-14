/***************************************************************************
                         KgProtocols.cpp  -  description
                             -------------------
    copyright            : (C) 2004 by Csaba Karai
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

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kgprotocols.h"
#include "../krusader.h"
#include "../krservices.h"
#include <klocale.h>
#include <kprotocolinfo.h>
#include <kmimetype.h>
#include <qvbox.h>
#include <qheader.h>
#include <kiconloader.h>
#include <qwhatsthis.h>

QString KgProtocols::defaultProtocols  = "krarc,iso,tar";
QString KgProtocols::defaultIsoMimes   = "application/x-iso";
QString KgProtocols::defaultKrarcMimes = "application/x-7z,application/x-7z-compressed,"
                                         "application/x-ace,application/x-ace-compressed,"
                                         "application/x-arj,application/x-arj-compressed,"
                                         "application/x-bzip2,"
                                         "application/x-cpio,application/x-deb,"
                                         "application/x-debian-package,"
                                         "application/x-gzip,application/x-jar,"
                                         "application/x-lha,application/x-lha-compressed,"
                                         "application/x-rar,application/x-rar-compressed,"
                                         "application/x-rpm,"
                                         "application/x-zip,application/x-zip-compressed";
QString KgProtocols::defaultTarMimes   = "application/x-tar,application/x-tarz,"
                                         "application/x-compressed-tar,"
                                         "application/x-tbz,application/x-tgz";

KgProtocols::KgProtocols( bool first, QWidget* parent,  const char* name ) :
      KonfiguratorPage( first, parent, name )
{
  QGridLayout *KgProtocolsLayout = new QGridLayout( parent );
  KgProtocolsLayout->setSpacing( 6 );

  //  -------------------------- LINK VIEW ----------------------------------
  
  QGroupBox *linkGrp = createFrame( i18n( "Links" ), parent, "linkGrp" );    
  QGridLayout *linkGrid = createGridLayout( linkGrp->layout() );
  
  linkList = new QListView( linkGrp, "linkList" );
  linkList->addColumn( i18n( "Defined Links" ) );
  linkList->header()->setStretchEnabled( true, 0 );
  linkList->setRootIsDecorated( true );
  
  linkGrid->addWidget( linkList, 0, 0 );
  KgProtocolsLayout->addMultiCellWidget( linkGrp, 0 ,1, 0, 0 );

  //  -------------------------- BUTTONS ----------------------------------

  QVBox *vbox1 = new QVBox( parent, "vbox1" )  ;
  
  addSpacer( vbox1 );
  btnAddProtocol = new QPushButton( vbox1, "btnAddProtocolButton" );
  btnAddProtocol->setPixmap( krLoader->loadIcon( "1leftarrow", KIcon::Small ) );
  QWhatsThis::add( btnAddProtocol, i18n( "Add protocol to the link list." ) );
  btnRemoveProtocol = new QPushButton( vbox1, "btnRemoveProtocolButton" );
  btnRemoveProtocol->setPixmap( krLoader->loadIcon( "1rightarrow", KIcon::Small ) );
  QWhatsThis::add( btnRemoveProtocol, i18n( "Remove protocol from the link list." ) );
  addSpacer( vbox1 );
  
  KgProtocolsLayout->addWidget( vbox1, 0 ,1 );

  QVBox *vbox2 = new QVBox( parent, "vbox2" )  ;
  
  addSpacer( vbox2 );
  btnAddMime = new QPushButton( vbox2, "btnAddMimeButton" );
  btnAddMime->setPixmap( krLoader->loadIcon( "1leftarrow", KIcon::Small ) );
  QWhatsThis::add( btnAddMime, i18n( "Add mime to the selected protocol on the link list." ) );
  btnRemoveMime = new QPushButton( vbox2, "btnRemoveMimeButton" );
  btnRemoveMime->setPixmap( krLoader->loadIcon( "1rightarrow", KIcon::Small ) );
  QWhatsThis::add( btnRemoveMime, i18n( "Remove mime from the link list." ) );
  addSpacer( vbox2 );
  
  KgProtocolsLayout->addWidget( vbox2, 1 ,1 );
  
  //  -------------------------- PROTOCOLS LISTBOX ----------------------------------

  QGroupBox *protocolGrp = createFrame( i18n( "Protocols" ), parent, "protocolGrp" );    
  QGridLayout *protocolGrid = createGridLayout( protocolGrp->layout() );
  
  protocolList = new QListBox( protocolGrp, "protocolList" );
  loadListCapableProtocols();
  protocolGrid->addWidget( protocolList, 0, 0 );

  KgProtocolsLayout->addWidget( protocolGrp, 0 ,2 );

  //  -------------------------- MIMES LISTBOX ----------------------------------

  QGroupBox *mimeGrp = createFrame( i18n( "Mimes" ), parent, "mimeGrp" );    
  QGridLayout *mimeGrid = createGridLayout( mimeGrp->layout() );
  
  mimeList = new QListBox( mimeGrp, "protocolList" );
  loadMimes();
  mimeGrid->addWidget( mimeList, 0, 0 );

  KgProtocolsLayout->addWidget( mimeGrp, 1 ,2 );
  
  //  -------------------------- CONNECT TABLE ----------------------------------  
  
  connect( protocolList,      SIGNAL( selectionChanged() ), this, SLOT( slotDisableButtons() ) );
  connect( linkList,          SIGNAL( selectionChanged() ), this, SLOT( slotDisableButtons() ) );
  connect( mimeList,          SIGNAL( selectionChanged() ), this, SLOT( slotDisableButtons() ) );
  connect( linkList,          SIGNAL( currentChanged( QListViewItem *) ),   this, SLOT( slotDisableButtons() ) );
  connect( btnAddProtocol,    SIGNAL( clicked() )         , this, SLOT( slotAddProtocol() ) );
  connect( btnRemoveProtocol, SIGNAL( clicked() )         , this, SLOT( slotRemoveProtocol() ) );
  connect( btnAddMime,        SIGNAL( clicked() )         , this, SLOT( slotAddMime() ) );
  connect( btnRemoveMime,     SIGNAL( clicked() )         , this, SLOT( slotRemoveMime() ) );
  
  loadInitialValues();
  slotDisableButtons();
}
  
QWidget* KgProtocols::addSpacer( QWidget *parent, const char *widgetName )
{
  QWidget *widget = new QWidget( parent, widgetName );
  QVBoxLayout *vboxlayout = new QVBoxLayout( widget );
  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  vboxlayout->addItem( spacer );
  return widget;
}

void KgProtocols::loadListCapableProtocols()
{
  QStringList protocols = KProtocolInfo::protocols();
  protocols.sort();
  
  for ( QStringList::Iterator it = protocols.begin(); it != protocols.end();) 
  {
    if( !KProtocolInfo::supportsListing( *it ) )
    {
      it = protocols.remove( it );
      continue;
    }
    ++it;
  }
  protocolList->insertStringList( protocols );
}

void KgProtocols::loadMimes()
{
  KMimeType::List mimes = KMimeType::allMimeTypes();
  
  for( QValueListIterator<KMimeType::Ptr> it = mimes.begin(); it != mimes.end(); it++ )
    mimeList->insertItem( (*it)->name() );
    
  mimeList->sort();
}

void KgProtocols::slotDisableButtons()
{
  btnAddProtocol->setEnabled( protocolList->selectedItem() != 0 );
  QListViewItem *listViewItem = linkList->currentItem();
  bool isProtocolSelected = ( listViewItem == 0 ? false : listViewItem->parent() == 0 );
  btnRemoveProtocol->setEnabled( isProtocolSelected );
  btnAddMime->setEnabled( listViewItem != 0 && mimeList->selectedItem() != 0 );
  btnRemoveMime->setEnabled( listViewItem == 0 ? false : listViewItem->parent() != 0 );
  
  if( linkList->currentItem() == 0 && linkList->firstChild() != 0 )
    linkList->setCurrentItem( linkList->firstChild() );
  if( linkList->selectedItem() == 0 && linkList->currentItem() != 0 )
    linkList->setSelected( linkList->currentItem(), true );
}

void KgProtocols::slotAddProtocol()
{
  QListBoxItem *item = protocolList->selectedItem();
  if( item )
  {
    addProtocol( item->text(), true );
    slotDisableButtons();
    emit sigChanged();
  }
}

void KgProtocols::addProtocol( QString name, bool changeCurrent )
{
  QListBoxItem *item = protocolList->findItem( name, ExactMatch );
  if( item )
  {
    protocolList->removeItem( protocolList->index( item ) );
    QListViewItem *listViewItem = new QListViewItem( linkList, name );
    listViewItem->setPixmap( 0, krLoader->loadIcon( "exec", KIcon::Small ) );
    
    if( changeCurrent )
      linkList->setCurrentItem( listViewItem );
  }
}

void KgProtocols::slotRemoveProtocol()
{
  QListViewItem *item = linkList->currentItem();
  if( item )
  {
    removeProtocol( item->text( 0 ) );
    slotDisableButtons();
    emit sigChanged();
  }
}

void KgProtocols::removeProtocol( QString name )
{
  QListViewItem *item = linkList->findItem( name, 0 );
  if( item )
  {
    while( item->childCount() != 0 )
      removeMime( item->firstChild()->text( 0 ) );
     
    linkList->takeItem( item );
    protocolList->insertItem( name );
    protocolList->sort();
  }
}

void KgProtocols::slotAddMime()
{
  QListBoxItem *item = mimeList->selectedItem();
  if( item && linkList->currentItem() != 0 )
  {
    QListViewItem *itemToAdd = linkList->currentItem();
    if( itemToAdd->parent() )
      itemToAdd = itemToAdd->parent();
      
    addMime( item->text(), itemToAdd->text( 0 ) );
    slotDisableButtons();
    emit sigChanged();
  }
}

void KgProtocols::addMime( QString name, QString protocol )
{
  QListBoxItem *item = mimeList->findItem( name, ExactMatch );
  QListViewItem *currentListItem = linkList->findItem( protocol, 0 );
  
  if( item && currentListItem && currentListItem->parent() == 0 )
  {
    mimeList->removeItem( mimeList->index( item ) );
    QListViewItem *listViewItem = new QListViewItem( currentListItem, name );
    listViewItem->setPixmap( 0, krLoader->loadIcon( "mime", KIcon::Small ) );
    currentListItem->setOpen( true );
  }
}

void KgProtocols::slotRemoveMime()
{
  QListViewItem *item = linkList->currentItem();
  if( item )
  {
    removeMime( item->text( 0 ) );
    slotDisableButtons();
    emit sigChanged();
  }
}

void KgProtocols::removeMime( QString name )
{
  QListViewItem *currentMimeItem = linkList->findItem( name, 0 );
  
  if( currentMimeItem && currentMimeItem->parent() != 0 )
  {
    mimeList->insertItem( currentMimeItem->text( 0 ) );
    mimeList->sort();
    currentMimeItem->parent()->takeItem( currentMimeItem );
  }
}

void KgProtocols::loadInitialValues()
{
  while( linkList->childCount() != 0 )
    removeProtocol( linkList->firstChild()->text( 0 ) );
  
  krConfig->setGroup( "Protocols" );
  QStringList protList = krConfig->readListEntry( "Handled Protocols" );
    
  for( QStringList::Iterator it = protList.begin(); it != protList.end(); it++ ) 
  {
    addProtocol( *it );
    
    QStringList mimes = krConfig->readListEntry( QString( "Mimes For %1" ).arg( *it ) );
    
    for( QStringList::Iterator it2 = mimes.begin(); it2 != mimes.end(); it2++ )
      addMime( *it2, *it );
  }
  
  if( linkList->firstChild() != 0 )
    linkList->setCurrentItem( linkList->firstChild() );
  slotDisableButtons();
}

void KgProtocols::setDefaults()
{
  while( linkList->childCount() != 0 )
    removeProtocol( linkList->firstChild()->text( 0 ) );
  
  addProtocol( "iso" );
  addMime( "application/x-iso", "iso" );
  
  addProtocol( "krarc" );
  QStringList krarcMimes = QStringList::split( ',', defaultKrarcMimes );
  for( QStringList::Iterator it = krarcMimes.begin(); it != krarcMimes.end(); it++ )
    addMime( *it, "krarc" );
  
  addProtocol( "tar" );
  QStringList tarMimes = QStringList::split( ',', defaultTarMimes );
  for( QStringList::Iterator it = tarMimes.begin(); it != tarMimes.end(); it++ )
    addMime( *it, "tar" );
    
  slotDisableButtons();
    
  if( isChanged() )
    emit sigChanged();
}

bool KgProtocols::isChanged()
{
  krConfig->setGroup( "Protocols" );
  QStringList protList = krConfig->readListEntry( "Handled Protocols" );
  
  if( (int)protList.count() != linkList->childCount() )
    return true;
  
  QListViewItem *item = linkList->firstChild();
  while( item )
  {
    if( !protList.contains( item->text( 0 ) ) )
      return true;
      
    QStringList mimes = krConfig->readListEntry( QString( "Mimes For %1" ).arg( item->text( 0 ) ) );
    
    if( (int)mimes.count() != item->childCount() )
      return true;
    QListViewItem *childs = item->firstChild();
    while( childs )
    {
      if( !mimes.contains( childs->text( 0 ) ) )
        return true;
      childs = childs->nextSibling();
    }
      
    item = item->nextSibling();
  }
    
  return false;
}

bool KgProtocols::apply()
{
  krConfig->setGroup( "Protocols" );
  
  QStringList protocolList;
  
  QListViewItem *item = linkList->firstChild();
  while( item )
  {
    protocolList.append( item->text( 0 ) );
    
    QStringList mimes;
    QListViewItem *childs = item->firstChild();
    while( childs )
    {
      mimes.append( childs->text( 0 ) );
      childs = childs->nextSibling();
    }
    krConfig->writeEntry( QString( "Mimes For %1" ).arg( item->text( 0 ) ), mimes );
    
    item = item->nextSibling();
  }  
  krConfig->writeEntry( "Handled Protocols", protocolList );
  krConfig->sync();  
  
  KrServices::clearProtocolCache();
  
  emit sigChanged();  
  return false;
}

void KgProtocols::init()
{
  if( !krConfig->groupList().contains( "Protocols" ) )
  {
    krConfig->setGroup( "Protocols" );
    krConfig->writeEntry( "Handled Protocols", defaultProtocols );
    krConfig->writeEntry( "Mimes For iso",     defaultIsoMimes );
    krConfig->writeEntry( "Mimes For krarc",   defaultKrarcMimes );
	 krConfig->writeEntry( "Mimes For tar",     defaultTarMimes );
  }
}

#include "kgprotocols.moc"
