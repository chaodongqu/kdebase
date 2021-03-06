/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kcmdisplay.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 * with minor additions and based on ideas from
 * Torsten Rahn <torsten@kde.org>
 *
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#ifndef __icons_h__
#define __icons_h__

#include <qcolor.h>
#include <qimage.h>
#include <qvaluelist.h>

#include <kcmodule.h>
#include <kdialogbase.h>

class QCheckBox;
class QColor;
class QComboBox;
class QGridLayout;
class QGroupBox;
class QIconView;
class QLabel;
class QListBox;
class QListView;
class QPushButton;
class QSlider;
class QTabWidget;
class QWidget;

class KColorButton;
class KConfig;
class KIconEffect;
class KIconLoader;
class KIconTheme;

struct Effect 
{
    int type;
    float value;
    QColor color;
    QColor color2;
    bool transparant;
};


/**
 * The General Icons tab in kcontrol.
 */
class KIconConfig: public KCModule
{
    Q_OBJECT

public:
    KIconConfig(QWidget *parent, const char *name=0);
    ~KIconConfig();

    virtual void load();
    virtual void load( bool useDefaults );
    virtual void save();
    virtual void defaults();
    void preview();

private slots:
    void slotEffectSetup0() { EffectSetup(0); }
    void slotEffectSetup1() { EffectSetup(1); }
    void slotEffectSetup2() { EffectSetup(2); }
    
    void slotUsage(int index);
    void slotSize(int index);
    void slotDPCheck(bool check);
    void slotAnimatedCheck(bool check);

private:
    void preview(int i);
    void EffectSetup(int state);
    QPushButton *addPreviewIcon(int i, const QString &str, QWidget *parent, QGridLayout *lay);
    void init();
    void initDefaults();
    void read();
    void apply();


    bool mbDP[6], mbChanged[6], mbAnimated[6];
    int mSizes[6];
    QValueList<int> mAvSizes[6];

    Effect mEffects[6][3];
    Effect mDefaultEffect[3];
    
    int mUsage;
    QString mTheme, mExample;
    QStringList mGroups, mStates;

    KIconEffect *mpEffect;
    KIconTheme *mpTheme;
    KIconLoader *mpLoader;
    KConfig *mpConfig;

    typedef QLabel *QLabelPtr;
    QLabelPtr mpPreview[3];

    QListBox *mpUsageList;
    QComboBox *mpSizeBox;
    QCheckBox *mpDPCheck, *wordWrapCB, *underlineCB, *mpAnimatedCheck;
    QTabWidget *m_pTabWidget;
    QWidget *m_pTab1;                                    
};

class KIconEffectSetupDialog: public KDialogBase
{
    Q_OBJECT
     
public:
    KIconEffectSetupDialog(const Effect &, const Effect &,
                           const QString &, const QImage &,
			   QWidget *parent=0L, char *name=0L);
    ~KIconEffectSetupDialog();
    Effect effect() { return mEffect; }

protected:
    void preview();
    void init();

protected slots:
    void slotEffectValue(int value);
    void slotEffectColor(const QColor &col);
    void slotEffectColor2(const QColor &col);
    void slotEffectType(int type);
    void slotSTCheck(bool b);
    void slotDefault();

private:
    KIconEffect *mpEffect;
    QListBox *mpEffectBox;
    QCheckBox *mpSTCheck;
    QSlider *mpEffectSlider;
    KColorButton *mpEColButton;
    KColorButton *mpECol2Button;
    Effect mEffect;
    Effect mDefaultEffect;
    QImage mExample;
    QGroupBox *mpEffectGroup;
    QLabel *mpPreview, *mpEffectLabel, *mpEffectColor, *mpEffectColor2;
};                      
                      
#endif
