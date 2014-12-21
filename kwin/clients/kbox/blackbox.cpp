/* $Id: blackbox.cpp,v 1.8 2003/01/18 05:00:16 fault Exp $
  some code based upon the IceWM kwin client by gallium (gallium@kde.org)
  and kimageeffect.cpp by Mosfet (mosfet@kde.org)
  as well as Blackbox by Brad Hughes.
 */

// TODO: refactor many parameters functions into single struct ptr as a parameter functions where able

/* Qt */
#include <qtcommon.hpp> /* include/qtcommon.hpp */
#include <qbitmap.h>
#include <qimage.h>

/* KDE */
#include <kdecommon.hpp> /* include/kdecommon.hpp */
#include <kimageeffect.h>
#include <kpixmapeffect.h>
#include <kdrawutil.h>
#include <kpixmap.h>
#include <kpixmapio.h>

/* Xorg */
#include <X11/SM/SMlib.h>
#include <X11/Xresource.h>

/* KWin */
#include <core/common.hpp>
#include <kdecorationfactory.h>

using KWinInternal::SUPPORTED_WINDOW_TYPES_MASK;

/* Blackbox */
#include "blackbox.h"

static QString *themeName;

static bool config_styleTitleTextColors = true;
static bool config_styleTitleFont = false;
static int config_styleTitleJust = 0;
static int config_styleBG = false;

static bool pixmaps_created = false;

static unsigned long title_texture[2];
static QColor *title_color[2];
static QColor *title_colorTo[2];

static unsigned long label_texture[2];
static QColor *label_color[2];
static QColor *label_colorTo[2];
static QColor *label_textColor[2];

static unsigned long button_texture[3];
static QColor *button_color[3];
static QColor *button_colorTo[3];
static QColor *button_picColor[3];

static unsigned long handle_texture[2];
static QColor *handle_color[2];
static QColor *handle_colorTo[2];

static unsigned long grip_texture[2];
static QColor *grip_color[2];
static QColor *grip_colorTo[2];

static QColor *border_color;
static QColor *frame_color;
static QColor *framein_color;

static unsigned int g_bvsz; // Bevel size
static unsigned int frameSize2;
static unsigned int g_bsz; // Border size
static unsigned int g_hlsz; // Handle size

static bool customButtonPositions = false;	// Let the theme dictate the btn pos.

static QString *titleButtonsLeft;
static QString *titleButtonsRight;

static QString *title_font=0;
static QString *title_xftfont=0;
static int title_xftsize;
static QString *title_xftflags=0;

static QString *rootCmd;

static unsigned int label_just;

static unsigned long get_texture(XrmDatabase, const char *, const char *, QColor * color = 0, QColor * colorto = 0, QColor * textcolor = 0, QColor * piccolor = 0);
static void get_color(XrmDatabase, const char *, const char *, QColor *);
static unsigned int get_number(XrmDatabase database, const char *key, const char *cls);
static void get_string(XrmDatabase database, const char *key, const char *cls, QString * raw);

static KPixmap *render (KPixmap *, unsigned long, QColor *, QColor *);
static void bevel1 (QSize size, QImage img);
static void bevel2 (QSize size, QImage img);
static void invert (QSize size, QImage img);
static void gradient(const QSize & size, const QColor & ca, const QColor & cb, KImageEffect::GradientType eff, int ncols, QPixmap & pix, bool invert, bool interlace, int type);

static unsigned long get_texture(XrmDatabase db, const char *key, const char *cls, QColor * color, QColor * colorTo, QColor * textColor, QColor * picColor) {
  XrmValue value;
  char *value_type;

  if(XrmGetResource(db, key, cls, &value_type, &value)) {
    QString raw;
    unsigned long val = 0;
    raw = qstrdup(value.addr);
    raw.lower();

    if(raw.contains("parentrelative")) { val = BImage_ParentRelative; }

    if(raw.contains("solid")) { val |= BImage_Solid; }
    else if (raw.contains("gradient")) {
      val |= BImage_Gradient;
      if(raw.contains("crossdiagonal")) { val |= BImage_CrossDiagonal; }
      else if(raw.contains("rectangle")) { val |= BImage_Rectangle; }
      else if(raw.contains("pyramid")) { val |= BImage_Pyramid; }
      else if(raw.contains("pipecross")) { val |= BImage_PipeCross; }
      else if(raw.contains("elliptic")) { val |= BImage_Elliptic; }
      else if(raw.contains("diagonal")) { val |= BImage_Diagonal; }
      else if(raw.contains("horizontal")) { val |= BImage_Horizontal; }
      else if(raw.contains("vertical")) { val |= BImage_Vertical; }
      else { val |= BImage_Diagonal; }
    }
    else { val |= BImage_Solid; }

    if(raw.contains ("raised")) { val |= BImage_Raised; }
    else if(raw.contains ("sunken")) { val |= BImage_Sunken; }
    else if(raw.contains ("flat")) { val |= BImage_Flat; }
    else { val |= BImage_Raised; }

    if(!(val & BImage_Flat)) {
      if(raw.contains ("bevel2")) { val |= BImage_Bevel2; }
      else { val |= BImage_Bevel1; }
    }

    if(raw.contains ("interlaced")) { val |= BImage_Interlaced; }
    raw.truncate (0);

    //if (val & BImage_Solid){

    if(color) {
      QString k = key;
      QString cl = cls;
      k.append(".color");
      cl.append(".Color");
      get_color(db, k.latin1 (), cl.latin1 (), color);
      if(colorTo) {
        k.append("To");
        cl.append("To");
        get_color(db, k.latin1 (), cl.latin1 (), colorTo);
      }
    }
    if(textColor) {
      QString k = key;
      QString cl = cls;
      k.append(".textColor");
      cl.append(".TextColor");
      get_color(db, k.latin1(), cl.latin1(), textColor);
    }
    else if(picColor) {
      QString k = key;
      QString cl = cls;
      k.append(".picColor");
      cl.append(".PicColor");
      get_color(db, k.latin1(), cl.latin1(), picColor);
    }
    return val;
  }
  return(BImage_Solid|BImage_Flat);
}

/* FIXME: deallocate colors */
static void get_color(XrmDatabase database, const char *key, const char *cls, QColor *c) {
  XrmValue value;
  char *value_type;
  c->setRgb(0, 0, 0);
  if(XrmGetResource(database, key, cls, &value_type, &value)) {
    QString raw = qstrdup(value.addr);
#ifdef DEBUG
    kdDebug () << "key : " << cls << " color: " << raw << endl;
#endif
    c->setNamedColor(raw);
    raw.truncate (0);
  } else {
#ifdef DEBUG
    kdDebug () << "key : " << cls << " not found!!!! " << endl;
#endif
  }
}

static unsigned int get_number (XrmDatabase database, const char *key, const char *cls) {
  XrmValue value;
#ifdef DEBUG
  kdDebug() << "colorkey: "<<key<<endl;
#endif
  char *value_type;
  unsigned int val = 1; // default==1 in most?
  if(XrmGetResource(database, key, cls, &value_type, &value)) {
    QString raw = qstrdup(value.addr);
    val = raw.toUInt();
    raw.truncate(0);
  }
  return(val);
}

static void get_string(XrmDatabase database, const char *key, const char *cls, QString *raw) {
  XrmValue value;
#ifdef DEBUG
  kdDebug() << "colorkey: "<<key<<endl;
#endif
  char *value_type;
  // unsigned int val = 1; // default==1 in most?
  if(XrmGetResource(database, key, cls, &value_type, &value)) {
#ifdef DEBUG
    kdDebug() << "foo :" <<value.addr<<endl;
#endif
    *raw = qstrdup(value.addr);
    return;
  }
  *raw = "";
}

static KPixmap *render(KPixmap * pix, unsigned long val, QColor * c, QColor * cto) {
  if(val & BImage_ParentRelative) {
    delete pix;
    pix = 0;
  }

  if(val & BImage_Solid) {
    int height = pix->height();
    int width = pix->width();
    QPainter p;
    p.begin(pix);
    p.fillRect(pix->rect(), QBrush(*c));
    p.setPen(*c);
    if(val & BImage_Interlaced) {
      p.setPen(*cto);
      unsigned int i = 0;
      for(; i < (unsigned int)height; i+=2) { p.drawLine(0, i, width, i); }
      p.setPen(*c);
    }

    if(val & BImage_Bevel1) {
      if(val & BImage_Raised) {
        p.setPen((*c).dark ());
        p.drawLine(0, height-1, width-1, height-1);
        p.drawLine(width-1, height-1, width-1, 0);
        p.setPen((*c).light ());
        p.drawLine(0, 0, width-1, 0);
        p.drawLine(0, height-1, 0, 0);
      }
      else if(val & BImage_Sunken) {
        p.setPen((*c).light ());
        p.drawLine(0, height-1, width-1, height-1);
        p.drawLine(width-1, height-1, width-1, 0);
        p.setPen((*c).dark ());
        p.drawLine(0, 0, width-1, 0);
        p.drawLine(0, height-1, 0, 0);
      }
    }
    else if(val & BImage_Bevel2) {
      if(val & BImage_Raised) {
        p.setPen((*c).dark ());
        p.drawLine(1, height-3, width-3, height-3);
        p.drawLine(width-3, height-3, width-3, 1);
        p.setPen((*c).light ());
        p.drawLine(1, 1, width-3, 1);
        p.drawLine(1, height-3, 1, 1);
      }
      else if(val & BImage_Sunken) {
        p.setPen((*c).light ());
        p.drawLine(1, height-3, width-3, height-3);
        p.drawLine(width-3, height-3, width-3, 1);
        p.setPen((*c).dark ());
        p.drawLine(1, 1, width-3, 1);
        p.drawLine(1, height-3, 1, 1);
      }
    }

    p.end();
  }
  else if(val & BImage_Gradient) {
    bool interlaced = val & BImage_Interlaced;
    bool inverted = false;
    QColor from, to;
    /*if(val & BImage_Sunken) {
        from = *cto;
        to = *c;

        if(!(val & BImage_Invert)) { inverted = true; }
      }
      else {*/
    from = *c;
    to = *cto;
      /*if(val & BImage_Invert) { inverted = true; }
    }*/

    int bevelType = 0;
    if(val & BImage_Bevel1) { bevelType = 1; }
    if(val & BImage_Bevel2) { bevelType = 2; }

    if(val & BImage_Diagonal) {
      gradient(pix->size(), from, to, KImageEffect::DiagonalGradient, 3, *pix, inverted, interlaced, bevelType);
    }
    else if(val & BImage_Elliptic) {
      gradient(pix->size(), from, to, KImageEffect::EllipticGradient, 3, *pix, inverted, interlaced, bevelType);
    }
    else if(val & BImage_Horizontal) {
      gradient(pix->size(), from, to, KImageEffect::HorizontalGradient, 3, *pix, inverted, interlaced, bevelType);
    }
    else if(val & BImage_Pyramid) {
      gradient(pix->size(), from, to, KImageEffect::PyramidGradient, 3, *pix, inverted, interlaced, bevelType);
    }
    else if(val & BImage_Rectangle) {
      gradient(pix->size(), from, to, KImageEffect::RectangleGradient, 3, *pix, inverted, interlaced, bevelType);
    }
    else if(val & BImage_Vertical) {
      gradient(pix->size(), from, to, KImageEffect::VerticalGradient, 3, *pix, inverted, interlaced, bevelType);
    }
    else if(val & BImage_CrossDiagonal) {
      gradient(pix->size(), from, to, KImageEffect::CrossDiagonalGradient, 3, *pix, inverted, interlaced, bevelType);
    }
    else if(val & BImage_PipeCross) {
      gradient(pix->size(), from, to, KImageEffect::PipeCrossGradient, 3, *pix, inverted, interlaced, bevelType);
    }
  }

  return pix;
}

static void invert(QSize /*size*/, QImage /*img*/) {
/*unsigned int i, j, wh = (size.width() * size.height()) - 1;
  unsigned char tmp;//,rr,gg,bb;
unsigned int *src=(unsigned int *)img.bits();
  for (i = 0, j = wh; j > i; j--, i++) {
    tmp = *(src + j);
    *(src + j) = *(src + i);
    *(src + i) = tmp;
  }*/
}

static void bevel1(QSize size, QImage img) {
  unsigned int *o_src = (unsigned int *)img.bits();
  unsigned int *src = o_src;
  unsigned char r, g, b, rr, gg, bb;

  unsigned int w = size.width(), h = size.height()-1, wh = w*h;
  while(--w) {
    r = qRed(*src);
    rr = r + (r >> 1);
    if(rr < r) { rr = ~0; }
    g = qGreen (*src);
    gg = g + (g >> 1);
    if(gg < g) { gg = ~0; }
    b = qBlue (*src);
    bb = b + (b >> 1);
    if(bb < b) { bb = ~0; }
    *src = qRgb(rr, gg, bb);
    r = qRed(*(src + wh));
    rr = (r >> 2) + (r >> 1);
    if(rr > r) { rr = 0; }
    g = qGreen(*(src + wh));
    gg = (g >> 2) + (g >> 1);
    if(gg > g) { gg = 0; }
    b = qBlue(*(src + wh));
    bb = (b >> 2) + (b >> 1);
    if(bb > b) { bb = 0; }

    *((src++) + wh) = qRgb(rr, gg, bb);
  }

  r = qRed(*src);
  rr = r + (r >> 1);
  if(rr < r) { rr = ~0; }
  g = qGreen (*src);
  gg = g + (g >> 1);
  if(gg < g) { gg = ~0; }
  b = qBlue (*src);
  bb = b + (b >> 1);
  if(bb < b) { bb = ~0; }

  *src = qRgb(rr, gg, bb);

  r = *(src + wh);
  rr = (r >> 2) + (r >> 1);
  if(rr > r) { rr = 0; }
  g = *(src + wh);
  gg = (g >> 2) + (g >> 1);
  if(gg > g) { gg = 0; }
  b = *(src + wh);
  bb = (b >> 2) + (b >> 1);
  if(bb > b) { bb = 0; }

  *(src + wh) = qRgb(rr, gg, bb);

  src = o_src + size.width();

#ifdef DEBUG
  kdDebug()<<"passed1"<<endl;
#endif

  while(--h) {
    r = qRed(*src);
    rr = r + (r >> 1);
    if(rr < r) { rr = ~0; }
    g = qGreen (*src);
    gg = g + (g >> 1);
    if(gg < g) { gg = ~0; }
    b = qBlue (*src);
    bb = b + (b >> 1);
    if(bb < b) { bb = ~0; }

    *src = qRgb(rr, gg, bb);

    src += size.width() - 1;

    r = qRed(*src);
    rr = (r >> 2) + (r >> 1);
    if(rr > r) { rr = 0; }
    g = qGreen (*src);
    gg = (g >> 2) + (g >> 1);
    if(gg > g) { gg = 0; }
    b = qBlue (*src);
    bb = (b >> 2) + (b >> 1);
    if(bb > b) { bb = 0; }

    *(src++) = qRgb(rr, gg, bb);
  }

  r = qRed(*src);
  rr = r + (r >> 1);
  if(rr < r) { rr = ~0; }
  g = qGreen (*src);
  gg = g + (g >> 1);
  if(gg < g) { gg = ~0; }
  b = qBlue (*src);
  bb = b + (b >> 1);
  if(bb < b) { bb = ~0; }
  *src = qRgb (rr, gg, bb);

  src += size.width() - 1;

  r = qRed(*src);
  rr = (r >> 2) + (r >> 1);
  if(rr > r) { rr = 0; }
  g = qGreen (*src);
  gg = (g >> 2) + (g >> 1);
  if(gg > g) { gg = 0; }
  b = qBlue (*src);
  bb = (b >> 2) + (b >> 1);
  if(bb > b) { bb = 0; }
  *src = qRgb(rr, gg, bb);
}

static void bevel2(QSize size, QImage img) {
  if(size.width() > 4 && size.height() > 4) {
    unsigned char r, g, b, rr, gg, bb;
    unsigned int w = size.width()-2, h = size.height()-1, wh = size.width() * (size.height()-3);
    unsigned int *o_src = (unsigned int *)img.bits ();
    unsigned int *src = o_src;
    while(--w) {
      r = qRed(*src);
      rr = r + (r >> 1);
      if(rr < r) { rr = ~0; }
      g = qGreen (*src);
      gg = g + (g >> 1);
      if(gg < g) { gg = ~0; }
      b = qBlue (*src);
      bb = b + (b >> 1);
      if(bb < b) { bb = ~0; }

      *src = qRgb(rr, gg, bb);

      r = qRed(*(src + wh));
      rr = (r >> 2) + (r >> 1);
      if(rr > r) { rr = 0; }
      g = qGreen(*(src + wh));
      gg = (g >> 2) + (g >> 1);
      if(gg > g) { gg = 0; }
      b = qBlue(*(src + wh));
      bb = (b >> 2) + (b >> 1);
      if(bb > b) { bb = 0; }

      *((src++) + wh) = qRgb(rr, gg, bb);
    }

    src = o_src + size.width();

    while(--h) {
      r = qRed(*src);
      rr = r + (r >> 1);
      if(rr < r) { rr = ~0; }
      g = qGreen(*src);
      gg = g + (g >> 1);
      if(gg < g) { gg = ~0; }
      b = qBlue(*src);
      bb = b + (b >> 1);
      if(bb < b) { bb = ~0; }

      *(++src) = qRgb(rr, gg, bb);

      src += size.width() - 3;

      r = qRed(*src);
      rr = (r >> 2) + (r >> 1);
      if(rr > r) { rr = 0; }
      g = qGreen(*src);
      gg = (g >> 2) + (g >> 1);
      if(gg > g) { gg = 0; }
      b = qBlue(*src);
      bb = (b >> 2) + (b >> 1);
      if(bb > b) { bb = 0; }

      *(src++) = qRgb(rr, gg, bb);

      src++;
    }
  }
}

static void gradient(const QSize & size, const QColor & ca, const QColor & cb, KImageEffect::GradientType eff, int ncols, QPixmap & pix, bool inverted, bool interlace, int bevelType) {
  int rDiff, gDiff, bDiff;
  int rca, gca, bca, rcb, gcb, bcb;

  QImage image(size, 32);

  register int x, y;

  rDiff = (rcb = cb.red()) - (rca = ca.red());
  gDiff = (gcb = cb.green()) - (gca = ca.green());
  bDiff = (bcb = cb.blue()) - (bca = ca.blue());

  if (eff == KImageEffect::VerticalGradient || eff == KImageEffect::HorizontalGradient) {
    uint *p;
    uint rgb;

    register int rl = rca << 16;
    register int gl = gca << 16;
    register int bl = bca << 16;

    if(eff == KImageEffect::VerticalGradient) {
      int rcdelta = ((1 << 16) / size.height()) * rDiff;
      int gcdelta = ((1 << 16) / size.height()) * gDiff;
      int bcdelta = ((1 << 16) / size.height()) * bDiff;

      for(y=0; y < size.height(); y++) {
        p = (uint *)image.scanLine(y);

        rl += rcdelta;
        gl += gcdelta;
        bl += bcdelta;

        rgb = qRgb((rl >> 16), (gl >> 16), (bl >> 16));

        for(x=0; x < size.width(); x++) {
          *p = rgb;
          p++;
        }
      }
    }
    else {
      // must be HorizontalGradient
      unsigned int *o_src = (unsigned int *)image.scanLine(0);
      unsigned int *src = o_src;

      int rcdelta = ((1 << 16) / size.width()) * rDiff;
      int gcdelta = ((1 << 16) / size.width()) * gDiff;
      int bcdelta = ((1 << 16) / size.width()) * bDiff;

      //if(!interlace) {
        for(x = 0; x < size.width (); x++) {
          rl += rcdelta;
          gl += gcdelta;
          bl += bcdelta;

          *src++ = qRgb((rl >> 16), (gl >> 16), (bl >> 16));
        }
      //}  // rl = xr; gl = xg; bl = xb;
      //else {
      //  for(x = 0; x < size.width(); x++) { }
      //}

      src = o_src;

      //NOTE: Believe it or not, manually copying in a for loop is faster than calling memcpy for each scanline
      // (on the order of ms...). I think this is due to the function call overhead (mosfet).

      for(y = 1; y < size.height(); ++y) {
        p = (unsigned int *)image.scanLine(y);
        src = o_src;
        for(x = 0; x < size.width(); ++x) { *p++ = *src++; }
      }
    }
  }
  else {
    // Diagonal dgradient code inspired by BlackBox (mosfet)
    // BlackBox dgradient is (C) Brad Hughes, <bhughes@tcac.net> and
    // Mike Cole <mike@mydot.com>.
    float rfd, gfd, bfd;
    float rd = rca, gd = gca, bd = bca;

    unsigned char *xtable[3];
    unsigned char *ytable[3];

    unsigned int w = size.width(), h = size.height();
    xtable[0] = new unsigned char[w];
    xtable[1] = new unsigned char[w];
    xtable[2] = new unsigned char[w];
    ytable[0] = new unsigned char[h];
    ytable[1] = new unsigned char[h];
    ytable[2] = new unsigned char[h];
    w *= 2, h *= 2;

    if(eff == KImageEffect::DiagonalGradient || eff == KImageEffect::CrossDiagonalGradient) {
      rfd = (float)rDiff / w;
      gfd = (float)gDiff / w;
      bfd = (float)bDiff / w;

      int dir;
      for(x = 0; x < size.width(); x++, rd += rfd, gd += gfd, bd += bfd) {
        dir = eff == KImageEffect::DiagonalGradient ? x : size.width () - x - 1;
        xtable[0][dir] = (unsigned char)rd;
        xtable[1][dir] = (unsigned char)gd;
        xtable[2][dir] = (unsigned char)bd;
      }
      rfd = (float)rDiff / h;
      gfd = (float)gDiff / h;
      bfd = (float)bDiff / h;
      rd = gd = bd = 0;
      for(y = 0; y < size.height(); y++, rd += rfd, gd += gfd, bd += bfd) {
        ytable[0][y] = (unsigned char)rd;
        ytable[1][y] = (unsigned char)gd;
        ytable[2][y] = (unsigned char)bd;
      }
      if(!interlace) {
        //qWarning("blah\n");
#ifdef DEBUG
        kdDebug() << "not using fake inter" << endl;
#endif
        for(y = 0; y < size.height(); y++) {
          unsigned int *scanline = (unsigned int *)image.scanLine(y);
          for(x = 0; x < size.width(); x++) {
            scanline[x] = qRgb(xtable[0][x] + ytable[0][y], xtable[1][x] + ytable[1][y], xtable[2][x] + ytable[2][y]);
          }
        }
      }
      else {
        //qWarning("blah2\n");
#ifdef DEBUG
        kdDebug() << "using fake inter" << endl;
#endif
        //NOTE: faked interlacing effect
        unsigned char c, cr, cg, cb;
        for(y = 0; y < size.height(); y++) {
          unsigned int *scanline = (unsigned int *)image.scanLine(y);
          for(x = 0; x < size.width(); x++) {
            if(y & 1) {
              c = xtable[0][x] + ytable[0][y];
              cr = (c >> 1) + (c >> 2);
              if (cr > c) { cr = (unsigned char)0; }
              c = xtable[1][x] + ytable[1][y];
              cg = (c >> 1) + (c >> 2);
              if (cg > c) { cg = (unsigned char)0; }
              c = xtable[2][x] + ytable[2][y];
              cb = (c >> 1) + (c >> 2);
              if (cb > c) { cb = (unsigned char)0; }
              scanline[x] = qRgb(cr, cg, cb);
            }
            else {
              c = xtable[0][x] + ytable[0][y];
              cr = c + (c >> 3);
              if(cr < c) { cr = (unsigned char)-0; }
              c = xtable[1][x] + ytable[1][y];
              cg = c + (c >> 3);
              if(cg < c) { cg = (unsigned char)-0; }
              c = xtable[2][x] + ytable[2][y];
              cb = c + (c >> 3);
              if(cb < c) { cb = (unsigned char)-0; }
              scanline[x] = qRgb(cr, cg, cb);
            }
          }
        }
      }
    }
    else if(eff == KImageEffect::RectangleGradient || eff == KImageEffect::PyramidGradient ||
      eff == KImageEffect::PipeCrossGradient || eff == KImageEffect::EllipticGradient) {
      int rSign = rDiff > 0 ? 1 : -1;
      int gSign = gDiff > 0 ? 1 : -1;
      int bSign = bDiff > 0 ? 1 : -1;

      rfd = (float)rDiff / size.width();
      gfd = (float)gDiff / size.width();
      bfd = (float)bDiff / size.width();

      rd = (float)rDiff / 2;
      gd = (float)gDiff / 2;
      bd = (float)bDiff / 2;

      for(x = 0; x < size.width (); x++, rd -= rfd, gd -= gfd, bd -= bfd) {
        xtable[0][x] = (unsigned char)abs((int)rd);
        xtable[1][x] = (unsigned char)abs((int)gd);
        xtable[2][x] = (unsigned char)abs((int)bd);
      }

      rfd = (float)rDiff / size.height();
      gfd = (float)gDiff / size.height();
      bfd = (float)bDiff / size.height();

      rd = (float)rDiff / 2;
      gd = (float)gDiff / 2;
      bd = (float)bDiff / 2;

      for(y = 0; y < size.height (); y++, rd -= rfd, gd -= gfd, bd -= bfd) {
        ytable[0][y] = (unsigned char)abs((int)rd);
        ytable[1][y] = (unsigned char)abs((int)gd);
        ytable[2][y] = (unsigned char)abs((int)bd);
      }
      unsigned int rgb;
      int h = (size.height() + 1) >> 1;
      for(y=0; y < h; y++) {
        unsigned int *sl1 = (unsigned int *)image.scanLine(y);
        unsigned int *sl2 = (unsigned int *)image.scanLine(QMAX(size.height()-y-1, y));

        int w = (size.width() + 1) >> 1;
        int x2 = size.width() - 1;

        for(x=0; x < w; x++, x2--) {
          rgb = 0;
          if(eff == KImageEffect::PyramidGradient) {
            rgb = qRgb(rcb - rSign * (xtable[0][x] + ytable[0][y]),
                       gcb - gSign * (xtable[1][x] + ytable[1][y]),
                       bcb - bSign * (xtable[2][x] + ytable[2][y]));
          }
          if(eff == KImageEffect::RectangleGradient) {
            rgb = qRgb(rcb - rSign * QMAX(xtable[0][x], ytable[0][y]) * 2,
                       gcb - gSign * QMAX(xtable[1][x], ytable[1][y]) * 2,
                       bcb - bSign * QMAX(xtable[2][x], ytable[2][y]) * 2);
          }
          if(eff == KImageEffect::PipeCrossGradient) {
            rgb = qRgb(rcb - rSign * QMIN(xtable[0][x], ytable[0][y]) * 2,
                       gcb - gSign * QMIN(xtable[1][x], ytable[1][y]) * 2,
                       bcb - bSign * QMIN(xtable[2][x], ytable[2][y]) * 2);
          }
          if(eff == KImageEffect::EllipticGradient) {
            rgb = qRgb(rcb - rSign * (int)sqrt((xtable[0][x] * xtable[0][x] + ytable[0][y] * ytable[0][y]) * 2.0),
                       gcb - gSign * (int)sqrt((xtable[1][x] * xtable[1][x] + ytable[1][y] * ytable[1][y]) * 2.0),
                       bcb - bSign * (int)sqrt((xtable[2][x] * xtable[2][x] + ytable[2][y] * ytable[2][y]) * 2.0));
          }
          sl1[x] = sl2[x] = rgb;
          sl1[x2] = sl2[x2] = rgb;
        }
      }
    }

    delete[]xtable[0];
    delete[]xtable[1];
    delete[]xtable[2];
    delete[]ytable[0];
    delete[]ytable[1];
    delete[]ytable[2];
  }

  //bevel1
  if(bevelType == 1) { bevel1(size, image); }
  else if(bevelType == 2) { bevel2(size, image); }

  // dither if necessary
  if(ncols && (QPixmap::defaultDepth() < 15)) {
    if(ncols < 2 || ncols > 256) { ncols = 3; }
    QColor *dPal = new QColor[ncols];
    for(int i=0; i < ncols; i++) {
      dPal[i].setRgb(rca + rDiff * i / (ncols - 1), gca + gDiff * i / (ncols - 1), bca + bDiff * i / (ncols - 1));
    }
    KImageEffect::dither(image, dPal, ncols);
    delete[]dPal;
  }
  if(inverted) {
    invert(size, image);
  }
  KPixmapIO io;
  pix = io.convertToPixmap(image);
  // pix.convertFromImage(image);
}

static void create_pixmaps() {
  if(pixmaps_created) { return; }

  pixmaps_created = true;
  QString f = locate("appdata", QString ("blackbox-styles/") + *themeName);

  XrmDatabase database = XrmGetFileDatabase(f.latin1());

  title_color[0] = new QColor ();
  title_colorTo[0] = new QColor ();
  title_texture[0] = get_texture (database, "window.title.focus", "Window.Title.Focus", title_color[0], title_colorTo[0]);

  title_color[1] = new QColor ();
  title_colorTo[1] = new QColor ();
  title_texture[1] = get_texture (database, "window.title.unfocus", "Window.Title.UnFocus", title_color[1], title_colorTo[1]);

  label_color[0] = new QColor ();
  label_colorTo[0] = new QColor ();
  label_textColor[0] = new QColor ();
  label_texture[0] = get_texture (database, "window.label.focus", "Window.Label.Focus", label_color[0], label_colorTo[0], label_textColor[0]);

  label_color[1] = new QColor ();
  label_colorTo[1] = new QColor ();
  label_textColor[1] = new QColor ();
  label_texture[1] = get_texture (database, "window.label.unfocus", "Window.Label.UnFocus", label_color[1], label_colorTo[1], label_textColor[1]);

  button_color[0] = new QColor ();
  button_colorTo[0] = new QColor ();
  button_picColor[0] = new QColor ();
  button_texture[0] = get_texture (database, "window.button.focus", "Window.Button.Focus", button_color[0], button_colorTo[0], 0, button_picColor[0]);

  button_color[1] = new QColor ();
  button_colorTo[1] = new QColor ();
  button_picColor[1] = new QColor ();
  button_texture[1] = get_texture (database, "window.button.unfocus", "Window.Button.UnFocus", button_color[1], button_colorTo[1], 0, button_picColor[1]);

  button_color[2] = new QColor ();
  button_colorTo[2] = new QColor ();
  button_picColor[2] = new QColor ();
  button_texture[2] = get_texture (database, "window.button.pressed", "Window.Button.Pressed", button_color[2], button_colorTo[2], 0, button_picColor[2]);

  handle_color[0] = new QColor ();
  handle_colorTo[0] = new QColor ();
  handle_texture[0] = get_texture (database, "window.handle.focus", "Window.Handle.Focus", handle_color[0], handle_colorTo[0]);

  handle_color[1] = new QColor ();
  handle_colorTo[1] = new QColor ();
  handle_texture[1] = get_texture (database, "window.handle.unfocus", "Window.Handle.UnFocus", handle_color[1], handle_colorTo[1]);

  grip_color[0] = new QColor ();
  grip_colorTo[0] = new QColor ();
  grip_texture[0] = get_texture (database, "window.grip.focus", "Window.Grip.Focus", grip_color[0], grip_colorTo[0]);

  grip_color[1] = new QColor ();
  grip_colorTo[1] = new QColor ();
  grip_texture[1] = get_texture (database, "window.grip.unfocus", "Window.Grip.UnFocus", grip_color[1], grip_colorTo[1]);

  border_color = new QColor ();
  get_color (database, "borderColor", "BorderColor", border_color);

  frame_color = new QColor ();
  get_color (database, "window.frame.focusColor", "Window.Frame.FocusColor", frame_color);

  framein_color = new QColor ();
  get_color (database, "window.frame.unfocusColor", "Window.Frame.UnfocusColor", framein_color);

  frameSize2 = get_number (database, "frameWidth", "FrameWidth");
  g_bvsz = get_number (database, "bevelWidth", "BevelWidth");
  g_bsz = get_number (database, "borderWidth", "BorderWidth");
  g_hlsz = get_number (database, "handleWidth", "HandleWidth");

  title_font = new QString ();
  get_string (database, "window.font", "Window.Font", title_font);

  title_xftfont=new QString();
  get_string (database, "window.xft.font", "Window.Xft.Font", title_xftfont);

  title_xftsize=get_number(database,"window.xft.size","Window.Xft.Size");

  title_xftflags=new QString();
  get_string (database, "window.xft.flags", "Window.Xft.Flags", title_xftflags);
#ifdef DEBUG
  kdDebug() << *title_font << endl;
#endif

  rootCmd=new QString;
  get_string(database,"rootCommand","RootCommand",rootCmd);

  QString *foo = new QString ();
  get_string (database, "window.justify", "Window.Justify", foo);
  if (foo->lower () == "left") { label_just = 1; }
  else if (foo->lower () == "right") { label_just = 3; }
  else if (foo->lower () == "center") { label_just = 2; }
  else { label_just = 1; }
#ifdef DEBUG
  kdDebug() << *foo << endl;
#endif
  XrmDestroyDatabase(database);

  QColor c(KDecoration::options()->color(KDecorationOptions::ColorButtonBg, false));
}

static void delete_pixmaps() {
#ifdef DEBUG
  kdDebug() << "begin delete" << endl;
#endif
  for(int i=0; i < 3; i++) {
    if(title_color[i]) { delete title_color[i]; }
#ifdef DEBUG
    kdDebug() << "1" << endl;
#endif
    if(title_colorTo[i]) { delete title_colorTo[i]; }
#ifdef DEBUG
    kdDebug() << "2" << endl;
#endif
    if (label_color[i]) { delete label_color[i]; }
#ifdef DEBUG
    kdDebug() << "3" << endl;
#endif
    if(label_colorTo[i]) { delete label_colorTo[i]; }
#ifdef DEBUG
    kdDebug() << "4" << endl;
#endif
    if(i != 2) {
      if(label_textColor[i]) { delete label_textColor[i]; }
    }
#ifdef DEBUG
    kdDebug() << "5" << endl;
#endif
    if(handle_color[i]) { delete handle_color[i]; }
#ifdef DEBUG
    kdDebug() << "6" << endl;
#endif
    if(handle_colorTo[i]) { delete handle_colorTo[i]; }
#ifdef DEBUG
    kdDebug() << "7" << endl;
#endif
    if(grip_color[i]) { delete grip_color[i]; }
#ifdef DEBUG
    kdDebug() << "8" << endl;
#endif
    if(grip_colorTo[i]) { delete grip_colorTo[i]; }
#ifdef DEBUG
    kdDebug() << "9" << endl;
#endif
    title_texture[i] = 0;
#ifdef DEBUG
    kdDebug() << "10" << endl;
#endif
    label_texture[i] = 0;
#ifdef DEBUG
    kdDebug() << "11" << endl;
#endif
    handle_texture[i] = 0;
#ifdef DEBUG
    kdDebug() << "12" << endl;
#endif
    grip_texture[i] = 0;
  }
#ifdef DEBUG
  kdDebug() << "2------" << endl;
#endif
  for(int j=0; j < 4; j++) {
    if(button_color[j]) { delete button_color[j]; }
    if(button_colorTo[j]) { delete button_colorTo[j]; }
    if(button_picColor[j]) { delete button_picColor[j]; }
    button_texture[j] = 0;
  }
#ifdef DEBUG
  kdDebug() << "3--" << endl;
#endif
  delete themeName;
#ifdef DEBUG
  kdDebug() << "4--" << endl;
#endif
}

static bool read_config() {
#ifdef DEBUG
  kdDebug() << "yo" << endl;
#endif
  KConfig conf("kwinblackboxrc");
#ifdef DEBUG
  kdDebug() << "lo" << endl;
#endif
  conf.setGroup("General");
#ifdef DEBUG
  kdDebug() << "1-" << endl;
#endif
  QString t = conf.readEntry("Currentstyle", "default");
  config_styleTitleFont = conf.readBoolEntry("styleFont", false);

  config_styleTitleTextColors = conf.readBoolEntry("styleTitleTextColors", true);
  config_styleBG=conf.readBoolEntry("styleBG", false);
  config_styleTitleJust = conf.readNumEntry("styleTitleJust", 0);
  //themeName= new char [t.length()+1];
#ifdef DEBUG
  kdDebug() << "2-" << endl;
#endif

  if (t == "default")
    t = "deep";
  themeName = new QString;
  *themeName = t;
#ifdef DEBUG
  kdDebug() << "3-" << endl;
#endif
  customButtonPositions = KDecoration::options()->customButtonPositions();
  titleButtonsLeft = new QString();
  titleButtonsRight = new QString();
#ifdef DEBUG
  kdDebug() << "4-" << endl;
#endif
  if(customButtonPositions) {
    *titleButtonsLeft = KDecoration::options()->titleButtonsLeft();
    *titleButtonsRight = KDecoration::options()->titleButtonsRight();
  }
  else {
    *titleButtonsLeft = "I";
    *titleButtonsRight = "AX";
  }
  return true;
}

Blackbox::BlackboxClient::BlackboxClient(KDecorationBridge *b, KDecorationFactory *f)
: KDecoration(b, f)
, titlebar(0)
{
}

void Blackbox::BlackboxClient::init() {
  createMainWidget(/* WResizeNoErase|WRepaintNoErase */ 0);
  widget()->installEventFilter(this);
  widget()->setBackgroundMode(NoBackground);

  if(config_styleTitleFont) {
#ifdef DEBUG
    kdDebug() << "font    : " << *title_font << endl;
    kdDebug() << "xftfont : " << *title_xftfont << endl;
#endif
    if(!title_xftfont->isEmpty() && 0 == title_xftfont->compare(*title_font)) {
      m_font.setRawName(*title_xftfont);
      if(m_font.rawMode()) {
        m_font.setRawMode(false);
        m_font.setFamily(*title_xftfont);
      }
      if(title_xftsize) { m_font.setPointSize(title_xftsize); }
      if((*title_xftflags).contains("bold",false)) { m_font.setBold(true); }
    }
    else {
      m_font.setRawName(*title_font);
      //m_font.setRawMode(false);
#ifdef DEBUG
      kdDebug() << "using rawfont" << *title_font << endl;
      kdDebug() << "using fnt " << m_font.family() << endl;
#endif
    }
  }
  else {
    m_font = options()->font(isActive(), NET::Toolbar == windowType(SUPPORTED_WINDOW_TYPES_MASK));
  }

  m_fm=new QFontMetrics(m_font);

  titleHeight = m_fm->height() + (g_bvsz*2) + (g_bsz*2);
#ifdef DEBUG
  kdDebug() << "rawfont " << titleHeight << endl;
#endif
  if(titleHeight < 16) { titleHeight = 16; }
  smallButtons = false;

  pixmade = false;
  madeButtons = false;

  lastButton = BtnNone;

  for(int i = 0; i < 2; i++) {
    //NOTE: set 0 for extra checking
    pixTitle[i] = 0;
    pixLabel[i] = 0;
    pixButton[i] = 0;
    pixHandle[i] = 0;
    pixGrip[i] = 0;
  }

  pixButton[2] = 0;

  setupLayout ();
  connect(widget(), SIGNAL(resetClients()), widget(), SLOT (slotReset()));
}

void Blackbox::BlackboxClient::reset(unsigned long /*changed*/) {
  //Debug () << "texture==" << title_texture[0] << endl;
  //read_config();
  //button[BtnMax]->reset ();
  //button[BtnClose]->reset ();
  //button[BtnMin]->reset ();
  /*if(pixmade) {
    pixmade = false;
    for(int i = 0; i < 2; i++) {
      if(pixLabel[i]) {
        delete pixLabel[i];
        if(madeButtons) {
          if(pixButton[i]) { delete pixButton[i]; }
          madeButtons = false;
        }
        if(pixHandle[i]) { delete pixHandle[i]; }
        if(pixGrip[i]) { delete pixGrip[i]; }
      }
    }
  }*/
#ifdef DEBUG
  kdDebug()<< "reset=1"<<endl;
#endif
  pixmade = false;
}

void Blackbox::BlackboxClient::resize(const QSize &s) {
  widget()->resize(s);
}

void Blackbox::BlackboxClient::activeChange() {
}

void Blackbox::BlackboxClient::captionChange() {
}

void Blackbox::BlackboxClient::iconChange() {
}

void Blackbox::BlackboxClient::maximizeChange() {
}

void Blackbox::BlackboxClient::desktopChange() {
}

void Blackbox::BlackboxClient::shadeChange() {
}

void Blackbox::BlackboxClient::setupLayout() {
  titlebar = new QSpacerItem(0, titleHeight, QSizePolicy::Expanding, QSizePolicy::Minimum);

  //connect(widget(), SIGNAL(resetClients()), widget(), SLOT(slotReset()));

  mainLayout = new QVBoxLayout(widget());
  mainLayout->setResizeMode(QLayout::FreeResize);
  titleLayout = new QHBoxLayout();
  windowLayout = new QHBoxLayout();
  mainLayout->addLayout(titleLayout);
  mainLayout->addLayout(windowLayout, 1);

  // add bottom margin

  botSpacerItem = new QSpacerItem(0,frameSize2+g_hlsz+(2*g_bsz), QSizePolicy::Minimum,QSizePolicy::Fixed);
  mainLayout->addItem(botSpacerItem);

  // add left margin
  windowLayout->addSpacing(g_bsz + frameSize2);
  // add window
  if(isPreview()) { windowLayout->addWidget(new QLabel(i18n("<center><b>MyDecoration</b></center>"), widget())); }
  else { windowLayout->addItem(new QSpacerItem(0, 0)); }
  // add right margin
  windowLayout->addSpacing(g_bsz + frameSize2);
  makeButtons();

  addClientButtons(*titleButtonsLeft);
  titleLayout->addItem(titlebar);

  //if(titleButtonsRight->length())
  addClientButtons(*titleButtonsRight);
}

void Blackbox::BlackboxClient::addClientButtons(const QString &s) {
  titleLayout->addSpacing(1);
  if(s.length() == 0) { return; }
  for(unsigned int i = 0; i < s.length(); i++) {
    switch(s[i].latin1()) {
      case 'I':
        //if(isMinimizable ()) {
          button[BtnMin]->show();
          titleLayout->addSpacing(3);
          titleLayout->addItem(button[BtnMin]);
        //}
        break;
      case 'A':
        //if(isMaximizable()) {
          button[BtnMax]->show();
          titleLayout->addSpacing(3);
          titleLayout->addItem(button[BtnMax]);
        //}
        break;
      case 'X': {
        button[BtnClose]->show();
        titleLayout->addSpacing(3);
        titleLayout->addItem(button[BtnClose]);
        break; }
      case '_': {
        titleLayout->addSpacing(4);
        break; }
    }
  }
  titleLayout->addSpacing(3);
}

void Blackbox::BlackboxClient::makeButtons() {
  if(!madeButtons) {
    int buttonSize = titleHeight - (2 * g_bsz) - (2 * g_bvsz) - 1;
    for(int k=0; k<3; k++) {
      if(!(button_texture[k] & BImage_ParentRelative)) {
        pixButton[k] = new KPixmap();
        pixButton[k]->resize(buttonSize, buttonSize);
        render(pixButton[k], button_texture[k], button_color[k], button_colorTo[k]);
      }
    }

    button[BtnMin] = new BlackboxButton(this, "iconify", BtnMin, buttonSize, pixButton);
    button[BtnMax] = new BlackboxButton(this, "maximize", BtnMax, buttonSize, pixButton);
    button[BtnClose] = new BlackboxButton(this, "close", BtnClose, buttonSize, pixButton);

    madeButtons = true;
  }
}

void Blackbox::BlackboxClient::paintEvent(QPaintEvent */*pe*/) {
  QColorGroup g;
  bool active = isActive();
  QPainter p(widget());
  int w = width();
  int h = height();

  QRect t = titlebar->geometry();
  t.setTop (1);

#ifdef DEBUG
  kdDebug() << "before: " << pixmade << endl;
#endif

  if(!pixmade) {
    for(int k = 0; k < 2; k++) {
      pixTitle[k] = new KPixmap ();
      pixTitle[k]->resize(geometry().width() - g_bsz, titleHeight - ((2 * g_bsz)));
      render(pixTitle[k], title_texture[k], title_color[k], title_colorTo[k]);
#ifdef DEBUG
      kdDebug() << "render pixtitle: " << k << endl;
#endif
      if(!(label_texture[k] & BImage_ParentRelative)) {
        pixLabel[k] = new KPixmap();
#ifdef DEBUG
        kdDebug() << "render pixlabel: " << k << endl;
        kdDebug() << "render title: height-" << t.height() - (2 * g_bvsz) <<
            " titlHEight- " << t.height() << " borderSize- "<< g_bsz << "  bevelSize- " <<
            g_bvsz << " frameSize-" << frameSize2 <<endl;
#endif
        pixLabel[k]->resize (t.width(), t.height() - (2 * g_bsz) - (2 * g_bvsz));
        render(pixLabel[k], label_texture[k], label_color[k], label_colorTo[k]);
      }
      if(!(grip_texture[k] & BImage_ParentRelative)) {
        pixGrip[k] = new KPixmap();
        pixGrip[k]->resize(2 * (t.height() * .6666) + 1, g_hlsz);
#ifdef DEBUG
        kdDebug() << "render pixgrip: " << k << endl;
#endif
        render(pixGrip[k], grip_texture[k], grip_color[k], grip_colorTo[k]);
      }
      if(!(handle_texture[k] & BImage_ParentRelative)) {
        pixHandle[k] = new KPixmap();
        pixHandle[k]->resize(w - (4 * (t.height() * .6666) + (2 * g_bsz)) - g_bsz, g_hlsz);
#ifdef DEBUG
        kdDebug() << "render winSIZE: " << w << endl;
        kdDebug() << "render handleSIZE: " << pixHandle[k]->width() << endl;
#endif
        render(pixHandle[k], handle_texture[k], handle_color[k], handle_colorTo[k]);
      }
    }
    pixmade = true;
  }
#ifdef DEBUG
  kdDebug() << "after: " << pixmade << endl;
#endif
  int index = (active?0:1);
  p.drawPixmap(g_bsz, g_bsz, *pixTitle[index]);
#ifdef DEBUG
  // kdDebug() << "yo " << bottomWidget->height() << endl;
#endif
  if(pixLabel[index]) {
    p.drawPixmap(t.x(), t.y() + (g_bvsz) + (g_bsz - 1), *pixLabel[index]);
  }
  if(!isShade()){
    if(pixGrip[index]) {
#ifdef DEBUG
      kdDebug() << "drawing grip: " << pixGrip[index]->width() << "  " << pixHandle[index]->height() << endl;
#endif
      p.drawPixmap(g_bsz, h - (g_hlsz + g_bsz), *pixGrip[index]);	//left

      if(pixHandle[index]) {
#ifdef DEBUG
        kdDebug() << "drawing handle: " << pixHandle[index]->width() << "  " << pixHandle[index]->height() << endl;
#endif
        p.drawPixmap(g_bsz + 2 * (t.height() * .6666) + g_bsz, h - (g_hlsz + g_bsz), *pixHandle[index]);
      }
      p.drawPixmap(w - 2 * (t.height() * .6666) - g_bsz, h - (g_hlsz + g_bsz), *pixGrip[index]);	//right
    }
  }

  button[BtnMin]->drawButton (&p);
  button[BtnMax]->drawButton (&p);
  button[BtnClose]->drawButton (&p);

  int x = 0, z=0;
  if(!config_styleTitleTextColors) { p.setPen(options()->color(KDecorationOptions::ColorFont, isActive())); }
  else { p.setPen(*label_textColor[0]); }
  x = config_styleTitleJust;
  z=m_fm->width(caption());
  if(z >= (t.width()-26)) { x=t.x()+13; }
  else {
    if(x == 0) { x = label_just; }
    if(x == 1) { x = t.x() + 13; }
    else if(x == 2) { x = t.x() + (t.width() / 2) - (m_fm->width(caption())/2); }
    else if(x == 3) { x = t.x() + t.width() - (2*g_bsz)-m_fm->width(caption()); }
  }

  p.setFont(m_font);
  p.drawText(x, 2, t.width() - 13, t.height() - 3, AlignVCenter, caption());

  p.fillRect(g_bsz, t.height(), frameSize2, h - t.height() - (2 * g_bsz) - g_hlsz, isActive()? QBrush(*frame_color) : QBrush (*framein_color)); //left border
  p.fillRect(g_bsz + frameSize2, h - (2 * g_bsz) - g_hlsz - frameSize2, w - g_bsz - frameSize2 - g_bsz, frameSize2, isActive()? QBrush(*frame_color) : QBrush(*framein_color)); //bottom border
  p.fillRect(w - (g_bsz) - frameSize2, t.height() + g_bsz, frameSize2, h - t.height() - (2 * g_bsz) - g_hlsz - frameSize2, isActive() ? QBrush(*frame_color) : QBrush(*framein_color)); //right border
  p.fillRect(1, t.height() - frameSize2 + 1, w - 1, t.height(), isActive() ? QBrush(*frame_color) : QBrush(*framein_color));

  p.fillRect(0, 0, w, g_bsz, QBrush(*border_color)); // top titlebar border
  p.fillRect(0, g_bsz, g_bsz, h, QBrush(*border_color)); //left border

  p.fillRect(w - g_bsz, g_bsz, g_bsz, h - g_bsz, QBrush(*border_color)); //right border
  QColor tmp = Qt::black;

  if(isShade()) {
    if(pixGrip[index]) {
      p.drawPixmap(g_bsz, g_bsz+h - ((2 * g_bsz) + g_hlsz), *pixGrip[index]);  // left
      if(pixHandle[index]) {
        p.drawPixmap(g_bsz + 2 * (t.height() * .6666) + g_bsz, g_bsz+h - ((2 * g_bsz) + g_hlsz), *pixHandle[index]);
      }
      p.drawPixmap(w - 2 * (t.height() * .6666) - g_bsz, g_bsz+h - ((2 * g_bsz) + g_hlsz), *pixGrip[index]);	//right
    }
  }

  //if (!isShade()){
    p.fillRect(g_bsz, h - g_bsz, w, g_bsz, QBrush(*border_color));	//bottom border
    p.fillRect(g_bsz, h - ((2 * g_bsz) + g_hlsz), w - g_bsz, g_bsz, QBrush(*border_color));	//win/handle seperator

    p.fillRect(g_bsz + 2 * (t.height() * .6666), h - (g_hlsz + g_bsz), g_bsz, g_hlsz, QBrush(*border_color)); //grip/handlesep left
    p.fillRect(w - 2 * (t.height() * .6666) - (2 * g_bsz), h - (g_hlsz + g_bsz), g_bsz, g_hlsz, QBrush(*border_color)); //grip-hande left

    p.fillRect(g_bsz, t.height() - frameSize2 - g_bsz + 1, w - (2 * g_bsz), g_bsz, QBrush(*border_color));
  //}

  if(!madeButtons) { makeButtons(); }
}

void Blackbox::BlackboxClient::resizeEvent(QResizeEvent *e) {
  setMask(QRegion(0, 0, width(), height()));
  if(widget()->isVisibleToTLW() && !widget()->testWFlags(Qt::WStaticContents)) {
    //QRect t = titlebar->geometry();
    if(pixmade) {
      pixmade = false;
      for(int i=0; i<2; i++) {
        if(pixLabel[i]) { delete pixLabel[i]; }
        /*if(madeButtons) {
            if(pixButton[i]) { delete pixButton[i]; }
            madeButtons=false;
          }*/
        if(pixHandle[i]) { delete pixHandle[i]; }
        if(pixGrip[i]) { delete pixGrip[i]; }
      }
    }
  }
}

void Blackbox::BlackboxClient::mousePressEvent(QMouseEvent *e) {
  if(madeButtons) {
    if(button[BtnMin]->isVisible()) {
      if(button[BtnMin]->geometry().contains(e->pos())) {
        button[BtnMin]->press();
        button[BtnMin]->last_button = e->button();
        lastButton = BtnMin;
        widget()->repaint(false);
        return;
      }
    }
    if(button[BtnMax]->isVisible()) {
      if(button[BtnMax]->geometry().contains(e->pos())) {
        button[BtnMax]->press();
        button[BtnMax]->last_button = e->button();
        lastButton = BtnMax;
        widget()->repaint(false);
        return;
      }
    }
    if(button[BtnClose]->isVisible()) {
      if(button[BtnClose]->geometry().contains(e->pos())) {
        button[BtnClose]->press();
        button[BtnClose]->last_button = e->button();
        lastButton = BtnClose;
        widget()->repaint(false);
        return;
      }
    }
  }
  lastButton = BtnNone;
}

void Blackbox::BlackboxClient::mouseReleaseEvent(QMouseEvent *e) {
  if(lastButton != BtnNone) {
    if(button[lastButton]->isVisible()) {
      button[lastButton]->unpress();
      widget()->repaint(FALSE);
      if(button[lastButton]->geometry().contains(e->pos())) {
        if(lastButton == BtnMin) {
          widget()->showMinimized();
        }
        else if(lastButton == BtnMax) {
          slotMaximize();
        }
        else if(lastButton == BtnClose) {
          closeWindow();
        }
      }
    }
  }
}

void Blackbox::BlackboxClient::mouseDoubleClickEvent(QMouseEvent *e) {
  if(titlebar->geometry().contains(e->pos())) {
    titlebarDblClickOperation();
  }
}

void Blackbox::BlackboxClient::showEvent(QShowEvent */*e*/) {
}

KDecoration::Position Blackbox::BlackboxClient::mousePosition(const QPoint &p) const {
  Position m = KDecorationDefines::PositionCenter;

  if(p.y() < (height() - 6)) { m = KDecoration::mousePosition(p); }
  else {
    if(p.x() >= (width() - 26)) { m = KDecorationDefines::PositionBottomRight; }
    else if(p.x() <= 26) { m = KDecorationDefines::PositionBottomLeft; }
    else { m = KDecorationDefines::PositionBottom; }
  }

  return m;
}

bool Blackbox::BlackboxClient::eventFilter(QObject *o, QEvent *e) {
  if(o != widget()) { return false; }

  switch(e->type()) {
    case QEvent::Resize: resizeEvent(static_cast<QResizeEvent*>(e)); return true;
    case QEvent::Paint: paintEvent(static_cast<QPaintEvent*>(e)); return true;
    case QEvent::MouseButtonDblClick: mouseDoubleClickEvent(static_cast<QMouseEvent*>(e)); return true;
    case QEvent::Wheel: /*titlebar->wheelEvent(static_cast<QWheelEvent*>(e));*/ return true;
    case QEvent::MouseButtonPress: mousePressEvent(static_cast<QMouseEvent*>(e));  return true;
    case QEvent::Show: showEvent(static_cast<QShowEvent*>(e)); return true;
    default: break;
  }

  return false;
}

void Blackbox::BlackboxClient::captionChange(const QString &) { widget()->repaint(); }

void Blackbox::BlackboxClient::borders(int &l, int &r, int &t, int &b) const {
#ifdef DEBUG
  kdDebug() << "Border: " << g_bsz << " Bevel: " << g_bvsz << " FrameSz2: " << frameSize2 << endl;
#endif
  l = r = g_bsz;
  b = g_hlsz;
  t = frameSize2;
}

QSize Blackbox::BlackboxClient::minimumSize() const {
  int left, right, top, bottom;
  borders(left, right, top, bottom);
  return QSize(left + right + 2 * button[BtnClose]->sizeHint().width(), top + bottom);
}

void Blackbox::BlackboxClient::activeChange(bool) { widget()->repaint(false); }

void Blackbox::BlackboxClient::shadeChange(bool shaded) {
  QRegion mask(0, 0, width(), height());
  if(shaded) { mask -= QRegion(0,titleHeight-1,width(),height()-titleHeight+1); }
  else { mask += QRegion(0,titleHeight,width(),frameSize2+g_hlsz+(2*g_bsz)); }
  setMask(mask);
}

void Blackbox::BlackboxClient::slotMaximize() {
  switch(button[BtnMax]->last_button) {
    case MidButton: { maximize(KDecorationDefines::MaximizeVertical); } break;
    case RightButton: { maximize(KDecorationDefines::MaximizeHorizontal); } break;
    default: { maximize(KDecorationDefines::MaximizeVertical|KDecorationDefines::MaximizeHorizontal); }
  }
}

Blackbox::BlackboxButton::BlackboxButton(BlackboxClient *parent, const char */*name*/, BlackboxClient::Buttons type, int siz, KPixmap * pixmaps[3])
: QSpacerItem(siz, siz)
{
  for(int i=0; i<3; i++) { pixButton[i] = pixmaps[i] ? pixmaps[i] : 0; }
  m_type = type;
  m_size = siz;
  m_down = false;
  m_hide = true;
  client = parent;
}

QSize Blackbox::BlackboxButton::sizeHint() const {
  return QSize(m_size, m_size);
}

void Blackbox::BlackboxButton::drawButton(QPainter *p) {
  KPixmap *button = 0;
#ifdef DEBUG
  kdDebug() << "drawing button: " << geometry().x() << endl;
#endif
  if((m_down && client->isActive()) || (client->maximizeMode() == KDecorationDefines::MaximizeFull && (m_type == BlackboxClient::BtnMax))) {
    button = pixButton[2];
  }
  else if(client->isActive()) {
    button = pixButton[0];
  }
  else {
    button = pixButton[1];
  }
  if(button) {
    if(!button->isNull()) {
#ifdef DEBUG
      kdDebug() << "drawing button pixmap" << endl;
#endif
      p->drawPixmap(geometry().x(), geometry().y() + g_bvsz + g_bsz, *button);
    }
  }
  QColor pic = (m_down ? *button_picColor[2] : (client->isActive()? *button_picColor[0] : *button_picColor[1]));
  p->setPen(pic);
  switch(m_type) { /// TODO: limit to ~12 pixels
    case BlackboxClient::BtnMax: {
#ifdef DEBUG
      kdDebug() << "drawed max button: " << endl;
#endif
      p->fillRect(geometry().x() + 2, geometry().y() + 2 + g_bvsz + g_bsz, geometry().width() - 4, 2, pic);
      p->drawRect(geometry().x() + 2, geometry().y() + 2 + g_bvsz + g_bsz, geometry().width() - 4, geometry().width() - 4);
      break; }
    case BlackboxClient::BtnMin: {
#ifdef DEBUG
      kdDebug() << "drawed min button: " << endl;
#endif
      p->drawRect(geometry().x() + 2, geometry().width() - 2, geometry().width() - 4, 3);
      break; }
    case BlackboxClient::BtnClose: {
#ifdef DEBUG
      kdDebug() << "drawed close button: " << endl;
#endif
      p->drawLine(geometry().x() + 2, geometry().y() + 2 + g_bvsz + g_bsz, geometry().x() + geometry().width() - 3, geometry().y() + g_bvsz + g_bsz + geometry().width() - 3);
      p->drawLine(geometry().x() + geometry().width() - 3, geometry().y() + 2 + g_bvsz + g_bsz, geometry().x() + 2, geometry().y() + g_bvsz + g_bsz + geometry().width() - 3);
      break; }
  }
#ifdef DEBUG
  kdDebug() << "x=="<< geometry ().x ()<< " y=="<<geometry ().y () <<" width==" <<geometry ().width () << " height=="<<geometry ().height ()<<endl;
#endif
}

Blackbox::BlackboxClientFactory::BlackboxClientFactory() {
#ifdef DEBUG
  kdDebug() << "Blackbox: start.\n";
#endif
  read_config();
  create_pixmaps();
  if((*rootCmd != "") && config_styleBG){
    /*KProcess p;
    QStringList root = QStringList::split(' ',*rootCmd);
    for(QStringList::Iterator it = root.begin(); it != root.end(); ++it) {
      p<<*it;
    }
    p.start();*/
    //KRun::runCommand(*rootCmd);
#ifdef DEBUG
    kdDebug() << "cmd=" << *rootCmd << endl;
#endif
  }
}

Blackbox::BlackboxClientFactory::~BlackboxClientFactory() {
  delete_pixmaps();
}

KDecoration* Blackbox::BlackboxClientFactory::createDecoration(KDecorationBridge *b) {
  return new Blackbox::BlackboxClient(b, this);
}

bool Blackbox::BlackboxClientFactory::reset(unsigned long changed) {
  bool needsReset = (changed & SettingColors) ? true : false;
  if(changed & SettingFont) {
    pixmaps_created=false;
    delete_pixmaps();
    create_pixmaps();
    pixmaps_created=true;
    needsReset = true;
  }
  // init();
  //Workspace::self()->slotResetAllClientsDelayed();

  return needsReset;
}

bool Blackbox::BlackboxClientFactory::supports(Ability ability) {
  switch(ability) {
    case AbilityAnnounceButtons:
    case AbilityButtonMenu:
    case AbilityButtonOnAllDesktops:
    case AbilityButtonSpacer:
    case AbilityButtonHelp:
    case AbilityButtonMinimize:
    case AbilityButtonMaximize:
    case AbilityButtonClose:
    case AbilityButtonAboveOthers:
    case AbilityButtonBelowOthers:
    case AbilityButtonShade:
    case AbilityButtonResize:
      return true;
    default:
      return false;
  };
}

extern "C" KDE_EXPORT KDecorationFactory* create_factory() {
  return new Blackbox::BlackboxClientFactory();
}

#include "blackbox.moc"
