#ifndef KWIN_BLACKBOX_H
#define KWIN_BLACKBOX_H

// bevel options
#define BImage_Flat (1l<<1)
#define BImage_Sunken (1l<<2)
#define BImage_Raised (1l<<3)

// textures
#define BImage_Solid (1l<<4)
#define BImage_Gradient (1l<<5)

// gradients
#define BImage_Horizontal (1l<<6)
#define BImage_Vertical (1l<<7)
#define BImage_Diagonal (1l<<8)
#define BImage_CrossDiagonal (1l<<9)
#define BImage_Rectangle (1l<<10)
#define BImage_Pyramid (1l<<11)
#define BImage_PipeCross (1l<<12)
#define BImage_Elliptic (1l<<13)

// bevel types
#define BImage_Bevel1 (1l<<14)
#define BImage_Bevel2 (1l<<15)

// inverted image
#define BImage_Invert (1l<<16)

// parent relative image
#define BImage_ParentRelative (1l<<17)

// fake interlaced image
#define BImage_Interlaced (1l<<18)

class QSpacerItem;
class QBoxLayout;

namespace Blackbox {
  class BlackboxButton;

  class BlackboxClient : public KDecoration {
    Q_OBJECT
    public:
      enum Buttons { BtnMax = 0, BtnMin, BtnClose, BtnNone };

      BlackboxClient(KDecorationBridge *b, KDecorationFactory *f);
      ~BlackboxClient() {}

      void init();
      virtual void reset(unsigned long changed);
      virtual void resize(const QSize &s);

      virtual void activeChange();
      virtual void captionChange();
      virtual void iconChange();
      virtual void maximizeChange();
      virtual void desktopChange();
      virtual void shadeChange();

    protected: // Events
      void resizeEvent(QResizeEvent *);
      void mousePressEvent(QMouseEvent *);
      void paintEvent(QPaintEvent *);
      void mouseDoubleClickEvent(QMouseEvent *);
      void showEvent(QShowEvent *);
      void captionChange(const QString &);
      //void stickyChange(bool on);
      void shadeChange(bool shaded);
      //void maximizeChange(bool);
      void addClientButtons(const QString & s);
      void activeChange(bool);
      void makeButtons();
      // Return mouse position
      void mouseReleaseEvent(QMouseEvent *);
      virtual Position mousePosition(const QPoint &) const;

      bool eventFilter(QObject*, QEvent *);

      virtual void borders(int &left, int &right, int &top, int &bottom) const;
      virtual QSize minimumSize() const;

    protected slots:
      void slotMaximize();

    //signals:
      //void stkyChange (bool);
      //void maxChange (bool);

    private:
      bool smallButtons;
      int titleHeight;
      // titlebar widget
      QSpacerItem *titlebar;
      // title bar buttons
      BlackboxButton *button[3];
      // layouts
      QVBoxLayout *mainLayout;
      QHBoxLayout *titleLayout;
      QHBoxLayout *windowLayout;
      QVBoxLayout *botLayout;

      QLayoutItem *botSpacerItem;
      void setupLayout();
      QFont m_font;
      QFontMetrics *m_fm;
      KPixmap *pixTitle[2];
      KPixmap *pixLabel[2];
      KPixmap *pixHandle[2];
      KPixmap *pixButton[3];
      KPixmap *pixGrip[2];
      bool pixmade;
      bool madeButtons;
      int lastButton;
  };

  class BlackboxButton: public QSpacerItem {
    public:
      BlackboxButton(BlackboxClient *parent=0, const char *name=0, const BlackboxClient::Buttons type = BlackboxClient::BtnMax, int size=14, KPixmap *pixmaps[3]=0);
      QSize sizeHint () const;
      void drawButton (QPainter *p);
      KPixmap *pixButton[3];
      BlackboxClient *client; /// owner
      BlackboxClient::Buttons m_type; /// button type
      bool m_down;
      bool m_hide;
      int last_button;
      int m_size;
      inline void hide (void) { m_hide = false; }
      inline void show (void) { m_hide = true; }
      inline bool isVisible(void) { return (m_hide); }
      inline void press(void) { m_down=true; }
      inline void unpress(void) { m_down=false; }
  };

  class BlackboxClientFactory : public KDecorationFactory {
  public:
    BlackboxClientFactory();
    virtual ~BlackboxClientFactory();

    virtual KDecoration* createDecoration(KDecorationBridge *);
    virtual bool reset(unsigned long changed);
    virtual bool supports(Ability ability);
    virtual QValueList< KDecorationDefines::BorderSize > borderSizes() const;
  };
}; /* namespace Blackbox */

#endif /* KWIN_BLACKBOX_H */
