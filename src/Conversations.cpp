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
// along with CenterIM.  If not, see <http://www.gnu.org/licenses/>.

#include "Conversations.h"

#include "gettext.h"
#include <cstring>

Conversations *Conversations::my_instance_ = nullptr;

Conversations *Conversations::instance()
{
  return my_instance_;
}

void Conversations::onScreenResized()
{
  CppConsUI::Rect r = CENTERIM->getScreenArea(CenterIM::CHAT_AREA);
  r.y = r.getBottom();
  r.height = 1;

  moveResizeRect(r);
}

void Conversations::focusActiveConversation()
{
  activateConversation(active_);
}

void Conversations::focusConversation(int i)
{
  g_assert(i >= 1);

  if (conversations_.empty())
    return;

  // The external conversation #1 is the internal conversation #0.
  i -= 1;

  int s = static_cast<int>(conversations_.size());
  if (i < s)
    activateConversation(i);
  else {
    // If there is less than i active conversations then active the last one.
    activateConversation(s - 1);
  }
}

void Conversations::focusPrevConversation()
{
  int i = prevActiveConversation(active_);
  if (i != -1)
    activateConversation(i);
}

void Conversations::focusNextConversation()
{
  int i = nextActiveConversation(active_);
  if (i != -1)
    activateConversation(i);
}

void Conversations::setExpandedConversations(bool expanded)
{
  if (expanded) {
    // Make small room on the left and right.
    left_spacer_->setWidth(1);
    right_spacer_->setWidth(1);
  }
  else {
    left_spacer_->setWidth(0);
    right_spacer_->setWidth(0);
  }

  // Trigger the Conversation::show() method to change the scrollbar setting of
  // the active conversation.
  activateConversation(active_);
}

Conversations::Conversations()
  : Window(0, 0, 80, 1, TYPE_NON_FOCUSABLE, false), active_(-1)
{
  setColorScheme(CenterIM::SCHEME_CONVERSATION);

  outer_list_ = new CppConsUI::HorizontalListBox(AUTOSIZE, 1);
  addWidget(*outer_list_, 0, 0);

  left_spacer_ = new CppConsUI::Spacer(0, 1);
  right_spacer_ = new CppConsUI::Spacer(0, 1);
  conv_list_ = new CppConsUI::HorizontalListBox(AUTOSIZE, 1);

  outer_list_->appendWidget(*left_spacer_);
  outer_list_->appendWidget(*conv_list_);
  outer_list_->appendWidget(*right_spacer_);

  // Init preferences.
  purple_prefs_add_none(CONF_PREFIX "/chat");
  purple_prefs_add_int(CONF_PREFIX "/chat/partitioning", 80);
  purple_prefs_add_int(CONF_PREFIX "/chat/roomlist_partitioning", 80);
  purple_prefs_add_bool(CONF_PREFIX "/chat/beep_on_msg", false);

  // send_typing caching.
  send_typing_ = purple_prefs_get_bool("/purple/conversations/im/send_typing");
  purple_prefs_connect_callback(this, "/purple/conversations/im/send_typing",
    send_typing_pref_change_, this);

  std::memset(&centerim_conv_ui_ops_, 0, sizeof(centerim_conv_ui_ops_));
  centerim_conv_ui_ops_.create_conversation = create_conversation_;
  centerim_conv_ui_ops_.destroy_conversation = destroy_conversation_;
  // centerim_conv_ui_ops_.write_chat = ;
  // centerim_conv_ui_ops_.write_im = ;
  centerim_conv_ui_ops_.write_conv = write_conv_;
  centerim_conv_ui_ops_.chat_add_users = chat_add_users_;
  centerim_conv_ui_ops_.chat_rename_user = chat_rename_user_;
  centerim_conv_ui_ops_.chat_remove_users = chat_remove_users_;
  centerim_conv_ui_ops_.chat_update_user = chat_update_user_;

  centerim_conv_ui_ops_.present = present_;
  // centerim_conv_ui_ops_.has_focus = ;
  // centerim_conv_ui_ops_.custom_smiley_add = ;
  // centerim_conv_ui_ops_.custom_smiley_write = ;
  // centerim_conv_ui_ops_.custom_smiley_close = ;
  // centerim_conv_ui_ops_.send_confirm = ;

  // Setup callbacks for conversations.
  purple_conversations_set_ui_ops(&centerim_conv_ui_ops_);

  void *handle = purple_conversations_get_handle();
  purple_signal_connect(
    handle, "buddy-typing", this, PURPLE_CALLBACK(buddy_typing_), this);
  purple_signal_connect(
    handle, "buddy-typing-stopped", this, PURPLE_CALLBACK(buddy_typing_), this);

  // Setup callbacks for connections in relation to conversations.
  void *connections_handle = purple_connections_get_handle();
  purple_signal_connect(connections_handle, "signed-on", this,
    PURPLE_CALLBACK(account_signed_on_), this);

  onScreenResized();
}

Conversations::~Conversations()
{
  // Close all opened conversations.
  while (!conversations_.empty())
    purple_conversation_destroy(
      conversations_.front().conv->getPurpleConversation());

  purple_conversations_set_ui_ops(nullptr);
  purple_prefs_disconnect_by_handle(this);
  purple_signals_disconnect_by_handle(this);
}

void Conversations::init()
{
  g_assert(my_instance_ == nullptr);

  my_instance_ = new Conversations;
  my_instance_->show();
}

void Conversations::finalize()
{
  g_assert(my_instance_ != nullptr);

  delete my_instance_;
  my_instance_ = nullptr;
}

int Conversations::findConversation(PurpleConversation *conv)
{
  for (int i = 0; i < static_cast<int>(conversations_.size()); ++i)
    if (conversations_[i].conv->getPurpleConversation() == conv)
      return i;

  return -1;
}

int Conversations::prevActiveConversation(int current)
{
  g_assert(current < static_cast<int>(conversations_.size()));

  if (conversations_.empty()) {
    // No conversations.
    return -1;
  }

  if (current == 0) {
    // Return the last conversation.
    return conversations_.size() - 1;
  }

  return current - 1;
}

int Conversations::nextActiveConversation(int current)
{
  g_assert(current < static_cast<int>(conversations_.size()));

  if (conversations_.empty()) {
    // No conversations.
    return -1;
  }

  if (current == static_cast<int>(conversations_.size() - 1)) {
    // Return the first conversation.
    return 0;
  }

  return current + 1;
}

void Conversations::activateConversation(int i)
{
  g_assert(i >= -1);
  g_assert(i < static_cast<int>(conversations_.size()));

  if (active_ == i) {
    if (active_ != -1)
      conversations_[active_].conv->show();
    return;
  }

  if (i != -1) {
    // Show a new active conversation.
    conversations_[i].label->setVisibility(true);
    conversations_[i].label->setColorScheme(
      CenterIM::SCHEME_CONVERSATION_ACTIVE);
    conversations_[i].conv->show();
  }

  // Hide old active conversation if there is any.
  if (active_ != -1) {
    conversations_[active_].label->setColorScheme(0);
    conversations_[active_].conv->hide();
  }

  active_ = i;
}

void Conversations::updateLabel(int i)
{
  g_assert(i >= 0);
  g_assert(i < static_cast<int>(conversations_.size()));

  char *name = g_strdup_printf(
    " %d|%s%c", i + 1, purple_conversation_get_title(
                         conversations_[i].conv->getPurpleConversation()),
    conversations_[i].typing_status);
  conversations_[i].label->setText(name);
  g_free(name);
}

void Conversations::updateLabels()
{
  // Note: This can be a little slow if there are too many open conversations.
  for (int i = 0; i < static_cast<int>(conversations_.size()); ++i)
    updateLabel(i);
}

void Conversations::create_conversation(PurpleConversation *conv)
{
  g_return_if_fail(conv != nullptr);
  g_return_if_fail(findConversation(conv) == -1);

  PurpleConversationType type = purple_conversation_get_type(conv);
  if (type != PURPLE_CONV_TYPE_IM && type != PURPLE_CONV_TYPE_CHAT) {
    purple_conversation_destroy(conv);
    LOG->error(_("Unhandled conversation type '%d'."), type);
    return;
  }

  auto conversation = new Conversation(conv);

  conv->ui_data = static_cast<void *>(conversation);

  ConvChild c;
  c.conv = conversation;
  c.label = new CppConsUI::Label(AUTOSIZE, 1);
  c.typing_status = ' ';
  conv_list_->appendWidget(*c.label);
  conversations_.push_back(c);
  updateLabels();

  // Show the first conversation if there is not any already.
  if (active_ == -1)
    activateConversation(conversations_.size() - 1);
}

void Conversations::destroy_conversation(PurpleConversation *conv)
{
  g_return_if_fail(conv != nullptr);

  int i = findConversation(conv);

  // Destroying unhandled conversation type.
  if (i == -1)
    return;

  if (i == active_) {
    if (conversations_.size() == 1) {
      // The last conversation is closed.
      active_ = -1;
    }
    else {
      if (active_ == static_cast<int>(conversations_.size() - 1))
        focusPrevConversation();
      else
        focusNextConversation();
    }
  }

  delete conversations_[i].conv;
  conv_list_->removeWidget(*conversations_[i].label);
  conversations_.erase(conversations_.begin() + i);

  if (active_ > i) {
    // Fix up the number of the active conversation.
    --active_;
  }

  updateLabels();
}

void Conversations::write_conv(PurpleConversation *conv, const char *name,
  const char *alias, const char *message, PurpleMessageFlags flags,
  time_t mtime)
{
  g_return_if_fail(conv != nullptr);

  int i = findConversation(conv);

  // Message to unhandled conversation type.
  if (i == -1)
    return;

  if (i != active_)
    conversations_[i].label->setColorScheme(CenterIM::SCHEME_CONVERSATION_NEW);

  // Delegate it to Conversation object.
  conversations_[i].conv->write(name, alias, message, flags, mtime);
}

ConversationRoomList *Conversations::getRoomList(PurpleConversation *conv)
{
  if (conv != nullptr) {
    Conversation *conversation = static_cast<Conversation *>(conv->ui_data);
    if (conversation != nullptr)
      return conversation->getRoomList();
  }

  return nullptr;
}

void Conversations::chat_add_users(
  PurpleConversation *conv, GList *cbuddies, gboolean new_arrivals)
{
  ConversationRoomList *room_list = getRoomList(conv);
  if (room_list != nullptr)
    room_list->add_users(cbuddies, new_arrivals);
}

void Conversations::chat_rename_user(PurpleConversation *conv,
  const char *old_name, const char *new_name, const char *new_alias)
{
  ConversationRoomList *room_list = getRoomList(conv);
  if (room_list != nullptr)
    room_list->rename_user(old_name, new_name, new_alias);
}

void Conversations::chat_remove_users(PurpleConversation *conv, GList *users)
{
  ConversationRoomList *room_list = getRoomList(conv);
  if (room_list != nullptr)
    room_list->remove_users(users);
}

void Conversations::chat_update_user(PurpleConversation *conv, const char *user)
{
  ConversationRoomList *room_list = getRoomList(conv);
  if (room_list != nullptr)
    room_list->update_user(user);
}

void Conversations::present(PurpleConversation *conv)
{
  g_return_if_fail(conv);

  int i = findConversation(conv);

  // Unhandled conversation type.
  if (i == -1)
    return;

  activateConversation(i);
}

void Conversations::buddy_typing(PurpleAccount *account, const char *who)
{
  PurpleConversation *conv =
    purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM, who, account);
  if (!conv)
    return;

  int i = findConversation(conv);
  if (i == -1) {
    // This should probably never happen.
    return;
  }

  PurpleConvIm *im = PURPLE_CONV_IM(conv);
  g_assert(im != nullptr);
  if (purple_conv_im_get_typing_state(im) == PURPLE_TYPING)
    conversations_[i].typing_status = '*';
  else
    conversations_[i].typing_status = ' ';

  updateLabel(i);
}

void Conversations::account_signed_on(PurpleConnection *gc)
{
  for (ConvChild &conv_child : conversations_) {
    PurpleConversation *purple_conv = conv_child.conv->getPurpleConversation();

    // Only process chats for this connection.
    if (purple_conversation_get_type(purple_conv) != PURPLE_CONV_TYPE_CHAT ||
      purple_conversation_get_gc(purple_conv) != gc)
      continue;

    // TODO Add and consult the "want-to-rejoin" configuration parameter?

    // Look up the chat from the buddy list.
    PurpleChat *purple_chat =
      purple_blist_find_chat(purple_conversation_get_account(purple_conv),
        purple_conversation_get_name(purple_conv));

    GHashTable *components = NULL;

    if (purple_chat != nullptr)
      components = purple_chat_get_components(purple_chat);
    else {
      // Use defaults if the chat cannot be found.
      PurplePluginProtocolInfo *info =
        PURPLE_PLUGIN_PROTOCOL_INFO(purple_connection_get_prpl(gc));
      components =
        info->chat_info_defaults(gc, purple_conversation_get_name(purple_conv));
    }

    serv_join_chat(gc, components);
  }
}

void Conversations::send_typing_pref_change(
  const char *name, PurplePrefType /*type*/, gconstpointer /*val*/)
{
  g_assert(std::strcmp(name, "/purple/conversations/im/send_typing") == 0);
  send_typing_ = purple_prefs_get_bool(name);
}

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
