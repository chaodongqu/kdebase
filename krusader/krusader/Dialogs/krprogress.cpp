/* This file is part of the KDE libraries
   Copyright (C) 2000 Matej Koss <koss@miesto.sk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qtimer.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qdatetime.h>

#include <kapplication.h>
#include <kdialog.h>
#include <kstringhandler.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <kwin.h>

#include <kio/jobclasses.h>

#include "krprogress.h"
#include "../krusader.h"

KrProgress::KrProgress( KIO::Job* job )
  : ProgressBase( krApp ),
  m_iTotalSize(0), m_iTotalFiles(0), m_iTotalDirs(0),
  m_iProcessedSize(0), m_iProcessedDirs(0), m_iProcessedFiles(0){

#ifdef Q_WS_X11 //FIXME(E): Remove once all the KWin::foo calls have been ported to QWS
  // Set a useful icon for this window!
  KWin::setIcons( winId(),
          KGlobal::iconLoader()->loadIcon( "filesave", KIcon::NoGroup, 32 ),
          KGlobal::iconLoader()->loadIcon( "filesave", KIcon::NoGroup, 16 ) );
#endif

  QVBoxLayout *topLayout = new QVBoxLayout( this, KDialog::marginHint(),
                                            KDialog::spacingHint() );
  topLayout->addStrut( 360 );   // makes dlg at least that wide

  QGridLayout *grid = new QGridLayout( 2, 3 );
  topLayout->addLayout(grid);
  grid->addColSpacing(1, KDialog::spacingHint());
  // filenames or action name
  grid->addWidget(new QLabel(i18n("Source:"), this), 0, 0);

  sourceLabel = new KSqueezedTextLabel(this);
  grid->addWidget(sourceLabel, 0, 2);

  destInvite = new QLabel(i18n("Destination:"), this);
  grid->addWidget(destInvite, 1, 0);

  destLabel = new KSqueezedTextLabel(this);
  grid->addWidget(destLabel, 1, 2);

  m_pProgressBar = new KProgress(this);
  topLayout->addWidget( m_pProgressBar );

  // processed info
  QHBoxLayout *hBox = new QHBoxLayout();
  topLayout->addLayout(hBox);

  sizeLabel = new QLabel(this);
  hBox->addWidget(sizeLabel);

  resumeLabel = new QLabel(this);
  hBox->addWidget(resumeLabel);

  progressLabel = new QLabel( this );
/*  progressLabel->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding,
                                             QSizePolicy::Preferred ) );*/
  progressLabel->setAlignment( QLabel::AlignRight );
  hBox->addWidget( progressLabel );

  hBox = new QHBoxLayout();
  topLayout->addLayout(hBox);

  speedLabel = new QLabel(this);
  hBox->addWidget(speedLabel, 1);

  QFrame *line = new QFrame( this );
  line->setFrameShape( QFrame::HLine );
  line->setFrameShadow( QFrame::Sunken );
  topLayout->addWidget( line );

  hBox = new QHBoxLayout();
  topLayout->addLayout(hBox);

  hBox->addStretch(1);

  KPushButton *pb = new KPushButton( KStdGuiItem::cancel(), this );
  connect( pb, SIGNAL( clicked() ), SLOT( slotStop() ) );
  hBox->addWidget( pb );

  resize( sizeHint() );
  setMaximumHeight(sizeHint().height());

  setCaption(i18n("Krusader Progress")); // show something better than kio_uiserver

  setJob(job);
  setOnlyClean(false);
  setStopOnClose(true);
  // Connect global progress info signals
  connect( job, SIGNAL( percent( KIO::Job*, unsigned long ) ),
                SLOT( slotPercent( KIO::Job*, unsigned long ) ) );
  connect( job, SIGNAL( infoMessage( KIO::Job*, const QString & ) ),
                SLOT( slotInfoMessage( KIO::Job*, const QString & ) ) );
  connect( job, SIGNAL( totalSize( KIO::Job*, KIO::filesize_t ) ),
                SLOT( slotTotalSize( KIO::Job*, KIO::filesize_t ) ) );
  connect( job, SIGNAL( processedSize( KIO::Job*, KIO::filesize_t ) ),
                SLOT( slotProcessedSize( KIO::Job*, KIO::filesize_t ) ) );
  connect( job, SIGNAL( speed( KIO::Job*, unsigned long ) ),
                SLOT( slotSpeed( KIO::Job*, unsigned long ) ) );

  // change to modal & move to Krusader's center
    QPoint center((krApp->width()-width())/2,(krApp->height()-height())/2);
  center = center+(krApp->pos());
  reparent(krApp,Qt::WType_Dialog|Qt::WShowModal,center);
  //setWFlags(WType_Modal);
  //move((krApp->width()-width())/2,(krApp->height()-height())/2);
  show();
}

KrProgress::~KrProgress(){}

void KrProgress::slotTotalSize( KIO::Job*, KIO::filesize_t bytes ){
  m_iTotalSize = bytes;
}


void KrProgress::slotTotalFiles( KIO::Job*, unsigned long files ){
  m_iTotalFiles = files;
  showTotals();
}


void KrProgress::slotTotalDirs( KIO::Job*, unsigned long dirs ){
  m_iTotalDirs = dirs;
  showTotals();
}

void KrProgress::showTotals(){
  // Show the totals in the progress label, if we still haven't
  // processed anything. This is useful when the stat'ing phase
  // of CopyJob takes a long time (e.g. over networks).
  if ( m_iProcessedFiles == 0 && m_iProcessedDirs == 0 )
  {
    QString tmps;
    if ( m_iTotalDirs > 1 )
      // that we have a singular to translate looks weired but is only logical
      tmps = i18n("%n directory", "%n directories", m_iTotalDirs) + "   ";
    tmps += i18n("%n file", "%n files", m_iTotalFiles);
    progressLabel->setText( tmps );
  }
}

void KrProgress::slotPercent( KIO::Job*, unsigned long percent ){
  QString tmp(i18n( "%1% of %2 ").arg( percent ).arg( KIO::convertSize(m_iTotalSize)));
  m_pProgressBar->setValue( percent );
  tmp.append(i18n(" (Reading)"));

  setCaption( tmp );
}


void KrProgress::slotInfoMessage( KIO::Job*, const QString & msg )
{
  speedLabel->setText( msg );
  speedLabel->setAlignment( speedLabel->alignment() & ~Qt::WordBreak );
}


void KrProgress::slotProcessedSize( KIO::Job*, KIO::filesize_t bytes ) {
  m_iProcessedSize = bytes;

  QString tmp;
  tmp = i18n( "%1 of %2 complete").arg( KIO::convertSize(bytes) ).arg( KIO::convertSize(m_iTotalSize));
  sizeLabel->setText( tmp );
}


void KrProgress::slotProcessedDirs( KIO::Job*, unsigned long dirs )
{
  m_iProcessedDirs = dirs;

  QString tmps;
  tmps = i18n("%1 / %n directory", "%1 / %n directories", m_iTotalDirs).arg( m_iProcessedDirs );
  tmps += "   ";
  tmps += i18n("%1 / %n file", "%1 / %n files", m_iTotalFiles).arg( m_iProcessedFiles );
  progressLabel->setText( tmps );
}


void KrProgress::slotProcessedFiles( KIO::Job*, unsigned long files )
{
  m_iProcessedFiles = files;

  QString tmps;
  if ( m_iTotalDirs > 1 ) {
    tmps = i18n("%1 / %n directory", "%1 / %n directories", m_iTotalDirs).arg( m_iProcessedDirs );
    tmps += "   ";
  }
  tmps += i18n("%1 / %n file", "%1 / %n files", m_iTotalFiles).arg( m_iProcessedFiles );
  progressLabel->setText( tmps );
}


void KrProgress::slotSpeed( KIO::Job*, unsigned long bytes_per_second )
{
  if ( bytes_per_second == 0 ) {
    speedLabel->setText( i18n( "Working") );
  } else {
#if KDE_IS_VERSION(3,4,0)
    unsigned int seconds = KIO::calculateRemainingSeconds( m_iTotalSize, m_iProcessedSize, bytes_per_second );
	 QString remaining = KIO::convertSeconds(seconds);
#else
    QString remaining = KIO::calculateRemaining( m_iTotalSize, m_iProcessedSize, bytes_per_second ).toString();
#endif
    speedLabel->setText( i18n( "%1/s ( %2 remaining )").arg( KIO::convertSize( bytes_per_second )).arg( remaining ) );
  }
}


void KrProgress::setDestVisible( bool visible )
{
  // We can't hide the destInvite/destLabel labels,
  // because it screws up the QGridLayout.
  if (visible)
  {
    destInvite->setText( i18n("Destination:") );
  }
  else
  {
    destInvite->setText( QString::null );
    destLabel->setText( QString::null );
  }
}

void KrProgress::virtual_hook( int id, void* data ){
  ProgressBase::virtual_hook( id, data );
}

void KrProgress::slotStop(){
  if ( m_pJob ) {
    m_pJob->kill(false); // this will call slotFinished
    m_pJob = 0L;
  } else {
    slotFinished( 0 ); // here we call it ourselves
  }

  emit stopped();
}

void KrProgress::closeEvent( QCloseEvent* ) { hide(); slotStop(); }

#include "krprogress.moc"
