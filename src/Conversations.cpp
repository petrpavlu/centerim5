/*
 * Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
 *
 * This file is part of CenterIM.
 *
 * CenterIM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * CenterIM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "Conversations.h"

#include "gettext.h"

Conversations *Conversations::my_instance = NULL;

Conversations *Conversations::instance()
{
  return my_instance;
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
  activateConversation(active);
}

void Conversations::focusConversation(int i)
{
  g_assert(i >= 1);

  if (conversations.empty())
    return;

  // the external conversation #1 is the internal conversation #0
  i -= 1;

  int s = static_cast<int>(conversations.size());
  if (i < s)
    activateConversation(i);
  else {
    // if there is less than i active conversations then active the last one
    activateConversation(s - 1);
  }
}

void Conversations::focusPrevConversation()
{
  int i = prevActiveConversation(active);
  if (i != -1)
    activateConversation(i);
}

void Conversations::focusNextConversation()
{
  int i = nextActiveConversation(active);
  if (i != -1)
    activateConversation(i);
}

void Conversations::setExpandedConversations(bool expanded)
{
  if (expanded) {
    // make small room on the left and right
    left_spacer->setWidth(1);
    right_spacer->setWidth(1);
  }
  else {
    left_spacer->setWidth(0);
    right_spacer->setWidth(0);
  }

  /* Trigger the Conversation::show() method to change the scrollbar setting
   * of the active conversation. */
  activateConversation(active);
}

Conversations::Conversations()
  : Window(0, 0, 80, 1, TYPE_NON_FOCUSABLE, false), active(-1)
{
  setColorScheme("conversation");

  outer_list = new CppConsUI::HorizontalListBox(AUTOSIZE, 1);
  addWidget(*outer_list, 0, 0);

  left_spacer = new CppConsUI::Spacer(0, 1);
  right_spacer = new CppConsUI::Spacer(0, 1);
  conv_list = new CppConsUI::HorizontalListBox(AUTOSIZE, 1);

  outer_list->appendWidget(*left_spacer);
  outer_list->appendWidget(*conv_list);
  outer_list->appendWidget(*right_spacer);

  // init prefs
  purple_prefs_add_none(CONF_PREFIX "/chat");
  purple_prefs_add_int(CONF_PREFIX "/chat/partitioning", 80);
  purple_prefs_add_int(CONF_PREFIX "/chat/roomlist_partitioning", 80);
  purple_prefs_add_bool(CONF_PREFIX "/chat/beep_on_msg", false);

  // send_typing caching
  send_typing = purple_prefs_get_bool("/purple/conversations/im/send_typing");
  purple_prefs_connect_callback(this, "/purple/conversations/im/send_typing",
    send_typing_pref_change_, this);

  memset(&centerim_conv_ui_ops, 0, sizeof(centerim_conv_ui_ops));
  centerim_conv_ui_ops.create_conversation = create_conversation_;
  centerim_conv_ui_ops.destroy_conversation = destroy_conversation_;
  // centerim_conv_ui_ops.write_chat = ;
  // centerim_conv_ui_ops.write_im = ;
  centerim_conv_ui_ops.write_conv = write_conv_;
  centerim_conv_ui_ops.chat_add_users = chat_add_users_;
  centerim_conv_ui_ops.chat_rename_user = chat_rename_user_;
  centerim_conv_ui_ops.chat_remove_users = chat_remove_users_;
  centerim_conv_ui_ops.chat_update_user = chat_update_user_;

  centerim_conv_ui_ops.present = present_;
  // centerim_conv_ui_ops.has_focus = ;
  // centerim_conv_ui_ops.custom_smiley_add = ;
  // centerim_conv_ui_ops.custom_smiley_write = ;
  // centerim_conv_ui_ops.custom_smiley_close = ;
  // centerim_conv_ui_ops.send_confirm = ;

  // setup the callbacks for conversations
  purple_conversations_set_ui_ops(&centerim_conv_ui_ops);

  void *handle = purple_conversations_get_handle();
  purple_signal_connect(
    handle, "buddy-typing", this, PURPLE_CALLBACK(buddy_typing_), this);
  purple_signal_connect(
    handle, "buddy-typing-stopped", this, PURPLE_CALLBACK(buddy_typing_), this);

  onScreenResized();
}

Conversations::~Conversations()
{
  // close all opened conversations
  while (conversations.size())
    purple_conversation_destroy(
      conversations.front().conv->getPurpleConversation());

  purple_conversations_set_ui_ops(NULL);
  purple_prefs_disconnect_by_handle(this);
  purple_signals_disconnect_by_handle(this);
}

void Conversations::init()
{
  g_assert(!my_instance);

  my_instance = new Conversations;
  my_instance->show();
}

void Conversations::finalize()
{
  g_assert(my_instance);

  delete my_instance;
  my_instance = NULL;
}

int Conversations::findConversation(PurpleConversation *conv)
{
  for (int i = 0; i < static_cast<int>(conversations.size()); i++)
    if (conversations[i].conv->getPurpleConversation() == conv)
      return i;

  return -1;
}

int Conversations::prevActiveConversation(int current)
{
  g_assert(current < static_cast<int>(conversations.size()));

  if (conversations.empty()) {
    // there is no conversation
    return -1;
  }

  if (current == 0) {
    // return the last conversation
    return conversations.size() - 1;
  }

  return current - 1;
}

int Conversations::nextActiveConversation(int current)
{
  g_assert(current < static_cast<int>(conversations.size()));

  if (conversations.empty()) {
    // there is no conversation
    return -1;
  }

  if (current == static_cast<int>(conversations.size() - 1)) {
    // return the first conversation
    return 0;
  }

  return current + 1;
}

void Conversations::activateConversation(int i)
{
  g_assert(i >= -1);
  g_assert(i < static_cast<int>(conversations.size()));

  if (active == i) {
    if (active != -1)
      conversations[active].conv->show();
    return;
  }

  if (i != -1) {
    // show a new active conversation
    conversations[i].label->setVisibility(true);
    conversations[i].label->setColorScheme("conversation-active");
    conversations[i].conv->show();
  }

  // hide old active conversation if there is any
  if (active != -1) {
    conversations[active].label->setColorScheme(NULL);
    conversations[active].conv->hide();
  }

  active = i;
}

void Conversations::updateLabel(int i)
{
  g_assert(i >= 0);
  g_assert(i < static_cast<int>(conversations.size()));

  char *name = g_strdup_printf(
    " %d|%s%c", i + 1, purple_conversation_get_title(
                         conversations[i].conv->getPurpleConversation()),
    conversations[i].typing_status);
  conversations[i].label->setText(name);
  g_free(name);
}

void Conversations::updateLabels()
{
  /* Note: This can be a little slow if there are too many opened
   * conversations. */
  for (int i = 0; i < static_cast<int>(conversations.size()); i++)
    updateLabel(i);
}

void Conversations::create_conversation(PurpleConversation *conv)
{
  g_return_if_fail(conv);
  g_return_if_fail(findConversation(conv) == -1);

  PurpleConversationType type = purple_conversation_get_type(conv);
  if (type != PURPLE_CONV_TYPE_IM && type != PURPLE_CONV_TYPE_CHAT) {
    purple_conversation_destroy(conv);
    LOG->error(_("Unhandled conversation type '%d'."), type);
    return;
  }

  Conversation *conversation = new Conversation(conv);

  conv->ui_data = static_cast<void *>(conversation);

  ConvChild c;
  c.conv = conversation;
  c.label = new CppConsUI::Label(AUTOSIZE, 1);
  c.typing_status = ' ';
  conv_list->appendWidget(*c.label);
  conversations.push_back(c);
  updateLabels();

  // show the first conversation if there isn't any already
  if (active == -1)
    activateConversation(conversations.size() - 1);
}

void Conversations::destroy_conversation(PurpleConversation *conv)
{
  g_return_if_fail(conv);

  int i = findConversation(conv);

  // destroying unhandled conversation type
  if (i == -1)
    return;

  if (i == active) {
    if (conversations.size() == 1) {
      // the last conversation is closed
      active = -1;
    }
    else {
      if (active == static_cast<int>(conversations.size() - 1))
        focusPrevConversation();
      else
        focusNextConversation();
    }
  }

  delete conversations[i].conv;
  conv_list->removeWidget(*conversations[i].label);
  conversations.erase(conversations.begin() + i);

  if (active > i) {
    // fix up the number of the active conversation
    active--;
  }

  updateLabels();
}

void Conversations::write_conv(PurpleConversation *conv, const char *name,
  const char *alias, const char *message, PurpleMessageFlags flags,
  time_t mtime)
{
  g_return_if_fail(conv);

  int i = findConversation(conv);

  // message to unhandled conversation type
  if (i == -1)
    return;

  if (i != active)
    conversations[i].label->setColorScheme("conversation-new");

  // delegate it to Conversation object
  conversations[i].conv->write(name, alias, message, flags, mtime);
}

ConversationRoomList *Conversations::getRoomList(PurpleConversation *conv)
{
  ConversationRoomList *ret = NULL;

  if (NULL != conv) {
    Conversation *conversation = static_cast<Conversation *>(conv->ui_data);
    if (NULL != conversation) {
      if (conversation->getRoomList()) {
        ret = conversation->getRoomList();
      }
    }
  }

  return ret;
}

void Conversations::chat_add_users(
  PurpleConversation *conv, GList *cbuddies, gboolean new_arrivals)
{
  ConversationRoomList *room_list = getRoomList(conv);
  if (room_list)
    room_list->add_users(cbuddies, new_arrivals);
}

void Conversations::chat_rename_user(PurpleConversation *conv,
  const char *old_name, const char *new_name, const char *new_alias)
{
  ConversationRoomList *room_list = getRoomList(conv);
  if (room_list)
    room_list->rename_user(old_name, new_name, new_alias);
}

void Conversations::chat_remove_users(PurpleConversation *conv, GList *users)
{
  ConversationRoomList *room_list = getRoomList(conv);
  if (room_list)
    room_list->remove_users(users);
}

void Conversations::chat_update_user(PurpleConversation *conv, const char *user)
{
  ConversationRoomList *room_list = getRoomList(conv);
  if (room_list)
    room_list->update_user(user);
}

void Conversations::present(PurpleConversation *conv)
{
  g_return_if_fail(conv);

  int i = findConversation(conv);

  // unhandled conversation type
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
    // this should probably never happen
    return;
  }

  PurpleConvIm *im = PURPLE_CONV_IM(conv);
  g_assert(im);
  if (purple_conv_im_get_typing_state(im) == PURPLE_TYPING)
    conversations[i].typing_status = '*';
  else
    conversations[i].typing_status = ' ';

  updateLabel(i);
}

void Conversations::send_typing_pref_change(
  const char *name, PurplePrefType /*type*/, gconstpointer /*val*/)
{
  g_assert(!strcmp(name, "/purple/conversations/im/send_typing"));
  send_typing = purple_prefs_get_bool(name);
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
