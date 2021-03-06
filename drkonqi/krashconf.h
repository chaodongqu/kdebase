/*****************************************************************
 * drkonqi - The KDE Crash Handler
 *
 * Copyright (C) 2000-2003 Hans Petter Bieker <bieker@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************/

#ifndef KRASHCONF_H
#define KRASHCONF_H

#include <kaboutdata.h>
#include <qstring.h>
#include <qobject.h>

#include "krashdcopinterface.h"

class KrashConfig : public QObject, public KrashDCOPInterface
{
  Q_OBJECT

public:
  KrashConfig();
  virtual ~KrashConfig();

k_dcop:
  virtual QString programName() const { return m_aboutData->programName(); };
  virtual QCString appName() const { return m_aboutData->appName(); };
  virtual int signalNumber() const { return m_signalnum; };
  virtual int pid() const { return m_pid; };
  virtual bool startedByKdeinit() const { return m_startedByKdeinit; };
  virtual bool safeMode() const { return m_safeMode; };
  virtual QString signalName() const { return m_signalName; };
  virtual QString signalText() const { return m_signalText; };
  virtual QString whatToDoText() const { return m_whatToDoText; }
  virtual QString errorDescriptionText() const { return m_errorDescriptionText; };

  virtual ASYNC registerDebuggingApplication(const QString& launchName);

public:
  QString debugger() const { return m_debugger; }
  QString debuggerBatch() const { return m_debuggerBatch; }
  QString tryExec() const { return m_tryExec; }
  QString backtraceCommand() const { return m_backtraceCommand; }
  QString removeFromBacktraceRegExp() const { return m_removeFromBacktraceRegExp; }
  QString invalidStackFrameRegExp() const { return m_invalidStackFrameRegExp; }
  QString frameRegExp() const { return m_frameRegExp; }
  QString neededInValidBacktraceRegExp() const { return m_neededInValidBacktraceRegExp; }
  QString kcrashRegExp() const { return m_kcrashRegExp; }
  bool showBacktrace() const { return m_showbacktrace; };
  bool showDebugger() const { return m_showdebugger && !m_debugger.isNull(); };
  bool showBugReport() const { return m_showbugreport; };
  bool disableChecks() const { return m_disablechecks; };
  const KAboutData *aboutData() const { return m_aboutData; }
  QString execName() const { return m_execname; }

  void expandString(QString &str, bool shell, const QString &tempFile = QString::null) const;

  void acceptDebuggingApp();

signals:
  void newDebuggingApplication(const QString& launchName);

private:
  void readConfig();

private:
  KAboutData *m_aboutData;
  int m_pid;
  int m_signalnum;
  bool m_showdebugger;
  bool m_showbacktrace;
  bool m_showbugreport;
  bool m_startedByKdeinit;
  bool m_safeMode;
  bool m_disablechecks;
  QString m_signalName;
  QString m_signalText;
  QString m_whatToDoText;
  QString m_errorDescriptionText;
  QString m_execname;

  QString m_debugger;
  QString m_debuggerBatch;
  QString m_tryExec;
  QString m_backtraceCommand;
  QString m_removeFromBacktraceRegExp;
  QString m_invalidStackFrameRegExp;
  QString m_frameRegExp;
  QString m_neededInValidBacktraceRegExp;
  QString m_kcrashRegExp;
};

#endif
