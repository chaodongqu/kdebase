/*****************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 1999, 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

You can Freely distribute this program under the GNU General Public
License. See the file "COPYING" for the exact licensing terms.
******************************************************************/

#ifndef KWIN_CLIENT_H
#define KWIN_CLIENT_H

class QTimer;
class KProcess;
class KStartupInfoData;

namespace KWinInternal {
  class Workspace;
  class Client;
  class WinInfo;
  class SessionInfo;
  class Bridge;

  /** \class WinInfo
   * \brief NET WM Protocol handler class
   */
  class WinInfo : public NETWinInfo {
    private:
      typedef KWinInternal::Client Client; // because of NET::Client

    public:
      WinInfo(Client* c, Display * display, Window window, Window rwin, const unsigned long pr[], int pr_size);
      virtual void changeDesktop(int desktop);
      virtual void changeState(unsigned long state, unsigned long mask);

    private:
      Client * m_client;
  };

  /** \class Client
   * \brief The X11 window for KWin
   */
  class Client : public QObject, public KDecorationDefines {
    Q_OBJECT
    public:
      Client(Workspace *ws);

    private:
      // use Workspace::createClient()
      /**
       * NOTE: use destroyClient() or releaseWindow()
       */
      virtual ~Client();

    public:
      inline Window window() const { return client; }
      inline Window frameId() const { return frame; }
      inline Window wrapperId() const { return wrapper; }
      inline Window decorationId() const { return decoration != NULL ? decoration->widget()->winId() : None; }

      inline Workspace* workspace() const { return wspace; }
      inline const Client* transientFor() const { return transient_for; }
      inline Client* transientFor() { return transient_for; }
      inline bool isTransient() const { return transient_for_id != None; }
      inline bool groupTransient() const { return transient_for_id == workspace()->rootWin(); }

      /**
       * needed because verifyTransientFor() may set transient_for_id to root window,
       * if the original value has a problem (window doesn't exist, etc.)
       */
      inline bool wasOriginallyGroupTransient() const { return original_transient_for_id == workspace()->rootWin(); }

      ClientList mainClients() const; // call once before loop , is not indirect
      bool hasTransient(const Client* c, bool indirect) const;
      // is not indirect
      inline const ClientList& transients() const { return transients_list; }
      void checkTransient(Window w);
      Client* findModal();
      inline const Group* group() const { return in_group; }
      inline Group* group() { return in_group; }
      void checkGroup(Group* gr = NULL, bool force = false);
      void changeClientLeaderGroup(Group* gr);
      // prefer isXXX() instead
      NET::WindowType windowType(bool direct = false, int supported_types = SUPPORTED_WINDOW_TYPES_MASK) const;

      inline const WindowRules* rules() const { return &client_rules; }
      inline void removeRule(Rules* r) { client_rules.remove(r); }
      void setupWindowRules(bool ignore_temporary);
      void applyWindowRules();
      void updateWindowRules();

      inline QRect geometry() const { return frame_geometry; }
      inline QSize size() const { return frame_geometry.size(); }
      QSize minSize() const;
      QSize maxSize() const;
      inline QPoint pos() const { return frame_geometry.topLeft(); }
      inline QRect rect() const { return QRect(0, 0, width(), height()); }
      inline int x() const { return frame_geometry.x(); }
      inline int y() const { return frame_geometry.y(); }
      inline int width() const { return frame_geometry.width(); }
      inline int height() const { return frame_geometry.height(); }
      // inside of geometry()
      inline QPoint clientPos() const { return QPoint(border_left, border_top); }
      inline QSize clientSize() const { return client_size; }

      bool windowEvent(XEvent* e);
      virtual bool eventFilter(QObject* o, QEvent* e);

      bool manage(Window w, bool isMapped);

      void releaseWindow(bool on_shutdown = false);

      /// How to resize the window in order to obey constains (mainly aspect ratios)
      enum Sizemode {
        SizemodeAny,
        SizemodeFixedW, /// try not to affect width
        SizemodeFixedH, /// try not to affect height
        SizemodeMax /// try not to make it larger in either direction
      };

      // DON'T reorder - saved to config files !!!
      enum FullScreenMode {
        FullScreenNone,
        FullScreenNormal,
        FullScreenHack /// FullScreenHack - non-NETWM fullscreen (noborder,size of desktop)
      };

      QSize adjustedSize(const QSize&, Sizemode mode = SizemodeAny) const;
      QSize adjustedSize() const;

      inline QPixmap icon() const { return icon_pix; }
      inline QPixmap miniIcon() const { return miniicon_pix; }

      inline bool isActive() const { return active; }
      void setActive(bool, bool updateOpacity = true);

      /**
       * Returns the virtual desktop within the workspace() the client window is located in,
       * 0 if it isn't located on any special desktop (not mapped yet), or NET::OnAllDesktops.
       * Do not use desktop() directly, use isOnDesktop() instead.
       */
      inline int desktop() const { return desk; }

      void setDesktop(int);
      /**
       * Returns whether the client is on the virtual desktop \a d. This is always TRUE
       * for onAllDesktops clients.
       */
      inline bool isOnDesktop(int d) const { return desk == d || /*desk == 0 ||*/ isOnAllDesktops(); }

      bool isOnCurrentDesktop() const;
      inline bool isOnAllDesktops() const { return desk == NET::OnAllDesktops; }
      void setOnAllDesktops(bool set);

      bool isOnScreen(int screen) const;   // true if it's at least partially there
      int screen() const; // the screen where the center is

      // !isMinimized() && not hidden, i.e. normally visible on some virtual desktop
      inline bool isShown(bool shaded_is_shown) const {
        return !isMinimized() && (!isShade() || shaded_is_shown) && !hidden;
      }

      // true only for ShadeNormal
      inline bool isShade() const { return shade_mode == ShadeNormal; }

      //NOTE: prefer isShade()
      inline ShadeMode shadeMode() const { return shade_mode; }

      void setShade(ShadeMode mode);
      bool isShadeable() const;

      bool isTiled() const;
      void setTiled();
      void toggleTiled();

      inline bool isMinimized() const { return minimized; }
      bool isMaximizable() const;
      inline QRect geometryRestore() const { return geom_restore; }
      inline MaximizeMode maximizeModeRestore() const { return maxmode_restore; }
      inline MaximizeMode maximizeMode() const { return max_mode; }
      bool isMinimizable() const;
      void setMaximize(bool vertically, bool horizontally);

      void setFullScreen(bool set, bool user);
      inline bool isFullScreen() const { return fullscreen_mode != FullScreenNone; }
      bool isFullScreenable(bool fullscreen_hack = false) const;
      bool userCanSetFullScreen() const;
      QRect geometryFSRestore() const { return geom_fs_restore; } //NOTE: only for session saving
      int fullScreenMode() const { return fullscreen_mode; } //NOTE: only for session saving

      bool isUserNoBorder() const;
      void setUserNoBorder(bool set);
      bool userCanSetNoBorder() const;
      bool noBorder() const;

      /**
       * @p outs - from outside
       */
      inline bool skipTaskbar(bool outs=false) const { return outs?original_skip_taskbar:skip_taskbar; }

      void setSkipTaskbar(bool set, bool from_outside);

      inline bool skipPager() const { return skip_pager; }
      void setSkipPager(bool);

      inline bool keepAbove() const { return keep_above; }
      void setKeepAbove(bool);
      inline bool keepBelow() const { return keep_below; }
      void setKeepBelow(bool);
      Layer layer() const;
      Layer belongsToLayer() const;
      inline void invalidateLayer() { in_layer = UnknownLayer; }

      void setModal(bool modal);
      inline bool isModal() const { return modal; }

      // auxiliary functions, depend on the windowType
      bool wantsTabFocus() const;
      bool wantsInput() const;
      inline bool hasNETSupport() const { return info->hasNETSupport(); }
      bool isMovable() const;
      bool isDesktop() const;
      bool isDock() const;
      bool isToolbar() const;
      bool isTopMenu() const;
      bool isMenu() const;
      bool isNormalWindow() const; // normal as in 'NET::Normal or NET::Unknown non-transient'
      bool isDialog() const;
      bool isSplash() const;
      bool isUtility() const;
      /** Returns true for "special" windows and false for windows which are "normal"
       * (normal=window which has a border, can be moved by the user, can be closed, etc.)
       * true for Desktop, Dock, Splash, Override and TopMenu (and Toolbar??? - for now)
       * false for Normal, Dialog, Utility and Menu (and Toolbar??? - not yet) TODO
       */
      bool isSpecialWindow() const;

      bool isResizable() const;
      bool isCloseable() const; // may be closed by the user (may have a close button)

      void takeActivity(int flags, bool handled, allowed_t);   // takes ActivityFlags as arg (in utils.h)
      void takeFocus(allowed_t);
      void demandAttention(bool set = true);

      void setMask(const QRegion& r, int mode = Unsorted);
      QRegion mask() const;

      void updateDecoration(bool check_workspace_pos, bool force = false);
      void checkBorderSizes();

    // shape extensions
      inline bool shape() const { return is_shape; }
      void updateShape();

      void setGeometry(int x, int y, int w, int h, ForceGeometry_t force = NormalGeometrySet);
      inline void setGeometry(const QRect& r, ForceGeometry_t force = NormalGeometrySet) {
        setGeometry(r.x(), r.y(), r.width(), r.height(), force);
      }
      void move(int x, int y, ForceGeometry_t force = NormalGeometrySet);
      inline void move(const QPoint & p, ForceGeometry_t force = NormalGeometrySet) { move(p.x(), p.y(), force); }
      // plainResize() simply resizes
      void plainResize(int w, int h, ForceGeometry_t force = NormalGeometrySet);
      inline void plainResize(const QSize& s, ForceGeometry_t force = NormalGeometrySet) {
        plainResize(s.width(), s.height(), force);
      }
      // resizeWithChecks() resizes according to gravity, and checks workarea position
      void resizeWithChecks(int w, int h, ForceGeometry_t force = NormalGeometrySet);
      inline void resizeWithChecks(const QSize& s, ForceGeometry_t force = NormalGeometrySet) {
        resizeWithChecks(s.width(), s.height(), force);
      }
      void keepInArea(QRect area, bool partial = false);

      void growHorizontal();
      void shrinkHorizontal();
      void growVertical();
      void shrinkVertical();

      bool providesContextHelp() const;
      inline KShortcut shortcut() const { return _shortcut; }
      void setShortcut(const QString& cut);

      bool performMouseCommand(Options::MouseCommand, QPoint globalPos, bool handled = false);

      inline QCString windowRole() const { return window_role; }
      QCString sessionId();

      /** resourceName
       * NOTE: it is always lowercase
       */
      inline QCString resourceName() const { return resource_name; }

      /**
       * NOTE: it is always lowercase
       */
      inline QCString resourceClass() const { return resource_class; }

      QCString wmCommand();
      QCString wmClientMachine(bool use_localhost) const;
      Window   wmClientLeader() const;
      inline pid_t pid() const { return info->pid(); }

      QRect adjustedClientArea(const QRect& desktop, const QRect& area) const;

      inline Colormap colormap() const { return cmap; }

      // updates visibility depending on being shaded, virtual desktop, etc.
      void updateVisibility();
      // hides a client - basically like minimize, but without effects, it's simply hidden
      void hideClient(bool hide);

      QString caption(bool full = true) const;
      void updateCaption();

      void keyPressEvent(uint key_code);   // FRAME ??
      void updateMouseGrab();
      inline Window moveResizeGrabWindow() const { return move_resize_grab_window; }

      const QPoint calculateGravitation(bool invert, int gravity = 0) const;   // FRAME public?

      void NETMoveResize(int x_root, int y_root, NET::Direction direction);
      void NETMoveResizeWindow(int flags, int x, int y, int width, int height);
      void restackWindow(Window above, int detail, NET::RequestSource source, Time timestamp, bool send_event = false);

      void gotPing(Time timestamp);

      static QCString staticWindowRole(WId);
      static QCString staticSessionId(WId);
      static QCString staticWmCommand(WId);
      static QCString staticWmClientMachine(WId);
      static Window staticWmClientLeader(WId);

      void checkWorkspacePosition();
      void updateUserTime(Time time = CurrentTime);
      Time userTime() const;
      inline bool hasUserTimeSupport() const { return info->userTime() != -1U; }
      inline bool ignoreFocusStealing() const { return ignore_focus_stealing; }

      /**
       * NOTE: does 'delete c;'
       */
      static void deleteClient(Client* c, allowed_t);

      static bool resourceMatch(const Client* c1, const Client* c2);
      static bool belongToSameApplication(const Client* c1, const Client* c2, bool active_hack = false);
      static void readIcons(Window win, QPixmap* icon, QPixmap* miniicon);

      void minimize(bool avoid_animation = false);
      void unminimize(bool avoid_animation = false);
      void closeWindow();
      void killWindow();
      void maximize(MaximizeMode);
      void toggleShade();
      void showContextHelp();
      void cancelShadeHover();
      void cancelAutoRaise();
      void destroyClient();
      void checkActiveModal();
      void setOpacity(bool translucent, uint opacity = 0);
      void setShadowSize(uint shadowSize);
      void updateOpacity();
      void updateShadowSize();
      bool hasCustomOpacity() { return custom_opacity; }
      void setCustomOpacityFlag(bool custom=true);
      bool getWindowOpacity();
      int opacityPercentage();
      void checkAndSetInitialRuledOpacity();
      uint ruleOpacityInactive();
      uint ruleOpacityActive();
      unsigned int opacity();
      inline bool isBMP() { return isBMP_; }
      inline void setBMP(bool b) { isBMP_ = b; }
      bool touches(const Client* c);
      void setShapable(bool b);
      bool hasStrut() const;

    private slots:
      void autoRaise();
      void shadeHover();
      void shortcutActivated();

    private:
      friend class Bridge; // FRAME
      virtual void processMousePressEvent(QMouseEvent* e);

      Position mousePosition(const QPoint&) const;
      void setCursor(Position m);
      void setCursor(const QCursor& c);

      void  animateMinimizeOrUnminimize(bool minimize);
      QPixmap animationPixmap(int w);
    // transparent stuff
      void drawbound(const QRect& geom);
      void clearbound();
      void doDrawbound(const QRect& geom, bool clear);

    // handlers for X11 events
      bool mapRequestEvent(XMapRequestEvent* e);
      void unmapNotifyEvent(XUnmapEvent*e);
      void destroyNotifyEvent(XDestroyWindowEvent*e);
      void configureRequestEvent(XConfigureRequestEvent* e);
      void propertyNotifyEvent(XPropertyEvent* e);
      void clientMessageEvent(XClientMessageEvent* e);
      void enterNotifyEvent(XCrossingEvent* e);
      void leaveNotifyEvent(XCrossingEvent* e);
      void focusInEvent(XFocusInEvent* e);
      void focusOutEvent(XFocusOutEvent* e);

      bool buttonPressEvent(Window w, int button, int state, int x, int y, int x_root, int y_root);
      bool buttonReleaseEvent(Window w, int button, int state, int x, int y, int x_root, int y_root);
      bool motionNotifyEvent(Window w, int state, int x, int y, int x_root, int y_root);

      void processDecorationButtonPress(int button, int state, int x, int y, int x_root, int y_root);

    private slots:
      void pingTimeout();
      void processKillerExited();
      void demandAttentionKNotify();

      // ICCCM 4.1.3.1, 4.1.4 , NETWM 2.5.1
    private:
      void setMappingState(int s);
      inline int mappingState() const { return mapping_state; }
      inline bool isIconicState() const { return mapping_state == IconicState; }
      inline bool isNormalState() const { return mapping_state == NormalState; }

      /**
       * @return false if this client is not yet managed
       */
      inline bool isManaged() const { return mapping_state != WithdrawnState; }

      void updateAllowedActions(bool force = false);
      QSize sizeForClientSize(const QSize&, Sizemode mode = SizemodeAny, bool noframe = false) const;
      void changeMaximize(bool horizontal, bool vertical, bool adjust);
      void checkMaximizeGeometry();
      int checkFullScreenHack(const QRect& geom) const;   // 0 - none, 1 - one xinerama screen, 2 - full area
      void updateFullScreenHack(const QRect& geom);
      void getWmNormalHints();
      void getMotifHints();
      void getIcons();
      void getWmClientLeader();
      void getWmClientMachine();
      void fetchName();
      void fetchIconicName();
      QString readName() const;
      void setCaption(const QString& s, bool force=false);
      bool hasTransientInternal(const Client* c, bool indirect, ConstClientList& set) const;
      void finishWindowRules();
      void setShortcutInternal(const KShortcut& cut);

      void updateWorkareaDiffs();
      void checkDirection(int new_diff, int old_diff, QRect& rect, const QRect& area);
      static int computeWorkareaDiff(int left, int right, int a_left, int a_right);
      void configureRequest(int value_mask, int rx, int ry, int rw, int rh, int gravity, bool from_tool);
      NETExtendedStrut strut() const;
      int checkShadeGeometry(int w, int h);
      void postponeGeometryUpdates(bool postpone);

      bool startMoveResize();
      void finishMoveResize(bool cancel);
      void leaveMoveResize();
      void checkUnrestrictedMoveResize();
      void handleMoveResize(int x, int y, int x_root, int y_root);
      void positionGeometryTip();
      void grabButton(int mod);
      void ungrabButton(int mod);
      void resetMaximize();
      void resizeDecoration(const QSize& s);
      void setDecoHashProperty(uint topHeight, uint rightWidth, uint bottomHeight, uint leftWidth);
      void unsetDecoHashProperty();

      void pingWindow();
      void killProcess(bool ask, Time timestamp = CurrentTime);
      void updateUrgency();
      static void sendClientMessage(Window w, Atom a, Atom protocol, long data1 = 0, long data2 = 0, long data3 = 0);

      void embedClient(Window w, const XWindowAttributes &attr);
      void detectNoBorder();
      void detectShapable();
      void destroyDecoration();
      void updateFrameExtents();

      void rawShow(); // just shows it
      void rawHide(); // just hides it

      Time readUserTimeMapTimestamp(const KStartupInfoId* asn_id, const KStartupInfoData* asn_data, bool session) const;
      Time readUserCreationTime() const;
      static bool sameAppWindowRoleMatch(const Client* c1, const Client* c2, bool active_hack);
      void startupIdChanged();

      Window client;
      Window wrapper;
      Window frame;
      KDecoration* decoration;
      Workspace* wspace;
      Bridge* bridge;
      int desk;
      bool buttonDown;
      bool moveResizeMode;
      bool move_faked_activity;
      Window move_resize_grab_window;
      bool unrestrictedMoveResize;
      bool isMove() const { return moveResizeMode && mode == PositionCenter; }
      bool isResize() const { return moveResizeMode && mode != PositionCenter; }

      Position mode;
      QPoint moveOffset;
      QPoint invertedMoveOffset;
      QRect moveResizeGeom;
      QRect initialMoveResizeGeom;
      XSizeHints  xSizeHint;
      void sendSyntheticConfigureNotify();
      int mapping_state;
      void readTransient();
      Window verifyTransientFor(Window transient_for, bool set);
      void addTransient(Client* cl);
      void removeTransient(Client* cl);
      void removeFromMainClients();
      void cleanGrouping();
      void checkGroupTransients();
      void setTransient(Window new_transient_for_id);
      Client* transient_for;
      Window transient_for_id;
      Window original_transient_for_id;
      ClientList transients_list; // SELI make this ordered in stacking order?
      ShadeMode shade_mode;
      uint active :1;
      uint deleting : 1; /// true when doing cleanup and destroying the client
      uint keep_above : 1; /// NET::KeepAbove (was stays_on_top)
      uint is_shape :1;
      uint skip_taskbar :1;
      uint original_skip_taskbar :1; /// unaffected by KWin
      uint Pdeletewindow :1; /// does the window understand the DeleteWindow protocol?
      uint Ptakefocus :1; /// does the window understand the TakeFocus protocol?
      uint Ptakeactivity : 1; /// does it support _NET_WM_TAKE_ACTIVITY
      uint Pcontexthelp : 1; /// does the window understand the ContextHelp protocol?
      uint Pping : 1; /// does it support _NET_WM_PING?
      uint input :1; /// does the window want input in its wm_hints
      uint skip_pager : 1;
      uint motif_noborder : 1;
      uint motif_may_resize : 1;
      uint motif_may_move :1;
      uint motif_may_close : 1;
      uint keep_below : 1; /// NET::KeepBelow
      uint minimized : 1;
      uint hidden : 1; /// Forcibly hidden by calling hide()
      uint modal : 1; /// NET::Modal
      uint noborder : 1;
      uint user_noborder : 1;
      uint urgency : 1; /// XWMHints, UrgencyHint
      uint ignore_focus_stealing : 1; /// don't apply focus stealing prevention to this client
      uint demands_attention : 1;
      uint is_tiled : 1;  /// becomes 1 when client is in tiled mode
      WindowRules client_rules;
      void getWMHints();
      void readIcons();
      void getWindowProtocols();
      QPixmap icon_pix;
      QPixmap miniicon_pix;
      QCursor cursor;
      FullScreenMode fullscreen_mode;
      MaximizeMode max_mode;
      QRect geom_restore;
      QRect geom_fs_restore;
      MaximizeMode maxmode_restore;
      int workarea_diff_x, workarea_diff_y;
      WinInfo* info;
      QTimer* autoRaiseTimer;
      QTimer* shadeHoverTimer;
      Colormap cmap;
      QCString resource_name;
      QCString resource_class;
      QCString client_machine;
      QString cap_normal, cap_iconic, cap_suffix;
      WId wmClientLeaderWin;
      QCString window_role;
      Group* in_group;
      Window window_group;
      Layer in_layer;
      QTimer* ping_timer;
      KProcess* process_killer;
      Time ping_timestamp;
      Time user_time;
      unsigned long allowed_actions;
      QRect frame_geometry;
      QSize client_size;
      int postpone_geometry_updates; /// >0 - new geometry is remembered, but not actually set
      bool pending_geometry_update;
      bool shade_geometry_change;
      int border_left, border_right, border_top, border_bottom;
      QRegion _mask;
      static bool check_active_modal; /// \see Client::checkActiveModal()
      KShortcut _shortcut;
      friend struct FetchNameInternalPredicate;
      friend struct CheckIgnoreFocusStealingProcedure;
      friend struct ResetupRulesProcedure;
      friend class GeometryUpdatesPostponer;
      // void show() { assert(false); }   // SELI remove after Client is no longer QWidget
      // void hide() { assert(false); }
      uint opacity_;
      uint savedOpacity_;
      bool custom_opacity;
      uint rule_opacity_active; /// translucency rules
      uint rule_opacity_inactive; /// dto.
      // int shadeOriginalHeight;
      bool isBMP_;
      QTimer* demandAttentionKNotifyTimer;

      friend bool performTransiencyCheck();
  }; /* class Client */

  /** \class GeometryUpdatesPostponer
   * \brief helper for Client::postponeGeometryUpdates() being called in pairs (true/false)
   */
  class GeometryUpdatesPostponer {
    public:
      GeometryUpdatesPostponer(Client* c) : cl(c) { cl->postponeGeometryUpdates(true); }
      ~GeometryUpdatesPostponer() { cl->postponeGeometryUpdates(false); }

    private:
      Client* cl;
  };

  KWIN_PROCEDURE(CheckIgnoreFocusStealingProcedure, cl->ignore_focus_stealing = options->checkIgnoreFocusStealing(cl));

#ifdef NDEBUG
  inline kndbgstream& operator<<(kndbgstream& stream, const Client*) { return stream; }
  inline kndbgstream& operator<<(kndbgstream& stream, const ClientList&) { return stream; }
  inline kndbgstream& operator<<(kndbgstream& stream, const ConstClientList&) { return stream; }
#else
  kdbgstream& operator<<(kdbgstream& stream, const Client*);
  kdbgstream& operator<<(kdbgstream& stream, const ClientList&);
  kdbgstream& operator<<(kdbgstream& stream, const ConstClientList&);
#endif

  KWIN_COMPARE_PREDICATE(WindowMatchPredicate, Window, cl->window() == value);
  KWIN_COMPARE_PREDICATE(FrameIdMatchPredicate, Window, cl->frameId() == value);
  KWIN_COMPARE_PREDICATE(WrapperIdMatchPredicate, Window, cl->wrapperId() == value);

}; /* namespace KWinInternal */

#endif /* KWIN_CLIENT_H */
