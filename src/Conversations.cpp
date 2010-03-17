/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
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
#include "Conversation.h"

#include <vector>
#include <cstring>

Conversations *Conversations::Instance()
{
	static Conversations instance;
	return &instance;
}

Conversations::Conversations()
{
	memset(&centerim_conv_uiops, 0, sizeof(centerim_conv_uiops));
	centerim_conv_uiops.create_conversation = create_conversation_;
	centerim_conv_uiops.destroy_conversation = destroy_conversation_;
	//centerim_conv_uiops.write_chat = ;
	//centerim_conv_uiops.write_im = ;
	centerim_conv_uiops.write_conv = write_conv_;
	//centerim_conv_uiops.chat_add_users = ;
	//centerim_conv_uiops.chat_rename_user = ;
	//centerim_conv_uiops.chat_remove_users = ;
	//centerim_conv_uiops.chat_update_user = ;
	//centerim_conv_uiops.present = ;
	//centerim_conv_uiops.has_focus = ;
	//centerim_conv_uiops.custom_smiley_add = ;
	//centerim_conv_uiops.custom_smiley_write = ;
	//centerim_conv_uiops.custom_smiley_close = ;
	//centerim_conv_uiops.send_confirm = ;

	// setup the callbacks for conversations
	purple_conversations_set_ui_ops(&centerim_conv_uiops);
}

Conversations::~Conversations()
{
	/* We can't simply iterate over conversations because iterator's element
	 * is removed from conversations when we call
	 * purple_conversation_destroy(). */
	while (conversations.size())
		purple_conversation_destroy(conversations.begin()->second->GetPurpleConversation());

	purple_conversations_set_ui_ops(NULL);
}

void Conversations::create_conversation(PurpleConversation *conv)
{
	g_assert(conv);
	g_assert(conversations.find(conv) == conversations.end());

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

	conversations[conv] = conversation;
	conversation->Show();
}

void Conversations::destroy_conversation(PurpleConversation *conv)
{
	g_assert(conv);

	ConversationMap::iterator i = conversations.find(conv);
	// destroying unhandled conversation type
	if (i == conversations.end())
		return;

	delete i->second;
	conversations.erase(conv);
}

void Conversations::write_conv(PurpleConversation *conv, const char *name,
	const char *alias, const char *message, PurpleMessageFlags flags, time_t mtime)
{
	g_assert(conv);

	ConversationMap::iterator i = conversations.find(conv);
	// message to unhandled conversation type
	if (i == conversations.end())
		return;

	// delegate it to Conversation object
	i->second->Receive(name, alias, message, flags, mtime);
}

void Conversations::ShowConversation(PurpleConversationType type,
		PurpleAccount *account, const char *name)
{
	g_assert(account);
	g_assert(name);

	PurpleConversation *conv = purple_find_conversation_with_account(type, name, account);
	if (conv) {
		ConversationMap::iterator i = conversations.find(conv);
		// unhandled conversation type
		if (i == conversations.end())
			return;

		i->second->Show();
		return;
	}

	purple_conversation_new(type, account, name);
}
