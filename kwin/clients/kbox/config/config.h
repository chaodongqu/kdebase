/*
 *	$Id: config.h,v 1.2 2003/01/17 21:40:11 fault Exp $
 *
 *   based on icewm/config.h
 */

#ifndef _BlackboxConfig_H
#define _BlackboxConfig_H

class BlackboxConfig: public QObject {
  Q_OBJECT

  public:
    BlackboxConfig( KConfig* conf, QWidget* parent );
    ~BlackboxConfig();

  // These public signals/slots work similar to KCM modules
  signals:
    void changed();

  public slots:
    void load(KConfig* conf);
    void save(KConfig* conf);
    void defaults();

  protected slots:
    void slotSelectionChanged();	// Internal use
    void callURL(const QString& s);
    void findblackboxstyles();

  private:
    KConfig*   m_bbcfg;
    QCheckBox* cbstyleButtonPositions;
    QCheckBox* cbstyleTitleTextColors;
    QCheckBox* cbstyleFont;
    QCheckBox* cbstyleBG;
    QGroupBox* gb1;
    QGroupBox* gb2;
    QListBox*  styleListBox;
    QLabel*	   styleLabel;
    KURLLabel* urlLabel;
    QString    localstylestring;
}; /* class BlackboxConfig */

#endif /* _BlackboxConfig_H */
