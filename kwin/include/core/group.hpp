/*****************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 1999, 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

You can Freely distribute this program under the GNU General Public
License. See the file "COPYING" for the exact licensing terms.
******************************************************************/

#ifndef KWIN_GROUP_H
#define KWIN_GROUP_H

namespace KWinInternal {
  class Client;
  class Workspace;
  /** \class Group
   */
  class Group {
    public:
      Group( Window leader, Workspace* workspace );
      ~Group();
      inline Window leader() const { return leader_wid; }
      inline const Client* leaderClient() const { return leader_client; }
      inline Client* leaderClient() { return leader_client; }
      inline const ClientList& members() const { return _members; }
      QPixmap icon() const;
      QPixmap miniIcon() const;
      void addMember( Client* member );
      void removeMember( Client* member );
      void gotLeader( Client* leader );
      void lostLeader();
      inline Workspace* workspace() { return _workspace; }
      bool groupEvent( XEvent* e );
      void updateUserTime( Time time = CurrentTime );
      inline Time userTime() const { return user_time; }
      void ref();
      void deref();

    private:
      void getIcons();
      void startupIdChanged();
      ClientList _members;
      Client* leader_client;
      Window leader_wid;
      Workspace* _workspace;
      NETWinInfo* leader_info;
      Time user_time;
      int refcount;
  };

  typedef QValueList<Group*> GroupList;
  typedef QValueList<const Group*> ConstGroupList;
}; /* namespace KWinInternal */

#endif /* KWIN_GROUP_H */
