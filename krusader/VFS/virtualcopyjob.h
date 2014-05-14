/***************************************************************************
                       virtualcopyjob.h  -  description
                             -------------------
    copyright            : (C) 2006 + by Csaba Karai
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

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __VIRTUAL_COPY_JOB_H__
#define __VIRTUAL_COPY_JOB_H__

#include <kio/job.h>
#include "preservingcopyjob.h"
#include <qvaluelist.h>
#include <qtimer.h>
#include <qdict.h>
#include <qmap.h>

class vfs;

typedef enum {
  ST_STARTING               = 0,
  ST_CALCULATING_TOTAL_SIZE = 1,
  ST_CREATING_DIRECTORY     = 2,
  ST_COPYING                = 3
} State;

class VirtualCopyJob : public KIO::Job
{
  Q_OBJECT

public:
  VirtualCopyJob( const QStringList *names, vfs * vfs, const KURL& dest, const KURL& baseURL,
                  PreserveMode pmode, KIO::CopyJob::CopyMode mode, bool asMethod, bool showProgressInfo );

protected:
  void statNextDir();
  void createNextDir();
  void copyCurrentDir();
  void directoryFinished( const QString & );
  
protected slots:
  void slotStart();
  void slotReport();
  
  void slotKdsResult( KIO::Job * );
  void slotStatResult( KIO::Job * );
  void slotMkdirResult( KIO::Job * );
  void slotCopyResult( KIO::Job * );

  void slotCopying(KIO::Job *, const KURL &, const KURL &);
  void slotMoving(KIO::Job *, const KURL &, const KURL &);
  void slotCreatingDir(KIO::Job *, const KURL &);
  
  void slotProcessedFiles (KIO::Job *, unsigned long);
  void slotProcessedDirs (KIO::Job *, unsigned long);
  void slotProcessedSize (KIO::Job *, KIO::filesize_t);

signals:
  void totalFiles( KIO::Job *job, unsigned long files );
  void totalDirs( KIO::Job *job, unsigned long dirs );
  void processedFiles( KIO::Job *job, unsigned long files );
  void processedDirs( KIO::Job *job, unsigned long dirs );
  
private:
  KIO::filesize_t          m_totalSize;
  unsigned long            m_totalFiles;
  unsigned long            m_totalSubdirs;

  KIO::filesize_t          m_processedSize;
  unsigned long            m_processedFiles;
  unsigned long            m_processedSubdirs;  
    
  KIO::filesize_t          m_tempSize;
  unsigned long            m_tempFiles;
  unsigned long            m_tempSubdirs;  
    
  QValueList<KURL>         m_dirsToGetSize;
  
  QDict<KURL::List>        m_filesToCopy;
  
  QMap<QString,int>        m_size;
  QMap<QString,int>        m_filenum;
  QMap<QString,int>        m_subdirs;
  
  KURL                     m_baseURL;
  KURL                     m_dest;
  PreserveMode             m_pmode;
  KIO::CopyJob::CopyMode   m_mode;
  bool                     m_asMethod;
  bool                     m_showProgressInfo;
  
  State                    m_state;
  
  QTimer                   m_reportTimer;
  
  KURL                     m_current;
  QString                  m_currentDir;
  
  QStringList              m_dirStack;
};

#endif /* __VIRTUAL_COPY_JOB_H__ */
