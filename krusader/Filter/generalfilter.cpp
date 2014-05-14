/***************************************************************************
                      generalfilter.cpp  -  description
                             -------------------
    copyright            : (C) 2003 + by Shie Erlich & Rafi Yanai & Csaba Karai
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

#include "generalfilter.h"
#include "filtertabs.h"
#include "../krusader.h"
#include "../VFS/vfs.h"

#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>

GeneralFilter::GeneralFilter( FilterTabs *tabs, int properties, QWidget *parent, const char *name ) : QWidget( parent, name ),
    profileManager( 0 ), fltTabs( tabs )
{
  QGridLayout *filterLayout = new QGridLayout( this );
  filterLayout->setSpacing( 6 );
  filterLayout->setMargin( 11 );

  this->properties = properties;

  // Options for name filtering

  QGroupBox *nameGroup = new QGroupBox( this, "nameGroup" );
  nameGroup->setTitle( i18n( "File name" ) );
  nameGroup->setColumnLayout(0, Qt::Vertical );
  nameGroup->layout()->setSpacing( 0 );
  nameGroup->layout()->setMargin( 0 );
  QGridLayout *nameGroupLayout = new QGridLayout( nameGroup->layout() );
  nameGroupLayout->setAlignment( Qt::AlignTop );
  nameGroupLayout->setSpacing( 6 );
  nameGroupLayout->setMargin( 11 );

  searchForCase = new QCheckBox( nameGroup, "searchForCase" );
  searchForCase->setText( i18n( "&Case sensitive" ) );
  searchForCase->setChecked( false );
  nameGroupLayout->addWidget( searchForCase, 1, 2 );

  QLabel *searchForLabel = new QLabel( nameGroup, "searchForLabel" );
  searchForLabel->setText( i18n( "Search &for:" ) );
  nameGroupLayout->addWidget( searchForLabel, 0, 0 );

  searchFor = new KHistoryCombo( false, nameGroup, "searchFor" );
  searchFor->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, searchFor->sizePolicy().hasHeightForWidth() ) );
  searchFor->setEditable( true );
  searchFor->setDuplicatesEnabled( false );
  searchFor->setMaxCount( 25 );
  searchForLabel->setBuddy( searchFor );
  nameGroupLayout->addMultiCellWidget( searchFor, 0, 0, 1, 2 );

  QString s = "<p><img src='toolbar|find'></p>" + i18n("<p>The filename filtering criteria is defined here.</p><p>You can make use of wildcards. Multiple patterns are separated by space (means logical OR) and patterns are excluded from the search using the pipe symbol.</p><p>If the pattern is ended with a slash (<code>*pattern*/</code>), that means that pattern relates to recursive search of directories.<ul><li><code>pattern</code> - means to search those files/directories that name is <code>pattern</code>, recursive search goes through all subdirectories independently of the value of <code>pattern</code></li><li><code>pattern/</code> - means to search all files/directories, but recursive search goes through/excludes the directories that name is <code>pattern</code></li></ul><p></p><p>It's allowed to use quotation marks for names that contain space. Filter <code>\"Program&nbsp;Files\"</code> searches out those files/directories that name is <code>Program&nbsp;Files</code>.</p><p>Examples:<ul><code><li>*.o</li><li>*.h *.c\?\?</li><li>*.cpp *.h | *.moc.cpp</li><li>* | CVS/ .svn/</li></code></ul><b>Note</b>: the search term '<code>text</code>' is equivalent to '<code>*text*</code>'.</p>");
  QWhatsThis::add(searchFor, s);
  QWhatsThis::add(searchForLabel, s);

  QLabel *searchType = new QLabel( nameGroup, "searchType" );
  searchType->setText( i18n( "&Of type:" ) );
  nameGroupLayout->addWidget( searchType, 1, 0 );

  ofType = new KComboBox( false, nameGroup, "ofType" );
  ofType->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, ofType->sizePolicy().hasHeightForWidth() ) );
  ofType->setEditable( false );
  searchType->setBuddy(ofType);
  ofType->insertItem(i18n("All Files"));
  ofType->insertItem(i18n("Archives"));
  ofType->insertItem(i18n("Directories"));
  ofType->insertItem(i18n("Image Files"));
  ofType->insertItem(i18n("Text Files"));
  ofType->insertItem(i18n("Video Files"));
  ofType->insertItem(i18n("Audio Files"));

  nameGroupLayout->addWidget( ofType, 1, 1 );
  filterLayout->addWidget( nameGroup, 0, 0 );

  middleLayout = new QHBoxLayout();
  middleLayout->setSpacing( 6 );
  middleLayout->setMargin( 0 );
  QSpacerItem* middleSpacer = new QSpacerItem( 1, 1, QSizePolicy::Fixed, QSizePolicy::Fixed );
  middleLayout->addItem( middleSpacer );

  if( properties & FilterTabs::HasProfileHandler )
  {
    // The profile handler

    QGroupBox *profileHandler = new QGroupBox( this, "profileHandler" );
    profileHandler->setTitle( i18n( "&Profile handler" ) );
    profileHandler->setColumnLayout(0, Qt::Vertical );
    profileHandler->layout()->setSpacing( 0 );
    profileHandler->layout()->setMargin( 0 );
    QGridLayout *profileLayout = new QGridLayout( profileHandler->layout() );
    profileLayout->setAlignment( Qt::AlignTop );
    profileLayout->setSpacing( 6 );
    profileLayout->setMargin( 11 );

    profileListBox = new QListBox( profileHandler, "profileListBox" );
    profileLayout->addMultiCellWidget( profileListBox, 0, 3, 0, 0 );

    profileAddBtn = new QPushButton( i18n( "&Add" ), profileHandler, "profileAddBtn" );
    profileLayout->addWidget( profileAddBtn, 0, 1 );

    profileLoadBtn = new QPushButton( i18n( "&Load" ), profileHandler, "profileLoadBtn" );
    profileLoadBtn->setEnabled( false );
    profileLayout->addWidget( profileLoadBtn, 1, 1 );

    profileOverwriteBtn = new QPushButton( i18n( "&Overwrite" ), profileHandler, "profileOverwriteBtn" );
    profileOverwriteBtn->setEnabled( false );
    profileLayout->addWidget( profileOverwriteBtn, 2, 1 );

    profileRemoveBtn = new QPushButton( i18n( "&Remove" ), profileHandler, "profileRemoveBtn" );
    profileRemoveBtn->setEnabled( false );
    profileLayout->addWidget( profileRemoveBtn, 3, 1 );

    profileManager = new ProfileManager( "SelectionProfile", this, "selectionProfileManager" );
    profileManager->hide();

    middleLayout->addWidget( profileHandler );

    refreshProfileListBox();
  }

  if( properties & FilterTabs::HasSearchIn )
  {
    // Options for search in

    QGroupBox *searchInGroup = new QGroupBox( this, "searchInGroup" );
    searchInGroup->setTitle( i18n( "&Search in" ) );
    searchInGroup->setColumnLayout(0, Qt::Vertical );
    searchInGroup->layout()->setSpacing( 0 );
    searchInGroup->layout()->setMargin( 0 );
    QGridLayout *searchInLayout = new QGridLayout( searchInGroup->layout() );
    searchInLayout->setAlignment( Qt::AlignTop );
    searchInLayout->setSpacing( 6 );
    searchInLayout->setMargin( 11 );

    searchIn = new KURLListRequester( searchInGroup, "searchIn" );
    searchInLayout->addWidget( searchIn, 0, 0 );

    middleLayout->addWidget( searchInGroup );
  }

  if( properties & FilterTabs::HasDontSearchIn )
  {
    // Options for don't search in

    QGroupBox *dontSearchInGroup = new QGroupBox( this, "dontSearchInGroup" );
    dontSearchInGroup->setTitle( i18n( "&Don't search in" ) );
    dontSearchInGroup->setColumnLayout(0, Qt::Vertical );
    dontSearchInGroup->layout()->setSpacing( 0 );
    dontSearchInGroup->layout()->setMargin( 0 );
    QGridLayout *dontSearchInLayout = new QGridLayout( dontSearchInGroup->layout() );
    dontSearchInLayout->setAlignment( Qt::AlignTop );
    dontSearchInLayout->setSpacing( 6 );
    dontSearchInLayout->setMargin( 11 );

    dontSearchIn = new KURLListRequester( dontSearchInGroup, "dontSearchIn" );
    dontSearchInLayout->addWidget( dontSearchIn, 0, 0 );

    middleLayout->addWidget( dontSearchInGroup );
  }

  filterLayout->addLayout( middleLayout, 1, 0 );

  // Options for containing text

  QGroupBox *containsGroup = new QGroupBox( this, "containsGroup" );
  containsGroup->setTitle( i18n( "Containing text" ) );
  containsGroup->setColumnLayout(0, Qt::Vertical );
  containsGroup->layout()->setSpacing( 0 );
  containsGroup->layout()->setMargin( 0 );
  QGridLayout *containsLayout = new QGridLayout( containsGroup->layout() );
  containsLayout->setAlignment( Qt::AlignTop );
  containsLayout->setSpacing( 6 );
  containsLayout->setMargin( 11 );

  QHBoxLayout *containsTextLayout = new QHBoxLayout();
  containsTextLayout->setSpacing( 6 );
  containsTextLayout->setMargin( 0 );

  QLabel *containsLabel = new QLabel( containsGroup, "containsLabel" );
  containsLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)1, containsLabel->sizePolicy().hasHeightForWidth() ) );
  containsLabel->setText( i18n( "&Text:" ) );
  containsTextLayout->addWidget( containsLabel );

  containsText = new KHistoryCombo( false, containsGroup, "containsText" );
  containsText->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, containsText->sizePolicy().hasHeightForWidth() ) );
  containsText->setDuplicatesEnabled( false );
  containsText->setMaxCount( 25 );
  containsTextLayout->addWidget( containsText );
  containsLabel->setBuddy(containsText);

  containsLayout->addLayout( containsTextLayout, 0, 0 );

  QHBoxLayout *containsCbsLayout = new QHBoxLayout();
  containsCbsLayout->setSpacing( 6 );
  containsCbsLayout->setMargin( 0 );
  QSpacerItem* cbSpacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  containsCbsLayout->addItem( cbSpacer );

  remoteContentSearch = new QCheckBox( containsGroup, "remoteContentSearch" );
  remoteContentSearch->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, remoteContentSearch->sizePolicy().hasHeightForWidth() ) );
  remoteContentSearch->setText( i18n( "&Remote content search" ) );
  remoteContentSearch->setChecked( false );
  containsCbsLayout->addWidget( remoteContentSearch );
  if( !( properties & FilterTabs::HasRemoteContentSearch ) )
    remoteContentSearch->hide();

  containsWholeWord = new QCheckBox( containsGroup, "containsWholeWord" );
  containsWholeWord->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, containsWholeWord->sizePolicy().hasHeightForWidth() ) );
  containsWholeWord->setText( i18n( "&Match whole word only" ) );
  containsWholeWord->setChecked( false );
  containsCbsLayout->addWidget( containsWholeWord );

  containsTextCase = new QCheckBox( containsGroup, "containsTextCase" );
  containsTextCase->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, containsTextCase->sizePolicy().hasHeightForWidth() ) );
  containsTextCase->setText( i18n( "Cas&e sensitive" ) );
  containsTextCase->setChecked( true );
  containsCbsLayout->addWidget( containsTextCase );

  containsLayout->addLayout( containsCbsLayout, 1, 0 );

  filterLayout->addWidget( containsGroup, 2, 0 );

  if( properties & FilterTabs::HasRecurseOptions )
  {
    // Options for recursive searching

    QHBoxLayout *recurseLayout = new QHBoxLayout();
    recurseLayout->setSpacing( 6 );
    recurseLayout->setMargin( 0 );
    QSpacerItem* recurseSpacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    recurseLayout->addItem( recurseSpacer );

    searchInDirs = new QCheckBox( this, "searchInDirs" );
    searchInDirs->setText( i18n( "Search in s&ubdirectories" ) );
    searchInDirs->setChecked( true );
    recurseLayout->addWidget( searchInDirs );

    searchInArchives = new QCheckBox( this, "searchInArchives" );
    searchInArchives->setText( i18n( "Search in arch&ives" ) );
    recurseLayout->addWidget( searchInArchives );

    followLinks = new QCheckBox( this, "followLinks" );
    followLinks->setText( i18n( "Follow &links" ) );
    recurseLayout->addWidget( followLinks );

    filterLayout->addLayout( recurseLayout, 3, 0 );
  }

  // Connection table

  if( properties & FilterTabs::HasProfileHandler )
  {
    connect( profileAddBtn,       SIGNAL(clicked()) , this, SLOT( slotAddBtnClicked() ) );
    connect( profileLoadBtn,      SIGNAL(clicked()) , this, SLOT( slotLoadBtnClicked() ) );
    connect( profileOverwriteBtn, SIGNAL(clicked()) , this, SLOT( slotOverwriteBtnClicked() ) );
    connect( profileRemoveBtn,    SIGNAL(clicked()) , this, SLOT( slotRemoveBtnClicked() ) );
    connect( profileListBox,      SIGNAL(doubleClicked(QListBoxItem *)) , this, SLOT( slotProfileDoubleClicked(QListBoxItem *) ) );
    connect( profileManager,      SIGNAL(loadFromProfile(QString )), fltTabs, SLOT( loadFromProfile(QString ) ) );
    connect( profileManager,      SIGNAL(saveToProfile(QString )), fltTabs, SLOT( saveToProfile(QString ) ) );
  }

  connect( searchFor, SIGNAL(activated(const QString&)), searchFor, SLOT(addToHistory(const QString&)));
  connect( containsText, SIGNAL(activated(const QString&)), containsText, SLOT(addToHistory(const QString&)));

  // load the completion and history lists
  // ==> search for
  krConfig->setGroup("Search");
  QStringList list = krConfig->readListEntry("SearchFor Completion");
  searchFor->completionObject()->setItems(list);
  list = krConfig->readListEntry("SearchFor History");
  searchFor->setHistoryItems(list);
  // ==> grep
  krConfig->setGroup("Search");
  list = krConfig->readListEntry("ContainsText Completion");
  containsText->completionObject()->setItems(list);
  list = krConfig->readListEntry("ContainsText History");
  containsText->setHistoryItems(list);

  setTabOrder( searchFor, containsText );  // search for -> content 
  setTabOrder( containsText, searchType ); // content -> search type
}

GeneralFilter::~GeneralFilter()
{
  // save the history combos
  // ==> search for
  QStringList list = searchFor->completionObject()->items();
  krConfig->setGroup("Search");
  krConfig->writeEntry("SearchFor Completion", list);
  list = searchFor->historyItems();
  krConfig->writeEntry("SearchFor History", list);
  // ==> grep text
  list = containsText->completionObject()->items();
  krConfig->setGroup("Search");
  krConfig->writeEntry("ContainsText Completion", list);
  list = containsText->historyItems();
  krConfig->writeEntry("ContainsText History", list);

  krConfig->sync();
}

bool GeneralFilter::fillQuery( KRQuery *query )
{
  // check that we have (at least) what to search, and where to search in
  if (searchFor->currentText().simplifyWhiteSpace().isEmpty()) {
    KMessageBox::error(this ,i18n("No search criteria entered!"));
    searchFor->setFocus();
    return false;
  }

  // now fill the query

  query->setNameFilter( searchFor->currentText().stripWhiteSpace(), searchForCase->isChecked() );

  bool remoteContent = (properties & FilterTabs::HasRemoteContentSearch ) ? 
                       remoteContentSearch->isChecked() : false;

  query->setContent( containsText->currentText(),
                     containsTextCase->isChecked(),
                     containsWholeWord->isChecked(),
                     remoteContent );

  if (ofType->currentText()!=i18n("All Files"))
    query->setMimeType( ofType->currentText() );
  else query->setMimeType( QString::null );

  if ( properties & FilterTabs::HasRecurseOptions )
  {
    query->setSearchInArchives( searchInArchives->isChecked() );
    query->setRecursive( searchInDirs->isChecked() );
    query->setFollowLinks( followLinks->isChecked() );

    // create the lists
  }
  if( properties & FilterTabs::HasSearchIn )
  {
    query->setSearchInDirs( searchIn->urlList() );

    // checking the lists

    if (query->searchInDirs().isEmpty() ) { // we need a place to search in
      KMessageBox::error(this ,i18n("Please specify a location to search in."));
      searchIn->lineEdit()->setFocus();
      return false;
    }
  }

  if( properties & FilterTabs::HasDontSearchIn )
    query->setDontSearchInDirs( dontSearchIn->urlList() );

  return true;
}

void GeneralFilter::queryAccepted()
{
  searchFor->addToHistory(searchFor->currentText());
  containsText->addToHistory(containsText->currentText());
}

void GeneralFilter::loadFromProfile( QString name )
{
  krConfig->setGroup( name );

  searchForCase->setChecked( krConfig->readBoolEntry( "Case Sensitive Search", false ) );
  containsTextCase->setChecked( krConfig->readBoolEntry( "Case Sensitive Content", false ) );
  remoteContentSearch->setChecked( krConfig->readBoolEntry( "Remote Content Search", false ) );
  containsWholeWord->setChecked( krConfig->readBoolEntry( "Match Whole Word Only", false ) );
  containsText->setEditText( krConfig->readEntry( "Contains Text", "" ) );
  searchFor->setEditText( krConfig->readEntry( "Search For", "" ) );

  QString mime = krConfig->readEntry( "Mime Type", "" );
  for( int i = ofType->count(); i >= 0; i-- )
  {
    ofType->setCurrentItem( i );
    if( ofType->currentText() == mime )
      break;
  }

  if( properties & FilterTabs::HasRecurseOptions )
  {
    searchInDirs->setChecked( krConfig->readBoolEntry( "Search In Subdirectories", true ) );
    searchInArchives->setChecked( krConfig->readBoolEntry( "Search In Archives", false ) );
    followLinks->setChecked( krConfig->readBoolEntry( "Follow Symlinks", false ) );
  }

  if( properties & FilterTabs::HasSearchIn )
  {
    searchIn->lineEdit()->setText( krConfig->readEntry( "Search In Edit", "" ) );

    searchIn->listBox()->clear();
    QStringList searchInList = krConfig->readListEntry( "Search In List" );
    if( !searchInList.isEmpty() )
      searchIn->listBox()->insertStringList( searchInList );
  }

  if( properties & FilterTabs::HasDontSearchIn )
  {
    dontSearchIn->lineEdit()->setText( krConfig->readEntry( "Dont Search In Edit", "" ) );

    dontSearchIn->listBox()->clear();
    QStringList dontSearchInList = krConfig->readListEntry( "Dont Search In List" );
    if( !dontSearchInList.isEmpty() )
      dontSearchIn->listBox()->insertStringList( dontSearchInList );
  }
}

void GeneralFilter::saveToProfile( QString name )
{
  krConfig->setGroup( name );

  krConfig->writeEntry( "Case Sensitive Search", searchForCase->isChecked() );
  krConfig->writeEntry( "Case Sensitive Content", containsTextCase->isChecked() );
  krConfig->writeEntry( "Remote Content Search", remoteContentSearch->isChecked() );
  krConfig->writeEntry( "Match Whole Word Only", containsWholeWord->isChecked() );
  krConfig->writeEntry( "Contains Text", containsText->currentText() );
  krConfig->writeEntry( "Search For", searchFor->currentText() );

  krConfig->writeEntry( "Mime Type", ofType->currentText() );

  if( properties & FilterTabs::HasRecurseOptions )
  {
    krConfig->writeEntry( "Search In Subdirectories", searchInDirs->isChecked() );
    krConfig->writeEntry( "Search In Archives", searchInArchives->isChecked() );
    krConfig->writeEntry( "Follow Symlinks", followLinks->isChecked() );
  }

  if( properties & FilterTabs::HasSearchIn )
  {
    krConfig->writeEntry( "Search In Edit", searchIn->lineEdit()->text() );

    QStringList searchInList;
    for ( QListBoxItem *item = searchIn->listBox()->firstItem(); item != 0; item = item->next() )
      searchInList.append( item->text().simplifyWhiteSpace() );
    krConfig->writeEntry( "Search In List", searchInList );
  }

  if( properties & FilterTabs::HasDontSearchIn )
  {
    krConfig->writeEntry( "Dont Search In Edit", dontSearchIn->lineEdit()->text() );

    QStringList dontSearchInList;
    for ( QListBoxItem *item = dontSearchIn->listBox()->firstItem(); item != 0; item = item->next() )
      dontSearchInList.append( item->text().simplifyWhiteSpace() );
    krConfig->writeEntry( "Dont Search In List", dontSearchInList );
  }
}

void GeneralFilter::refreshProfileListBox()
{
  profileListBox->clear();
  profileListBox->insertStringList( ProfileManager::availableProfiles( "SelectionProfile" ) );

  if( profileListBox->count() != 0 )
  {
    profileLoadBtn->setEnabled( true );
    profileOverwriteBtn->setEnabled( true );
    profileRemoveBtn->setEnabled( true );
  }
  else
  {
    profileLoadBtn->setEnabled( false );
    profileOverwriteBtn->setEnabled( false );
    profileRemoveBtn->setEnabled( false );
  }
}

void GeneralFilter::slotAddBtnClicked()
{
  profileManager->newProfile( searchFor->currentText().simplifyWhiteSpace() );
  refreshProfileListBox();
}

void GeneralFilter::slotOverwriteBtnClicked()
{
  QListBoxItem *item = profileListBox->item( profileListBox->currentItem() );
  if( item != 0 )
    profileManager->overwriteProfile( item->text() );
}

void GeneralFilter::slotRemoveBtnClicked()
{
  QListBoxItem *item = profileListBox->item( profileListBox->currentItem() );
  if( item != 0 )
  {
    profileManager->deleteProfile( item->text() );
    refreshProfileListBox();
  }
}

void GeneralFilter::slotProfileDoubleClicked( QListBoxItem *item )
{
  if( item != 0 )
  {
    QString profileName = item->text();
    profileManager->loadProfile( profileName );
    fltTabs->close( true );
  }
}

void GeneralFilter::slotLoadBtnClicked()
{
  QListBoxItem *item = profileListBox->item( profileListBox->currentItem() );
  if( item != 0 )
    profileManager->loadProfile( item->text() );
}

#include "generalfilter.moc"
