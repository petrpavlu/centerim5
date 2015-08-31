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

#include "ConversationRoomList.h"

// Move the widget to a position according to sorting function.
void ConversationRoomList::moveToSortedPosition(Buddy *new_buddy)
{
  Buddy *buddy = nullptr;
  Children::iterator iter;

  g_assert(new_buddy != nullptr);

  for (iter = children_.begin(); iter != children_.end(); ++iter) {
    buddy = dynamic_cast<Buddy *>(*iter);
    g_assert(buddy != nullptr);

    // If buddy is in the list already, skip it.
    if (*buddy == *new_buddy)
      continue;

    // Break once insertion point is found.
    if (!Buddy::less_than_op_away_name(*buddy, *new_buddy))
      break;
  }

  // Do not change anything if wanting to put into the same position (also
  // returns on single node case).
  if (*buddy == *new_buddy)
    return;

  // TODO: HACK? Seems to be required to have the ListBox
  //   reorder the widgets after calling moveWidget*
  //   Doesn't seem right...
  // reposition_widgets = true;

  // Insert after if at end.
  if (iter == children_.end())
    moveWidgetAfter(*new_buddy, *buddy);
  else
    moveWidgetBefore(*new_buddy, *buddy);
}

void ConversationRoomList::add_users(GList *cbuddies, gboolean /*new_arrivals*/)
{
  for (GList *l = cbuddies; l != nullptr; l = l->next) {
    PurpleConvChatBuddy *pbuddy = static_cast<PurpleConvChatBuddy *>(l->data);

    auto buddy = new Buddy(pbuddy);
    buddy->setButtonText();
    buddy_map_[pbuddy->name] = buddy;

    appendWidget(*buddy);

    moveToSortedPosition(buddy);
  }
}

void ConversationRoomList::rename_user(
  const char *old_name, const char *new_name, const char * /*new_alias*/)
{
  // The old PurpleConvChatBuddy is still valid while
  // this function is executing
  // PurpleConvChatBuddy * old_pbuddy = purple_conv_chat_cb_find(
  //  conv_->u.chat, old_name);

  PurpleConvChat *conv = PURPLE_CONV_CHAT(conv_);
  g_assert(conv != nullptr);

  PurpleConvChatBuddy *new_pbuddy = purple_conv_chat_cb_find(conv, new_name);
  // g_assert(old_pbuddy != NULL);
  g_assert(new_pbuddy != nullptr);

  // NOTE: PurpleConvChatBuddy::ui_data is pidgin 2.9!!
  // Buddy * buddy = static_cast<Buddy *>(old_pbuddy->ui_data);
  Buddy *buddy = buddy_map_[old_name];
  g_assert(buddy != nullptr);

  // Update buddy.
  buddy->setPurpleBuddy(new_pbuddy);

  // Update buddy map.
  buddy_map_.erase(old_name);
  buddy_map_[new_name] = buddy;

  // Move and then update.
  moveToSortedPosition(buddy);
  buddy->setButtonText();
}

void ConversationRoomList::remove_users(GList *users)
{
  for (GList *l = users; l != nullptr; l = l->next) {
    const char *name = static_cast<const char *>(l->data);

    // NOTE: can't remove purple_conv_chat_cb_find, because the user
    //   and PurpleConvChatBuddy has already been removed

    BuddyMapIter iter = buddy_map_.find(name);

    if (buddy_map_.end() != iter) {
      buddy_map_.erase(iter);
      // NOTE: this deletes the buddy object.
      removeWidget(*iter->second);
    }
  }
}

void ConversationRoomList::update_user(const char *user)
{
  PurpleConvChat *conv = PURPLE_CONV_CHAT(conv_);
  g_assert(conv != nullptr);

  PurpleConvChatBuddy *pbuddy = purple_conv_chat_cb_find(conv, user);
  g_assert(pbuddy != nullptr);

  // NOTE: PurpleConvChatBuddy::ui_data is pidgin 2.9!!
  // Buddy * buddy = static_cast<Buddy *>(pbuddy->ui_data);
  Buddy *buddy = buddy_map_[user];
  g_assert(buddy != nullptr);

  // Move and then update.
  moveToSortedPosition(buddy);
  buddy->setButtonText();
}

ConversationRoomList::Buddy::Buddy(PurpleConvChatBuddy *pbuddy)
  : CppConsUI::Button(AUTOSIZE, 1, ""), pbuddy_(pbuddy)
{
  // Set ui data.
  // NOTE: PurpleConvChatBuddy::ui_data is pidgin 2.9!!
  // pbuddy_->ui_data = static_cast<void*>(this);
}

ConversationRoomList::Buddy::~Buddy()
{
}

void ConversationRoomList::Buddy::readFlags(
  bool &is_op, bool &is_typing, bool &is_away) const
{
  g_assert(pbuddy_ != nullptr);

  PurpleConvChatBuddyFlags flags = pbuddy_->flags;

  // TODO: how about founder?  Does that matter?

  is_op = (((flags & PURPLE_CBFLAGS_OP) != 0));
  is_typing = (((flags & PURPLE_CBFLAGS_TYPING) != 0));
#if PURPLE_VERSION_CHECK(2, 8, 0)
  is_away = (((flags & PURPLE_CBFLAGS_AWAY) != 0));
#else
  is_away = false;
#endif
}

void ConversationRoomList::Buddy::setButtonText()
{
  char *text = displayText();
  setText(text);
  g_free(text);
}

char *ConversationRoomList::Buddy::displayText() const
{
  char *ret;

  bool is_op = false;
  bool is_typing = false;
  bool is_away = false;

  readFlags(is_op, is_typing, is_away);

  ret = g_strdup_printf("[%s] %s%s%s", (is_away ? "a" : "o"),
    (is_op ? "@" : ""), displayName(), (is_typing ? "*" : ""));

  // TODO: elide long names?

  return ret;
}

const char *ConversationRoomList::Buddy::displayName() const
{
  g_assert(pbuddy_ != nullptr);

  // Prefer alias.
  // NOTE: pbuddy_->alias_key isn't used yet... (according to docs)
  if (pbuddy_->alias != nullptr)
    return pbuddy_->alias;
  else
    return pbuddy_->name;
}

bool ConversationRoomList::Buddy::operator==(const Buddy &rhs)
{
  return pbuddy_ == rhs.pbuddy_;
}

void ConversationRoomList::Buddy::setPurpleBuddy(PurpleConvChatBuddy *pbuddy)
{
  pbuddy_ = pbuddy;
  // NOTE: PurpleConvChatBuddy::ui_data is pidgin 2.9!!
  // pbuddy_->ui_data = static_cast<void*>(this);
}

bool ConversationRoomList::Buddy::less_than_op_away_name(
  const Buddy &lhs, const Buddy &rhs)
{
  // Sort order:
  //
  // 1. ops first
  // 2. online (vs away)
  // 3. name/alias

  bool lhs_is_op, lhs_is_typing, lhs_is_away;
  bool rhs_is_op, rhs_is_typing, rhs_is_away;

  lhs.readFlags(lhs_is_op, lhs_is_typing, lhs_is_away);
  rhs.readFlags(rhs_is_op, rhs_is_typing, rhs_is_away);

  // Probably a more elegant way to do this.
  if (lhs_is_op && !rhs_is_op)
    return true;
  else if (!lhs_is_op && rhs_is_op)
    return false;
  // Equal op or non-op status.
  else {
    if (lhs_is_away && !rhs_is_away)
      return false;
    else if (!lhs_is_away && rhs_is_away)
      return true;
    // On equal online/away status.
    else
      return g_utf8_collate(lhs.displayName(), rhs.displayName()) < 0;
  }
}

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
