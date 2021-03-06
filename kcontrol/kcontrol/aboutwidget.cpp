/*
  Copyright (c) 2000,2001 Matthias Elter <elter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <qpainter.h>
#include <qwhatsthis.h>
#include <qregexp.h>
#include <qlayout.h>
#include <qfile.h>

#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>
#include <kpixmap.h>
#include <kpixmapeffect.h>
#include <kcursor.h>
#include <kglobalsettings.h>
#include <khtml_part.h>
#include <kapplication.h>
#include <kiconloader.h>

#include "global.h"
#include "aboutwidget.h"
#include "aboutwidget.moc"
#include "modules.h"
#include "moduletreeview.h"

static const char kcc_text[] = I18N_NOOP("KDE Control Center");

static const char title_text[] = I18N_NOOP("Configure your desktop environment.");

static const char intro_text[] = I18N_NOOP("Welcome to the \"KDE Control Center\", "
                                "a central place to configure your "
                                "desktop environment. "
                                "Select an item from the index on the left "
                                "to load a configuration module.");

static const char kcc_infotext[] = I18N_NOOP("KDE Info Center");

static const char title_infotext[] = I18N_NOOP("Get system and desktop environment information");

static const char intro_infotext[] = I18N_NOOP("Welcome to the \"KDE Info Center\", "
                                "a central place to find information about your "
                                "computer system.");

static const char use_text[] = I18N_NOOP("Use the \"Search\" field if you are unsure "
                        "where to look for a particular configuration option.");

static const char version_text[] = I18N_NOOP("KDE version:");
static const char user_text[] = I18N_NOOP("User:");
static const char host_text[] = I18N_NOOP("Hostname:");
static const char system_text[] = I18N_NOOP("System:");
static const char release_text[] = I18N_NOOP("Release:");
static const char machine_text[] = I18N_NOOP("Machine:");

AboutWidget::AboutWidget(QWidget *parent , const char *name, QListViewItem* category, const QString &caption)
   : QHBox(parent, name),
      _moduleList(false),
      _category(category),
      _caption(caption)
{
    if (_category)
      _moduleList = true;

    setMinimumSize(400, 400);

    // set qwhatsthis help
    QWhatsThis::add(this, i18n(intro_text));
    _viewer = new KHTMLPart( this, "_viewer" );
    _viewer->widget()->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    connect( _viewer->browserExtension(),
             SIGNAL(openURLRequest(const KURL&, const KParts::URLArgs&)),
             this, SLOT(slotModuleLinkClicked(const KURL&)) );
    updatePixmap();
}

void AboutWidget::setCategory( QListViewItem* category, const QString& icon, const QString &caption )
{
  _icon = icon;
  _caption = caption;
  _category = category;
  if ( _category )
    _moduleList = true;
  else
    _moduleList = true;

  // Update the pixmap to be shown:
  updatePixmap();
}

void AboutWidget::updatePixmap()
{
    QString file = locate(  "data", "kcontrol/about/main.html" );
    QFile f( file );
    f.open( IO_ReadOnly );
    QTextStream t(  &f );
    QString res = t.read();

    res = res.arg(  locate(  "data", "kdeui/about/kde_infopage.css" ) );
    if (  kapp->reverseLayout() )
        res = res.arg(  "@import \"%1\";" ).arg(  locate(  "data", "kdeui/about/kde_infopage_rtl.css" ) );
    else
        res = res.arg(  "" );


    QString title, intro, caption;
    if (KCGlobal::isInfoCenter())
    {
       res = res.arg(i18n(kcc_infotext))
                .arg(i18n(title_infotext))
                .arg(i18n(intro_infotext));
    }
    else
    {
       res = res.arg(i18n(kcc_text))
                .arg(i18n(title_text))
                .arg(i18n(intro_text));
    }

    QString content;

    if (!_moduleList)
    {
        content += "<table class=\"kc_table\">\n";
#define KC_HTMLROW( a, b ) "<tr><td class=\"kc_leftcol\">" + i18n( a ) + "</td><td class=\"kc_rightcol\">" + b + "</tr>\n"
        content += KC_HTMLROW( version_text, KCGlobal::kdeVersion() );
        content += KC_HTMLROW( user_text, KCGlobal::userName() );
        content += KC_HTMLROW( host_text, KCGlobal::hostName() );
        content += KC_HTMLROW( system_text, KCGlobal::systemName() );
        content += KC_HTMLROW( release_text, KCGlobal::systemRelease() );
        content += KC_HTMLROW( machine_text, KCGlobal::systemMachine() );
#undef KC_HTMLROW
        content += "</table>\n";
        content += "<p class=\"kc_use_text\">" + i18n( use_text ) + "</p>\n";
    }
    else
    {
        KIconLoader *loader = KGlobal::instance()->iconLoader();
        QString iconPath;
        if (!_icon.isEmpty()) {
            iconPath = loader->iconPath( _icon, KIcon::Toolbar );
            content += "<div id=\"tableTitle\"><img src=\"" + iconPath +" \"</a>&nbsp;" + _caption + "</div>";
        }

        content += "<table class=\"kc_table\">\n";
        // traverse the list
        QListViewItem* pEntry = _category;
        while (pEntry != NULL)
        {
            QString szName;
            QString szComment;
            ConfigModule *module = static_cast<ModuleTreeItem*>(pEntry)->module();
            /* TODO: work out link */
            content += "<tr><td class=\"kc_leftcol\">";
            if (module)
            {
                szName = module->moduleName();
                szComment = module->comment();
                iconPath = loader->iconPath( module->icon(), KIcon::Small );

                content += "<img src=\"" + iconPath +" \"</a>&nbsp;<a href=\"%1\" class=\"kcm_link\">" + szName + "</a></td><td class=\"kc_rightcol\">" + szComment;
                KURL moduleURL( QString("kcm://%1").arg(QString().sprintf("%p",module)) );
                QString linkURL( moduleURL.url() );
                content = content.arg( linkURL );
                _moduleMap.insert( linkURL, module );
            }
            else
            {
                szName = static_cast<ModuleTreeItem*>(pEntry)->caption();
                content += szName + "</td><td class=\"kc_rightcol\">" + szName;
            }
            content += "</td></tr>\n";
            pEntry = pEntry->nextSibling();
        }
        content += "</table>";
    }
    _viewer->begin(KURL( file ));
    _viewer->write( res.arg( content ) );
    _viewer->end();
}

void AboutWidget::slotModuleLinkClicked( const KURL& url )
{
	ConfigModule* module;
	module = _moduleMap[url.url()];
	if ( module )
		emit moduleSelected( module );
}

