/***************************************************************************
                         dulines.cpp  -  description
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

#include "dulines.h"
#include "../kicons.h"
#include "../krusader.h"
#include "../VFS/krpermhandler.h"
#include <qheader.h>
#include <klocale.h>
#include <qpen.h>
#include <qpainter.h>
#include <qfontmetrics.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <kpopupmenu.h>

class DULinesItem : public QListViewItem
{
public:
  DULinesItem( DiskUsage *diskUsageIn, File *fileItem, QListView * parent, QString label1, 
               QString label2, QString label3, unsigned int italicPos ) : QListViewItem( parent, label1, label2, label3 ), 
               diskUsage( diskUsageIn ), file( fileItem ), isTruncated( false ), italicTextPos( italicPos ) {}
  DULinesItem( DiskUsage *diskUsageIn, File *fileItem, QListView * parent, QListViewItem * after, 
               QString label1, QString label2, QString label3, unsigned int italicPos ) : QListViewItem( parent, after, label1, 
               label2, label3 ), diskUsage( diskUsageIn ), file( fileItem ), isTruncated( false ), italicTextPos( italicPos ) {}
  
  virtual int compare ( QListViewItem * i, int col, bool ascending ) const 
  {
    if( text(0) == ".." ) return ascending ? -1 : 1;
    if( i->text(0) == "..") return ascending ? 1 : -1;
    
    DULinesItem *compWith = dynamic_cast< DULinesItem * >( i );
        
    QString buf1,buf2;
    
    switch( col )
    {
    case 0:    
    case 1:
      buf1.sprintf("%025llu",file->size());
      buf2.sprintf("%025llu",compWith->file->size());
      return -QString::compare( buf1, buf2 );
    default:    
      return QListViewItem::compare( i, col, ascending );
    }
  }

  virtual void paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int align )
  {
    if( column == 2 )
    {
      if ( isSelected() ) 
        p->fillRect( 0, 0, width, height(), cg.brush( QColorGroup::Highlight ) );
      else
        p->fillRect( 0, 0, width, height(), cg.brush( QColorGroup::Base ) );
        
      QListView *lv = listView();
      
      int pos = lv->itemMargin();      
            
      const QPixmap *icon = pixmap( column );
      if( icon )
      {
        int iconWidth = icon->width() + lv->itemMargin();
        int xo = pos;
        int yo = ( height() - icon->height() ) / 2;

        p->drawPixmap( xo, yo, *icon );
        
        pos += iconWidth;
      }
    
      QFontMetrics fm( p->fontMetrics() );
      
      if( isSelected() )
        p->setPen( cg.highlightedText() );
      else
        p->setPen( cg.text() );
      
      QString t = text( column );
      QString b;
      
      if( t.length() > italicTextPos )
      {
        b = t.mid( italicTextPos );
        t.truncate( italicTextPos );
      }
      
      isTruncated = false;
      if( !t.isEmpty() )
      {
        int remWidth = width-pos;
        
        if( fm.width( t ) > remWidth )
        {
          while( !t.isEmpty() )
          {
            t.truncate( t.length() - 1 );
            if( fm.width( t + "..." ) <= remWidth )
              break;
          }          
          t += "...";
          isTruncated = true;
        }
        
        p->drawText( pos, 0, width, height(), align, t );
        pos += fm.width( t );
      }
                    
      if( !b.isEmpty() && !isTruncated )
      {
        QFont font( p->font() );
        font.setItalic( true );
        p->setFont( font );

        QFontMetrics fm2( p->fontMetrics() );
        
        int remWidth = width-pos;
        
        if( fm2.width( b ) > remWidth )
        {
          while( !b.isEmpty() )
          {
            b.truncate( b.length() - 1 );
            if( fm2.width( b + "..." ) <= remWidth )
              break;
          }          
          b += "...";
          isTruncated = true;
        }
        
        p->drawText( pos, 0, width, height(), align, b );
      }
    }
    else
      QListViewItem::paintCell( p, cg, column, width, align );
  }
    
  inline File * getFile() { return file; }
  
private:
  DiskUsage *diskUsage;
  File *file;
  
  bool isTruncated;
  unsigned int italicTextPos;
};

class DULinesToolTip : public QToolTip
{
public:
    DULinesToolTip( DiskUsage *usage, QWidget *parent, QListView *lv );
    void maybeTip( const QPoint &pos );

    virtual ~DULinesToolTip() {}
private:
    QListView *view;
    DiskUsage *diskUsage;
};

DULinesToolTip::DULinesToolTip( DiskUsage *usage, QWidget *parent, QListView *lv )
  : QToolTip( parent ), view( lv ), diskUsage( usage )
{
}

void DULinesToolTip::maybeTip( const QPoint &pos )
{
  QListViewItem *item = view->itemAt( pos );
  QPoint contentsPos = view->viewportToContents( pos );
  if ( !item )
    return;
    
  int col = view->header()->sectionAt( contentsPos.x() );

  int width = item->width( QFontMetrics( view->font() ), view, col );
    
  QRect r = view->itemRect( item );
  int headerPos = view->header()->sectionPos( col );
  r.setLeft( headerPos );
  r.setRight( headerPos + view->header()->sectionSize( col ) );
    
  if( col != 0 && width > view->columnWidth( col ) )
    tip( r, item->text( col ) );
  else if( col == 1 && item->text( 0 ) != ".." )
  {
    File *fileItem = ((DULinesItem *)item)->getFile();
    tip( r, diskUsage->getToolTip( fileItem ) );
  }
}

DULines::DULines( DiskUsage *usage, const char *name )
  : QListView( usage, name ), diskUsage( usage ), refreshNeeded( false )
{
  setAllColumnsShowFocus(true);
  setVScrollBarMode(QScrollView::Auto);
  setHScrollBarMode(QScrollView::Auto);
  setShowSortIndicator(true);
  setTreeStepSize( 10 );

  int defaultSize = QFontMetrics(font()).width("W");
  
  krConfig->setGroup( diskUsage->getConfigGroup() ); 

  showFileSize = krConfig->readBoolEntry( "L Show File Size", true );
  
  int lineWidth  = krConfig->readNumEntry("L Line Width",  defaultSize * 20 );    
  addColumn( i18n("Line View"), lineWidth );
  setColumnWidthMode(0,QListView::Manual);
  int precentWidth  = krConfig->readNumEntry("L Percent Width",  defaultSize * 6 );    
  addColumn( i18n("Percent"), precentWidth );
  setColumnWidthMode(1,QListView::Manual);
  int nameWidth  = krConfig->readNumEntry("L Name Width",  defaultSize * 20 );
  addColumn( i18n("Name"), nameWidth );
  setColumnWidthMode(2,QListView::Manual);
  
  setColumnAlignment( 1, Qt::AlignRight );
  
  header()->setStretchEnabled( true, 0 );
  
  setSorting( 1 );
  
  toolTip = new DULinesToolTip( diskUsage, viewport(), this );

  connect( diskUsage, SIGNAL( enteringDirectory( Directory * ) ), this, SLOT( slotDirChanged( Directory * ) ) );
  connect( diskUsage, SIGNAL( clearing() ), this, SLOT( clear() ) );
  
  connect( header(), SIGNAL( sizeChange( int, int, int ) ), this, SLOT( sectionResized( int ) ) );

  connect( this, SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
           this, SLOT( slotRightClicked(QListViewItem *) ) );
  connect( diskUsage, SIGNAL( changed( File * ) ), this, SLOT( slotChanged( File * ) ) );
  connect( diskUsage, SIGNAL( deleted( File * ) ), this, SLOT( slotDeleted( File * ) ) );
}

DULines::~DULines()
{
  krConfig->setGroup( diskUsage->getConfigGroup() ); 
  krConfig->writeEntry("L Line Width",      columnWidth( 0 ) );
  krConfig->writeEntry("L Percent Width",   columnWidth( 1 ) );
  krConfig->writeEntry("L Name Width",      columnWidth( 2 ) );
  krConfig->writeEntry("L Show File Size",  showFileSize );
  
  delete toolTip;
}

void DULines::slotDirChanged( Directory *dirEntry )
{
  clear();  
  
  QListViewItem * lastItem = 0;
    
  if( ! ( dirEntry->parent() == 0 ) )
  {
    lastItem = new QListViewItem( this, ".." );
    lastItem->setPixmap( 0, FL_LOADICON( "up" ) );
    lastItem->setSelectable( false );
  }
          
  int maxPercent = -1;
  for( Iterator<File> it = dirEntry->iterator(); it != dirEntry->end(); ++it )
  {
    File *item = *it;
    if( !item->isExcluded() && item->intPercent() > maxPercent )
      maxPercent = item->intPercent();
  }
  
  for( Iterator<File> it = dirEntry->iterator(); it != dirEntry->end(); ++it )
  { 
    File *item = *it;
    
    QString fileName = item->name();
    
    unsigned int italicStart = fileName.length();
    
    if( showFileSize )
      fileName += "  [" + KIO::convertSize( item->size() ) + "]";
    
    if( lastItem == 0 )
      lastItem = new DULinesItem( diskUsage, item, this, "", item->percent() + "  ", fileName, italicStart );
    else
      lastItem = new DULinesItem( diskUsage, item, this, lastItem, "", item->percent() + "  ", fileName, italicStart );
   
    if( item->isExcluded() )
      lastItem->setVisible( false );
                                    
    lastItem->setPixmap( 2, diskUsage->getIcon( item->mime() ) );
    lastItem->setPixmap( 0, createPixmap( item->intPercent(), maxPercent, columnWidth( 0 ) - itemMargin() ) );
  }
  
  setCurrentItem( firstChild() );
}

QPixmap DULines::createPixmap( int percent, int maxPercent, int maxWidth )
{
  if( percent < 0 || percent > maxPercent || maxWidth < 2 || maxPercent == 0 )
    return QPixmap();
  maxWidth -= 2;

  int actualWidth = maxWidth*percent/maxPercent;
  if( actualWidth == 0 )
    return QPixmap();
    
  QPen pen;
  pen.setColor( Qt::black );  
  QPainter painter;
  
  int size = QFontMetrics(font()).height()-2;
  QRect rect( 0, 0, actualWidth, size );
  QPixmap pixmap( rect.width(), rect.height() );

  painter.begin( &pixmap );
  painter.setPen( pen );
  
  for( int i = 1; i < actualWidth - 1; i++ )
  {
    int color = (511*i/maxWidth);
    if( color < 256 )
      pen.setColor( QColor( 255-color, 255, 0 ) );
    else
      pen.setColor( QColor( color-256, 511-color, 0 ) );
    
    painter.setPen( pen );
    painter.drawLine( i, 1, i, size-1 );
  }
  
  pen.setColor( Qt::black );  
  painter.setPen( pen );
  painter.drawRect( rect );
  painter.end();
  pixmap.detach();
  return pixmap;
}

void DULines::sectionResized( int column )
{
  if( childCount() == 0 || column != 0 )
    return;
    
  Directory * currentDir = diskUsage->getCurrentDir();  
  if( currentDir == 0 )
    return;

  int maxPercent = -1;  
  for( Iterator<File> it = currentDir->iterator(); it != currentDir->end(); ++it )
  {
    File *item = *it;  
    
    if( !item->isExcluded() && item->intPercent() > maxPercent )
      maxPercent = item->intPercent();
  }
  
  DULinesItem *duItem = (DULinesItem *)firstChild();
  while( duItem )
  {
    if( duItem->text( 0 ) != ".." )
      duItem->setPixmap( 0, createPixmap( duItem->getFile()->intPercent(), maxPercent, columnWidth( 0 ) ) );
    duItem = (DULinesItem *)duItem->nextSibling();
  }
}

bool DULines::doubleClicked( QListViewItem * item )
{
  if( item )
  {
    if( item->text( 0 ) != ".." )
    {
      File *fileItem = ((DULinesItem *)item)->getFile();
      if( fileItem->isDir() )
        diskUsage->changeDirectory( dynamic_cast<Directory *> ( fileItem ) );
      return true;
    }
    else
    {
      Directory *upDir = (Directory *)diskUsage->getCurrentDir()->parent();
    
      if( upDir )
        diskUsage->changeDirectory( upDir );
      return true;
    }
  }
  return false;
}

void DULines::contentsMouseDoubleClickEvent ( QMouseEvent * e )
{
  if ( e || e->button() == LeftButton )
  {
    QPoint vp = contentsToViewport(e->pos());
    QListViewItem * item = itemAt( vp );

    if( doubleClicked( item ) )
      return;
    
  }
  QListView::contentsMouseDoubleClickEvent( e );
}


void DULines::keyPressEvent( QKeyEvent *e )
{
  switch ( e->key() )
  {
  case Key_Return :
  case Key_Enter :
    if( doubleClicked( currentItem() ) )
      return;
    break;
  case Key_Left :
  case Key_Right :
  case Key_Up :
  case Key_Down :
    if( e->state() == ShiftButton )
    {
      e->ignore();
      return;
    }
    break;
  case Key_Delete :
    e->ignore();
    return;
  }
  QListView::keyPressEvent( e );
}
 
void DULines::slotRightClicked( QListViewItem *item )
{
  File * file = 0;
  
  if ( item && item->text( 0 ) != ".." )
    file = ((DULinesItem *)item)->getFile();

  KPopupMenu linesPopup;    
  int lid = linesPopup.insertItem( i18n("Show file sizes"), this, SLOT( slotShowFileSizes() ) );
  linesPopup.setItemChecked( lid, showFileSize );
    
  diskUsage->rightClickMenu( file, &linesPopup, i18n( "Lines" ) );
}

void DULines::slotShowFileSizes()
{
  showFileSize = !showFileSize;
  slotDirChanged( diskUsage->getCurrentDir() );
}

File * DULines::getCurrentFile()
{
  QListViewItem *item = currentItem();
  
  if( item == 0 || item->text( 0 ) == ".." )
    return 0;
  
  return ((DULinesItem *)item)->getFile();
}

void DULines::slotChanged( File * item )
{
  QListViewItem *lvitem = firstChild();
  while( lvitem )
  {
    if( lvitem->text( 0 ) != ".." ) {
      DULinesItem *duItem = (DULinesItem *)( lvitem );
      if( duItem->getFile() == item )
      {
        duItem->setVisible( !item->isExcluded() );
        duItem->setText( 1, item->percent() );
        if( !refreshNeeded )
        {
          refreshNeeded = true;
          QTimer::singleShot( 0, this, SLOT( slotRefresh() ) );
        }
        break;
      }
    }
    lvitem = lvitem->nextSibling();
  }
}

void DULines::slotDeleted( File * item )
{
  QListViewItem *lvitem = firstChild();
  while( lvitem )
  {
    if( lvitem->text( 0 ) != ".." ) {
      DULinesItem *duItem = (DULinesItem *)( lvitem );
      if( duItem->getFile() == item )
      {
        delete duItem;
        break;
      }
    }
    lvitem = lvitem->nextSibling();
  }
}

#include "dulines.moc"
