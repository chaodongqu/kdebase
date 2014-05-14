/***************************************************************************
                         kggeneral.cpp  -  description
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

#include <qlabel.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qfontmetrics.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include "krresulttabledialog.h"
#include "kggeneral.h"
#include "../defaults.h"
#include "../krusader.h"
#include "../kicons.h"

KgGeneral::KgGeneral( bool first, QWidget* parent,  const char* name ) :
      KonfiguratorPage( first, parent, name )
{
if( first )  
    slotFindTools();

  QGridLayout *kgGeneralLayout = new QGridLayout( parent );
  kgGeneralLayout->setSpacing( 6 );

  //  -------------------------- GENERAL GROUPBOX ----------------------------------

  QGroupBox *generalGrp = createFrame( i18n( "General" ), parent, "kgGenGeneralGrp" );
  QGridLayout *generalGrid = createGridLayout( generalGrp->layout() );

  KONFIGURATOR_NAME_VALUE_TIP deleteMode[] =
  //            name            value    tooltip
    {{ i18n( "Delete files" ),  "false", i18n( "Files will be permanently deleted." ) },
     { i18n( "Move to trash" ), "true",  i18n( "Files will be moved to trash when deleted." ) }};

  KonfiguratorRadioButtons *trashRadio = createRadioButtonGroup( "General", "Move To Trash",
      _MoveToTrash ? "true" : "false", 2, 0, deleteMode, 2, generalGrp, "myRadio", false );
  generalGrid->addMultiCellWidget( trashRadio, 0, 0, 0, 1 );

  KonfiguratorCheckBox *checkBox = createCheckBox( "General", "Mimetype Magic", _MimetypeMagic,
                     i18n( "Use mimetype magic" ), generalGrp, false,
                     i18n( "Mimetype magic allows better distinction of file types, but is slower." ) );
  generalGrid->addMultiCellWidget( checkBox, 1, 1, 0, 1 );

  QFrame *line1 = createLine( generalGrp, "line1" );
  generalGrid->addMultiCellWidget( line1, 2, 2, 0, 1 );

  // editor
  QLabel *label1 = new QLabel( i18n( "Editor:" ), generalGrp, "EditorLabel" );
  generalGrid->addWidget( label1, 3, 0 );
  KonfiguratorURLRequester *urlReq = createURLRequester( "General", "Editor", "internal editor",
                                      generalGrp, false );
  generalGrid->addWidget( urlReq, 3, 1 );

  QLabel *label2 = new QLabel( i18n( "Hint: use 'internal editor' if you want to use Krusader's fast built-in editor" ), generalGrp, "EditorLabel" );
  generalGrid->addMultiCellWidget( label2, 4, 4, 0, 1 );

QFrame *line2 = createLine( generalGrp, "line2" );
  generalGrid->addMultiCellWidget( line2, 5, 5, 0, 1 );

  // viewer

  QHBox * hbox2 = new QHBox( generalGrp );
  QVBox * vbox = new QVBox( hbox2 );

  new QLabel( i18n("Default viewer mode:"), vbox);
  
  KONFIGURATOR_NAME_VALUE_TIP viewMode[] =
  //            name            value    tooltip
    {{ i18n( "Generic mode" ),  "generic", i18n( "Use the system's default viewer" ) },
     { i18n( "Text mode" ), "text",  i18n( "View the file in text-only mode" ) }, 
     { i18n( "Hex mode" ), "hex",  i18n( "View the file in hex-mode (better for binary files)" ) } };
  createRadioButtonGroup( "General", "Default Viewer Mode",
      "generic", 0, 3, viewMode, 3, vbox, "myRadio2", false );

  createCheckBox( "General", "View In Separate Window", _ViewInSeparateWindow,
                     i18n( "Internal editor and viewer opens each file in a separate window" ), vbox, false,
                     i18n( "If checked, each file will open in a separate window, otherwise, the viewer will work in a single, tabbed mode" ) );

  generalGrid->addMultiCellWidget(hbox2, 6, 8, 0, 1);

  // atomic extensions
  QFrame * frame21 = createLine( hbox2, "line2.1", true );
  frame21->setMinimumWidth( 15 );
  QVBox * vbox2 = new QVBox( hbox2 );

  QHBox * hbox3 = new QHBox( vbox2 );
  QLabel * atomLabel = new QLabel( i18n("Atomic extensions:"), hbox3);

  int size = QFontMetrics( atomLabel->font() ).height();

  QToolButton *addButton = new QToolButton( hbox3, "addBtnList" );
  QPixmap icon = krLoader->loadIcon("add",KIcon::Desktop, size );
  addButton->setFixedSize( icon.width() + 4, icon.height() + 4 );
  addButton->setPixmap( icon );
  connect( addButton, SIGNAL( clicked() ), this, SLOT( slotAddExtension() ) );

  QToolButton *removeButton = new QToolButton( hbox3, "removeBtnList" );
  icon = krLoader->loadIcon("remove",KIcon::Desktop, size );
  removeButton->setFixedSize( icon.width() + 4, icon.height() + 4 );
  removeButton->setPixmap( icon );
  connect( removeButton, SIGNAL( clicked() ), this, SLOT( slotRemoveExtension() ) );

  QStringList defaultAtomicExtensions;
  defaultAtomicExtensions += ".tar.gz";
  defaultAtomicExtensions += ".tar.bz2";
  defaultAtomicExtensions += ".moc.cpp";

  listBox = createListBox( "Look&Feel", "Atomic Extensions", 
      defaultAtomicExtensions, vbox2, true, false );

  QFrame *line3 = createLine( generalGrp, "line3" );
  generalGrid->addMultiCellWidget( line3, 9, 9, 0, 1 );

	// terminal
  QLabel *label3 = new QLabel( i18n( "Terminal:" ), generalGrp, "TerminalLabel" );
  generalGrid->addWidget( label3, 10, 0 );
  KonfiguratorURLRequester *urlReq2 = createURLRequester( "General", "Terminal", "konsole",
                                      generalGrp, false );
  generalGrid->addWidget( urlReq2, 10, 1 );

  KonfiguratorCheckBox *checkBox1 = createCheckBox( "General", "Send CDs", _SendCDs,
                     i18n( "Terminal Emulator sends Chdir on panel change" ), generalGrp, false,
                     i18n( "When checked, whenever the panel is changed (for example, by pressing TAB), krusader changes the current directory in the terminal emulator." ) );
  generalGrid->addMultiCellWidget( checkBox1, 11, 11, 0, 1 );

  QFrame *line31 = createLine( generalGrp, "line4" );
  generalGrid->addMultiCellWidget( line31, 12, 12, 0, 1 );

	// temp dir
  QHBox *hbox = new QHBox( generalGrp, "hbox" );
  new QLabel( i18n( "Temp Directory:" ), hbox, "TempDirectory" );
  KonfiguratorURLRequester *urlReq3 = createURLRequester( "General", "Temp Directory", "/tmp/krusader.tmp",
                                      hbox, false );
  urlReq3->setMode( KFile::Directory );
  connect( urlReq3->extension(), SIGNAL( applyManually(QObject *,QString, QString) ),
           this, SLOT( applyTempDir(QObject *,QString, QString) ) );
  generalGrid->addMultiCellWidget( hbox, 13, 13, 0, 1 );

  QLabel *label4 = new QLabel( i18n( "Note: you must have full permissions for the temporary directory!" ),
                               generalGrp, "NoteLabel"  );
  generalGrid->addMultiCellWidget( label4, 14, 14, 0, 1 );


  kgGeneralLayout->addWidget( generalGrp, 0 ,0 );
}

void KgGeneral::applyTempDir(QObject *obj,QString cls, QString name)
{
  KonfiguratorURLRequester *urlReq = (KonfiguratorURLRequester *)obj;
  QString value = QDir(urlReq->url()).path();

  krConfig->setGroup( cls );
  krConfig->writeEntry( name, value );
}

void KgGeneral::slotFindTools()
{
  KrResultTableDialog* dia = new KrResultTableDialog(this, KrResultTableDialog::Tool, i18n("Search results"), i18n("Searching for tools..."),
    "package_settings", i18n("Make sure to install new tools in your <code>$PATH</code> (e.g. /usr/bin)"));
  dia->exec();
}

void KgGeneral::slotAddExtension()
{
  bool ok;
  QString atomExt =
    KInputDialog::getText( i18n( "Add new atomic extension" ), i18n( "Extension: " ), QString::null, &ok );

  if( ok )
  {
    if( !atomExt.startsWith( "." ) || atomExt.find( '.', 1 ) == -1 )
      KMessageBox::error(krApp, i18n("Atomic extensions must start with '.'\n and must contain at least one more '.' character"), i18n("Error"));
    else
      listBox->addItem( atomExt );
  }
}

void KgGeneral::slotRemoveExtension()
{
  QListBoxItem * item = listBox->selectedItem();
  if( item )
    listBox->removeItem( item->text() );
}

#include "kggeneral.moc"
