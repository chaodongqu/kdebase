/**************************************************************************

    kpager.cpp  - KPager's main window
    Copyright (C) 2000  Antonio Larrosa Jimenez
			Matthias Ettrich
			Matthias Elter

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    Send comments and bug fixes to larrosa@kde.org

***************************************************************************/

/*
 * There is a features that is only configurable by manually editing the
 * config file due to the translation freeze . The key is
 * windowTransparentMode and the values are :
 *    0 = Never
 *    1 = Only maximized windows are painted transparent
 *    2 = Every window is painted transparent (default)
 *
 */

#include "config.h"
#include "kpager.h"

#include <qintdict.h>
#include <qptrlist.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kglobalsettings.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kwin.h>
#include <kwinmodule.h>
#include <netwm.h>
#include "desktop.h"
#include <qpopupmenu.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <assert.h>

KPagerMainWindow::KPagerMainWindow(QWidget *parent, const char *name)
	: DCOPObject("KPagerIface"), KMainWindow(parent, name)
{
    m_reallyClose=false;

    m_pPager = new KPager(this, 0);
    setCentralWidget(m_pPager);

    KConfig *cfg = kapp->config();
    cfg->setGroup("KPager");

    // Update the last used geometry
    int w = cfg->readNumEntry(m_pPager->lWidth(),-1);
    int h = cfg->readNumEntry(m_pPager->lHeight(),-1);
    if (w > 0 && h > 0)
        resize(w,h);
    else
        resize(m_pPager->sizeHint());
    //  resize(cfg->readNumEntry(lWidth(),200),cfg->readNumEntry(lHeight(),90));

    int xpos=cfg->readNumEntry("xPos",-1);
    int ypos=cfg->readNumEntry("yPos",-1);
    if (xpos > 0 && ypos > 0)
      move(xpos,ypos);
    else
    {
//      NETRootInfo ri( qt_xdisplay(), NET::WorkArea );
//      NETRect rect=ri.workArea(1);
//      move(rect.pos.x+rect.size.width-m_pPager->width(),
//	  rect.pos.y+rect.size.height-m_pPager->height());
// antonio:The above lines don't work. I should look at them when I have
// more time
        move(kapp->desktop()->width()-m_pPager->sizeHint().width()-5,kapp->desktop()->height()-m_pPager->sizeHint().height()-25);
    }

    // Set the wm flags to this window
    KWin::setState( winId(), NET::StaysOnTop | NET::SkipTaskbar | NET::Sticky | NET::SkipPager );
    KWin::setOnAllDesktops( winId(), true);
    if ( KWin::windowInfo( winId(), NET::WMWindowType, 0 ).windowType(NET::Normal) == NET::Normal )
    {
       KWin::setType( winId(), NET::Utility );
    }

    XWMHints *hints = XGetWMHints(x11Display(), winId());
    if( hints == NULL )
        hints = XAllocWMHints();
    hints->input = false;
    hints->flags |= InputHint;
    XSetWMHints(x11Display(), winId(), hints);
    XFree(reinterpret_cast<char *>(hints));

    timeout=new QTimer(this,"timeoutToQuit");
    connect(timeout,SIGNAL(timeout()),this, SLOT(reallyClose()));
}

KPagerMainWindow::~KPagerMainWindow()
{
}

extern bool closed_by_sm;

bool KPagerMainWindow::queryClose()
{
    KConfig *cfg=KGlobal::config();

    cfg->setGroup("KPager");
    cfg->writeEntry("layoutType", static_cast<int>(m_pPager->m_layoutType));
    cfg->writeEntry(m_pPager->lWidth(),width());
    cfg->writeEntry(m_pPager->lHeight(),height());
    cfg->writeEntry("xPos",x());
    cfg->writeEntry("yPos",y());
    cfg->sync();

	kdDebug() << "queryCLose " << m_reallyClose << " " << closed_by_sm << endl;
    if (m_reallyClose || closed_by_sm) return true;

    hide();
    timeout->start(10 /*minutes*/ *60000, true);

    return false;
}

void KPagerMainWindow::showEvent( QShowEvent *ev )
{
  timeout->stop();
  KMainWindow::showEvent(ev); 
}

void KPagerMainWindow::reallyClose()
{
  m_reallyClose=true;
  close();
}

void KPagerMainWindow::showAt(int x, int y)
{
  // Just in case we lost the sticky bit... (as when a window is hidden)
  KWin::setOnAllDesktops( winId(), true);
  
  if (x>kapp->desktop()->width()/2) // Right
    x-=m_pPager->width()+5;
  if (y>kapp->desktop()->height()/2) // Bottom
    y-=m_pPager->height()+25;
  move(x,y);
  show();
}

void KPagerMainWindow::toggleShow(int x, int y)
{
  if (isVisible())
  {
    hide();
    timeout->start(10 /*minutes*/ *60000, true);
  }
  else 
    showAt(x,y);
}

KPager::KPager(KPagerMainWindow *parent, const char *name)
	: QFrame (parent, name, WStyle_Customize | WStyle_NoBorder | WStyle_Tool)
    , m_layout(0)
    , m_mnu(0)
    , m_smnu(0)
    , m_dmnu(0)
{
    m_windows.setAutoDelete(true); // delete windows info after removal

    setBackgroundColor( black );
    m_winmodule=new KWinModule(this);
    m_currentDesktop=m_winmodule->currentDesktop();

    m_grabWinTimer=new QTimer(this,"grabWinTimer");
    connect(m_grabWinTimer, SIGNAL(timeout()), this, SLOT(slotGrabWindows()));

    KPagerConfigDialog::initConfiguration();

    KConfig *cfg = kapp->config();
    cfg->setGroup("KPager");
    m_showStickyOption=cfg->readBoolEntry("ShowStickyOption",false);

    int numberOfDesktops=m_winmodule->numberOfDesktops();
    for (int i=0;i<numberOfDesktops;i++)
    {
        Desktop *dsk=new Desktop(i+1,m_winmodule->desktopName(i),this);
        m_desktops.append(dsk);
    }

    m_layoutType=static_cast<enum KPager::LayoutTypes>( KPagerConfigDialog::m_layoutType );

    connect( m_winmodule, SIGNAL( activeWindowChanged(WId)),
             SLOT(slotActiveWindowChanged(WId)));
    connect( m_winmodule, SIGNAL( windowAdded(WId) ),
             SLOT( slotWindowAdded(WId) ) );
    connect( m_winmodule, SIGNAL( windowRemoved(WId) ),
             SLOT( slotWindowRemoved(WId) ) );
    connect( m_winmodule, SIGNAL( windowChanged(WId,unsigned int) ),
             SLOT( slotWindowChanged(WId,unsigned int) ) );
    connect( m_winmodule, SIGNAL( stackingOrderChanged() ),
             SLOT( slotStackingOrderChanged() ) );
    connect( m_winmodule, SIGNAL( desktopNamesChanged() ),
             SLOT( slotDesktopNamesChanged() ) );
    connect( m_winmodule, SIGNAL( numberOfDesktopsChanged(int) ),
             SLOT( slotNumberOfDesktopsChanged(int) ) );
    connect( m_winmodule, SIGNAL( currentDesktopChanged(int)),
             SLOT( slotCurrentDesktopChanged(int) ) );
    connect(kapp, SIGNAL(backgroundChanged(int)),
            SLOT(slotBackgroundChanged(int)));

    QFont defFont(KGlobalSettings::generalFont().family(), 10, QFont::Bold);
    defFont = cfg->readFontEntry("Font", &defFont);
    setFont(defFont);

    m_prefs_action = KStdAction::preferences(this, SLOT(configureDialog()), parent->actionCollection());
    m_quit_action = KStdAction::quit(kapp, SLOT(quit()), parent->actionCollection());

    updateLayout();
}

KPager::~KPager()
{

}

const QString KPager::lWidth()
{
    switch (m_layoutType) {
	case (Classical) :  return "layoutClassicalWidth";break;
	case (Horizontal) : return "layoutHorizontalWidth";break;
	case (Vertical) :   return "layoutVerticalWidth";break;
    };
    return "Width";
}

const QString KPager::lHeight()
{
    switch (m_layoutType) {
	case (Classical) :  return "layoutClassicalHeight";break;
	case (Horizontal) : return "layoutHorizontalHeight";break;
	case (Vertical) :   return "layoutVerticalHeight";break;
    };
    return "Height";
}

void KPager::updateLayout()
{
    int w=m_desktops[0]->width();
    int h=m_desktops[0]->height();

    delete m_layout;

    switch (m_layoutType)
    {
        case (Classical) :  m_layout=new QGridLayout(this, 2, 0); break;
        case (Horizontal) : m_layout=new QGridLayout(this, 0, 1); break;
        case (Vertical) :   m_layout=new QGridLayout(this, 1, 0); break;
    };

    QValueList <Desktop *>::Iterator it;
    int i,j;
    i=j=0;
    int ndesks=0;
    int halfdesks = (m_desktops.count() + 1) / 2;
    for( it = m_desktops.begin(); it != m_desktops.end(); ++it )
    {
        m_layout->addWidget(*it,i,j);
        ndesks++;
        switch (m_layoutType)
        {
            case (Classical) :  i= ndesks / halfdesks; j = ndesks % halfdesks; break;
            case (Horizontal) : j++; break;
            case (Vertical) :   i++; break;
        };
    }

    m_layout->activate();
    updateGeometry();

    switch (m_layoutType)
    {
        case (Classical) :  resize(w*(ndesks/2+(ndesks%2)),h*2);break;
        case (Horizontal) : resize(w*ndesks,h);break;
        case (Vertical) :   resize(w, h*ndesks);break;
    };

}

void KPager::showPopupMenu( WId wid, QPoint pos)
{
    if (wid <= 0) {
	if(!m_smnu) {
	    m_smnu = new QPopupMenu(this);
	    m_prefs_action->plug(m_smnu);
	    m_quit_action->plug(m_smnu);
	}
	m_smnu->popup(pos);
    }
    else {
        m_winfo = KWin::windowInfo(wid);

        if (!m_mnu) {
            m_mnu = new KPopupMenu(this);

            m_mnu->insertTitle( QString::fromUtf8("KPager"), 1);
            m_mnu->setCheckable(true);
            connect(m_mnu, SIGNAL(aboutToShow()), SLOT(clientPopupAboutToShow()));
            connect(m_mnu, SIGNAL(activated(int)), SLOT(clientPopupActivated(int)));

            m_dmnu = new QPopupMenu(m_mnu);
            m_dmnu->setCheckable(true);
            connect(m_dmnu, SIGNAL(aboutToShow()), SLOT(desktopPopupAboutToShow()));
            connect(m_dmnu, SIGNAL(activated(int)), SLOT(sendToDesktop(int)));

            m_mnu->insertItem( i18n("Mi&nimize"), IconifyOp );
            m_mnu->insertItem( i18n("Ma&ximize"), MaximizeOp );
            if (m_showStickyOption)
                m_mnu->insertItem( QString("&Sticky"), StickyOp );  // Add translation
            m_mnu->insertSeparator();

            m_mnu->insertItem(i18n("&To Desktop"), m_dmnu );
            m_mnu->insertSeparator();

            m_mnu->insertItem(SmallIcon("fileclose"),i18n("&Close"), CloseOp);

            m_mnu->insertSeparator();
	    m_prefs_action->plug(m_mnu);
	    m_quit_action->plug(m_mnu);
        }
        m_mnu->popup(pos);
    }
}

void KPager::configureDialog()
{
    KPagerConfigDialog *dialog= new KPagerConfigDialog(this);
    if (dialog->exec())
    {
        m_layoutType=static_cast<enum KPager::LayoutTypes>(KPagerConfigDialog::m_layoutType);
	KConfig *cfg=KGlobal::config();
	int nWd = (parent() ? ((QWidget *)parent())->width() : width());
	int nHg = (parent() ? ((QWidget *)parent())->width() : width());

	cfg->setGroup("KPager");

	cfg->writeEntry(lWidth(),nWd);
	cfg->writeEntry(lHeight(),nHg);
	cfg->writeEntry("windowDrawMode",KPagerConfigDialog::m_windowDrawMode);
	cfg->writeEntry("layoutType",KPagerConfigDialog::m_layoutType);
	cfg->writeEntry("showNumber",KPagerConfigDialog::m_showNumber);
	cfg->writeEntry("showName",KPagerConfigDialog::m_showName);
	cfg->writeEntry("showWindows",KPagerConfigDialog::m_showWindows);
	cfg->writeEntry("showBackground",KPagerConfigDialog::m_showBackground);
	cfg->writeEntry("windowDragging",KPagerConfigDialog::m_windowDragging);

        updateLayout();
        for( QValueList <Desktop *>::Iterator it = m_desktops.begin(); it != m_desktops.end(); ++it )
            (*it)->repaint();
    }
}

KWin::WindowInfo* KPager::info( WId win )
{
    KWin::WindowInfo* info = m_windows[win];
    if (!info )
    {
        info = new KWin::WindowInfo( KWin::windowInfo( win ) );
        if( !info->valid() || info->desktop() == 0 )
        {
            delete info;
            return NULL; // window no longer valid
        }
        m_windows.insert( (long) win, info );
    }
    return info;
}

void KPager::slotActiveWindowChanged( WId win )
{
    KWin::WindowInfo* inf1 = info( m_activeWin );
    KWin::WindowInfo* inf2 = info( win );
    m_activeWin = win;

    // update window pixmap
    // in case of active desktop change it will be updated anyway by timer
//    if (!m_grabWinTimer->isActive())
//        Desktop::removeCachedPixmap(win);

    for ( int i=1; i <= (int) m_desktops.count(); ++i)
    {
        if ( (inf1 && inf1->isOnDesktop(i))
             || (inf2 && inf2->isOnDesktop(i) ) )
            m_desktops[i-1]->repaint(false);
    }
}

void KPager::slotWindowAdded( WId win)
{
    KWin::WindowInfo* inf = info( win );
    if (!inf)
        return; // never should be here

    for ( int i=1; i <= (int) m_desktops.count(); ++i)
    {
        if ( inf->isOnDesktop( i ))
            m_desktops[i-1]->repaint(false);
    }
}

void KPager::slotWindowRemoved( WId win )
{
    KWin::WindowInfo* inf = m_windows[win];
    if (inf)
    {
        bool onAllDesktops = inf->onAllDesktops();
        int desktop = inf->desktop();
        m_windows.remove( (long)win );
        Desktop::removeCachedPixmap(win);
        for (int i = 1; i <= (int) m_desktops.count(); ++i)
        {
            if (onAllDesktops || desktop == i)
                m_desktops[i-1]->repaint(false);
        }
    }
}

void KPager::slotWindowChanged( WId win , unsigned int prop)
{
    bool repaint=false;

    KWin::WindowInfo* inf = m_windows[win];
    if (!inf)
    {
      inf=info(win);
      prop=0; // info already calls KWin::info, so there's no need
      // to update anything else.
      repaint=true;
    };

    bool onAllDesktops = inf ? inf->onAllDesktops() : false;
    int desktop = inf ? inf->desktop() : 0;

    if (prop)
    {
      m_windows.remove( (long) win );
      inf = info( win );
    }
    
    if((prop & ~( NET::WMName | NET::WMVisibleName )) != 0 )
	repaint = true;

    if (repaint)
    for ( int i=1; i <= (int) m_desktops.count(); ++i)
    {
      if ((inf && (inf->isOnDesktop(i)))
	  || onAllDesktops || desktop == i )
        {
            m_desktops[i-1]->repaint(false);
        }
    }
//	redrawDesktops();
}

void KPager::slotStackingOrderChanged()
{
    m_desktops[m_currentDesktop-1]->m_grabWindows=true;
    for ( int i=1; i <= (int) m_desktops.count(); ++i)
    {
        m_desktops[i-1]->repaint(false);
    }
//    repaint(true);
}

void KPager::slotDesktopNamesChanged()
{
    for ( int i=1; i <= (int) m_desktops.count(); ++i)
    {
        QToolTip::remove(m_desktops[i-1]);
        QToolTip::add(m_desktops[i-1], kwin()->desktopName(i));
    }

    update();
    emit updateLayout();
}

void KPager::slotNumberOfDesktopsChanged(int ndesktops)
{
    unsigned int nDesktops=static_cast<unsigned int>(ndesktops);
    if (nDesktops<m_desktops.count())
    {
        QValueList <Desktop *>::Iterator it;
        for ( int i=m_desktops.count()-nDesktops; i > 0; i--)
        {
            it = m_desktops.fromLast();
            delete (*it);
            m_desktops.remove(it);
        }

        emit updateLayout();
    }
    else if (nDesktops>m_desktops.count())
    {
        int i,j;
        i=j=m_desktops.count();
        switch (m_layoutType)
        {
            case (Classical) :  i%=2;j/=2; break;
            case (Horizontal) : i=0; break;
            case (Vertical) :   j=0; break;
        }

        for (unsigned int d=m_desktops.count()+1;d<=nDesktops; d++)
        {
            Desktop *dsk=new Desktop(d,kwin()->desktopName(d-1),this);
            m_desktops.append(dsk);
            dsk->show();
        }

        emit updateLayout();
    }
}


void KPager::slotCurrentDesktopChanged(int desk)
{
    if (m_currentDesktop==desk) return;
    m_desktops[m_currentDesktop-1]->paintFrame( false );
    m_desktops[m_currentDesktop-1]->update();
    m_desktops[desk-1]->paintFrame( true );
    m_desktops[desk-1]->update();
//    m_desktops[m_currentDesktop-1]->repaint();
//    m_desktops[desk-1]->repaint();

    m_currentDesktop=desk;

    if (m_grabWinTimer->isActive()) m_grabWinTimer->stop();

    if ( static_cast<Desktop::WindowDrawMode>( KPagerConfigDialog::m_windowDrawMode ) == Desktop::Pixmap )
        m_grabWinTimer->start(1000,true);
}

void KPager::slotBackgroundChanged(int desk)
{
    m_desktops[desk-1]->loadBgPixmap();
}

void KPager::sendToDesktop(int desk)
{
    if (desk == 0)
	KWin::setOnAllDesktops(m_winfo.win(), true);
    else {
	KWin::setOnDesktop(m_winfo.win(), desk);
    }
}

void KPager::clientPopupAboutToShow()
{
    if (!m_mnu) return;

    m_mnu->changeTitle(1,KWin::icon(m_winfo.win(),16,16,true), m_winfo.name());
    m_mnu->setItemChecked(IconifyOp, m_winfo.isMinimized());
    m_mnu->setItemChecked(MaximizeOp, m_winfo.state() & NET::Max);
    if (m_showStickyOption)   // Add translation
        m_mnu->changeItem(StickyOp,
                          (m_winfo.onAllDesktops()) ? QString("Un&sticky"):QString("&Sticky"));
}

void KPager::desktopPopupAboutToShow()
{
    if (!m_dmnu) return;

    m_dmnu->clear();
    m_dmnu->insertItem( i18n("&All Desktops"), 0 );
    m_dmnu->insertSeparator();

    if (m_winfo.onAllDesktops())
        m_dmnu->setItemChecked( 0, true );

    int id;
    for ( int i = 1; i <= m_winmodule->numberOfDesktops(); i++ ) {
        id = m_dmnu->insertItem( QString("&")+QString::number(i )+QString(" ")
                                 + m_winmodule->desktopName(i), i );
        if ( m_winfo.desktop() == i )
            m_dmnu->setItemChecked( id, TRUE );
    }
}

void KPager::clientPopupActivated( int id )
{
    switch ( id ) {
	case MaximizeOp:
	    if ( (m_winfo.state() & NET::Max)  == 0 ) {
		NETWinInfo ni( qt_xdisplay(),  m_winfo.win(), qt_xrootwin(), 0);
		ni.setState( NET::Max, NET::Max );
	    } else {
		NETWinInfo ni( qt_xdisplay(),  m_winfo.win(), qt_xrootwin(), 0);
		ni.setState( 0, NET::Max );
	    }
	    break;
	case IconifyOp:
	    if ( !m_winfo.isMinimized() ) {
		KWin::iconifyWindow( m_winfo.win());
	    } else {
		KWin::forceActiveWindow( m_winfo.win() );
	    }
	    break;
	case StickyOp:
	    if ( m_winfo.onAllDesktops() ) {
		KWin::setOnAllDesktops(m_winfo.win(), false);
	    } else {
		KWin::setOnAllDesktops(m_winfo.win(), true);
	    }
	    break;
	case CloseOp: {
	    NETRootInfo ri( qt_xdisplay(),  0 );
	    ri.closeWindowRequest( m_winfo.win() );
	} break;
	default:
	    break;
    }
}

void KPager::redrawDesktops()
{
    QValueList <Desktop *>::Iterator it;
    for( it = m_desktops.begin(); it != m_desktops.end(); ++it )
        (*it)->repaint();
}

void KPager::slotGrabWindows()
{
    m_desktops[m_currentDesktop-1]->m_grabWindows=true;
    m_desktops[m_currentDesktop-1]->repaint();
}

QSize KPager::sizeHint() const
{
    int n=m_desktops.count();
    int w=-1,h=-1;

    QSize size=m_desktops[0]->sizeHint();
    int wDsk=size.width();
    int hDsk=size.height();
    switch (m_layoutType)
    {
        case (Classical) :  w=wDsk*(n/2+(n%2)); h=hDsk*2;break;
        case (Horizontal) : w=wDsk*n; h=hDsk;break;
        case (Vertical) :   w=wDsk; h=hDsk*n;break;
    };
    return QSize(w,h);
}

const KPager::LayoutTypes KPager::c_defLayout=KPager::Horizontal;

#include "kpager.moc"
