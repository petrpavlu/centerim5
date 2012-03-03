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

#include "CenterIM.h"

#include <string.h>
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
  int prev, cur;
  prev = -1;
  cur = NextActiveConversation(prev);

  while (cur > prev) {
    if (i == 1) {
      ActivateConversation(cur);
      return;
    }

    i--;
    prev = cur;
    cur = NextActiveConversation(cur);
  }

  // if there is less than i active conversations then active the last one
  if (prev != -1)
    ActivateConversation(prev);
}

void Conversations::FocusPrevConversation()
{
  int i = PrevActiveConversation(active);
  if (i == -1)
    ActivateConversation(active);
  else
    ActivateConversation(i);
}

void Conversations::FocusNextConversation()
{
  int i = NextActiveConversation(active);
  if (i == -1)
    ActivateConversation(active);
  else
    ActivateConversation(i);
}

Conversations::Conversations()
: FreeWindow(0, 0, 80, 1, TYPE_NON_FOCUSABLE)
, active(-1)
{
  SetColorScheme("conversation");

  list = new CppConsUI::HorizontalListBox(AUTOSIZE, 1);
  AddWidget(*list, 0, 0);

  // init prefs
  purple_prefs_add_none(CONF_PREFIX "/chat");
  purple_prefs_add_int(CONF_PREFIX "/chat/partitioning", 80);

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
}

Conversations::~Conversations()
{
  // close all opened conversations
  while (conversations.size())
    purple_conversation_destroy(conversations.front().purple_conv);

  purple_conversations_set_ui_ops(NULL);
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

void Conversations::UpdateLabels()
{
  /* Note: This can be a little slow if there are too many opened
   * conversations. */
  int i = 1;
  for (ConversationsVector::iterator j = conversations.begin();
      j != conversations.end(); j++) {
    char *name = g_strdup_printf(" %d|%s", i,
        purple_conversation_get_title(j->purple_conv));
    j->label->SetText(name);
    g_free(name);
    i++;
  }
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
  list->AppendWidget(*c.label);
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
  list->RemoveWidget(*conversations[i].label);
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

/* vim: set tabstop=2 shiftwidth=2 tw=78 expandtab : */
