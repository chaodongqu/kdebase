//
// C++ Interface: common
//
// Description:
//  Contains common includes and code objects required by KWin
//
// Author: The package is maintained by Iegor <rmtdev@gmail.com>, (C) 2014
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef KWIN_COMMON_HEADER_H
#define KWIN_COMMON_HEADER_H

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
// #ifndef KCMRULES
#include <assert.h>
// #endif
#include <math.h>

#ifndef KCMRULES

// #ifndef NOXINCLUDES
// #include <X11/Xlib.h>
// #include <X11/Xutil.h>
// #include <X11/Xatom.h>
// #include <X11/keysym.h>
// #include <X11/keysymdef.h>
// #include <X11/cursorfont.h>
// #include <X11/extensions/shape.h>
// #include <X11/SM/SMlib.h>
// #include <X11/Xresource.h>
// #endif /* NOXINCLUDES */

extern Atom qt_wm_state;
extern Time qt_x_time;
extern Atom qt_window_role;
extern Atom qt_sm_client_id;

#endif /* KCMRULES */

/* KWin */
#include <KWinInterface.h>
#include <kdecoration_plugins_p.h>
#include <kdecoration_p.h>

#ifndef KCMRULES

#include <core/atoms.hpp>

#endif /* KCMRULES */

#include <core/utils.hpp>
#include <core/placement.hpp>
#include <core/options.hpp>
#include <core/rules.hpp>

#ifndef KCMRULES

#include <core/sm.hpp>
#include <core/group.hpp>
#include <core/tabbox.hpp>
#include <core/workspace.hpp>
#include <core/bridge.hpp>
#include <core/client.hpp>
#include <core/plugins.hpp>
#include <core/geometrytip.hpp>
#include <core/notifications.hpp>
#include <core/popupinfo.hpp>
#include <core/killwindow.hpp>
#include <core/main.hpp>

#endif /* KCMRULES */

#endif /* KWIN_COMMON_HEADER_H */
