/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010 by CenterIM developers
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
 * */

#include "Conversations.h"

#include "CenterIM.h"

#include <cstring>

Conversations *Conversations::Instance()
{
	static Conversations instance;
	return &instance;
}

Conversations::Conversations()
: FreeWindow(0, 0, 80, 1, TYPE_NON_FOCUSABLE)
, active(-1)
{
	SetColorScheme("conversation");

	list = new HorizontalListBox(width - 2, 1);
	AddWidget(*list, 2, 0);

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
	//centerim_conv_ui_ops.present = ;
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
	// all conversations should be closed at this time
	g_assert(conversations.empty());

	purple_conversations_set_ui_ops(NULL);
}

void Conversations::MoveResize(int newx, int newy, int neww, int newh)
{
	FreeWindow::MoveResize(newx, newy, neww, newh);

	list->MoveResize(1, 0, neww - 2, 1);
}

void Conversations::Close()
{
}

void Conversations::ScreenResized()
{
	Rect r = CENTERIM->ScreenAreaSize(CenterIM::ChatArea);
	r.y = r.Bottom();
	r.height = 1;

	MoveResizeRect(r);
}

void Conversations::ShowConversation(PurpleConversationType type,
		PurpleAccount *account, const char *name)
{
	// this is called by cim directly, so assert instead of return_if_fail
	g_assert(account);
	g_assert(name);

	PurpleConversation *conv = purple_find_conversation_with_account(type,
			name, account);
	if (conv) {
		int i = FindConversation(conv);

		// unhandled conversation type
		if (i == -1)
			return;

		if (i != active)
			ActivateConversation(i);

		return;
	}

	conv = purple_conversation_new(type, account, name);

	// this conversation was opened from the buddy list so force the show
	int i = FindConversation(conv);

	// unhandled conversation type
	if (i == -1)
		return;

	ActivateConversation(i);
}

void Conversations::FocusActiveConversation()
{
	ActivateConversation(active);
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

int Conversations::FindConversation(PurpleConversation *conv)
{
	for (int i = 0; i < (int) conversations.size(); i++)
		if (conversations[i].purple_conv == conv)
			return i;

	return -1;
}

int Conversations::PrevActiveConversation(int current)
{
	g_assert(current < (int) conversations.size());

	int i = current - 1;
	while (i >= 0) {
		if (conversations[i].conv->GetStatus() == Conversation::STATUS_ACTIVE)
			return i;
		i--;
	}
	i = conversations.size() - 1;
	while (i > current) {
		if (conversations[i].conv->GetStatus() == Conversation::STATUS_ACTIVE)
			return i;
		i--;
	}

	return -1;
}

int Conversations::NextActiveConversation(int current)
{
	g_assert(current < (int) conversations.size());

	int i = current + 1;
	while (i < (int) conversations.size()) {
		if (conversations[i].conv->GetStatus() == Conversation::STATUS_ACTIVE)
			return i;
		i++;
	}
	i = 0;
	while (i < current) {
		if (conversations[i].conv->GetStatus() == Conversation::STATUS_ACTIVE)
			return i;
		i++;
	}

	return -1;
}

void Conversations::ActivateConversation(int i)
{
	g_assert(i >= -1);
	g_assert(i < (int) conversations.size());

	if (active == i) {
		if (active != -1)
			conversations[active].conv->Show();
		return;
	}

	// hide old active conversation if there is any
	if (active != -1) {
		conversations[active].label->SetColorScheme(NULL);
		conversations[active].conv->Hide();
	}

	if (i == -1) {
		active = -1;
		return;
	}

	// show a new active conversation
	conversations[i].label->SetVisibility(true);
	conversations[i].label->SetColorScheme("conversation-active");
	conversations[i].conv->Show();
	active = i;
}

void Conversations::OnConversationClose(Conversation& conv)
{
	int i = FindConversation(conv.GetPurpleConversation());
	g_assert(i != -1);

	conversations[i].label->SetVisibility(false);

	if (i == active) {
		i = PrevActiveConversation(i);
		ActivateConversation(i);
	}
}

void Conversations::create_conversation(PurpleConversation *conv)
{
	g_return_if_fail(conv);
	g_return_if_fail(FindConversation(conv) == -1);

	Conversation *conversation;

	PurpleConversationType type = purple_conversation_get_type(conv);
	if (type == PURPLE_CONV_TYPE_IM)
		conversation = new ConversationIm(conv);
	else if (type == PURPLE_CONV_TYPE_CHAT)
		conversation = new ConversationChat(conv);
	else {
		LOG->Write(Log::Level_error, "unhandled conversation type: %i\n", type);
		return;
	}

	ConvChild c;
	c.purple_conv = conv;
	c.conv = conversation;
	c.sig_close = conversation->signal_close.connect(sigc::group(sigc::mem_fun(this,
				&Conversations::OnConversationClose), sigc::ref(*conversation)));
	char *name = g_strdup_printf("\\%s/", purple_conversation_get_name(conv));
	c.label = new Label(Curses::onscreen_width(name), 1, name);
	g_free(name);
	list->AppendWidget(*c.label);
	conversations.push_back(c);

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
		int j = PrevActiveConversation(i);
		ActivateConversation(j);
	}

	delete conversations[i].conv;
	list->RemoveWidget(*conversations[i].label);
	conversations.erase(conversations.begin() + i);
	if (active > i)
		active--;
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
	conversations[i].conv->Receive(name, alias, message, flags, mtime);
}
