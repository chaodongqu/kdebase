//kate: space-indent on; tab-width 2; indent-width 2; indent-mode cstyle; encoding UTF-8;
/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#include "shutdowndlg.h"
#include <qapplication.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qvbuttongroup.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qtimer.h>
#include <qstyle.h>
#include <qcombobox.h>
#include <qcursor.h>
#include <qmessagebox.h>
#include <qbuttongroup.h>
#include <qiconset.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qtooltip.h>
#include <qimage.h>
#include <qpainter.h>
#include <qfontmetrics.h>
#include <qregexp.h>

#include <klocale.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <kguiitem.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kwin.h>
#include <kuser.h>
#include <kpixmap.h>
#include <kimageeffect.h>
#include <kdialog.h>
#include <kseparator.h>
#include <kconfig.h>

#include <dcopclient.h>
#include <dcopref.h>

#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <dmctl.h>
#include <kaction.h>


#include <X11/Xlib.h>

#include "shutdowndlg.moc"

KSMShutdownFeedback * KSMShutdownFeedback::s_pSelf = 0L;

KSMShutdownFeedback::KSMShutdownFeedback()
: QWidget(0L, "feedbackwidget", WType_Popup)
{
  resize(0, 0);
  setShown(true);
}

//////

KSMShutdownDlg::KSMShutdownDlg(QWidget* parent, bool maysd, KApplication::ShutdownType sdtype)
: QDialog(parent, 0, TRUE, WType_Popup), targets(0)
// this is a WType_Popup on purpose. Do not change that! Not
// having a popup here has severe side effects.
{
  QVBoxLayout* vbox = new QVBoxLayout(this);

  QFrame* frame = new QFrame(this);
  frame->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
  frame->setLineWidth(style().pixelMetric(QStyle::PM_DefaultFrameWidth, frame));
  // we need to set the minimum size for the logout box, since it
  // gets too small if there isn't all options available
  frame->setMinimumWidth(100);
  vbox->addWidget(frame);
  vbox = new QVBoxLayout(frame, 2*KDialog::marginHint(), 2*KDialog::spacingHint());

  // slighty more space for the new logout
  int factor=2;

  QLabel* label = new QLabel(i18n("End Session for \"%1\"").arg(KUser().loginName()), frame);
  QFont fnt = label->font();
  fnt.setBold(true);
  fnt.setPointSize(fnt.pointSize() * 3/2);
  label->setFont(fnt);
  vbox->addWidget(label, 0, AlignHCenter);

  // for the basic layout, within this box either the ubuntu dialog or
  // standard konqy+buttons will be placed.
  QHBoxLayout* hbox = new QHBoxLayout(vbox, factor * KDialog::spacingHint());

  // from here on we have to adapt to the two different dialogs
  //QFrame* lfrm;
  QVBoxLayout* buttonlay;
  QFont btnFont;

  // konqy
  //lfrm = new QFrame(frame);
  //lfrm->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  //hbox->addWidget(lfrm, AlignCenter);

  buttonlay = new QVBoxLayout(hbox, factor * KDialog::spacingHint());
  buttonlay->setAlignment(Qt::AlignHCenter);

  // QLabel* icon = new QLabel(lfrm);
  // icon->setPixmap(UserIcon("shutdownkonq"));
  // lfrm->setFixedSize(icon->sizeHint());
  // icon->setFixedSize(icon->sizeHint());

  buttonlay->addStretch(1);
  // End session
  KPushButton* btnLogout = new KPushButton(KGuiItem(i18n("&End Current Session"), "undo"), frame);
  btnFont = btnLogout->font();
  buttonlay->addWidget(btnLogout);
  connect(btnLogout, SIGNAL(clicked()), SLOT(slotLogout()));

#ifdef COMPILE_HALBACKEND
  m_halCtx = NULL;
#endif

  if (maysd) {
    // respect lock on resume & disable suspend/hibernate settings
    // from power-manager
    KConfig config("power-managerrc");
    bool disableSuspend = config.readBoolEntry("disableSuspend", false);
    bool disableHibernate = config.readBoolEntry("disableHibernate", false);
    m_lockOnResume = config.readBoolEntry("lockOnResume", true);

    bool canSuspend = false;
    bool canHibernate = false;

#ifdef COMPILE_HALBACKEND
    // Query HAL for suspend/resume support
    m_halCtx = libhal_ctx_new();

    DBusError error;
    dbus_error_init(&error);
    m_dbusConn = dbus_connection_open_private(DBUS_SYSTEM_BUS, &error);
    if (!m_dbusConn) {
      dbus_error_free(&error);
      libhal_ctx_free(m_halCtx);
      m_halCtx = NULL;
    } else {
      dbus_bus_register(m_dbusConn, &error);
      if (dbus_error_is_set(&error)) {
        dbus_error_free(&error);
        libhal_ctx_free(m_halCtx);
        m_dbusConn = NULL;
        m_halCtx = NULL;
      } else {
        libhal_ctx_set_dbus_connection(m_halCtx, m_dbusConn);
        if (!libhal_ctx_init(m_halCtx, &error)) {
          if (dbus_error_is_set(&error)) dbus_error_free(&error);
          libhal_ctx_free(m_halCtx);
          m_dbusConn = NULL;
          m_halCtx = NULL;
        }
      }
    }

    if (m_halCtx) {
      if (libhal_device_get_property_bool(m_halCtx,
                                          "/org/freedesktop/Hal/devices/computer",
                                          "power_management.can_suspend",
                                          NULL)) {
        canSuspend = true;
      }

      if (libhal_device_get_property_bool(m_halCtx,
                                          "/org/freedesktop/Hal/devices/computer",
                                          "power_management.can_hibernate",
                                          NULL)) {
        canHibernate = true;
      }
    }
#endif

    // Shutdown
    KPushButton* btnHalt = new KPushButton(KGuiItem(i18n("&Turn Off Computer"), "exit"), frame);
    btnHalt->setFont(btnFont);
    buttonlay->addWidget(btnHalt);
    connect(btnHalt, SIGNAL(clicked()), SLOT(slotHalt()));
    if (sdtype == KApplication::ShutdownTypeHalt) btnHalt->setFocus();

    // Reboot
    KSMDelayedPushButton* btnReboot = new KSMDelayedPushButton(KGuiItem(i18n("&Restart Computer"), "reload"), frame);
    btnReboot->setFont(btnFont);
    buttonlay->addWidget(btnReboot);

    connect(btnReboot, SIGNAL(clicked()), SLOT(slotReboot()));
    if (sdtype == KApplication::ShutdownTypeReboot) btnReboot->setFocus();

    // this section is copied as-is into ubuntulogout as well
    int def, cur;
    if (DM().bootOptions(rebootOptions, def, cur)) {
      targets = new QPopupMenu(frame);
      if (cur == -1) cur = def;

      int index = 0;
      for (QStringList::ConstIterator it = rebootOptions.begin(); it != rebootOptions.end(); ++it, ++index) {
        QString label = (*it);
        label=label.replace('&',"&&");
        if (index == cur) {
          targets->insertItem(label + i18n("current option in boot loader", " (current)"), index);
        } else {
          targets->insertItem(label, index);
        }
      }

      btnReboot->setPopup(targets);
      connect(targets, SIGNAL(activated(int)), SLOT(slotReboot(int)));
    }

    if (canSuspend && !disableSuspend) {
      KPushButton* btnSuspend = new KPushButton(KGuiItem(i18n("&Suspend Computer"), "suspend"), frame);
      btnSuspend->setFont(btnFont);
      buttonlay->addWidget(btnSuspend);
      connect(btnSuspend, SIGNAL(clicked()), SLOT(slotSuspend()));
    }

    if (canHibernate && !disableHibernate) {
      KPushButton* btnHibernate = new KPushButton(KGuiItem(i18n("&Hibernate Computer"), "hibernate"), frame);
      btnHibernate->setFont(btnFont);
      buttonlay->addWidget(btnHibernate);
      connect(btnHibernate, SIGNAL(clicked()), SLOT(slotHibernate()));
    }

    buttonlay->addStretch(1);

    // Separator
    buttonlay->addWidget(new KSeparator(frame));

    // Back to Desktop
    KPushButton* btnBack = new KPushButton(KStdGuiItem::cancel(), frame);
    buttonlay->addWidget(btnBack);
    connect(btnBack, SIGNAL(clicked()), SLOT(reject()));
  } else {
    // finish the dialog correctly
    // Separator
    buttonlay->addWidget(new KSeparator(frame));

    // Back to Desktop
    KPushButton* btnBack = new KPushButton(KStdGuiItem::cancel(), frame);
    buttonlay->addWidget(btnBack);

    connect(btnBack, SIGNAL(clicked()), SLOT(reject()));
  }
}

KSMShutdownDlg::~KSMShutdownDlg() {
#ifdef COMPILE_HALBACKEND
  if (m_halCtx) {
    DBusError error;
    dbus_error_init(&error);
    libhal_ctx_shutdown(m_halCtx, &error);
    libhal_ctx_free(m_halCtx);
  }
#endif
}

void KSMShutdownDlg::slotLogout() {
  m_shutdownType = KApplication::ShutdownTypeNone;
  accept();
}

void KSMShutdownDlg::slotReboot() {
  // no boot option selected -> current
  m_bootOption = QString::null;
  m_shutdownType = KApplication::ShutdownTypeReboot;
  accept();
}

void KSMShutdownDlg::slotReboot(int opt) {
  if (int(rebootOptions.size()) > opt) m_bootOption = rebootOptions[opt];
  m_shutdownType = KApplication::ShutdownTypeReboot;
  accept();
}

void KSMShutdownDlg::slotHalt() {
  m_bootOption = QString::null;
  m_shutdownType = KApplication::ShutdownTypeHalt;
  accept();
}

void KSMShutdownDlg::slotSuspend() {
#ifdef COMPILE_HALBACKEND
  if (m_lockOnResume) {
    DCOPRef("kdesktop", "KScreensaverIface").send("lock");
  }

  if (m_dbusConn) {
    DBusMessage *msg = dbus_message_new_method_call(
                            "org.freedesktop.Hal",
                            "/org/freedesktop/Hal/devices/computer",
                            "org.freedesktop.Hal.Device.SystemPowerManagement",
                            "Suspend");

    int wakeup=0;
    dbus_message_append_args(msg, DBUS_TYPE_INT32, &wakeup, DBUS_TYPE_INVALID);
    dbus_connection_send(m_dbusConn, msg, NULL);
    dbus_message_unref(msg);
  }

  reject(); // continue on resume
#endif
}

void KSMShutdownDlg::slotHibernate() {
#ifdef COMPILE_HALBACKEND
  if (m_lockOnResume) {
    DCOPRef("kdesktop", "KScreensaverIface").send("lock");
  }

  if (m_dbusConn) {
    DBusMessage *msg = dbus_message_new_method_call(
                            "org.freedesktop.Hal",
                            "/org/freedesktop/Hal/devices/computer",
                            "org.freedesktop.Hal.Device.SystemPowerManagement",
                            "Hibernate");

    dbus_connection_send(m_dbusConn, msg, NULL);
    dbus_message_unref(msg);
  }

  reject(); // continue on resume
#endif
}

bool KSMShutdownDlg::confirmShutdown(bool maysd, KApplication::ShutdownType& sdtype, QString& bootOption) {
  kapp->enableStyles();
  KSMShutdownDlg* l = new KSMShutdownDlg(0,
                                         // KSMShutdownFeedback::self(),
                                         maysd, sdtype);

  // Show dialog (will save the background in showEvent)
  QSize sh = l->sizeHint();
  QRect rect = KGlobalSettings::desktopGeometry(QCursor::pos());

  l->move(rect.x() + (rect.width() - sh.width())/2,
          rect.y() + (rect.height() - sh.height())/2);
  bool result = l->exec();
  sdtype = l->m_shutdownType;
  bootOption = l->m_bootOption;

  delete l;

  kapp->disableStyles();
  return result;
}

KSMDelayedPushButton::KSMDelayedPushButton(const KGuiItem &item, QWidget *parent, const char *name)
: KPushButton(item, parent, name)
, pop(0)
, popt(0)
{
  connect(this, SIGNAL(pressed()), SLOT(slotPressed()));
  connect(this, SIGNAL(released()), SLOT(slotReleased()));
  popt = new QTimer(this);
  connect(popt, SIGNAL(timeout()), SLOT(slotTimeout()));
}

void KSMDelayedPushButton::setPopup(QPopupMenu *p) {
  pop = p;
  setIsMenuButton(p != 0);
}

void KSMDelayedPushButton::slotPressed() {
  if (pop) popt->start(QApplication::startDragTime());
}

void KSMDelayedPushButton::slotReleased() {
  popt->stop();
}

void KSMDelayedPushButton::slotTimeout() {
  QPoint bl = mapToGlobal(rect().bottomLeft());
  pop->popup(bl);
  popt->stop();
  setDown(false);
}

KSMPushButton::KSMPushButton(const KGuiItem &item, QWidget *parent, const char *name)
: KPushButton(item, parent, name)
, m_pressed(false)
{
  setDefault(false);
  setAutoDefault(false);
}

void KSMPushButton::keyPressEvent(QKeyEvent* e) {
  switch (e->key()) {
    case Key_Enter:
    case Key_Return:
    case Key_Space:
      m_pressed = TRUE;
      setDown(true);
      emit pressed();
      break;
    case Key_Escape:
      e->ignore();
      break;
    default:
      e->ignore();
  }

  QPushButton::keyPressEvent(e);
}

void KSMPushButton::keyReleaseEvent(QKeyEvent* e) {
  switch (e->key()) {
    case Key_Space:
    case Key_Enter:
    case Key_Return:
      if (m_pressed) {
        setDown(false);
        m_pressed = FALSE;
        emit released();
        emit clicked();
      }
      break;
    case Key_Escape:
      e->ignore();
      break;
    default:
      e->ignore();
  }
}

FlatButton::FlatButton(QWidget *parent, const char *name)
: QToolButton(parent, name/*, WNoAutoErase*/)
, m_pressed(false)
{
  init();
}

FlatButton::~FlatButton() {
}

void FlatButton::init() {
  setUsesTextLabel(true);
  setUsesBigPixmap(true);
  setAutoRaise(true);
  setTextPosition(QToolButton::Under);
  setFocusPolicy(QWidget::StrongFocus);
}

void FlatButton::keyPressEvent(QKeyEvent* e) {
  switch (e->key()) {
    case Key_Enter:
    case Key_Return:
    case Key_Space:
      m_pressed = TRUE;
      setDown(true);
      emit pressed();
      break;
    case Key_Escape:
      e->ignore();
      break;
    default:
      e->ignore();
  }

  QToolButton::keyPressEvent(e);
}

void FlatButton::keyReleaseEvent(QKeyEvent* e) {
  switch (e->key()) {
    case Key_Space:
    case Key_Enter:
    case Key_Return:
      if (m_pressed) {
        setDown(false);
        m_pressed = FALSE;
        emit released();
        emit clicked();
      }
      break;
    case Key_Escape:
      e->ignore();
      break;
    default:
      e->ignore();
  }
}
