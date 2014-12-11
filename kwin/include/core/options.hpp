/*****************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 1999, 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

You can Freely distribute this program under the GNU General Public
License. See the file "COPYING" for the exact licensing terms.
******************************************************************/

#ifndef KWIN_OPTIONS_H
#define KWIN_OPTIONS_H

namespace KWinInternal {
  class Client;

  class Options : public KDecorationOptions {
    public:

      Options();
      ~Options();

      virtual unsigned long updateSettings();

      /**
       * Different focus policies:
       * <ul>
       *
       * <li>ClickToFocus - Clicking into a window activates it. This is also the default.
       *
       * <li>FocusFollowsMouse - Moving the mouse pointer actively onto a normal window activates it.
       * For convenience, the desktop and windows on the dock are excluded. They require clicking.
       *
       * <li>FocusUnderMouse - The window that happens to be under the mouse pointer becomes active.
       * The invariant is: no window can have focus that is not under the mouse. This also means that
       * Alt-Tab won't work properly and popup dialogs are usually unsable with the keyboard.
       * Note that the desktop and windows on the dock are excluded for convenience. They get focus only when
       * clicking on it.
       *
       * <li>FocusStrictlyUnderMouse - this is even worse than FocusUnderMouse. Only the window under the mouse pointer
       * is active. If the mouse points nowhere, nothing has the focus. If the mouse points onto the desktop,
       * the desktop has focus. The same holds for windows on the dock.
       *
       * Note that FocusUnderMouse and FocusStrictlyUnderMouse are not particulary useful. They are only provided for
       * old-fashined die-hard UNIX people ;-)
       *
       * </ul>
       */
      enum FocusPolicy { ClickToFocus, FocusFollowsMouse, FocusUnderMouse, FocusStrictlyUnderMouse };
      FocusPolicy focusPolicy;

      bool clickRaise; /// Whether clicking on a window raises it in FocusFollowsMouse mode or not
      bool autoRaise; /// Whether autoraise is enabled FocusFollowsMouse mode or not
      int autoRaiseInterval; /// Autoraise interval
      bool delayFocus; /// Whether delay focus is enabled or not
      int delayFocusInterval; /// Delayed focus interval
      bool shadeHover; /// Whether shade hover is enabled or not
      int shadeHoverInterval; /// Shade hover interval

      /**
       * Different Alt-Tab-Styles:
       * <ul>
       *
       * <li> KDE - the recommended KDE style. Alt-Tab opens a nice icon box that makes it easy to select the window
       * you want to tab to. The order automatically adjusts to the most recently used windows. Note that KDE style
       * does not work with the FocusUnderMouse and FocusStrictlyUnderMouse focus policies.
       * Choose ClickToFocus or FocusFollowsMouse instead.
       *
       * <li> CDE - the old-fashion CDE style. Alt-Tab cycles between the windows in static order. The current window
       * gets raised, the previous window gets lowered.
       *
       * </ul>
       */
      enum AltTabStyle { KDE, CDE };
      AltTabStyle altTabStyle;

      /// Whether to see Xinerama screens separately for focus (in Alt+Tab, when activating next client)
      bool separateScreenFocus;
      bool activeMouseScreen; /// whether active Xinerama screen is the one with mouse (or with the active window)

      /**
       * Xinerama options
       */
      bool xineramaEnabled;
      bool xineramaPlacementEnabled;
      bool xineramaMovementEnabled;
      bool xineramaMaximizeEnabled;
      bool xineramaFullscreenEnabled;

      // number, or -1 = active screen (Workspace::activeScreen())
      int xineramaPlacementScreen;

      /**
       * MoveResizeMode, either Tranparent or Opaque.
       */
      enum MoveResizeMode { Transparent, Opaque };

      MoveResizeMode resizeMode;
      MoveResizeMode moveMode;

      static MoveResizeMode stringToMoveResizeMode( const QString& s );
      static const char* moveResizeModeToString( MoveResizeMode mode );

      Placement::Policy placement;

      bool focusPolicyIsReasonable() { return focusPolicy == ClickToFocus || focusPolicy == FocusFollowsMouse; }

      bool animateShade; /// Whether we animate the shading of windows to titlebar or not
      int borderSnapZone; /// The size of the zone that triggers snapping on desktop borders
      int windowSnapZone; /// The number of animation steps (would this be general?)
      bool snapOnlyWhenOverlapping; /// Snap only when windows will overlap
      bool animateMinimize; /// Whether we animate the minimization of windows or not
      int animateMinimizeSpeed; /// Animation speed (0 .. 10)
      bool showDesktopIsMinimizeAll;
      bool rollOverDesktops; /// Whether or not we roll over to the other edge when switching desktops past the edge
      int focusStealingPreventionLevel; /// 0 - 4 , see Workspace::allowClientActivation()
      QStringList ignorePositionClasses; /// List of window classes to ignore PPosition size hint

      bool checkIgnoreFocusStealing( const Client* c );

      WindowOperation operationTitlebarDblClick() { return OpTitlebarDblClick; }

      enum MouseCommand {
        MouseRaise, MouseLower, MouseOperationsMenu, MouseToggleRaiseAndLower,
        MouseActivateAndRaise, MouseActivateAndLower, MouseActivate,
        MouseActivateRaiseAndPassClick, MouseActivateAndPassClick,
        MouseMove, MouseUnrestrictedMove,
        MouseActivateRaiseAndMove, MouseActivateRaiseAndUnrestrictedMove,
        MouseResize, MouseUnrestrictedResize,
        MouseShade, MouseSetShade, MouseUnsetShade,
        MouseMaximize, MouseRestore, MouseMinimize,
        MouseNextDesktop, MousePreviousDesktop,
        MouseAbove, MouseBelow,
        MouseOpacityMore, MouseOpacityLess,
        MouseNothing
      };

      enum MouseWheelCommand {
        MouseWheelRaiseLower, MouseWheelShadeUnshade, MouseWheelMaximizeRestore,
        MouseWheelAboveBelow, MouseWheelPreviousNextDesktop,
        MouseWheelChangeOpacity,
        MouseWheelNothing
      };

      MouseCommand operationTitlebarMouseWheel(int delta) { return wheelToMouseCommand(CmdTitlebarWheel, delta); }
      MouseCommand operationWindowMouseWheel(int delta) { return wheelToMouseCommand(CmdAllWheel, delta); }

      MouseCommand commandActiveTitlebar1() { return CmdActiveTitlebar1; }
      MouseCommand commandActiveTitlebar2() { return CmdActiveTitlebar2; }
      MouseCommand commandActiveTitlebar3() { return CmdActiveTitlebar3; }
      MouseCommand commandInactiveTitlebar1() { return CmdInactiveTitlebar1; }
      MouseCommand commandInactiveTitlebar2() { return CmdInactiveTitlebar2; }
      MouseCommand commandInactiveTitlebar3() { return CmdInactiveTitlebar3; }
      MouseCommand commandWindow1() { return CmdWindow1; }
      MouseCommand commandWindow2() { return CmdWindow2; }
      MouseCommand commandWindow3() { return CmdWindow3; }
      MouseCommand commandAll1() { return CmdAll1; }
      MouseCommand commandAll2() { return CmdAll2; }
      MouseCommand commandAll3() { return CmdAll3; }
      uint keyCmdAllModKey() { return CmdAllModKey; }

      static WindowOperation windowOperation(const QString &name, bool restricted);
      static MouseCommand mouseCommand(const QString &name, bool restricted);
      static MouseWheelCommand mouseWheelCommand(const QString &name);

      /**
       * @returns true if the Geometry Tip should be shown during a window move/resize.
       * @since 3.2
       */
      bool showGeometryTip();

      enum { ElectricDisabled = 0, ElectricMoveOnly = 1, ElectricAlways = 2 };

      /**
       * @returns true if electric borders are enabled. With electric borders
       * you can change desktop by moving the mouse pointer towards the edge
       * of the screen
       */
      int electricBorders();

      /**
       * @returns the activation delay for electric borders in milliseconds.
       */
      int electricBorderDelay();

      bool topMenuEnabled() const { return topmenus; }
      bool desktopTopMenu() const { return desktop_topmenu; }

      int killPingTimeout; /// timeout before non-responding application will be killed after attempt to close
      bool hideUtilityWindowsForInactive; /// Whether to hide utility windows for inactive applications

      // Translucency settings:
      bool useTranslucency;
      bool translucentActiveWindows;
      uint activeWindowOpacity;
      bool translucentInactiveWindows;
      uint inactiveWindowOpacity;
      bool translucentMovingWindows;
      uint movingWindowOpacity;
      bool removeShadowsOnResize;
      bool removeShadowsOnMove;
      bool translucentDocks;
      uint dockOpacity;
      bool keepAboveAsActive;
      bool useTitleMenuSlider;
      uint activeWindowShadowSize;
      uint inactiveWindowShadowSize;
      uint dockShadowSize;
      bool onlyDecoTranslucent;
      bool resetKompmgr;
      bool tabboxOutline;

    private:
      WindowOperation OpTitlebarDblClick;

      // Mouse bindings:
      MouseCommand CmdActiveTitlebar1;
      MouseCommand CmdActiveTitlebar2;
      MouseCommand CmdActiveTitlebar3;
      MouseCommand CmdInactiveTitlebar1;
      MouseCommand CmdInactiveTitlebar2;
      MouseCommand CmdInactiveTitlebar3;
      MouseWheelCommand CmdTitlebarWheel;
      MouseCommand CmdWindow1;
      MouseCommand CmdWindow2;
      MouseCommand CmdWindow3;
      MouseCommand CmdAll1;
      MouseCommand CmdAll2;
      MouseCommand CmdAll3;
      MouseWheelCommand CmdAllWheel;
      uint CmdAllModKey;

      int electric_borders;
      int electric_border_delay;
      bool show_geometry_tip;
      bool topmenus;
      bool desktop_topmenu;
      QStringList ignoreFocusStealingClasses; /// List of window classes for which not to use focus stealing prevention

      MouseCommand wheelToMouseCommand(MouseWheelCommand com, int delta);
  };
  extern Options* options;
}; /* namespace KWinInternal */

#endif /* KWIN_OPTIONS_H */
