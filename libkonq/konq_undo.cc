/* This file is part of the KDE project
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "konq_undo.h"

#undef Always

#include <kio/uiserver_stub.h>
#include "konq_operations.h"

#include <assert.h>

#include <dcopclient.h>
#include <dcopref.h>

#include <kapplication.h>
#include <kdatastream.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kconfig.h>
#include <kipc.h>

#include <kio/job.h>
#include <kdirnotify_stub.h>

inline const char *dcopTypeName( const KonqCommand & ) { return "KonqCommand"; }
inline const char *dcopTypeName( const KonqCommand::Stack & ) { return "KonqCommand::Stack"; }

/**
 * checklist:
 * copy dir -> overwrite -> works
 * move dir -> overwrite -> works
 * copy dir -> rename -> works
 * move dir -> rename -> works
 *
 * copy dir -> works
 * move dir -> works
 *
 * copy files -> works
 * move files -> works (TODO: optimize (change FileCopyJob to use the renamed arg for copyingDone)
 *
 * copy files -> overwrite -> works
 * move files -> overwrite -> works
 *
 * copy files -> rename -> works
 * move files -> rename -> works
 */

class KonqUndoJob : public KIO::Job
{
public:
    KonqUndoJob() : KIO::Job( true ) { KonqUndoManager::incRef(); };
    virtual ~KonqUndoJob() { KonqUndoManager::decRef(); }

    virtual void kill( bool q) { KonqUndoManager::self()->stopUndo( true ); KIO::Job::kill( q ); }
};

class KonqCommandRecorder::KonqCommandRecorderPrivate
{
public:
  KonqCommandRecorderPrivate()
  {
  }
  ~KonqCommandRecorderPrivate()
  {
  }

  KonqCommand m_cmd;
};

KonqCommandRecorder::KonqCommandRecorder( KonqCommand::Type op, const KURL::List &src, const KURL &dst, KIO::Job *job )
  : QObject( job, "konqcmdrecorder" )
{
  d = new KonqCommandRecorderPrivate;
  d->m_cmd.m_type = op;
  d->m_cmd.m_valid = true;
  d->m_cmd.m_src = src;
  d->m_cmd.m_dst = dst;
  connect( job, SIGNAL( result( KIO::Job * ) ),
           this, SLOT( slotResult( KIO::Job * ) ) );

  if ( op != KonqCommand::MKDIR ) {
      connect( job, SIGNAL( copyingDone( KIO::Job *, const KURL &, const KURL &, bool, bool ) ),
               this, SLOT( slotCopyingDone( KIO::Job *, const KURL &, const KURL &, bool, bool ) ) );
      connect( job, SIGNAL( copyingLinkDone( KIO::Job *, const KURL &, const QString &, const KURL & ) ),
               this, SLOT( slotCopyingLinkDone( KIO::Job *, const KURL &, const QString &, const KURL & ) ) );
  }

  KonqUndoManager::incRef();
}

KonqCommandRecorder::~KonqCommandRecorder()
{
  KonqUndoManager::decRef();
  delete d;
}

void KonqCommandRecorder::slotResult( KIO::Job *job )
{
  if ( job->error() )
    return;

  KonqUndoManager::self()->addCommand( d->m_cmd );
}

void KonqCommandRecorder::slotCopyingDone( KIO::Job *job, const KURL &from, const KURL &to, bool directory, bool renamed )
{
  KonqBasicOperation op;
  op.m_valid = true;
  op.m_directory = directory;
  op.m_renamed = renamed;
  op.m_src = from;
  op.m_dst = to;
  op.m_link = false;

  if ( d->m_cmd.m_type == KonqCommand::TRASH )
  {
      Q_ASSERT( from.isLocalFile() );
      Q_ASSERT( to.protocol() == "trash" );
      QMap<QString, QString> metaData = job->metaData();
      QMap<QString, QString>::ConstIterator it = metaData.find( "trashURL-" + from.path() );
      if ( it != metaData.end() ) {
          // Update URL
          op.m_dst = it.data();
      }
  }

  d->m_cmd.m_opStack.prepend( op );
}

void KonqCommandRecorder::slotCopyingLinkDone( KIO::Job *, const KURL &from, const QString &target, const KURL &to )
{
  KonqBasicOperation op;
  op.m_valid = true;
  op.m_directory = false;
  op.m_renamed = false;
  op.m_src = from;
  op.m_target = target;
  op.m_dst = to;
  op.m_link = true;
  d->m_cmd.m_opStack.prepend( op );
}

KonqUndoManager *KonqUndoManager::s_self = 0;
unsigned long KonqUndoManager::s_refCnt = 0;

class KonqUndoManager::KonqUndoManagerPrivate
{
public:
  KonqUndoManagerPrivate()
  {
      m_uiserver = new UIServer_stub( "kio_uiserver", "UIServer" );
      m_undoJob = 0;
  }
  ~KonqUndoManagerPrivate()
  {
      delete m_uiserver;
  }

  bool m_syncronized;

  KonqCommand::Stack m_commands;

  KonqCommand m_current;
  KIO::Job *m_currentJob;
  UndoState m_undoState;
  QValueStack<KURL> m_dirStack;
  QValueStack<KURL> m_dirCleanupStack;
  QValueStack<KURL> m_fileCleanupStack;
  QValueList<KURL> m_dirsToUpdate;

  bool m_lock;

  UIServer_stub *m_uiserver;
  int m_uiserverJobId;

  KonqUndoJob *m_undoJob;
};

KonqUndoManager::KonqUndoManager()
: DCOPObject( "KonqUndoManager" )
{
  if ( !kapp->dcopClient()->isAttached() )
      kapp->dcopClient()->attach();

  d = new KonqUndoManagerPrivate;
  d->m_syncronized = initializeFromKDesky();
  d->m_lock = false;
  d->m_currentJob = 0;
}

KonqUndoManager::~KonqUndoManager()
{
  delete d;
}

void KonqUndoManager::incRef()
{
  s_refCnt++;
}

void KonqUndoManager::decRef()
{
  s_refCnt--;
  if ( s_refCnt == 0 && s_self )
  {
    delete s_self;
    s_self = 0;
  }
}

KonqUndoManager *KonqUndoManager::self()
{
  if ( !s_self )
  {
    if ( s_refCnt == 0 )
      s_refCnt++; // someone forgot to call incRef
    s_self = new KonqUndoManager;
  }
  return s_self;
}

void KonqUndoManager::addCommand( const KonqCommand &cmd )
{
  broadcastPush( cmd );
}

bool KonqUndoManager::undoAvailable() const
{
  return ( d->m_commands.count() > 0 ) && !d->m_lock;
}

QString KonqUndoManager::undoText() const
{
  if ( d->m_commands.count() == 0 )
    return i18n( "Und&o" );

  KonqCommand::Type t = d->m_commands.top().m_type;
  if ( t == KonqCommand::COPY )
    return i18n( "Und&o: Copy" );
  else if ( t == KonqCommand::LINK )
    return i18n( "Und&o: Link" );
  else if ( t == KonqCommand::MOVE )
    return i18n( "Und&o: Move" );
  else if ( t == KonqCommand::TRASH )
    return i18n( "Und&o: Trash" );
  else if ( t == KonqCommand::MKDIR )
    return i18n( "Und&o: Create Folder" );
  else
    assert( false );
  /* NOTREACHED */
  return QString::null;
}

void KonqUndoManager::undo()
{
  KonqCommand cmd = d->m_commands.top();
  assert( cmd.m_valid );

  d->m_current = cmd;

  QValueList<KonqBasicOperation>& opStack = d->m_current.m_opStack;

  // Let's first ask for confirmation if we need to delete any file (#99898)
  KURL::List fileCleanupStack;
  QValueList<KonqBasicOperation>::Iterator it = opStack.begin();
  for ( ; it != opStack.end() ; ++it ) {
      if ( !(*it).m_directory && !(*it).m_link && d->m_current.m_type == KonqCommand::COPY ) {
          fileCleanupStack.append( (*it).m_dst );
      }
  }
  if ( !fileCleanupStack.isEmpty() ) {
      // Because undo can happen with an accidental Ctrl-Z, we want to always confirm.
      if ( !KonqOperations::askDeleteConfirmation( fileCleanupStack, KonqOperations::DEL,
                                                   KonqOperations::FORCE_CONFIRMATION,
                                                   0 /* TODO parent */ ) )
          return;
  }

  d->m_dirCleanupStack.clear();
  d->m_dirStack.clear();
  d->m_dirsToUpdate.clear();

  d->m_undoState = MOVINGFILES;

  broadcastPop();
  broadcastLock();

  it = opStack.begin();
  QValueList<KonqBasicOperation>::Iterator end = opStack.end();
  while ( it != end )
  {
    if ( (*it).m_directory && !(*it).m_renamed )
    {
      d->m_dirStack.push( (*it).m_src );
      d->m_dirCleanupStack.prepend( (*it).m_dst );
      it = d->m_current.m_opStack.remove( it );
      d->m_undoState = MAKINGDIRS;
      kdDebug(1203) << "KonqUndoManager::undo MAKINGDIRS" << endl;
    }
    else if ( (*it).m_link )
    {
      if ( !d->m_fileCleanupStack.contains( (*it).m_dst ) )
        d->m_fileCleanupStack.prepend( (*it).m_dst );

      if ( d->m_current.m_type != KonqCommand::MOVE )
        it = d->m_current.m_opStack.remove( it );
      else
        ++it;
    }
    else
      ++it;
  }

  /* this shouldn't be necessary at all:
   * 1) the source list may contain files, we don't want to
   *    create those as... directories
   * 2) all directories that need creation should already be in the
   *    directory stack
  if ( d->m_undoState == MAKINGDIRS )
  {
    KURL::List::ConstIterator it = d->m_current.m_src.begin();
    KURL::List::ConstIterator end = d->m_current.m_src.end();
    for (; it != end; ++it )
      if ( !d->m_dirStack.contains( *it) )
        d->m_dirStack.push( *it );
  }
  */

  if ( d->m_current.m_type != KonqCommand::MOVE )
    d->m_dirStack.clear();

  d->m_undoJob = new KonqUndoJob;
  d->m_uiserverJobId = d->m_undoJob->progressId();
  undoStep();
}

void KonqUndoManager::stopUndo( bool step )
{
    d->m_current.m_opStack.clear();
    d->m_dirCleanupStack.clear();
    d->m_fileCleanupStack.clear();
    d->m_undoState = REMOVINGDIRS;
    d->m_undoJob = 0;

    if ( d->m_currentJob )
        d->m_currentJob->kill( true );

    d->m_currentJob = 0;

    if ( step )
        undoStep();
}

void KonqUndoManager::slotResult( KIO::Job *job )
{
  d->m_uiserver->jobFinished( d->m_uiserverJobId );
  if ( job->error() )
  {
    job->showErrorDialog( 0L );
    d->m_currentJob = 0;
    stopUndo( false );
    if ( d->m_undoJob )
    {
        delete d->m_undoJob;
        d->m_undoJob = 0;
    }
  }

  undoStep();
}


void KonqUndoManager::addDirToUpdate( const KURL& url )
{
  if ( d->m_dirsToUpdate.find( url ) == d->m_dirsToUpdate.end() )
    d->m_dirsToUpdate.prepend( url );
}

void KonqUndoManager::undoStep()
{
  d->m_currentJob = 0;

  if ( d->m_undoState == MAKINGDIRS )
      undoMakingDirectories();

  if ( d->m_undoState == MOVINGFILES )
      undoMovingFiles();

  if ( d->m_undoState == REMOVINGFILES )
      undoRemovingFiles();

  if ( d->m_undoState == REMOVINGDIRS )
      undoRemovingDirectories();

  if ( d->m_currentJob )
    connect( d->m_currentJob, SIGNAL( result( KIO::Job * ) ),
             this, SLOT( slotResult( KIO::Job * ) ) );
}

void KonqUndoManager::undoMakingDirectories()
{
    if ( !d->m_dirStack.isEmpty() ) {
      KURL dir = d->m_dirStack.pop();
      kdDebug(1203) << "KonqUndoManager::undoStep creatingDir " << dir.prettyURL() << endl;
      d->m_currentJob = KIO::mkdir( dir );
      d->m_uiserver->creatingDir( d->m_uiserverJobId, dir );
    }
    else
      d->m_undoState = MOVINGFILES;
}

void KonqUndoManager::undoMovingFiles()
{
    if ( !d->m_current.m_opStack.isEmpty() )
    {
      KonqBasicOperation op = d->m_current.m_opStack.pop();

      assert( op.m_valid );
      if ( op.m_directory )
      {
        if ( op.m_renamed )
        {
          kdDebug(1203) << "KonqUndoManager::undoStep rename " << op.m_dst.prettyURL() << " " << op.m_src.prettyURL() << endl;
          d->m_currentJob = KIO::rename( op.m_dst, op.m_src, false );
          d->m_uiserver->moving( d->m_uiserverJobId, op.m_dst, op.m_src );
        }
        else
          assert( 0 ); // this should not happen!
      }
      else if ( op.m_link )
      {
        kdDebug(1203) << "KonqUndoManager::undoStep symlink " << op.m_target << " " << op.m_src.prettyURL() << endl;
        d->m_currentJob = KIO::symlink( op.m_target, op.m_src, true, false );
      }
      else if ( d->m_current.m_type == KonqCommand::COPY )
      {
        kdDebug(1203) << "KonqUndoManager::undoStep file_delete " << op.m_dst.prettyURL() << endl;
        d->m_currentJob = KIO::file_delete( op.m_dst );
        d->m_uiserver->deleting( d->m_uiserverJobId, op.m_dst );
      }
      else if ( d->m_current.m_type == KonqCommand::MOVE
                || d->m_current.m_type == KonqCommand::TRASH )
      {
        kdDebug(1203) << "KonqUndoManager::undoStep file_move " << op.m_dst.prettyURL() << " " << op.m_src.prettyURL() << endl;
        d->m_currentJob = KIO::file_move( op.m_dst, op.m_src, -1, true );
        d->m_uiserver->moving( d->m_uiserverJobId, op.m_dst, op.m_src );
      }

      // The above KIO jobs are lowlevel, they don't trigger KDirNotify notification
      // So we need to do it ourselves (but schedule it to the end of the undo, to compress them)
      KURL url( op.m_dst );
      url.setPath( url.directory() );
      addDirToUpdate( url );

      url = op.m_src;
      url.setPath( url.directory() );
      addDirToUpdate( url );
    }
    else
      d->m_undoState = REMOVINGFILES;
}

void KonqUndoManager::undoRemovingFiles()
{
    kdDebug(1203) << "KonqUndoManager::undoStep REMOVINGFILES" << endl;
    if ( !d->m_fileCleanupStack.isEmpty() )
    {
      KURL file = d->m_fileCleanupStack.pop();
      kdDebug(1203) << "KonqUndoManager::undoStep file_delete " << file.prettyURL() << endl;
      d->m_currentJob = KIO::file_delete( file );
      d->m_uiserver->deleting( d->m_uiserverJobId, file );

      KURL url( file );
      url.setPath( url.directory() );
      addDirToUpdate( url );
    }
    else
    {
      d->m_undoState = REMOVINGDIRS;

      if ( d->m_dirCleanupStack.isEmpty() && d->m_current.m_type == KonqCommand::MKDIR )
        d->m_dirCleanupStack << d->m_current.m_dst;
    }
}

void KonqUndoManager::undoRemovingDirectories()
{
    if ( !d->m_dirCleanupStack.isEmpty() )
    {
      KURL dir = d->m_dirCleanupStack.pop();
      kdDebug(1203) << "KonqUndoManager::undoStep rmdir " << dir.prettyURL() << endl;
      d->m_currentJob = KIO::rmdir( dir );
      d->m_uiserver->deleting( d->m_uiserverJobId, dir );
      addDirToUpdate( dir );
    }
    else
    {
      d->m_current.m_valid = false;
      d->m_currentJob = 0;
      if ( d->m_undoJob )
      {
          kdDebug(1203) << "KonqUndoManager::undoStep deleting undojob" << endl;
          d->m_uiserver->jobFinished( d->m_uiserverJobId );
          delete d->m_undoJob;
          d->m_undoJob = 0;
      }
      KDirNotify_stub allDirNotify( "*", "KDirNotify*" );
      QValueList<KURL>::ConstIterator it = d->m_dirsToUpdate.begin();
      for( ; it != d->m_dirsToUpdate.end(); ++it ) {
          kdDebug() << "Notifying FilesAdded for " << *it << endl;
          allDirNotify.FilesAdded( *it );
      }
      broadcastUnlock();
    }
}

void KonqUndoManager::push( const KonqCommand &cmd )
{
  d->m_commands.push( cmd );
  emit undoAvailable( true );
  emit undoTextChanged( undoText() );
}

void KonqUndoManager::pop()
{
  d->m_commands.pop();
  emit undoAvailable( undoAvailable() );
  emit undoTextChanged( undoText() );
}

void KonqUndoManager::lock()
{
//  assert( !d->m_lock );
  d->m_lock = true;
  emit undoAvailable( undoAvailable() );
}

void KonqUndoManager::unlock()
{
//  assert( d->m_lock );
  d->m_lock = false;
  emit undoAvailable( undoAvailable() );
}

KonqCommand::Stack KonqUndoManager::get() const
{
  return d->m_commands;
}

void KonqUndoManager::broadcastPush( const KonqCommand &cmd )
{
  if ( !d->m_syncronized )
  {
    push( cmd );
    return;
  }

  DCOPRef( "kdesktop", "KonqUndoManager" ).send( "push", cmd );
  DCOPRef( "konqueror*", "KonqUndoManager" ).send( "push", cmd );
}

void KonqUndoManager::broadcastPop()
{
  if ( !d->m_syncronized )
  {
    pop();
    return;
  }
  DCOPRef( "kdesktop", "KonqUndoManager" ).send( "pop" );
  DCOPRef( "konqueror*", "KonqUndoManager" ).send( "pop" );
}

void KonqUndoManager::broadcastLock()
{
//  assert( !d->m_lock );

  if ( !d->m_syncronized )
  {
    lock();
    return;
  }
  DCOPRef( "kdesktop", "KonqUndoManager" ).send( "lock" );
  DCOPRef( "konqueror*", "KonqUndoManager" ).send( "lock" );
}

void KonqUndoManager::broadcastUnlock()
{
//  assert( d->m_lock );

  if ( !d->m_syncronized )
  {
    unlock();
    return;
  }
  DCOPRef( "kdesktop", "KonqUndoManager" ).send( "unlock" );
  DCOPRef( "konqueror*", "KonqUndoManager" ).send( "unlock" );
}

bool KonqUndoManager::initializeFromKDesky()
{
  // ### workaround for dcop problem and upcoming 2.1 release:
  // in case of huge io operations the amount of data sent over
  // dcop (containing undo information broadcasted for global undo
  // to all konqueror instances) can easily exceed the 64kb limit
  // of dcop. In order not to run into trouble we disable global
  // undo for now! (Simon)
  // ### FIXME: post 2.1
  return false;

  DCOPClient *client = kapp->dcopClient();

  if ( client->appId() == "kdesktop" ) // we are master :)
    return true;

  if ( !client->isApplicationRegistered( "kdesktop" ) )
    return false;

  d->m_commands = DCOPRef( "kdesktop", "KonqUndoManager" ).call( "get" );
  return true;
}

QDataStream &operator<<( QDataStream &stream, const KonqBasicOperation &op )
{
    stream << op.m_valid << op.m_directory << op.m_renamed << op.m_link
           << op.m_src << op.m_dst << op.m_target;
  return stream;
}
QDataStream &operator>>( QDataStream &stream, KonqBasicOperation &op )
{
  stream >> op.m_valid >> op.m_directory >> op.m_renamed >> op.m_link
         >> op.m_src >> op.m_dst >> op.m_target;
  return stream;
}

QDataStream &operator<<( QDataStream &stream, const KonqCommand &cmd )
{
  stream << cmd.m_valid << (Q_INT8)cmd.m_type << cmd.m_opStack << cmd.m_src << cmd.m_dst;
  return stream;
}

QDataStream &operator>>( QDataStream &stream, KonqCommand &cmd )
{
  Q_INT8 type;
  stream >> cmd.m_valid >> type >> cmd.m_opStack >> cmd.m_src >> cmd.m_dst;
  cmd.m_type = static_cast<KonqCommand::Type>( type );
  return stream;
}

#include "konq_undo.moc"
