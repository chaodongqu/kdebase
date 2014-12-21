/* based on icewm/config.cpp */

/* Qt */
#include <qtcommon.hpp> /* include/qtcommon.hpp */
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlistbox.h>
#include <qfile.h>
#include <qdir.h>
#include <qwhatsthis.h>

/* KDE */
#include <kdecommon.hpp> /* include/kdecommon.hpp */
#include <kurllabel.h>
#include <kdirwatch.h>

// #define NOXINCLUDES
// #include <core/common.hpp>
// #undef NOXINCLUDES

// #include <kglobal.h>
// #include <klocale.h>
// #include <kdebug.h>
// #include <kstddirs.h>
// #include <kapp.h>

/* Blackbox config */
#include "config.h"

extern "C" KDE_EXPORT QObject *allocate_config(KConfig *conf, QWidget *parent) {
  return (new BlackboxConfig(conf, parent));
}

BlackboxConfig::BlackboxConfig(KConfig *conf, QWidget *parent)
: QObject(parent)
{
#ifdef DEBUG
  kdDebug() << "Blackbox: config ctr\n";
  puts("test\n");
#endif
  KGlobal::locale()->insertCatalogue("libkwinblackbox_config");
  m_bbcfg = new KConfig("kwinblackboxrc");

  gb1 = new QGroupBox(1, Qt::Horizontal, i18n("Blackbox Style Selector"), parent);

  QPushButton *bt = new QPushButton(gb1, "btest");
  bt->setText(QString("test"));

  styleListBox = new QListBox(gb1);
  QWhatsThis::add(styleListBox, i18n("Make your style selection by clicking on a style here. "));

  /*styleLabel = new QLabel(i18n("To manage your blackbox styles, simply click on the "
        "link below to open a Konqueror window. Once shown, you "
        "will be able to add or remove native blackbox styles, by "
        "uncompressing blackbox, openbox, or fluxbox style files "
        "into this directory, or creating symlinks to "
        "existing blackbox styles on your system."), gb1);
  styleLabel->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred, FALSE));*/

  styleLabel = new QLabel(i18n("To manage your blackbox styles, simply click on the "
                "link below to open a Konqueror window. Once shown, you "
                "will be able to add or remove native <b>blackbox, openbox, "
                "or fluxbox</b> styles, by uncompressing the style file "
                "into this directory, or creating symlinks to existing styles on your system."), parent);

  urlLabel = new KURLLabel(parent);
  urlLabel-> setText(i18n("Open Konqueror Window at KDE's blackbox style directory"));

  gb2 = new QGroupBox(1, Qt::Horizontal, i18n("Blackbox Decoration Settings"), parent);

  cbstyleTitleTextColors = new QCheckBox(i18n("Use style &title text colors"), gb2);

  QWhatsThis::add (cbstyleTitleTextColors, i18n("When selected, titlebar colors will follow those set "
      "in the blackbox style. If not selected, the current KDE titlebar colors will be used instead."));

  cbstyleFont = new QCheckBox(i18n("Use style &font"), gb2);

  QWhatsThis::add(cbstyleFont, i18n("When selected, all window titlebars will be drawn "
      "using the font specified in the blackbox style. If not selected, the KDE font will be used instead."));

  cbstyleBG = new QCheckBox(i18n("Attempt to load style background"), gb2);

  QWhatsThis::add(cbstyleBG, i18n("When selected, will attempt to use the background "
      "specified in the style. If not selected, it will be ignored. "));

  // Ensure we track user changes properly
  connect(styleListBox, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));
  connect(urlLabel, SIGNAL(leftClickedURL(const QString &)), this, SLOT(callURL(const QString &)));
  connect(cbstyleTitleTextColors, SIGNAL(clicked()), this, SLOT(slotSelectionChanged()));
  connect(cbstyleFont, SIGNAL(clicked()), this, SLOT(slotSelectionChanged()));
  connect(cbstyleBG, SIGNAL(clicked()), this, SLOT(slotSelectionChanged()));

  // Create the style directory (if not found) ... and obtain the path as we do so.
  localstylestring = KGlobal::dirs()->saveLocation("data", "kwin");

  localstylestring += "/blackbox-styles";
#ifdef DEBUG
  kdDebug() << localstylestring;
#endif
  if(!QFile::exists(localstylestring)) { QDir().mkdir(localstylestring); }

  load(conf); // Load configuration options

  // Watch the blackbox style directory for style additions/removals
  KDirWatch::self()->addDir(localstylestring);
  connect(KDirWatch::self(), SIGNAL(dirty(const QString &)), this, SLOT(findblackboxstyles()));

  // Set the konqui link url
  QString urlstylestring = QString("file://") + localstylestring;
  urlstylestring.replace (QRegExp("~"), "$HOME");
  urlLabel->setURL(urlstylestring);

  // Make the widgets visible in kwindecoration
  gb1->show ();
  styleLabel->show ();
  urlLabel->show ();
  gb2->show ();

  KDirWatch::self()->startScan();
}

BlackboxConfig::~BlackboxConfig() {
  KDirWatch::self()->removeDir(localstylestring);
  KDirWatch::self()->stopScan();
  delete gb2;
  delete urlLabel;
  delete styleLabel;
  delete gb1;
  delete BlackboxConfiga;
}

// Searches for all installed blackbox styles, and adds them to the listBox.
void BlackboxConfig::findblackboxstyles() {
  QStringList dirList = KGlobal::dirs()->findDirs("data", "kwin/blackbox-styles");
  QStringList::ConstIterator it;

  // Remove any old themes in the list (if any)
  styleListBox->clear();
  //styleListBox->insertItem("Spiff");

  // Step through all kwin/icewm-themes directories...
  for(it = dirList.begin(); it != dirList.end(); it++) {
#ifdef DEBUG
    kdDebug() << "theme style= " << *it << endl;
#endif
    // List all directory names only...

    QDir d(*it, QString("*"), QDir::Name, QDir::Files|QDir::Readable);
    if(d.exists()) {
      QFileInfoListIterator it2 (*d.entryInfoList ());
      QFileInfo *finfo;
#ifdef DEBUG
      kdDebug() << "theme yoyo= "<<*it2<<endl ;
#endif
      while((finfo = it2.current())) {
        if(!styleListBox->findItem(finfo->fileName())) { styleListBox->insertItem(finfo->fileName()); }
        ++it2;
#ifdef DEBUG
        kdDebug() << "the " << *it << endl;
#endif
      }
    }
  }

  // Sort the items
  styleListBox->sort();

  QString themeName = BlackboxConfiga->readEntry("Currentstyle", "");

  // Provide a theme alias
  if(themeName == i18n("default")) { themeName = ""; }

  if(themeName == "") { styleListBox->setCurrentItem(styleListBox->findItem("deep")); }
  else { styleListBox->setCurrentItem(styleListBox->findItem(themeName)); }
}

void BlackboxConfig::callURL(const QString &s) {
  kapp->invokeBrowser(s);
}

void BlackboxConfig::slotSelectionChanged() {
  emit changed();
}

// Loads the configurable options from the kwinblackboxrc config file
void BlackboxConfig::load(KConfig *) {
	BlackboxConfiga->setGroup ("General");

	bool override = BlackboxConfiga->readBoolEntry ("styleTitleTextColors", true);
	cbstyleTitleTextColors->setChecked (override);

	override = BlackboxConfiga->readBoolEntry ("styleFont", false);
	cbstyleFont->setChecked (override);

	override = BlackboxConfiga->readBoolEntry ("styleBG", false);
	cbstyleBG->setChecked (override);

	findblackboxstyles ();
}

// Saves the configurable options to the kwinblackboxrc config file
void BlackboxConfig::save (KConfig *)
{
	BlackboxConfiga->setGroup ("General");
	BlackboxConfiga->writeEntry ("styleTitleTextColors",
		cbstyleTitleTextColors->isChecked ());
	BlackboxConfiga->writeEntry ("styleFont", cbstyleFont->isChecked ());
	BlackboxConfiga->writeEntry ("styleBG",
		cbstyleBG->isChecked ());

	if (styleListBox->currentText () == "deep")
		BlackboxConfiga->writeEntry ("Currentstyle", "default");
	else
		BlackboxConfiga->writeEntry ("Currentstyle", styleListBox->currentText ());

	BlackboxConfiga->sync ();
}

// Sets UI widget defaults which must correspond to config defaults
void BlackboxConfig::defaults ()
{
	cbstyleTitleTextColors->setChecked (true);
	cbstyleFont->setChecked (false);
	cbstyleBG->setChecked (false);
	styleListBox->setCurrentItem (styleListBox->findItem ("deep"));
}

#include "config.moc"
