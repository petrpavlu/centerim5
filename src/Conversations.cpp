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

Conversations* Conversations::instance = NULL;

Conversations* Conversations::Instance(void)
{
	if (!instance) instance = new Conversations();
	return instance;
}

void Conversations::Delete(void)
{
	if (instance) {
		delete instance;
		instance = NULL;
	}
}

void Conversations::create_conversation(PurpleConversation *conv)
{
	//TODO add the new object to a hashtable of some sort?
	Conversation *conversation = (Conversation*)conv->ui_data;
	if (conversation != NULL) {
		 //TODO should be debug()
		fprintf(stderr, "create_conversation(): already have an object\n");
		return;
	}

	conv->ui_data = new Conversation(conv);

	windowmanager->Add((Window*)conv->ui_data);
}

void Conversations::destroy_conversation(PurpleConversation *conv)
{
	Conversation *conversation = (Conversation*)conv->ui_data;
	if (conversation == NULL) {
		 //TODO should be debug()
		fprintf(stderr, "destroy_conversation(): no object to destroy\n");
		return;
	}

	windowmanager->Remove((Window*)conv->ui_data);
	delete conversation;
	conv->ui_data = NULL;
}

void Conversations::write_conv(PurpleConversation *conv, const char *name,
	const char *alias, const char *message, PurpleMessageFlags flags, time_t mtime)
{
	Conversation *conversation = (Conversation*)conv->ui_data;
	g_return_if_fail(conversation != NULL);

	conversation->Receive(name, alias, message, flags, mtime);
}

//TODO move this struct inside the conversations object
static PurpleConversationUiOps centerim_conv_uiops =
{
        Conversations::create_conversation_,
        Conversations::destroy_conversation_,
        NULL,
        NULL,
        Conversations::write_conv,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
};

Conversations::Conversations()
{
	/* setup the callbacks for conversations */
	purple_conversations_set_ui_ops(&centerim_conv_uiops);

	windowmanager = WindowManager::Instance();
}

Conversations::~Conversations()
{
	/* remove the callbacks for conversations
	 * after this no new conversations can be
	 * initiated/stopped, so make sure every
	 * conversation has been closed */
	//TODO close all conversations
	purple_conversations_set_ui_ops(NULL);
}


