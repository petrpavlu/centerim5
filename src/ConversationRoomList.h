// Copyright (C) 2015 Wade Berrier <wberrier@gmail.com>
//
// This file is part of CenterIM.
//
// CenterIM is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// CenterIM is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __CONVERSATION_ROOM_LIST_H__
#define __CONVERSATION_ROOM_LIST_H__

#include <cppconsui/ListBox.h>
#include <cppconsui/Button.h>
#include <libpurple/purple.h>

#include <map>

class ConversationRoomList : public CppConsUI::ListBox {
public:
  ConversationRoomList(int w, int h, PurpleConversation *conv)
    : CppConsUI::ListBox(w, h), conv_(conv)
  {
  }
  virtual ~ConversationRoomList() {}

  // Libpurple chatroom interfaces.
  void add_users(GList *cbuddies, gboolean new_arrivals);
  void rename_user(
    const char *old_name, const char *new_name, const char *new_alias);
  void remove_users(GList *users);
  void update_user(const char *user);

protected:
  PurpleConversation *conv_;

  // Represents a widget as well as pointer to libpurple data.
  class Buddy : public CppConsUI::Button {
  public:
    Buddy(PurpleConvChatBuddy *pbuddy);

    virtual ~Buddy();

    // Sets button text with displayName.
    void setButtonText();

    // Uses pbuddy info to generate button displayText. Returns a newly
    // allocated string that must be freed by the caller.
    char *displayText() const;

    // Prefer alias if it exists, and fall back on name.
    const char *displayName() const;

    // TODO
    // void onActivate(Button &);

    // Update purple buddy (for rename case).
    void setPurpleBuddy(PurpleConvChatBuddy *pbuddy);

    // Sorting method for: op/away/display_name
    // if less than: give priority
    // The idea that if more sorting methods are desired, they can be swapped
    // out at runtime based on config.
    static bool less_than_op_away_name(const Buddy &lhs, const Buddy &rhs);

    bool operator==(const Buddy &rhs);

  private:
    void readFlags(bool &is_op, bool &is_typing, bool &is_away) const;

    // Note: When remove_users op is called, this pointer is invalidated!.
    PurpleConvChatBuddy *pbuddy_;

    // Force public constructor.
    Buddy();

    CONSUI_DISABLE_COPY(Buddy);
  };

  // Move buddy to sorted position.
  void moveToSortedPosition(Buddy *buddy);

  // Have to keep this mapping to remove users
  // because when libpurple calls remove_user, the user is already
  // gone, along with the "ui_data"
  // Otherwise could store "name" in Buddy and iterate through "children"
  // NOTE: turns out that ui_data is new in libpurple 2.9, so for previous
  // versions this map is required anyways... :/
  std::map<std::string, Buddy *> buddy_map_;
  typedef std::map<std::string, Buddy *>::iterator BuddyMapIter;

private:
  CONSUI_DISABLE_COPY(ConversationRoomList);
};

#endif

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
