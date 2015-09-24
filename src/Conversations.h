// Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
// Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
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

#ifndef CONVERSATIONS_H
#define CONVERSATIONS_H

#include "Conversation.h"

#include <cppconsui/Window.h>
#include <cppconsui/HorizontalListBox.h>
#include <cppconsui/Label.h>
#include <cppconsui/Spacer.h>
#include <libpurple/purple.h>
#include <vector>

#define CONVERSATIONS (Conversations::instance())

class Conversations : public CppConsUI::Window {
public:
  static Conversations *instance();

  // Window
  virtual void onScreenResized() override;

  void focusActiveConversation();
  void focusConversation(int i);
  void focusPrevConversation();
  void focusNextConversation();

  void setExpandedConversations(bool expanded);

  bool getSendTypingPref() const { return send_typing_; }

private:
  struct ConvChild {
    Conversation *conv;
    CppConsUI::Label *label;
    char typing_status;
  };

  typedef std::vector<ConvChild> ConversationsVector;

  ConversationsVector conversations_;

  // Active conversation, -1 if none.
  int active_;

  CppConsUI::HorizontalListBox *outer_list_;
  CppConsUI::Spacer *left_spacer_;
  CppConsUI::Spacer *right_spacer_;
  CppConsUI::HorizontalListBox *conv_list_;

  // Cached value of the "/purple/conversations/im/send_typing" preference.
  bool send_typing_;

  PurpleConversationUiOps centerim_conv_ui_ops_;

  static Conversations *my_instance_;

  Conversations();
  virtual ~Conversations() override;
  CONSUI_DISABLE_COPY(Conversations);

  static void init();
  static void finalize();
  friend class CenterIM;

  // Find PurpleConversation in conversations.
  int findConversation(PurpleConversation *conv);

  int prevActiveConversation(int current);
  int nextActiveConversation(int current);

  void activateConversation(int i);

  // Update a single conversation label.
  void updateLabel(int i);
  // Update all conversation labels.
  void updateLabels();

  static void create_conversation_(PurpleConversation *conv)
  {
    CONVERSATIONS->create_conversation(conv);
  }
  static void destroy_conversation_(PurpleConversation *conv)
  {
    CONVERSATIONS->destroy_conversation(conv);
  }
  static void write_conv_(PurpleConversation *conv, const char *name,
    const char *alias, const char *message, PurpleMessageFlags flags,
    time_t mtime)
  {
    CONVERSATIONS->write_conv(conv, name, alias, message, flags, mtime);
  }
  static void chat_add_users_(
    PurpleConversation *conv, GList *cbuddies, gboolean new_arrivals)
  {
    CONVERSATIONS->chat_add_users(conv, cbuddies, new_arrivals);
  }
  static void chat_rename_user_(PurpleConversation *conv, const char *old_name,
    const char *new_name, const char *new_alias)
  {
    CONVERSATIONS->chat_rename_user(conv, old_name, new_name, new_alias);
  }
  static void chat_remove_users_(PurpleConversation *conv, GList *users)
  {
    CONVERSATIONS->chat_remove_users(conv, users);
  }
  static void chat_update_user_(PurpleConversation *conv, const char *user)
  {
    CONVERSATIONS->chat_update_user(conv, user);
  }
  static void present_(PurpleConversation *conv)
  {
    CONVERSATIONS->present(conv);
  }

  // Internal utility to get a room list for a conversation. Returns NULL if
  // none is present.
  ConversationRoomList *getRoomList(PurpleConversation *conv);

  void create_conversation(PurpleConversation *conv);
  void destroy_conversation(PurpleConversation *conv);
  void write_conv(PurpleConversation *conv, const char *name, const char *alias,
    const char *message, PurpleMessageFlags flags, time_t mtime);
  void chat_add_users(
    PurpleConversation *conv, GList *cbuddies, gboolean new_arrivals);
  void chat_rename_user(PurpleConversation *conv, const char *old_name,
    const char *new_name, const char *new_alias);
  void chat_remove_users(PurpleConversation *conv, GList *users);
  void chat_update_user(PurpleConversation *conv, const char *user);
  void present(PurpleConversation *conv);

  static void buddy_typing_(
    PurpleAccount *account, const char *who, gpointer data)
  {
    reinterpret_cast<Conversations *>(data)->buddy_typing(account, who);
  }
  void buddy_typing(PurpleAccount *account, const char *who);

  // Called when "/purple/conversations/im/send_typing" preference changes.
  static void send_typing_pref_change_(
    const char *name, PurplePrefType type, gconstpointer val, gpointer data)
  {
    reinterpret_cast<Conversations *>(data)->send_typing_pref_change(
      name, type, val);
  }
  void send_typing_pref_change(
    const char *name, PurplePrefType type, gconstpointer val);
};

#endif // CONVERSATIONS_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
