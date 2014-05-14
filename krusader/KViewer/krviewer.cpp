/***************************************************************************
                         krviewer.cpp  -  description
                            -------------------
   begin                : Thu Apr 18 2002
   copyright            : (C) 2002 by Shie Erlich & Rafi Yanai
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
// Qt includes
#include <qdatastream.h>
#include <qfile.h>
#include <qpopupmenu.h> 
#include <qtimer.h>
// KDE includes
#include <kmenubar.h>
#include <kmimetype.h>
#include <klocale.h>
#include <kparts/part.h>
#include <kparts/componentfactory.h>
#include <kmessagebox.h>
#include <klibloader.h>
#include <ktrader.h>
#include <kio/netaccess.h>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <kstatusbar.h>
#include <kdebug.h>
#include <klargefile.h>
#include <khtml_part.h>
#include <kprocess.h>
#include <kfileitem.h> 
// Krusader includes
#include "krviewer.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../kicons.h"

#include "panelviewer.h"

#define VIEW_ICON     "viewmag"
#define EDIT_ICON     "edit"
#define MODIFIED_ICON "filesaveas"


QPtrList<KrViewer> KrViewer::viewers;

KrViewer::KrViewer( QWidget *parent, const char *name ) :
KParts::MainWindow( parent, name ), manager( this, this ), tabBar( this ), returnFocusTo( 0 ), returnFocusTab( 0 ),
                                    reservedKeys(), reservedKeyIDs() {

	//setWFlags(WType_TopLevel | WDestructiveClose);
	setXMLFile( "krviewer.rc" ); // kpart-related xml file
	setHelpMenuEnabled( false );

	setAutoSaveSettings( "KrViewerWindow", true );
	tmpFile.setAutoDelete( true );

	connect( &manager, SIGNAL( activePartChanged( KParts::Part* ) ),
	         this, SLOT( createGUI( KParts::Part* ) ) );
	connect( &tabBar, SIGNAL( currentChanged( QWidget *) ),
	         this, SLOT( tabChanged(QWidget*) ) );
	connect( &tabBar, SIGNAL( closeRequest( QWidget *) ),
	         this, SLOT( tabCloseRequest(QWidget*) ) );

	tabBar.setTabReorderingEnabled(false);
#if KDE_IS_VERSION(3,4,0)
	tabBar.setAutomaticResizeTabs(true);
#endif
//	"edit"
//	"filesaveas"
	setCentralWidget( &tabBar );

	printAction = KStdAction::print( this, SLOT( print() ), 0, 0 );
	copyAction = KStdAction::copy( this, SLOT( copy() ), 0, 0 );

	viewerMenu = new QPopupMenu( this );
	viewerMenu->insertItem( i18n( "&Generic viewer" ), this, SLOT( viewGeneric() ), CTRL + SHIFT + Key_G, 1 );
	viewerMenu->insertItem( i18n( "&Text viewer" ), this, SLOT( viewText() ), CTRL + SHIFT + Key_T, 2 );
	viewerMenu->insertItem( i18n( "&Hex viewer" ), this, SLOT( viewHex() ), CTRL + SHIFT + Key_H, 3 );
	viewerMenu->insertSeparator();
	viewerMenu->insertItem( i18n( "Text &editor" ), this, SLOT( editText() ), CTRL + SHIFT + Key_E, 4 );
	viewerMenu->insertSeparator();
	viewerMenu->insertItem( i18n( "&Next tab" ), this, SLOT( nextTab() ), ALT+Key_Right );
	viewerMenu->insertItem( i18n( "&Previous tab" ), this, SLOT( prevTab() ), ALT+Key_Left );

	detachActionIndex = viewerMenu->insertItem( i18n( "&Detach tab" ), this, SLOT( detachTab() ), CTRL + SHIFT + Key_D );
	//no point in detaching only one tab..
	viewerMenu->setItemEnabled(detachActionIndex,false);	
	viewerMenu->insertSeparator();
	viewerMenu->insertItem( printAction->text(), this, SLOT( print() ), printAction->shortcut() );
	viewerMenu->insertItem( copyAction->text(), this, SLOT( copy() ), copyAction->shortcut() );
	viewerMenu->insertSeparator();
	tabCloseID = viewerMenu->insertItem( i18n( "&Close current tab" ), this, SLOT( tabCloseRequest() ), Key_Escape );
	closeID = viewerMenu->insertItem( i18n( "&Quit" ), this, SLOT( close() ), CTRL + Key_Q );

	//toolBar() ->insertLined("Edit:",1,"",this,"",true ,i18n("Enter an URL to edit and press enter"));
	
	tabBar.setHoverCloseButton(true);

	checkModified();
}

KrViewer::~KrViewer() {

	disconnect( &manager, SIGNAL( activePartChanged( KParts::Part* ) ),
	            this, SLOT( createGUI( KParts::Part* ) ) );

	viewers.remove( this );
	delete printAction;
	delete copyAction;
}

void KrViewer::createGUI( KParts::Part* part ) {
	if ( part == 0 )   /*     KHTMLPart calls this function with 0 at destruction.    */
		return ;        /*   Can cause crash after JavaScript self.close() if removed  */

	
	// and show the new part widget
	connect( part, SIGNAL( setStatusBarText( const QString& ) ),
	         this, SLOT( slotSetStatusBarText( const QString& ) ) );

	KParts::MainWindow::createGUI( part );
	toolBar() ->insertLineSeparator(0);

	PanelViewerBase *pvb = getPanelViewerBase( part );
	if( pvb )
		updateActions( pvb );

	toolBar() ->show();
	statusBar() ->show();

	// the KParts part may override the viewer shortcuts. We prevent it
	// by installing an event filter on the menuBar() and the part
	reservedKeys.clear();
	reservedKeyIDs.clear();
	
	// getting the key sequences of the viewer menu
	for( unsigned w=0; w != viewerMenu->count(); w++ )
	{
		int id = viewerMenu->idAt( w );
		QKeySequence sequence = viewerMenu->accel( id );
		if( sequence.count() > 0 )
		{
			reservedKeys.push_back( sequence[ 0 ] );
			reservedKeyIDs.push_back( id );
		}
	}
	
	// and "fix" the menubar
	menuBar() ->removeItem( 70 );
	menuBar() ->insertItem( i18n( "&KrViewer" ), viewerMenu, 70 );
	menuBar() ->show();
	
	// filtering out the key events
	menuBar() ->installEventFilter( this );
	part->installEventFilter( this );
}

bool KrViewer::eventFilter (  QObject * /* watched */, QEvent * e )
{
	if( e->type() == QEvent::AccelOverride )
	{
		QKeyEvent* ke = (QKeyEvent*) e;
		if( reservedKeys.contains( ke->key() ) )
		{
			ke->accept();
			
			int id = reservedKeyIDs[ reservedKeys.findIndex( ke->key() ) ];
			if( id != -1 )
			{
				// don't activate the close functions immediately!
				// it can cause crash
				if( id == tabCloseID )
					QTimer::singleShot( 0, this, SLOT( tabCloseRequest() ) );
				else if( id == closeID )
					QTimer::singleShot( 0, this, SLOT( close() ) );
				else {
					int index = viewerMenu->indexOf( id );
					viewerMenu->activateItemAt( index );
				}
			}
			return true;
		}
	}
	else if( e->type() == QEvent::KeyPress )
	{
		QKeyEvent* ke = (QKeyEvent*) e;
		if( reservedKeys.contains( ke->key() ) )
		{
			ke->accept();
			return true;
		}
	}
	return false;
}
void KrViewer::keyPressEvent( QKeyEvent *e ) {
	switch ( e->key() ) {
		case Key_F10:
			close();
			break;
		case Key_Escape:
			tabCloseRequest();
			break;
		default:
			e->ignore();
			break;
	}
}

KrViewer* KrViewer::getViewer(bool new_window){
	if( !new_window ){
		if( !viewers.first() ){	
			viewers.prepend( new KrViewer() ); // add to first (active)
		}
		else {
			if( viewers.first()->isMinimized() ) // minimized? -> show it again
				viewers.first()->showNormal();
			viewers.first()->raise();
			viewers.first()->setActiveWindow();
		}
		return viewers.first();
	}
	else {
		KrViewer *newViewer = new KrViewer();
		viewers.prepend( newViewer );
		return newViewer;
	}
}	

void KrViewer::view( KURL url, QWidget * parent ) {
	Mode defaultMode = Generic;
	bool defaultWindow = false;

	krConfig->setGroup( "General" );
	defaultWindow = krConfig->readBoolEntry( "View In Separate Window",_ViewInSeparateWindow );

	QString modeString = krConfig->readEntry( "Default Viewer Mode","generic" );

	if( modeString == "generic" ) defaultMode = Generic;
	else if( modeString == "text" ) defaultMode = Text;
	else if( modeString == "hex" ) defaultMode = Hex;

	view(url,defaultMode,defaultWindow, parent );
}

void KrViewer::view( KURL url, Mode mode,  bool new_window, QWidget * parent ) {
	KrViewer* viewer = getViewer(new_window);

	PanelViewerBase* viewWidget = new PanelViewer(&viewer->tabBar);
	KParts::Part* part = viewWidget->openURL(url,mode);
	viewer->addTab(viewWidget,i18n( "Viewing" ),VIEW_ICON,part);

	viewer->returnFocusTo = parent;
	viewer->returnFocusTab = viewWidget;
}

void KrViewer::edit( KURL url, QWidget * parent ) {
	edit( url, Text, -1, parent );
}

void KrViewer::edit( KURL url, Mode mode, int new_window, QWidget * parent ) {
	krConfig->setGroup( "General" );
	QString edit = krConfig->readEntry( "Editor", _Editor );
	
	if( new_window == -1 )
		new_window = krConfig->readBoolEntry( "View In Separate Window",_ViewInSeparateWindow );

	if ( edit != "internal editor" ) {
		KProcess proc;
		// if the file is local, pass a normal path and not a url. this solves
		// the problem for editors that aren't url-aware
		if ( url.isLocalFile() )
			proc << QStringList::split( ' ', edit ) << url.path();
		else proc << QStringList::split( ' ', edit ) << url.prettyURL();
		if ( !proc.start( KProcess::DontCare ) )
			KMessageBox::sorry( krApp, i18n( "Can't open " ) + "\"" + edit + "\"" );
		return ;
	}

	KrViewer* viewer = getViewer(new_window);

	PanelViewerBase* editWidget = new PanelEditor(&viewer->tabBar);
	KParts::Part* part = editWidget->openURL(url,mode);
	viewer->addTab(editWidget,i18n("Editing"),EDIT_ICON,part);
	
	viewer->returnFocusTo = parent;
	viewer->returnFocusTab = editWidget;
}

void KrViewer::addTab(PanelViewerBase* pvb, QString msg, QString iconName ,KParts::Part* part){
	if( !part ) return;

	KURL url = pvb->url();
	setCaption( msg+": " + url.prettyURL() );

	QIconSet icon = QIconSet(krLoader->loadIcon(iconName,KIcon::Small));

	manager.addPart( part, this );
	manager.setActivePart( part );
	tabBar.insertTab(pvb,icon,url.fileName()+"("+msg+")");	
	tabBar.setCurrentPage(tabBar.indexOf(pvb));
	tabBar.setTabToolTip(pvb,msg+": " + url.prettyURL());

	updateActions( pvb );

	// now we can offer the option to detach tabs (we have more than one)
	if( tabBar.count() > 1 ){
		viewerMenu->setItemEnabled(detachActionIndex,true);	
	}

	show();
	tabBar.show();
	
	connect( pvb, SIGNAL( urlChanged( PanelViewerBase *, const KURL & ) ), 
	         this,  SLOT( tabURLChanged(PanelViewerBase *, const KURL & ) ) );
}

void KrViewer::tabURLChanged( PanelViewerBase *pvb, const KURL & url ) {
	QString msg = pvb->isEditor() ? i18n( "Editing" ) : i18n( "Viewing" );
	tabBar.setTabLabel( pvb, url.fileName()+"("+msg+")" );
	tabBar.setTabToolTip(pvb,msg+": " + url.prettyURL());
}

void KrViewer::tabChanged(QWidget* w){
	manager.setActivePart( static_cast<PanelViewerBase*>(w)->part() );
	
	if( static_cast<PanelViewerBase*>(w) != returnFocusTab ) {
		returnFocusTo = 0;
		returnFocusTab = 0;
	}
	
	// set this viewer to be the main viewer
	if( viewers.remove( this ) ) viewers.prepend( this ); // move to first
}

void KrViewer::tabCloseRequest(QWidget *w){
	if( !w ) return;
	
	// important to save as returnFocusTo will be cleared at removePart
	QWidget * returnFocusToThisWidget = returnFocusTo;
	
	PanelViewerBase* pvb = static_cast<PanelViewerBase*>(w);
	
	if( !pvb->queryClose() )
		return;
		
	manager.removePart(pvb->part());
	
	pvb->closeURL();
	
	tabBar.removePage(w);

	if( tabBar.count() <= 0 ){
		if( returnFocusToThisWidget ){ 
			returnFocusToThisWidget->raise();
			returnFocusToThisWidget->setActiveWindow();
		}
		else {
			krApp->raise();
			krApp->setActiveWindow();
		}
		delete this;
		return;
	} else if( tabBar.count() == 1 ){
		//no point in detaching only one tab..
		viewerMenu->setItemEnabled(detachActionIndex,false);
	}

	if( returnFocusToThisWidget ){ 
		returnFocusToThisWidget->raise();
		returnFocusToThisWidget->setActiveWindow();
	}
}

void KrViewer::tabCloseRequest(){
	tabCloseRequest( tabBar.currentPage() ); 
}

bool KrViewer::queryClose() {
	for( int i=0; i != tabBar.count(); i++ ) {
		PanelViewerBase* pvb = static_cast<PanelViewerBase*>( tabBar.page( i ) );
		if( !pvb )
			continue;
		
		tabBar.setCurrentPage( i );
		
		if( !pvb->queryClose() )
			return false;
	}
	return true;
}

bool KrViewer::queryExit() {
	return true; // don't let the reference counter reach zero
}

void KrViewer::viewGeneric(){
	PanelViewerBase* pvb = static_cast<PanelViewerBase*>( tabBar.currentPage() );
	if( !pvb ) return;

	PanelViewerBase* viewerWidget = new PanelViewer(&tabBar);
	KParts::Part* part = viewerWidget->openURL(pvb->url(),Generic);
	addTab(viewerWidget,i18n("Viewing"),VIEW_ICON,part);
}

void KrViewer::viewText(){
	PanelViewerBase* pvb = static_cast<PanelViewerBase*>( tabBar.currentPage() );
	if( !pvb ) return;

	PanelViewerBase* viewerWidget = new PanelViewer(&tabBar);
	KParts::Part* part = viewerWidget->openURL(pvb->url(),Text);
	addTab(viewerWidget,i18n("Viewing"),VIEW_ICON,part);
}

void KrViewer::viewHex(){
	PanelViewerBase* pvb = static_cast<PanelViewerBase*>( tabBar.currentPage() );
	if( !pvb ) return;

	PanelViewerBase* viewerWidget = new PanelViewer(&tabBar);
	KParts::Part* part = viewerWidget->openURL(pvb->url(),Hex);
	addTab(viewerWidget,i18n("Viewing"),VIEW_ICON,part);
}

void KrViewer::editText(){
	PanelViewerBase* pvb = static_cast<PanelViewerBase*>( tabBar.currentPage() );
	if( !pvb ) return;

	PanelViewerBase* editWidget = new PanelEditor(&tabBar);
	KParts::Part* part = editWidget->openURL(pvb->url(),Text);
	addTab(editWidget,i18n("Editing"),EDIT_ICON,part);
}

void KrViewer::checkModified(){
	QTimer::singleShot( 1000, this, SLOT(checkModified()) );

	PanelViewerBase* pvb = static_cast<PanelViewerBase*>( tabBar.currentPage() );
	if( !pvb ) return;

	if( !pvb->part()->url().equals( pvb->url(), true ) ) {
		pvb->setUrl( pvb->part()->url() );
	}

	// add a * to modified files.
	if( pvb->isModified() ){
		QString label = tabBar.tabLabel(pvb);
		if( !label.startsWith("*" + pvb->part()->url().fileName() ) ){
			label.prepend("*");
			QIconSet icon = QIconSet(krLoader->loadIcon(MODIFIED_ICON,KIcon::Small));

			tabBar.changeTab(pvb,icon,label);
		}
	}
	// remove the * from previously modified files.
	else {
		QString label = tabBar.tabLabel(pvb);
		if( label.startsWith("*" + pvb->part()->url().fileName() ) ){
			label = label.mid( 1 );
			QIconSet icon = QIconSet(krLoader->loadIcon(EDIT_ICON,KIcon::Small));

			tabBar.changeTab(pvb,icon,label);
		}		
	}
}

void KrViewer::nextTab(){
	int index = (tabBar.currentPageIndex()+1)%tabBar.count();
	tabBar.setCurrentPage( index );
}

void KrViewer::prevTab(){
	int index = (tabBar.currentPageIndex()-1)%tabBar.count();
	while( index < 0 ) index+=tabBar.count();
	tabBar.setCurrentPage( index );
}

void KrViewer::detachTab(){
	PanelViewerBase* pvb = static_cast<PanelViewerBase*>( tabBar.currentPage() );
	if( !pvb ) return;

	KrViewer* viewer = getViewer(true);

	manager.removePart(pvb->part());
	tabBar.removePage(pvb);
	
	if( tabBar.count() == 1 ) {
		//no point in detaching only one tab..
		viewerMenu->setItemEnabled(detachActionIndex,false);
	}
	
	pvb->reparent(&viewer->tabBar,QPoint(0,0));

	if( pvb->isEditor() )
		viewer->addTab(pvb,i18n( "Editing" ),EDIT_ICON,pvb->part());
	else
		viewer->addTab(pvb,i18n( "Viewing" ),VIEW_ICON,pvb->part());
}

void KrViewer::windowActivationChange ( bool /* oldActive */ ) {
	if( isActiveWindow() )
		if( viewers.remove( this ) ) viewers.prepend( this ); // move to first
}

void KrViewer::print() {
	PanelViewerBase* pvb = static_cast<PanelViewerBase*>( tabBar.currentPage() );
	if( !pvb ) return;
	
	KParts::BrowserExtension * ext = KParts::BrowserExtension::childObject( pvb->part() );
	if( ext && ext->isActionEnabled( "print" ) )
		Invoker( ext, SLOT( print() ) ).invoke();
}

void KrViewer::copy() {
	PanelViewerBase* pvb = static_cast<PanelViewerBase*>( tabBar.currentPage() );
	if( !pvb ) return;
	
	KParts::BrowserExtension * ext = KParts::BrowserExtension::childObject( pvb->part() );
	if( ext && ext->isActionEnabled( "copy" ) )
		Invoker( ext, SLOT( copy() ) ).invoke();
}

PanelViewerBase * KrViewer::getPanelViewerBase( KParts::Part * part ) {
	for( int i=0; i != tabBar.count(); i++ ) {
		PanelViewerBase *pvb = static_cast<PanelViewerBase*>( tabBar.page( i ) );
		if( pvb && pvb->part() == part )
			return pvb;
	}
	return 0;
}

void KrViewer::updateActions( PanelViewerBase * pvb ) {
	if( pvb->isEditor() ) {
		printAction->unplugAll();
		copyAction->unplugAll();
	}
	else {
		if( !printAction->isPlugged( toolBar() ) )
			printAction->plug( toolBar(), 0 );
		if( !copyAction->isPlugged( toolBar() ) )
			copyAction->plug( toolBar(), 1 );
	}
}

#if 0
bool KrViewer::editGeneric( QString mimetype, KURL _url ) {
	KParts::ReadWritePart * kedit_part = 0L;
	KLibFactory *factory = 0;
	KTrader::OfferList offers = KTrader::self() ->query( mimetype );

	// in theory, we only care about the first one.. but let's try all
	// offers just in case the first can't be loaded for some reason
	KTrader::OfferList::Iterator it( offers.begin() );
	for ( ; it != offers.end(); ++it ) {
		KService::Ptr ptr = ( *it );
		// we now know that our offer can handle mimetype and is a part.
		// since it is a part, it must also have a library... let's try to
		// load that now
		factory = KLibLoader::self() ->factory( ptr->library().latin1() );
		if ( factory ) {
			kedit_part = static_cast<KParts::ReadWritePart *>( factory->create( this,
			             ptr->name().latin1(), "KParts::ReadWritePart" ) );
			if ( kedit_part )
				if ( kedit_part->openURL( _url ) ) break;
				else {
					delete kedit_part;
					kedit_part = 0L;
				}
		}
	}

	if ( !kedit_part ) {
		KMessageBox::error( this, i18n( "Sorry, can't find internal editor" ) );
		return false;
	}

	setCentralWidget( kedit_part->widget() );
	createGUI( kedit_part );
	kedit_part->widget() ->show();
	return true;
}

bool KrViewer::editText( bool create ) {
	if ( !editor_part ) {
		editor_part = static_cast<KParts::ReadWritePart*>( getPart( url, "text/plain", false, create ) );
		if ( !editor_part ) return false;
		manager.addPart( editor_part, this );
	}
	manager.setActivePart( editor_part );
	tabBar.addTab(editor_part->widget(),url.fileName());
	return true;
}

bool KrViewer::viewGeneric() {
	QString mimetype = KMimeType::findByURL( url ) ->name();
	// ugly hack: don't try to get a part for an XML file, it usually don't work
	if ( mimetype == "text/xml" ) return false;
	if ( url.prettyURL().startsWith( "man:" ) ) mimetype = "text/html";
	if ( mimetype == "text/plain" )
		viewerMenu->setItemEnabled( 1, false );

	if ( !generic_part ) {
		if ( mimetype.contains( "html" ) ) {
			KHTMLPart * p = new KHTMLPart( this, 0, 0, 0, KHTMLPart::BrowserViewGUI );
			connect( p->browserExtension(), SIGNAL( openURLRequest( const KURL &, const KParts::URLArgs & ) ),
			         this, SLOT( handleOpenURLRequest( const KURL &, const KParts::URLArgs & ) ) );
			/* At JavaScript self.close() the KHTMLPart destroys itself.  */
			/* After destruction, just close the window */
			connect( p, SIGNAL( destroyed() ), this, SLOT( close() ) );

			p-> openURL( url );
			generic_part = p;
		} else {
			generic_part = static_cast<KParts::ReadOnlyPart*>( getPart( url, mimetype, true ) );
		}
		if ( generic_part ) manager.addPart( generic_part, this );
		
		else return false;
	}

	manager.setActivePart( generic_part );
	tabBar.addTab(generic_part->widget(),url.fileName());
	return true;
}

bool KrViewer::viewText() {
	if ( !text_part ) {
		text_part = static_cast<KParts::ReadOnlyPart*>( getPart( url, "text/plain", true ) );
		if ( !text_part ) return false;
		manager.addPart( text_part, this );
	}
	manager.setActivePart( text_part );
	tabBar.addTab(text_part->widget(),url.fileName());
	return true;
}

void KrViewer::viewHex() {
	if ( !hex_part ) {
		QString file;
		// files that are not local must first be downloaded
		if ( !url.isLocalFile() ) {
			if ( !KIO::NetAccess::download( url, file ) ) {
				KMessageBox::sorry( this, i18n( "KrViewer is unable to download: " ) + url.url() );
				return ;
			}
		} else file = url.path();


		// create a hex file
		QFile f_in( file );
		f_in.open( IO_ReadOnly );
		QDataStream in( &f_in );

		FILE *out = KDE_fopen( tmpFile.name().local8Bit(), "w" );

		KIO::filesize_t fileSize = f_in.size();
		KIO::filesize_t address = 0;
		char buf[ 16 ];
		unsigned int* pBuff = ( unsigned int* ) buf;

		while ( address < fileSize ) {
			memset( buf, 0, 16 );
			int bufSize = ( ( fileSize - address ) > 16 ) ? 16 : ( fileSize - address );
			in.readRawBytes( buf, bufSize );
			fprintf( out, "0x%8.8llx: ", address );
			for ( int i = 0; i < 4; ++i ) {
				if ( i < ( bufSize / 4 ) ) fprintf( out, "%8.8x ", pBuff[ i ] );
				else fprintf( out, "         " );
			}
			fprintf( out, "| " );

			for ( int i = 0; i < bufSize; ++i ) {
				if ( buf[ i ] > ' ' && buf[ i ] < '~' ) fputc( buf[ i ], out );
				else fputc( '.', out );
			}
			fputc( '\n', out );

			address += 16;
		}
		// clean up
		f_in.close();
		fclose( out );
		if ( !url.isLocalFile() )
			KIO::NetAccess::removeTempFile( file );

		hex_part = static_cast<KParts::ReadOnlyPart*>( getPart( tmpFile.name(), "text/plain", true ) );
		if ( !hex_part ) return ;
		manager.addPart( hex_part, this );
	}
	manager.setActivePart( hex_part );
	tabBar.addTab(hex_part->widget(),url.fileName());
}
#endif


#include "krviewer.moc"
