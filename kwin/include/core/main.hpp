/*****************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 1999, 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

You can Freely distribute this program under the GNU General Public
License. See the file "COPYING" for the exact licensing terms.
******************************************************************/

#ifndef MAIN_H
#define MAIN_H

namespace KWinInternal {
  class Application : public  KApplication {
    Q_OBJECT
    public:
      Application();
      ~Application();

    protected:
      bool x11EventFilter( XEvent * );

    private slots:
      void lostSelection();

    private:
      KWinSelectionOwner owner;
  };
}; /* namespace KWinInternal */

#endif /* MAIN_H */
