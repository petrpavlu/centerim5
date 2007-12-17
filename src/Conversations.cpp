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

Conversations* Conversations::instance = NULL;

Conversations* Conversations::Instance(void)
{
	if (!instance) instance = new Conversations();
	return instance;
}

Conversation* Conversations::Find(PurpleBuddy* buddy)
{
	std::vector<Conversation*>::iterator i;
	Conversation* conv;
	ConversationIm* im;
	ConversationChat* chat;

	for (i = conversations.begin(); i != conversations.end(); i++) {
		conv = (Conversation*)(*i);
		if ((im = dynamic_cast<ConversationIm*>(conv)) && im->buddy == buddy) {
			return conv;
		} else if ((chat = dynamic_cast<ConversationChat*>(conv))) {
			/* we are not looking for a chat */;
		} //TODO throw exception or log error
	}

	return NULL;
}

Conversation* Conversations::Find(PurpleChat* chat)
{
	std::vector<Conversation*>::iterator i;
	Conversation* conv;
	ConversationIm* im;
	ConversationChat* convchat;

	for (i = conversations.begin(); i != conversations.end(); i++) {
		conv = (Conversation*)(*i);
		if ((convchat = dynamic_cast<ConversationChat*>(conv)) && convchat->chat == chat) {
			return conv;
		} else if ((im = dynamic_cast<ConversationIm*>(conv))) {
			/* we are not looking for a chat */;
		} //TODO throw exception or log error
	}

	return NULL;
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
	g_assert(conv != NULL);

	PurpleConversationType type;

	Conversation *conversation = (Conversation*)conv->ui_data;
	if (conversation != NULL) {
		 //TODO should be debug()
		fprintf(stderr, "create_conversation(conversation): already have an object\n");
		return;
	}

	type = purple_conversation_get_type(conv);
	if (type == PURPLE_CONV_TYPE_CHAT) {
		conversation = new ConversationChat(purple_conversation_get_chat_data(conv));
	} else if (type == PURPLE_CONV_TYPE_IM) {
		conversation = new ConversationIm(purple_conversation_get_im_data(conv));
	} else {
		log->Write(PURPLE_DEBUG_ERROR, "unhandled conversation type: %i\n", type);
		return;
	}

	conversations.push_back(conversation);

	windowmanager->Add(conversation);
}

void Conversations::create_conversation(PurpleBlistNode *node)
{
	Conversation *conversation;

	g_assert(node != NULL);

	if (PURPLE_BLIST_NODE_IS_BUDDY(node)) {
		conversation = Find((PurpleBuddy*)node);
	} else if (PURPLE_BLIST_NODE_IS_CHAT(node)) {
		conversation = Find((PurpleChat*)node);
	} //TODO log some error if no match here
	
	if (conversation != NULL) {
		 //TODO should be debug()
		log->Write(PURPLE_DEBUG_MISC, "create_conversation(buddy): already have an object\n");
		return;
	}

	if (PURPLE_BLIST_NODE_IS_BUDDY(node)) {
		conversation = new ConversationIm((PurpleBuddy*)node);
	} else if (PURPLE_BLIST_NODE_IS_CHAT(node)) {
		conversation = new ConversationChat((PurpleChat*)node);
	}
		log->Write(PURPLE_DEBUG_MISC, "conversation: %p\n", conversation);

	conversations.push_back(conversation);

	windowmanager->Add(conversation);
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
	g_return_if_fail(conversation != NULL);//TODO create new conversation here or throw exception?

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
	log = Log::Instance();
	conf = Conf::Instance();

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


