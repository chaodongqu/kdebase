#include <qwidgetstack.h>
#include <qtoolbutton.h>
#include <klocale.h>
#include <qimage.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kiconloader.h>
#include "panelmanager.h"
#include "Panel/listpanel.h"
#include "Panel/panelfunc.h"
#include "krusaderview.h"
#include "defaults.h"

#define HIDE_ON_SINGLE_TAB  false

#define SHOW    { _newTab->show(); _tabbar->show(); _closeTab->show(); }
#define HIDE    { _newTab->hide(); _tabbar->hide(); _closeTab->hide(); }

#define _self   (*_selfPtr)
#define _other  (*_otherPtr)

PanelManager::PanelManager( QWidget *parent, bool left ) :
QWidget( parent, "PanelManager" ), _layout( 0 ), _left( left ), 
       _selfPtr( _left ? &MAIN_VIEW->left : &MAIN_VIEW->right ), 
       _otherPtr( _left ? &MAIN_VIEW->right : &MAIN_VIEW->left ) {   
   _layout = new QGridLayout( this, 1, 1 );
   _stack = new QWidgetStack( this );

   // new tab button
   _newTab = new QToolButton( this );
   _newTab->setFixedSize( 22, 22 );
   _newTab->setTextLabel( i18n( "Open a new tab in home" ) );
   _newTab->setIconSet( SmallIcon( "tab_new" ) );
   _newTab->adjustSize();	
   connect( _newTab, SIGNAL( clicked() ), this, SLOT( slotNewTab() ) );

   // close tab button
   _closeTab = new QToolButton( this );
   _closeTab->setFixedSize( 22, 22 );
   _closeTab->setTextLabel( i18n( "Close current tab" ) );
   _closeTab->setIconSet( SmallIcon( "tab_remove" ) );
   _closeTab->adjustSize();   
   connect( _closeTab, SIGNAL( clicked() ), this, SLOT( slotCloseTab() ) );
   _closeTab->setEnabled( false ); // disabled when there's only 1 tab

   // tab-bar
   _tabbar = new PanelTabBar( this );
   connect( _tabbar, SIGNAL( changePanel( ListPanel* ) ), this, SLOT( slotChangePanel( ListPanel * ) ) );
   connect( _tabbar, SIGNAL( closeCurrentTab() ), this, SLOT( slotCloseTab() ) );
   connect( _tabbar, SIGNAL( newTab( const KURL& ) ), this, SLOT( slotNewTab( const KURL& ) ) );

   _layout->addMultiCellWidget( _stack, 0, 0, 0, 2 );
   _layout->addWidget( _newTab, 1, 0 );
   _layout->addWidget( _tabbar, 1, 1 );
   _layout->addWidget( _closeTab, 1, 2 );

   if ( HIDE_ON_SINGLE_TAB ) HIDE
      else SHOW
}

void PanelManager::slotChangePanel( ListPanel *p ) {
   _self = p;
   _self->otherPanel = _other;
   _other->otherPanel = _self;

   _stack->raiseWidget( _self );
   kapp->processEvents();
   _self->slotFocusOnMe();
}

ListPanel* PanelManager::createPanel( QString type, bool setCurrent ) {
   // create the panel and add it into the widgetstack
   ListPanel * p = new ListPanel( type, _stack, _left );
   _stack->addWidget( p );

   // now, create the corrosponding tab
   _tabbar->addPanel( p, setCurrent );

   // allow close button if more than 1 tab
   if ( _tabbar->count() > 1 ) {
      _closeTab->setEnabled( true );
      SHOW // needed if we were hidden
   }
   if( setCurrent )
     _stack->raiseWidget( p );

   // connect the activePanelChanged signal to enable/disable actions
   connect( p, SIGNAL( activePanelChanged( ListPanel* ) ), this, SLOT( slotRefreshActions() ) );
   return p;
}

void PanelManager::startPanel( ListPanel *panel, const KURL& path ) {
   panel->start( path );
}

void PanelManager::saveSettings( KConfig *config, const QString& key, bool localOnly ) {
   QStringList l;
   QStringList types;
   QValueList<int> props;
   int i=0, cnt=0;
   while (cnt < _tabbar->count()) {
      PanelTab *t = dynamic_cast<PanelTab*>(_tabbar->tabAt(i));
      if (t && t->panel) {
         l << ( localOnly ? t->panel->realPath() : vfs::pathOrURL( t->panel->virtualPath() ) );
         types << t->panel->getType();
         props << t->panel->getProperties();
         ++cnt;
      }
      ++i;
   }
   config->writePathEntry( key, l );
   config->writeEntry( key + " Types", types );
   config->writeEntry( key + " Props", props );
}

void PanelManager::loadSettings( KConfig *config, const QString& key ) {
   QStringList l = config->readPathListEntry( key );
   QStringList types = config->readListEntry( key + " Types" );
   QValueList<int> props = config->readIntListEntry( key + " Props" );
   
   if( l.count() < 1 )
     return;
     
   while( types.count() < l.count() )
   {
      KConfigGroupSaver saver( config, "Look&Feel");
      types << krConfig->readEntry( "Default Panel Type", _DefaultPanelType );
   }
   while( props.count() < l.count() )
      props << 0;
   
   int i=0, totalTabs = _tabbar->count();
   
   while (i < totalTabs && i < (int)l.count() ) 
   {
      PanelTab *t = dynamic_cast<PanelTab*>(_tabbar->tabAt(i));
      if (t && t->panel) 
      {
         if( t->panel->getType() != types[ i ] )
           t->panel->changeType( types[ i ] );
         t->panel->setProperties( props[ i ] );
         t->panel->otherPanel = _other;
         _other->otherPanel = t->panel;
         t->panel->func->files()->vfs_enableRefresh( true );
         t->panel->func->immediateOpenUrl( vfs::fromPathOrURL( l[ i ] ) );
      }
      ++i;
   }
   
   while( i <  totalTabs )
     slotCloseTab( --totalTabs );
      
   for(; i < (int)l.count(); i++ )
     slotNewTab( vfs::fromPathOrURL(l[i]), false, types[ i ], props[ i ] );
}

void PanelManager::slotNewTab(const KURL& url, bool setCurrent, QString type, int props ) {
   if( type.isNull() )
   {
       krConfig->setGroup( "Look&Feel" );
       type = krConfig->readEntry( "Default Panel Type", _DefaultPanelType );
   }
   ListPanel *p = createPanel( type, setCurrent );
   // update left/right pointers
   p->otherPanel = _other;
   if( setCurrent )
   {
     _self = p;
     _other->otherPanel = _self;
   }
   startPanel( p, url );
   p->setProperties( props );
}

void PanelManager::slotNewTab() {
   slotNewTab( QDir::home().absPath() );
}

void PanelManager::slotCloseTab() {
   if ( _tabbar->count() <= 1 )  /* if this is the last tab don't close it */
      return ;

   // setup current one
   ListPanel * oldp;
   _self = _tabbar->removeCurrentPanel( oldp );
   _stack->raiseWidget( _self );
   _stack->removeWidget( oldp );
   deletePanel( oldp );

   // setup pointers
   _self->otherPanel = _other;
   _other->otherPanel = _self;
   _self->slotFocusOnMe();

   // disable close button if only 1 tab is left
   if ( _tabbar->count() == 1 ) {
      _closeTab->setEnabled( false );
      if ( HIDE_ON_SINGLE_TAB ) HIDE
      }
}

void PanelManager::slotCloseTab( int index ) {
   PanelTab *t = dynamic_cast<PanelTab*>(_tabbar->tabAt( index ));
   if (t && t->panel) 
   {
      ListPanel *oldp = t->panel;
      disconnect( oldp );
      if( t->identifier() == _tabbar->currentTab() )
      {
         PanelTab *newCurrent = dynamic_cast<PanelTab*>( _tabbar->tabAt( 0 ) );
         if( newCurrent != 0 )
         {
            _tabbar->setCurrentTab( newCurrent );
            _self = newCurrent->panel;
            _self->otherPanel = _other;
            _other->otherPanel = _self;
         }
      }  
      _tabbar->removeTab( t );

      _stack->removeWidget( oldp );
      deletePanel( oldp );
   }

   if ( _tabbar->count() == 1 ) 
   {
      _closeTab->setEnabled( false );
      if ( HIDE_ON_SINGLE_TAB ) HIDE
   }
}

void PanelManager::slotRefreshActions() {
   krCloseTab->setEnabled( _tabbar->count() > 1 );
   krNextTab->setEnabled(_tabbar->count() > 1);
   krPreviousTab->setEnabled(_tabbar->count() > 1);	
}

int PanelManager::activeTab()
{
  return _tabbar->indexOf( _tabbar->currentTab() );
}

void PanelManager::setActiveTab( int panelIndex )
{
  QTab *current = _tabbar->tabAt( panelIndex );
  if( current == 0 )
    return;  
  _tabbar->setCurrentTab( current );
  slotChangePanel ( dynamic_cast<PanelTab*>( _tabbar->tabAt( panelIndex ) )->panel );
}

void PanelManager::setCurrentTab( int panelIndex )
{
  PanelTab *current = dynamic_cast<PanelTab*>(_tabbar->tabAt( panelIndex ) );
  if( current == 0 )
    return;  
  _tabbar->setCurrentTab( current );
  _self = current->panel;
  _self->otherPanel = _other;
  _other->otherPanel = _self;

  _stack->raiseWidget( _self );
}

void PanelManager::slotRecreatePanels() {
   int actTab = activeTab();
   
   for( int i = 0; i != _tabbar->count(); i++ )
   {
     PanelTab *updatedPanel = dynamic_cast<PanelTab*>(_tabbar->tabAt( i ) );
     
     ListPanel *oldPanel = updatedPanel->panel;
     QString type = oldPanel->getType();
     ListPanel *newPanel = new ListPanel( type, _stack, _left );
     _stack->addWidget( newPanel, i );
     _stack->removeWidget( oldPanel );

     disconnect( oldPanel );
     connect( newPanel, SIGNAL( activePanelChanged( ListPanel* ) ), this, SLOT( slotRefreshActions() ) );
     connect( newPanel, SIGNAL( pathChanged(ListPanel*) ), _tabbar, SLOT(updateTab(ListPanel*)));

     newPanel->otherPanel = _other;
     if( _other->otherPanel == oldPanel )
       _other->otherPanel = newPanel;         
     updatedPanel->panel = newPanel;
     newPanel->start( oldPanel->virtualPath(), true );          
     if( _self == oldPanel )
     {
        _self = newPanel;
        _self->otherPanel = _other;
        _other->otherPanel = _self;
     }
     deletePanel( oldPanel );
   
     _tabbar->updateTab( newPanel );
   }
   
   setActiveTab( actTab );
}

void PanelManager::slotNextTab() {
   int currTab = _tabbar->currentTab();
	int nextInd = (_tabbar->indexOf(currTab) == _tabbar->count()-1 ? 0 : _tabbar->indexOf(currTab)+1);
	ListPanel *nextp = dynamic_cast<PanelTab*>(_tabbar->tabAt(nextInd))->panel;
	_tabbar->setCurrentTab(_tabbar->tabAt(nextInd));	
	slotChangePanel(nextp);   
}


void PanelManager::slotPreviousTab() {
   int currTab = _tabbar->currentTab();
	int nextInd = (_tabbar->indexOf(currTab) == 0 ? _tabbar->count()-1 : _tabbar->indexOf(currTab)-1);
	ListPanel *nextp = dynamic_cast<PanelTab*>(_tabbar->tabAt(nextInd))->panel;
	_tabbar->setCurrentTab(_tabbar->tabAt(nextInd));	
	slotChangePanel(nextp);   
}

void PanelManager::refreshAllTabs( bool invalidate ) {
   int i=0;
   while (i < _tabbar->count()) {
      PanelTab *t = dynamic_cast<PanelTab*>(_tabbar->tabAt(i));
      if (t && t->panel && t->panel->func ) {
         vfs * vfs = t->panel->func->files();
         if( vfs ) {
            if( invalidate )
               vfs->vfs_invalidate();
            vfs->vfs_refresh();
         }
      }
      ++i;
   }
}

void PanelManager::deletePanel( ListPanel * p ) {
  if( ACTIVE_PANEL == p )
     ACTIVE_PANEL = _self;

  if( p && p->func && p->func->files() && !p->func->files()->vfs_canDelete() ) {
    connect( p->func->files(), SIGNAL( deleteAllowed() ), p, SLOT( deleteLater() ) );
    p->func->files()->vfs_requestDelete();
    return;
  }
  delete p;
}

void PanelManager::swapPanels() {
   _left = !_left;
   _selfPtr = _left ? &MAIN_VIEW->left : &MAIN_VIEW->right;
   _otherPtr = _left ? &MAIN_VIEW->right : &MAIN_VIEW->left;
}

#include "panelmanager.moc"
