/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2012 by CenterIM developers
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

Conversations *Conversations::instance = NULL;

Conversations *Conversations::Instance()
{
  return instance;
}

void Conversations::OnScreenResized()
{
  CppConsUI::Rect r = CENTERIM->GetScreenAreaSize(CenterIM::CHAT_AREA);
  r.y = r.GetBottom();
  r.height = 1;

  MoveResizeRect(r);
}

void Conversations::FocusActiveConversation()
{
  ActivateConversation(active);
}

void Conversations::FocusConversation(int i)
{
  g_assert(i >= 1);

  if (conversations.empty())
    return;

  // the external conversation #1 is the internal conversation #0
  i -= 1;

  int s = static_cast<int>(conversations.size());
  if (i < s)
    ActivateConversation(i);
  else {
    // if there is less than i active conversations then active the last one
    ActivateConversation(s - 1);
  }
}

void Conversations::FocusPrevConversation()
{
  int i = PrevActiveConversation(active);
  if (i != -1)
    ActivateConversation(i);
}

void Conversations::FocusNextConversation()
{
  int i = NextActiveConversation(active);
  if (i != -1)
    ActivateConversation(i);
}

void Conversations::SetExpandedConversations(bool expanded)
{
  if (expanded) {
    // make small room on the left and right
    left_spacer->SetWidth(1);
    right_spacer->SetWidth(1);
  }
  else {
    left_spacer->SetWidth(0);
    right_spacer->SetWidth(0);
  }

  /* Trigger the Conversation::Show() method to change the scrollbar setting
   * of the active conversation. */
  ActivateConversation(active);
}

Conversations::Conversations()
: FreeWindow(0, 0, 80, 1, TYPE_NON_FOCUSABLE)
, active(-1)
{
  SetColorScheme("conversation");

  outer_list = new CppConsUI::HorizontalListBox(AUTOSIZE, 1);
  AddWidget(*outer_list, 0, 0);

  left_spacer = new CppConsUI::Spacer(0, 1);
  right_spacer = new CppConsUI::Spacer(0, 1);
  conv_list = new CppConsUI::HorizontalListBox(AUTOSIZE, 1);

  outer_list->AppendWidget(*left_spacer);
  outer_list->AppendWidget(*conv_list);
  outer_list->AppendWidget(*right_spacer);

  // init prefs
  purple_prefs_add_none(CONF_PREFIX "/chat");
  purple_prefs_add_int(CONF_PREFIX "/chat/partitioning", 80);
  purple_prefs_add_bool(CONF_PREFIX "/chat/beep_on_msg", false);

  // send_typing caching
  send_typing = purple_prefs_get_bool("/purple/conversations/im/send_typing");
  purple_prefs_connect_callback(this, "/purple/conversations/im/send_typing",
      send_typing_pref_change_, this);

  memset(&centerim_conv_ui_ops, 0, sizeof(centerim_conv_ui_ops));
  centerim_conv_ui_ops.create_conversation = create_conversation_;
  centerim_conv_ui_ops.destroy_conversation = destroy_conversation_;
  //centerim_conv_ui_ops.write_chat = ;
  //centerim_conv_ui_ops.write_im = ;
  centerim_conv_ui_ops.write_conv = write_conv_;
  //centerim_conv_ui_ops.chat_add_users = ;
  //centerim_conv_ui_ops.chat_rename_user = ;
  //centerim_conv_ui_ops.chat_remove_users = ;
  //centerim_conv_ui_ops.chat_update_user = ;
  centerim_conv_ui_ops.present = present_;
  //centerim_conv_ui_ops.has_focus = ;
  //centerim_conv_ui_ops.custom_smiley_add = ;
  //centerim_conv_ui_ops.custom_smiley_write = ;
  //centerim_conv_ui_ops.custom_smiley_close = ;
  //centerim_conv_ui_ops.send_confirm = ;

  // setup the callbacks for conversations
  purple_conversations_set_ui_ops(&centerim_conv_ui_ops);

  void *handle = purple_conversations_get_handle();
  purple_signal_connect(handle, "buddy-typing", this,
      PURPLE_CALLBACK(buddy_typing_), this);
  purple_signal_connect(handle, "buddy-typing-stopped", this,
      PURPLE_CALLBACK(buddy_typing_), this);
}

Conversations::~Conversations()
{
  // close all opened conversations
  while (conversations.size())
    purple_conversation_destroy(conversations.front().purple_conv);

  purple_conversations_set_ui_ops(NULL);
  purple_prefs_disconnect_by_handle(this);
  purple_signals_disconnect_by_handle(this);
}

void Conversations::Init()
{
  g_assert(!instance);

  instance = new Conversations;
  instance->Show();
}

void Conversations::Finalize()
{
  g_assert(instance);

  delete instance;
  instance = NULL;
}

int Conversations::FindConversation(PurpleConversation *conv)
{
  for (int i = 0; i < static_cast<int>(conversations.size()); i++)
    if (conversations[i].purple_conv == conv)
      return i;

  return -1;
}

int Conversations::PrevActiveConversation(int current)
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

int Conversations::NextActiveConversation(int current)
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

void Conversations::ActivateConversation(int i)
{
  g_assert(i >= -1);
  g_assert(i < static_cast<int>(conversations.size()));

  if (active == i) {
    if (active != -1)
      conversations[active].conv->Show();
    return;
  }

  if (i != -1) {
    // show a new active conversation
    conversations[i].label->SetVisibility(true);
    conversations[i].label->SetColorScheme("conversation-active");
    conversations[i].conv->Show();
  }

  // hide old active conversation if there is any
  if (active != -1) {
    conversations[active].label->SetColorScheme(NULL);
    conversations[active].conv->Hide();
  }

  active = i;
}

void Conversations::UpdateLabel(int i)
{
  g_assert(i >= 0);
  g_assert(i < static_cast<int>(conversations.size()));

  char *name = g_strdup_printf(" %d|%s%c", i + 1,
      purple_conversation_get_title(conversations[i].purple_conv),
      conversations[i].typing_status);
  conversations[i].label->SetText(name);
  g_free(name);
}

void Conversations::UpdateLabels()
{
  /* Note: This can be a little slow if there are too many opened
   * conversations. */
  for (int i = 0; i < static_cast<int>(conversations.size()); i++)
    UpdateLabel(i);
}

void Conversations::create_conversation(PurpleConversation *conv)
{
  g_return_if_fail(conv);
  g_return_if_fail(FindConversation(conv) == -1);

  PurpleConversationType type = purple_conversation_get_type(conv);
  if (type != PURPLE_CONV_TYPE_IM && type != PURPLE_CONV_TYPE_CHAT) {
    purple_conversation_destroy(conv);
    LOG->Error(_("Unhandled conversation type: %i.\n"), type);
    return;
  }

  Conversation *conversation = new Conversation(conv);

  ConvChild c;
  c.purple_conv = conv;
  c.conv = conversation;
  c.label = new CppConsUI::Label(AUTOSIZE, 1);
  c.typing_status = ' ';
  conv_list->AppendWidget(*c.label);
  conversations.push_back(c);
  UpdateLabels();

  // show the first conversation if there isn't any already
  if (active == -1)
    ActivateConversation(conversations.size() - 1);
}

void Conversations::destroy_conversation(PurpleConversation *conv)
{
  g_return_if_fail(conv);

  int i = FindConversation(conv);

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
        FocusPrevConversation();
      else
        FocusNextConversation();
    }
  }

  delete conversations[i].conv;
  conv_list->RemoveWidget(*conversations[i].label);
  conversations.erase(conversations.begin() + i);

  if (active > i) {
    // fix up the number of the active conversation
    active--;
  }

  UpdateLabels();
}

void Conversations::write_conv(PurpleConversation *conv, const char *name,
    const char *alias, const char *message, PurpleMessageFlags flags,
    time_t mtime)
{
  g_return_if_fail(conv);

  int i = FindConversation(conv);

  // message to unhandled conversation type
  if (i == -1)
    return;

  if (i != active)
    conversations[i].label->SetColorScheme("conversation-new");

  // delegate it to Conversation object
  conversations[i].conv->Write(name, alias, message, flags, mtime);
}

void Conversations::present(PurpleConversation *conv)
{
  g_return_if_fail(conv);

  int i = FindConversation(conv);

  // unhandled conversation type
  if (i == -1)
    return;

  ActivateConversation(i);
}

void Conversations::buddy_typing(PurpleAccount *account, const char *who)
{
  PurpleConversation *conv = purple_find_conversation_with_account(
      PURPLE_CONV_TYPE_IM, who, account);
  if (!conv)
    return;

  int i = FindConversation(conv);
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

  UpdateLabel(i);
}

void Conversations::send_typing_pref_change(const char *name,
    PurplePrefType /*type*/, gconstpointer /*val*/)
{
  g_assert(!strcmp(name, "/purple/conversations/im/send_typing"));
  send_typing = purple_prefs_get_bool(name);
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
