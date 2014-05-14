/****************************************************************************
** Form implementation generated from reading ui file 'remotemanbase.ui'
**
** Created: Thu Jun 7 16:24:11 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "remotemanbase.h"

#include <klocale.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <kprotocolinfo.h>


/* 
 *  Constructs a remoteManBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
remoteManBase::remoteManBase( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "remoteManBase" );
    resize( 670, 502 ); 
    setCaption( i18n( "RemoteMan: Connection Manager" ) );
    setSizeGripEnabled( TRUE );
    remoteManBaseLayout = new QGridLayout( this );
    remoteManBaseLayout->setSpacing( 6 );
    remoteManBaseLayout->setMargin( 11 );

    Layout23 = new QVBoxLayout;
    Layout23->setSpacing( 6 );
    Layout23->setMargin( 0 );

    TextLabel1 = new QLabel( this, "TextLabel1" );
    TextLabel1->setText( i18n( "Session name:" ) );
    Layout23->addWidget( TextLabel1 );

    sessionName = new QLineEdit( this, "sessionName" );
    Layout23->addWidget( sessionName );

    remoteManBaseLayout->addLayout( Layout23, 0, 1 );

    Layout12 = new QHBoxLayout;
    Layout12->setSpacing( 6 );
    Layout12->setMargin( 0 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout12->addItem( spacer );

    moreBtn = new QPushButton( this, "moreBtn" );
    moreBtn->setText( i18n( "&More" ) );
    moreBtn->setAutoDefault( FALSE );
    moreBtn->setDefault( FALSE );
    Layout12->addWidget( moreBtn );

    closeBtn = new QPushButton( this, "closeBtn" );
    closeBtn->setText( i18n( "&Close" ) );
    closeBtn->setAutoDefault( FALSE );
    closeBtn->setDefault( FALSE );
    Layout12->addWidget( closeBtn );

    remoteManBaseLayout->addMultiCellLayout( Layout12, 7, 7, 0, 1 );

    sessions = new QListView( this, "sessions" );
    sessions->addColumn( i18n( "Sessions" ) );
    sessions->header()->setClickEnabled( FALSE, sessions->header()->count() - 1 );
    sessions->header()->setResizeEnabled( FALSE, sessions->header()->count() - 1 );
    sessions->setMinimumSize( QSize( 300, 400 ) );
    sessions->setVScrollBarMode( QListView::AlwaysOn );
    sessions->setHScrollBarMode( QListView::Auto );
    sessions->setRootIsDecorated( TRUE );

    remoteManBaseLayout->addMultiCellWidget( sessions, 0, 6, 0, 0 );

    Layout9 = new QVBoxLayout;
    Layout9->setSpacing( 6 );
    Layout9->setMargin( 0 );

    Layout10 = new QGridLayout;
    Layout10->setSpacing( 6 );
    Layout10->setMargin( 0 );

    TextLabel1_3_3 = new QLabel( this, "TextLabel1_3_3" );
    TextLabel1_3_3->setText( i18n( "Password:" ) );

    Layout10->addWidget( TextLabel1_3_3, 0, 1 );

    password = new QLineEdit( this, "password" );

    Layout10->addWidget( password, 1, 1 );

    TextLabel1_3 = new QLabel( this, "TextLabel1_3" );
    TextLabel1_3->setText( i18n( "User name:" ) );

    Layout10->addWidget( TextLabel1_3, 0, 0 );

    userName = new QLineEdit( this, "userName" );

    Layout10->addWidget( userName, 1, 0 );
    Layout9->addLayout( Layout10 );

    anonymous = new QCheckBox( this, "anonymous" );
    anonymous->setText( i18n( "Anonymous" ) );
    Layout9->addWidget( anonymous );

    remoteManBaseLayout->addLayout( Layout9, 2, 1 );

    Layout26 = new QVBoxLayout;
    Layout26->setSpacing( 6 );
    Layout26->setMargin( 0 );

    TextLabel1_3_2 = new QLabel( this, "TextLabel1_3_2" );
    TextLabel1_3_2->setText( i18n( "Remote directory:" ) );
    Layout26->addWidget( TextLabel1_3_2 );

    remoteDir = new QLineEdit( this, "remoteDir" );
    Layout26->addWidget( remoteDir );

    remoteManBaseLayout->addLayout( Layout26, 4, 1 );

    Layout27 = new QVBoxLayout;
    Layout27->setSpacing( 6 );
    Layout27->setMargin( 0 );

    TextLabel1_3_2_2 = new QLabel( this, "TextLabel1_3_2_2" );
    TextLabel1_3_2_2->setText( i18n( "Description:" ) );
    Layout27->addWidget( TextLabel1_3_2_2 );

    description = new QMultiLineEdit( this, "description" );
    Layout27->addWidget( description );

    remoteManBaseLayout->addLayout( Layout27, 5, 1 );

    layout = new QGridLayout;
    layout->setSpacing( 6 );
    layout->setMargin( 0 );

    removeBtn = new QPushButton( this, "removeBtn" );
    removeBtn->setText( i18n( "&Remove" ) );
    removeBtn->setAutoDefault( FALSE );
    removeBtn->setDefault( FALSE );

    layout->addWidget( removeBtn, 0, 2 );

    connectBtn = new QPushButton( this, "connectBtn" );
    connectBtn->setText( i18n( "Co&nnect" ) );
    connectBtn->setAutoDefault( FALSE );
    connectBtn->setDefault( FALSE );

    layout->addMultiCellWidget( connectBtn, 1, 1, 0, 2 );

    newGroupBtn = new QPushButton( this, "newGroupBtn" );
    newGroupBtn->setEnabled( TRUE );
    newGroupBtn->setText( i18n( "New &Group" ) );
    newGroupBtn->setAutoDefault( FALSE );
    newGroupBtn->setDefault( FALSE );
    newGroupBtn->setFlat( FALSE );

    layout->addWidget( newGroupBtn, 0, 0 );

    addBtn = new QPushButton( this, "addBtn" );
    addBtn->setEnabled( TRUE );
    addBtn->setText( i18n( "New Connec&tion" ) );
    addBtn->setAutoDefault( FALSE );
    addBtn->setDefault( FALSE );
    addBtn->setFlat( FALSE );

    layout->addWidget( addBtn, 0, 1 );

    remoteManBaseLayout->addLayout( layout, 6, 1 );

    Layout11 = new QGridLayout;
    Layout11->setSpacing( 6 );
    Layout11->setMargin( 0 );

    TextLabel1_2 = new QLabel( this, "TextLabel1_2" );
    TextLabel1_2->setText( i18n( "Host:" ) );

    Layout11->addMultiCellWidget( TextLabel1_2, 0, 0, 0, 1 );

    QStringList protocols = KProtocolInfo::protocols();

    protocol = new QComboBox( FALSE, this, "protocol" );
    if( protocols.contains("ftp") )
      protocol->insertItem( i18n( "ftp://" ) );
    if( protocols.contains("smb") )
      protocol->insertItem( i18n( "smb://" ) );
    if( protocols.contains("fish") )
      protocol->insertItem( i18n( "fish://" ));
    if( protocols.contains("sftp") )
      protocol->insertItem( i18n( "sftp://" ));
    protocol->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, protocol->sizePolicy().hasHeightForWidth() ) );

    Layout11->addWidget( protocol, 1, 0 );

    hostName = new QLineEdit( this, "hostName" );
    hostName->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, hostName->sizePolicy().hasHeightForWidth() ) );
    hostName->setMinimumSize( QSize( 0, 0 ) );

    Layout11->addWidget( hostName, 1, 1 );

    portNum = new QSpinBox( this, "portNum" );
    portNum->setMaxValue( 99999 );
    portNum->setValue( 21 );

    Layout11->addWidget( portNum, 1, 2 );

    TextLabel1_2_2 = new QLabel( this, "TextLabel1_2_2" );
    TextLabel1_2_2->setText( i18n( "Port:   " ) );

    Layout11->addWidget( TextLabel1_2_2, 0, 2 );

    remoteManBaseLayout->addLayout( Layout11, 1, 1 );

    TextLabel1_4 = new QLabel( this, "TextLabel1_4" );
    QFont TextLabel1_4_font(  TextLabel1_4->font() );
    TextLabel1_4_font.setPointSize( 10 );
    TextLabel1_4->setFont( TextLabel1_4_font );
    TextLabel1_4->setText( i18n( "* Warning: Storing your password is not secure !!!" ) );
    TextLabel1_4->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    remoteManBaseLayout->addWidget( TextLabel1_4, 3, 1 );

    // signals and slots connections
    connect( closeBtn, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( addBtn, SIGNAL( clicked() ), this, SLOT( addSession() ) );
    connect( removeBtn, SIGNAL( clicked() ), this, SLOT( removeSession() ) );
    connect( sessions, SIGNAL( selectionChanged(QListViewItem*) ), this, SLOT( refreshData() ) );
    connect( sessions, SIGNAL( selectionChanged() ), this, SLOT( refreshData() ) );
    connect( sessions, SIGNAL( expanded(QListViewItem*) ), this, SLOT( refreshData() ) );
    connect( sessions, SIGNAL( currentChanged(QListViewItem*) ), this, SLOT( refreshData() ) );
    connect( sessions, SIGNAL( mouseButtonClicked(int,QListViewItem*,const 
QPoint&,int) ), this, SLOT( refreshData() ) );
    connect( sessions, SIGNAL( collapsed(QListViewItem*) ), this, SLOT( refreshData() ) );
    connect( connectBtn, SIGNAL( clicked() ), this, SLOT( connection() ) );
    connect( sessionName, SIGNAL( textChanged(const QString&) ), this, SLOT( updateName(const QString&) ) );
    connect( newGroupBtn, SIGNAL( clicked() ), this, SLOT( addGroup() ) );
    connect( anonymous, SIGNAL( clicked() ), this, SLOT( refreshData() ) );
    connect( protocol, SIGNAL(activated(int)), this, SLOT(refreshData()));

    // tab order
    setTabOrder( sessionName, hostName );
    setTabOrder( hostName, userName );
    setTabOrder( userName, password );
    setTabOrder( password, remoteDir );
    setTabOrder( remoteDir, description );
    setTabOrder( description, connectBtn );
    setTabOrder( connectBtn, addBtn );
    setTabOrder( addBtn, newGroupBtn );
    setTabOrder( newGroupBtn, removeBtn );
    setTabOrder( removeBtn, moreBtn );
    setTabOrder( moreBtn, closeBtn );
    setTabOrder( closeBtn, sessions );
    setTabOrder( sessions, portNum );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
remoteManBase::~remoteManBase()
{
    // no need to delete child widgets, Qt does it all for us
}

/*  
 *  Main event handler. Reimplemented to handle application
 *  font changes
 */
bool remoteManBase::event( QEvent* ev )
{
    bool ret = QDialog::event( ev ); 
    if ( ev->type() == QEvent::ApplicationFontChange ) {
	QFont TextLabel1_4_font(  TextLabel1_4->font() );
	TextLabel1_4_font.setPointSize( 10 );
	TextLabel1_4->setFont( TextLabel1_4_font ); 
    }
    return ret;
}

void remoteManBase::addSession()
{
    qWarning( "remoteManBase::addSession(): Not implemented yet!" );
}

void remoteManBase::connection()
{
    qWarning( "remoteManBase::connection(): Not implemented yet!" );
}

void remoteManBase::moreInfo()
{
    qWarning( "remoteManBase::moreInfo(): Not implemented yet!" );
}

void remoteManBase::addGroup()
{
    qWarning( "remoteManBase::addGroup(): Not implemented yet!" );
}

void remoteManBase::refreshData()
{
    qWarning( "remoteManBase::refreshData(): Not implemented yet!" );
}

void remoteManBase::removeSession()
{
    qWarning( "remoteManBase::removeSession(): Not implemented yet!" );
}

void remoteManBase::updateName(const QString&)
{
    qWarning( "remoteManBase::updateName(const QString&): Not implemented yet!" );
}

#include "remotemanbase.moc"
