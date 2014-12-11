//
// C++ Interface: kdecommon
//
// Description:
//  Contains all common KDE headers included for usage within other files
//
// Author: The package is maintained by Iegor <rmtdev@gmail.com>, (C) 2014
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef KWIN_KDECOMMON_HEADER
#define KWIN_KDECOMMON_HEADER

/* KDE */
#include <kapplication.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kglobalaccel.h>
#include <kstartupinfo.h>
#include <kprocess.h>
#include <fixx11h.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kwin.h>
#include <netwm_def.h>
#include <netwm.h>
#include <kdebug.h>

#ifndef KCMRULES

#include <kxerrorhandler.h>

#endif /* KCMRULES */

#include <kxmessages.h>
#include <kmanagerselection.h>
#include <kshortcutdialog.h>

#endif /* KWIN_KDECOMMON_HEADER */