/*
  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 
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

#ifndef __moduletreeview_h__
#define __moduletreeview_h__

#include <qpalette.h>
#include <qptrlist.h>
#include <qlistview.h>
#include <klistview.h>
#include <qdict.h>


class ConfigModule;
class ConfigModuleList;
class QPainter;

class ModuleTreeItem : public QListViewItem
{

public:
  ModuleTreeItem(QListViewItem *parent, ConfigModule *module = 0);
  ModuleTreeItem(QListViewItem *parent, const QString& text);
  ModuleTreeItem(QListView *parent, ConfigModule *module = 0);
  ModuleTreeItem(QListView *parent, const QString& text);

  void setTag(const QString& tag) { _tag = tag; }
  void setCaption(const QString& caption) { _caption = caption; }
  void setModule(ConfigModule *m) { _module = m; }
  QString tag() const { return _tag; };
  QString caption() const { return _caption; };
  QString icon() const { return _icon; };
  ConfigModule *module() { return _module; };
  void regChildIconWidth(int width);
  int  maxChildIconWidth() { return _maxChildIconWidth; }

  void setPixmap(int column, const QPixmap& pm);
  void setGroup(const QString &path);

protected:
  void paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int align );

private:
  ConfigModule *_module;
  QString       _tag;
  QString       _caption;
  int _maxChildIconWidth;
  QString       _icon;
};

class ModuleTreeView : public KListView
{
  Q_OBJECT

public:
  ModuleTreeView(ConfigModuleList *list, QWidget * parent = 0, const char * name = 0);

  void makeSelected(ConfigModule* module);
  void makeVisible(ConfigModule *module);
  void fill();
  QSize sizeHint() const;

signals:
  void moduleSelected(ConfigModule*);
  void categorySelected(QListViewItem*);

protected slots:
  void slotItemSelected(QListViewItem*);

protected:
  void updateItem(ModuleTreeItem *item, ConfigModule* module); 
  void keyPressEvent(QKeyEvent *);
  void fill(ModuleTreeItem *parent, const QString &parentPath);

private:
  ConfigModuleList *_modules;
};

#endif
