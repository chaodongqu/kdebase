#include <kdialog.h>
#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './actionpropertybase.ui'
**
** Created: Sat Mar 15 11:41:43 2008
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.8   edited Jan 11 14:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "actionpropertybase.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kicondialog.h>
#include <ktextedit.h>
#include <kkeybutton.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <klistbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include "klineedit.h"
#include "kcombobox.h"
#include "kicondialog.h"
#include "ktextedit.h"
#include "kkeybutton.h"
#include "klistbox.h"

/*
 *  Constructs a ActionPropertyBase as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
ActionPropertyBase::ActionPropertyBase( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "ActionPropertyBase" );
    ActionPropertyBaseLayout = new QGridLayout( this, 1, 1, 0, 0, "ActionPropertyBaseLayout"); 

    tabWidget3 = new QTabWidget( this, "tabWidget3" );

    tab = new QWidget( tabWidget3, "tab" );
    tabLayout = new QGridLayout( tab, 1, 1, 11, 6, "tabLayout"); 

    ButtonAddStartpath = new QToolButton( tab, "ButtonAddStartpath" );

    tabLayout->addWidget( ButtonAddStartpath, 8, 3 );

    LabelDescription = new QLabel( tab, "LabelDescription" );
    LabelDescription->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, 0, 0, LabelDescription->sizePolicy().hasHeightForWidth() ) );

    tabLayout->addWidget( LabelDescription, 4, 0 );

    bgAccept = new QButtonGroup( tab, "bgAccept" );
    bgAccept->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)1, 0, 0, bgAccept->sizePolicy().hasHeightForWidth() ) );
    bgAccept->setColumnLayout(0, Qt::Vertical );
    bgAccept->layout()->setSpacing( 6 );
    bgAccept->layout()->setMargin( 11 );
    bgAcceptLayout = new QGridLayout( bgAccept->layout() );
    bgAcceptLayout->setAlignment( Qt::AlignTop );

    radioLocal = new QRadioButton( bgAccept, "radioLocal" );
    radioLocal->setChecked( TRUE );

    bgAcceptLayout->addWidget( radioLocal, 0, 0 );

    radioUrl = new QRadioButton( bgAccept, "radioUrl" );

    bgAcceptLayout->addWidget( radioUrl, 1, 0 );

    tabLayout->addMultiCellWidget( bgAccept, 9, 9, 2, 3 );

    leTitle = new KLineEdit( tab, "leTitle" );

    tabLayout->addMultiCellWidget( leTitle, 2, 2, 1, 3 );

    LabelTitle = new QLabel( tab, "LabelTitle" );
    LabelTitle->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, LabelTitle->sizePolicy().hasHeightForWidth() ) );

    tabLayout->addWidget( LabelTitle, 2, 0 );

    layout3 = new QHBoxLayout( 0, 0, 6, "layout3"); 

    layout2 = new QVBoxLayout( 0, 0, 6, "layout2"); 

    leDistinctName = new KLineEdit( tab, "leDistinctName" );
    layout2->addWidget( leDistinctName );

    cbCategory = new KComboBox( FALSE, tab, "cbCategory" );
    cbCategory->setEditable( TRUE );
    layout2->addWidget( cbCategory );
    layout3->addLayout( layout2 );

    ButtonIcon = new KIconButton( tab, "ButtonIcon" );
    ButtonIcon->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, ButtonIcon->sizePolicy().hasHeightForWidth() ) );
    ButtonIcon->setMinimumSize( QSize( 50, 50 ) );
    ButtonIcon->setMaximumSize( QSize( 50, 50 ) );
    layout3->addWidget( ButtonIcon );

    tabLayout->addMultiCellLayout( layout3, 0, 1, 1, 3 );

    LabelDistinctName = new QLabel( tab, "LabelDistinctName" );
    LabelDistinctName->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, LabelDistinctName->sizePolicy().hasHeightForWidth() ) );

    tabLayout->addWidget( LabelDistinctName, 0, 0 );

    LabelCommandline = new QLabel( tab, "LabelCommandline" );
    LabelCommandline->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, LabelCommandline->sizePolicy().hasHeightForWidth() ) );

    tabLayout->addWidget( LabelCommandline, 7, 0 );

    leTooltip = new KLineEdit( tab, "leTooltip" );

    tabLayout->addMultiCellWidget( leTooltip, 3, 3, 1, 3 );

    leStartpath = new KLineEdit( tab, "leStartpath" );

    tabLayout->addMultiCellWidget( leStartpath, 8, 8, 1, 2 );

    LabelTooltip = new QLabel( tab, "LabelTooltip" );
    LabelTooltip->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, LabelTooltip->sizePolicy().hasHeightForWidth() ) );

    tabLayout->addWidget( LabelTooltip, 3, 0 );

    leCommandline = new KLineEdit( tab, "leCommandline" );

    tabLayout->addMultiCellWidget( leCommandline, 7, 7, 1, 2 );

    LabelCategory = new QLabel( tab, "LabelCategory" );
    LabelCategory->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, LabelCategory->sizePolicy().hasHeightForWidth() ) );

    tabLayout->addWidget( LabelCategory, 1, 0 );

    ButtonAddPlaceholder = new QToolButton( tab, "ButtonAddPlaceholder" );
    ButtonAddPlaceholder->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, ButtonAddPlaceholder->sizePolicy().hasHeightForWidth() ) );
    ButtonAddPlaceholder->setMinimumSize( QSize( 0, 0 ) );

    tabLayout->addWidget( ButtonAddPlaceholder, 7, 3 );

    textDescription = new KTextEdit( tab, "textDescription" );
    textDescription->setWordWrap( KTextEdit::WidgetWidth );

    tabLayout->addMultiCellWidget( textDescription, 4, 6, 1, 3 );

    LabelStartpath = new QLabel( tab, "LabelStartpath" );
    LabelStartpath->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, LabelStartpath->sizePolicy().hasHeightForWidth() ) );

    tabLayout->addWidget( LabelStartpath, 8, 0 );
    spacer = new QSpacerItem( 80, 19, QSizePolicy::Minimum, QSizePolicy::Expanding );
    tabLayout->addItem( spacer, 6, 0 );

    layout4 = new QHBoxLayout( 0, 0, 6, "layout4"); 

    LabelShortcut = new QLabel( tab, "LabelShortcut" );
    layout4->addWidget( LabelShortcut );
    spacer6_2 = new QSpacerItem( 161, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout4->addItem( spacer6_2 );

    KeyButtonShortcut = new KKeyButton( tab, "KeyButtonShortcut" );
    layout4->addWidget( KeyButtonShortcut );

    tabLayout->addMultiCellLayout( layout4, 10, 10, 2, 3 );

    bgExecType = new QButtonGroup( tab, "bgExecType" );
    bgExecType->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)1, 0, 0, bgExecType->sizePolicy().hasHeightForWidth() ) );
    bgExecType->setColumnLayout(0, Qt::Vertical );
    bgExecType->layout()->setSpacing( 6 );
    bgExecType->layout()->setMargin( 11 );
    bgExecTypeLayout = new QGridLayout( bgExecType->layout() );
    bgExecTypeLayout->setAlignment( Qt::AlignTop );

    radioCollectOutput = new QRadioButton( bgExecType, "radioCollectOutput" );

    bgExecTypeLayout->addWidget( radioCollectOutput, 2, 0 );

    chkSeparateStdError = new QCheckBox( bgExecType, "chkSeparateStdError" );
    chkSeparateStdError->setEnabled( FALSE );

    bgExecTypeLayout->addWidget( chkSeparateStdError, 3, 0 );

    radioNormal = new QRadioButton( bgExecType, "radioNormal" );
    radioNormal->setChecked( TRUE );

    bgExecTypeLayout->addWidget( radioNormal, 0, 0 );

    radioTerminal = new QRadioButton( bgExecType, "radioTerminal" );

    bgExecTypeLayout->addWidget( radioTerminal, 1, 0 );

    tabLayout->addMultiCellWidget( bgExecType, 9, 10, 0, 1 );
    tabWidget3->insertTab( tab, QString::fromLatin1("") );

    tab_2 = new QWidget( tabWidget3, "tab_2" );
    tabLayout_2 = new QGridLayout( tab_2, 1, 1, 11, 6, "tabLayout_2"); 

    gbShowonly = new QGroupBox( tab_2, "gbShowonly" );
    gbShowonly->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, gbShowonly->sizePolicy().hasHeightForWidth() ) );
    gbShowonly->setColumnLayout(0, Qt::Vertical );
    gbShowonly->layout()->setSpacing( 6 );
    gbShowonly->layout()->setMargin( 11 );
    gbShowonlyLayout = new QGridLayout( gbShowonly->layout() );
    gbShowonlyLayout->setAlignment( Qt::AlignTop );

    tabShowonly = new QTabWidget( gbShowonly, "tabShowonly" );
    tabShowonly->setTabPosition( QTabWidget::Top );
    tabShowonly->setTabShape( QTabWidget::Triangular );

    TabPage = new QWidget( tabShowonly, "TabPage" );
    TabPageLayout = new QGridLayout( TabPage, 1, 1, 11, 6, "TabPageLayout"); 

    ButtonNewProtocol = new QToolButton( TabPage, "ButtonNewProtocol" );
    ButtonNewProtocol->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, ButtonNewProtocol->sizePolicy().hasHeightForWidth() ) );
    ButtonNewProtocol->setMinimumSize( QSize( 0, 0 ) );
    ButtonNewProtocol->setMaximumSize( QSize( 32767, 32767 ) );

    TabPageLayout->addWidget( ButtonNewProtocol, 0, 1 );

    ButtonEditProtocol = new QToolButton( TabPage, "ButtonEditProtocol" );
    ButtonEditProtocol->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, ButtonEditProtocol->sizePolicy().hasHeightForWidth() ) );
    ButtonEditProtocol->setMinimumSize( QSize( 0, 0 ) );
    ButtonEditProtocol->setMaximumSize( QSize( 32767, 32767 ) );

    TabPageLayout->addWidget( ButtonEditProtocol, 1, 1 );
    spacer6_3 = new QSpacerItem( 21, 58, QSizePolicy::Minimum, QSizePolicy::Expanding );
    TabPageLayout->addItem( spacer6_3, 3, 1 );

    ButtonRemoveProtocol = new QToolButton( TabPage, "ButtonRemoveProtocol" );
    ButtonRemoveProtocol->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, ButtonRemoveProtocol->sizePolicy().hasHeightForWidth() ) );
    ButtonRemoveProtocol->setMinimumSize( QSize( 0, 0 ) );
    ButtonRemoveProtocol->setMaximumSize( QSize( 32767, 32767 ) );

    TabPageLayout->addWidget( ButtonRemoveProtocol, 2, 1 );

    lbShowonlyProtocol = new KListBox( TabPage, "lbShowonlyProtocol" );

    TabPageLayout->addMultiCellWidget( lbShowonlyProtocol, 0, 3, 0, 0 );
    tabShowonly->insertTab( TabPage, QString::fromLatin1("") );

    tab_3 = new QWidget( tabShowonly, "tab_3" );
    tabLayout_3 = new QGridLayout( tab_3, 1, 1, 11, 6, "tabLayout_3"); 

    lbShowonlyPath = new KListBox( tab_3, "lbShowonlyPath" );

    tabLayout_3->addMultiCellWidget( lbShowonlyPath, 0, 3, 0, 0 );

    ButtonAddPath = new QToolButton( tab_3, "ButtonAddPath" );
    ButtonAddPath->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, ButtonAddPath->sizePolicy().hasHeightForWidth() ) );
    ButtonAddPath->setMinimumSize( QSize( 0, 0 ) );
    ButtonAddPath->setMaximumSize( QSize( 32767, 32767 ) );

    tabLayout_3->addWidget( ButtonAddPath, 0, 1 );

    ButtonEditPath = new QToolButton( tab_3, "ButtonEditPath" );
    ButtonEditPath->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, ButtonEditPath->sizePolicy().hasHeightForWidth() ) );
    ButtonEditPath->setMinimumSize( QSize( 0, 0 ) );
    ButtonEditPath->setMaximumSize( QSize( 32767, 32767 ) );

    tabLayout_3->addWidget( ButtonEditPath, 1, 1 );

    ButtonRemovePath = new QToolButton( tab_3, "ButtonRemovePath" );
    ButtonRemovePath->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, ButtonRemovePath->sizePolicy().hasHeightForWidth() ) );
    ButtonRemovePath->setMinimumSize( QSize( 0, 0 ) );
    ButtonRemovePath->setMaximumSize( QSize( 32767, 32767 ) );

    tabLayout_3->addWidget( ButtonRemovePath, 2, 1 );
    spacer4 = new QSpacerItem( 21, 61, QSizePolicy::Minimum, QSizePolicy::Expanding );
    tabLayout_3->addItem( spacer4, 3, 1 );
    tabShowonly->insertTab( tab_3, QString::fromLatin1("") );

    tab_4 = new QWidget( tabShowonly, "tab_4" );
    tabLayout_4 = new QGridLayout( tab_4, 1, 1, 11, 6, "tabLayout_4"); 

    lbShowonlyMime = new KListBox( tab_4, "lbShowonlyMime" );

    tabLayout_4->addMultiCellWidget( lbShowonlyMime, 0, 3, 0, 0 );

    ButtonAddMime = new QToolButton( tab_4, "ButtonAddMime" );
    ButtonAddMime->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, ButtonAddMime->sizePolicy().hasHeightForWidth() ) );
    ButtonAddMime->setMinimumSize( QSize( 0, 0 ) );
    ButtonAddMime->setMaximumSize( QSize( 32767, 32767 ) );

    tabLayout_4->addWidget( ButtonAddMime, 0, 1 );

    ButtonEditMime = new QToolButton( tab_4, "ButtonEditMime" );
    ButtonEditMime->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, ButtonEditMime->sizePolicy().hasHeightForWidth() ) );
    ButtonEditMime->setMinimumSize( QSize( 0, 0 ) );
    ButtonEditMime->setMaximumSize( QSize( 32767, 32767 ) );

    tabLayout_4->addWidget( ButtonEditMime, 1, 1 );

    ButtonRemoveMime = new QToolButton( tab_4, "ButtonRemoveMime" );
    ButtonRemoveMime->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, ButtonRemoveMime->sizePolicy().hasHeightForWidth() ) );
    ButtonRemoveMime->setMinimumSize( QSize( 0, 0 ) );
    ButtonRemoveMime->setMaximumSize( QSize( 32767, 32767 ) );

    tabLayout_4->addWidget( ButtonRemoveMime, 2, 1 );
    spacer5 = new QSpacerItem( 21, 41, QSizePolicy::Minimum, QSizePolicy::Expanding );
    tabLayout_4->addItem( spacer5, 3, 1 );
    tabShowonly->insertTab( tab_4, QString::fromLatin1("") );

    TabPage_2 = new QWidget( tabShowonly, "TabPage_2" );
    TabPageLayout_2 = new QGridLayout( TabPage_2, 1, 1, 11, 6, "TabPageLayout_2"); 

    lbShowonlyFile = new KListBox( TabPage_2, "lbShowonlyFile" );

    TabPageLayout_2->addMultiCellWidget( lbShowonlyFile, 0, 3, 0, 0 );

    ButtonNewFile = new QToolButton( TabPage_2, "ButtonNewFile" );
    ButtonNewFile->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, ButtonNewFile->sizePolicy().hasHeightForWidth() ) );
    ButtonNewFile->setMinimumSize( QSize( 0, 0 ) );
    ButtonNewFile->setMaximumSize( QSize( 32767, 32767 ) );

    TabPageLayout_2->addWidget( ButtonNewFile, 0, 1 );

    ButtonEditFile = new QToolButton( TabPage_2, "ButtonEditFile" );
    ButtonEditFile->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, ButtonEditFile->sizePolicy().hasHeightForWidth() ) );
    ButtonEditFile->setMinimumSize( QSize( 0, 0 ) );
    ButtonEditFile->setMaximumSize( QSize( 32767, 32767 ) );

    TabPageLayout_2->addWidget( ButtonEditFile, 1, 1 );

    ButtonRemoveFile = new QToolButton( TabPage_2, "ButtonRemoveFile" );
    ButtonRemoveFile->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, ButtonRemoveFile->sizePolicy().hasHeightForWidth() ) );
    ButtonRemoveFile->setMinimumSize( QSize( 0, 0 ) );
    ButtonRemoveFile->setMaximumSize( QSize( 32767, 32767 ) );

    TabPageLayout_2->addWidget( ButtonRemoveFile, 2, 1 );
    spacer6 = new QSpacerItem( 21, 41, QSizePolicy::Minimum, QSizePolicy::Expanding );
    TabPageLayout_2->addItem( spacer6, 3, 1 );
    tabShowonly->insertTab( TabPage_2, QString::fromLatin1("") );

    gbShowonlyLayout->addWidget( tabShowonly, 0, 0 );

    tabLayout_2->addMultiCellWidget( gbShowonly, 0, 0, 0, 1 );

    chkConfirmExecution = new QCheckBox( tab_2, "chkConfirmExecution" );

    tabLayout_2->addMultiCellWidget( chkConfirmExecution, 1, 1, 0, 1 );

    chkDifferentUser = new QCheckBox( tab_2, "chkDifferentUser" );

    tabLayout_2->addWidget( chkDifferentUser, 2, 0 );

    leDifferentUser = new KLineEdit( tab_2, "leDifferentUser" );
    leDifferentUser->setEnabled( FALSE );

    tabLayout_2->addWidget( leDifferentUser, 2, 1 );
    spacer3 = new QSpacerItem( 161, 102, QSizePolicy::Minimum, QSizePolicy::Expanding );
    tabLayout_2->addMultiCell( spacer3, 3, 3, 0, 1 );
    tabWidget3->insertTab( tab_2, QString::fromLatin1("") );

    ActionPropertyBaseLayout->addWidget( tabWidget3, 0, 0 );
    languageChange();
    resize( QSize(485, 470).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( radioCollectOutput, SIGNAL( toggled(bool) ), chkSeparateStdError, SLOT( setEnabled(bool) ) );
    connect( chkDifferentUser, SIGNAL( toggled(bool) ), leDifferentUser, SLOT( setEnabled(bool) ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
ActionPropertyBase::~ActionPropertyBase()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void ActionPropertyBase::languageChange()
{
    setCaption( tr2i18n( "Action Property" ) );
    ButtonAddStartpath->setText( tr2i18n( "..." ) );
    LabelDescription->setText( tr2i18n( "Description:" ) );
    QWhatsThis::add( LabelDescription, tr2i18n( "A detailed description of the <b>Useraction</b>. It is only displayed in the <i>Konfigurator</i> and via <code>Shift-F1</code>." ) );
    bgAccept->setTitle( tr2i18n( "Command accepts" ) );
    radioLocal->setText( tr2i18n( "Local files only (no URL's)" ) );
    QWhatsThis::add( radioLocal, tr2i18n( "Substitute the <b>Placeholders</b> with local filenames." ) );
    radioUrl->setText( tr2i18n( "URL's (remote and local)" ) );
    QWhatsThis::add( radioUrl, tr2i18n( "Substitute the <b>Placeholders</b> with valid URL's." ) );
    QWhatsThis::add( leTitle, tr2i18n( "The title displayed in the <b>Usermenu</b>." ) );
    LabelTitle->setText( tr2i18n( "Title:" ) );
    QWhatsThis::add( LabelTitle, tr2i18n( "The title displayed in the <b>Usermenu</b>." ) );
    QWhatsThis::add( leDistinctName, tr2i18n( "Unique name of the <b>Useraction</b>. It is only used in the <i>Konfigurator</i> and doesn't appear in any other menu.<p><b>Note</b>: The <i>Title</i> shown in the <b>Usermenu</b> can be set below." ) );
    QWhatsThis::add( cbCategory, tr2i18n( "<b>Useractions</b> can be grouped in categories for better distinction. Choose a existing <i>Category</i> or create a new one by entering a name." ) );
    ButtonIcon->setText( tr2i18n( "Icon" ) );
    QWhatsThis::add( ButtonIcon, tr2i18n( "Each <b>Useraction</b> can have its own icon. It will appear in front of the title in the <b>Usermenu</b>." ) );
    LabelDistinctName->setText( tr2i18n( "Identifier:" ) );
    QWhatsThis::add( LabelDistinctName, tr2i18n( "<p>Unique name of the <b>Useraction</b>. It is only used in the <i>Konfigurator</i> and doesn't appear in any other menu.</p><p><b>Note</b>: The <i>Title</i> shown in the <b>Usermenu</b> can be set below.</p>" ) );
    LabelCommandline->setText( tr2i18n( "Command:" ) );
    QWhatsThis::add( LabelCommandline, tr2i18n( "<p>The <i>Command</i> defines the command that will be executed when the <b>Useraction</b> is used. It can be a simple shell command or a complex sequence of multiple commands with <b>Placeholders</b>.</p><p>Examples:<ul><code><li>eject /mnt/cdrom</li><li>amarok --append %aList(\"Selected\")%</li></code></ul>\n"
"Please consult the handbook to learn more about the syntax.</p>" ) );
    QWhatsThis::add( leTooltip, tr2i18n( "The <i>Tooltip</i> is shown when the mouse cursor is hold over an entry of the <b>Useraction Toolbar</b>." ) );
    QWhatsThis::add( leStartpath, tr2i18n( "The <i>Workdir</i> defines in which directory the <i>Command</i> will be executed." ) );
    LabelTooltip->setText( tr2i18n( "Tooltip:" ) );
    QWhatsThis::add( LabelTooltip, tr2i18n( "The <i>Tooltip</i> is shown when the mouse cursor is hold over an entry of the <b>Useraction Toolbar</b>." ) );
    leCommandline->setText( QString::null );
    QWhatsThis::add( leCommandline, tr2i18n( "The <i>Command</i> defines the command that will be executed when the <b>Useraction</b> is used. It can be a simple shell command or a complex sequence of multiple commands with <b>Placeholders</b>.<p>\n"
"Examples:<ul><code><li>eject /mnt/cdrom</li><li>amarok --append %aList(\"Selected\")%</li></code></ul>\n"
"Please consult the handbook to learn more about the syntax." ) );
    LabelCategory->setText( tr2i18n( "Category:" ) );
    QWhatsThis::add( LabelCategory, tr2i18n( "<b>Useractions</b> can be grouped in categories for better distinction. Choose a existing <i>Category</i> or create a new one by entering a name." ) );
    ButtonAddPlaceholder->setText( tr2i18n( "&Add" ) );
    QWhatsThis::add( ButtonAddPlaceholder, tr2i18n( "Add <b>Placeholders</b> for the selected files in the panel." ) );
    QWhatsThis::add( textDescription, tr2i18n( "A detailed description of the <b>Useraction</b>. It is only displayed in the <i>Konfigurator</i> and via <code>Shift-F1</code>." ) );
    LabelStartpath->setText( tr2i18n( "Workdir:" ) );
    QWhatsThis::add( LabelStartpath, tr2i18n( "The <i>Workdir</i> defines in which directory the <i>Command</i> will be executed." ) );
    LabelShortcut->setText( tr2i18n( "Default shortcut:" ) );
    KeyButtonShortcut->setText( tr2i18n( "None" ) );
    QWhatsThis::add( KeyButtonShortcut, tr2i18n( "Set a default keyboard shortcut." ) );
    bgExecType->setTitle( tr2i18n( "Execution mode" ) );
    radioCollectOutput->setText( tr2i18n( "Collect output" ) );
    QWhatsThis::add( radioCollectOutput, tr2i18n( "Collect the output of the executed program." ) );
    chkSeparateStdError->setText( tr2i18n( "Separate standard error" ) );
    QWhatsThis::add( chkSeparateStdError, tr2i18n( "Separate standard out and standard error in the output collection." ) );
    radioNormal->setText( tr2i18n( "Normal" ) );
    radioTerminal->setText( tr2i18n( "Run in terminal" ) );
    QWhatsThis::add( radioTerminal, tr2i18n( "Run the command in a terminal." ) );
    tabWidget3->changeTab( tab, tr2i18n( "Basic Properties" ) );
    gbShowonly->setTitle( tr2i18n( "The Useraction is only available for" ) );
    ButtonNewProtocol->setText( tr2i18n( "&New..." ) );
    ButtonEditProtocol->setText( tr2i18n( "Chan&ge..." ) );
    ButtonRemoveProtocol->setText( tr2i18n( "De&lete" ) );
    QWhatsThis::add( lbShowonlyProtocol, tr2i18n( "Show the <b>Useraction</b> only for the values defined here." ) );
    tabShowonly->changeTab( TabPage, tr2i18n( "Protocol" ) );
    QWhatsThis::add( lbShowonlyPath, tr2i18n( "Show the <b>Useraction</b> only for the values defined here." ) );
    ButtonAddPath->setText( tr2i18n( "&New..." ) );
    ButtonEditPath->setText( tr2i18n( "Chan&ge..." ) );
    ButtonRemovePath->setText( tr2i18n( "De&lete" ) );
    tabShowonly->changeTab( tab_3, tr2i18n( "Path" ) );
    QWhatsThis::add( lbShowonlyMime, tr2i18n( "Show the <b>Useraction</b> only for the values defined here." ) );
    ButtonAddMime->setText( tr2i18n( "&New..." ) );
    ButtonEditMime->setText( tr2i18n( "Chan&ge..." ) );
    ButtonRemoveMime->setText( tr2i18n( "De&lete" ) );
    tabShowonly->changeTab( tab_4, tr2i18n( "Mime-type" ) );
    QWhatsThis::add( lbShowonlyFile, tr2i18n( "Show the <b>Useraction</b> only for the filenames defined here. The wildcards '<code>?</code>' and '<code>*</code>' can be used." ) );
    ButtonNewFile->setText( tr2i18n( "&New..." ) );
    ButtonEditFile->setText( tr2i18n( "Chan&ge..." ) );
    ButtonRemoveFile->setText( tr2i18n( "De&lete" ) );
    tabShowonly->changeTab( TabPage_2, tr2i18n( "Filename" ) );
    chkConfirmExecution->setText( tr2i18n( "Confirm each program call separately" ) );
    QWhatsThis::add( chkConfirmExecution, tr2i18n( "Allows to tweak the <i>Command</i> before it is executed." ) );
    chkDifferentUser->setText( tr2i18n( "Run as different user:" ) );
    QWhatsThis::add( chkDifferentUser, tr2i18n( "Execute the <i>Command</i> under a different user-id." ) );
    QWhatsThis::add( leDifferentUser, tr2i18n( "Execute the <i>Command</i> under a different user-id." ) );
    tabWidget3->changeTab( tab_2, tr2i18n( "Advanced Properties" ) );
}

#include "actionpropertybase.moc"
