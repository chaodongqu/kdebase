/*
 * mouse.h
 *
 * Copyright (c) 1997 Patrick Dowler dowler@morgul.fsh.uvic.ca
 *
 * Layout management, enhancements:
 * Copyright (c) 1999 Dirk A. Mueller <dmuell@gmx.net>
 *
 * SC/DC/AutoSelect/ChangeCursor:
 * Copyright (c) 2000 David Faure <faure@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef __MOUSECONFIG_H__
#define __MOUSECONFIG_H__

#include <qbuttongroup.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qlcdnumber.h>
#include <qpushbutton.h>
#include <qradiobutton.h>

#include <kapplication.h>

#include <kglobalsettings.h>
#include <knuminput.h>

#include <config.h>
#ifdef HAVE_LIBUSB
#include "logitechmouse.h"
#endif

#include <kcmodule.h>
#include "kmousedlg.h"
#include "themepage.h"

#define RIGHT_HANDED 0
#define LEFT_HANDED  1

class QCheckBox;
class QSlider;
class QTabWidget;

class MouseSettings
{
public:
  void save(KConfig *);
  void load(KConfig *);
  void apply(bool force=false);
public:
 int num_buttons;
 int middle_button;
 bool handedEnabled;
 bool m_handedNeedsApply;
 int handed;
 double accelRate;
 int thresholdMove;
 int doubleClickInterval;
 int dragStartTime;
 int dragStartDist;
 bool singleClick;
 int autoSelectDelay;
 int visualActivate;
 bool changeCursor;
 int wheelScrollLines;
 bool reverseScrollPolarity;

 #ifdef HAVE_LIBUSB
 // TODO: In Qt4, replace with a better container.
 QPtrList <LogitechMouse> logitechMouseList;
 #endif
};

class MouseConfig : public KCModule
{
  Q_OBJECT
public:
  MouseConfig(QWidget *parent=0, const char* name=0);
  ~MouseConfig();

  void save();
  void load();
  void load( bool useDefaults );
  void defaults();

private slots:

  void slotClick();
  /** No descriptions */
  void slotHandedChanged(int val);
  void slotScrollPolarityChanged();
  void checkAccess();
  void slotThreshChanged(int value);
  void slotDragStartDistChanged(int value);
  void slotWheelScrollLinesChanged(int value);

private:

  double getAccel();
  int getThreshold();
  int getHandedness();

  void setAccel(double);
  void setThreshold(int);
  void setHandedness(int);

  KDoubleNumInput *accel;
  KIntNumInput *thresh;
  KIntNumInput *doubleClickInterval;
  KIntNumInput *dragStartTime;
  KIntNumInput *dragStartDist;
  KIntNumInput *wheelScrollLines;

  QButtonGroup *handedBox;
//  QRadioButton *leftHanded, *rightHanded;
//  QCheckBox *doubleClick;
//  QCheckBox *cbAutoSelect;
  QLabel *lDelay;
//  QSlider *slAutoSelect;
//  QCheckBox *cbVisualActivate;
//  QCheckBox *cbCursor;
//  QCheckBox *cbLargeCursor;

  QTabWidget *tabwidget;
  QWidget *tab2;
  KMouseDlg* tab1;
  ThemePage* themetab;
  MouseSettings *settings;

 QCheckBox *mouseKeys;
  KIntNumInput *mk_delay, *mk_interval, *mk_time_to_max, *mk_max_speed,
    *mk_curve;


};

#endif

