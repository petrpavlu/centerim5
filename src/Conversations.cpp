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

void Conversations::Delete(void)
{
	if (instance) {
		delete instance;
		instance = NULL;
	}
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

Conversation* Conversations::Find(PurpleConversation* conv)
{
	std::vector<Conversation*>::iterator i;
	Conversation* conversation;

	for (i = conversations.begin(); i != conversations.end(); i++) {
		conversation = (Conversation*)(*i);
		if (conversation->conv == conv)
			return conversation;
	}

	return NULL;
}

void Conversations::Close(Conversation* conversation)
{
	if (conversation->conv)
		purple_conversation_destroy(conversation->conv);

	conversation->Hide();
	RemoveConversation(conversation);
	windowmanager->Remove(conversation);
	delete conversation;
}

void Conversations::AddConversation(Conversation *conv)
{
	std::vector<Conversation*>::iterator i;
	Conversation* conversation;

	for (i = conversations.begin(); i != conversations.end(); i++) {
		conversation = (Conversation*)(*i);
		if (conversation == conv) {
			log->Write(Log::Level_error, "connot add a conversation to the stack twice\n");
			return;
		}
	}

	conversations.push_back(conv);
}

void Conversations::RemoveConversation(Conversation *conv)
{
	std::vector<Conversation*>::iterator i;
	Conversation* conversation;

	for (i = conversations.begin(); i != conversations.end(); i++) {
		conversation = (Conversation*)(*i);
		if (conversation == conv) {
			conversations.erase(i);
			return;
		}
	}

	log->Write(Log::Level_error, "connot remove a conversation not on the stack\n");
}

void Conversations::create_conversation(PurpleConversation *conv)
{
	g_assert(conv != NULL);

	//TODO remov debug
	log->Write(Log::Type_cim, Log::Level_error, "create_conversation(purpleconv) calles: %p\n", conv);

	PurpleConversationType type;

	type = purple_conversation_get_type(conv);
	if (type == PURPLE_CONV_TYPE_IM) {
		create_conversation_im(conv);
	} else if (type == PURPLE_CONV_TYPE_CHAT) {
		create_conversation_chat(conv);
	} else {
		log->Write(Log::Type_cim, Log::Level_error, "unhandled conversation type: %i\n", type);
	}
}

void Conversations::create_conversation(PurpleBlistNode *node)
{
	g_assert(node != NULL);

	PurpleBlistNodeType type;

	type = purple_blist_node_get_type(node);
	if (type == PURPLE_BLIST_BUDDY_NODE) {
		create_conversation_im(node);
	} else if (type == PURPLE_BLIST_CHAT_NODE) {
		create_conversation_chat(node);
	} else {
		log->Write(Log::Type_cim, Log::Level_error, "unhandled conversation type: %i\n", type);
	}
}

void Conversations::create_conversation_im(PurpleConversation *conv)
{
	PurpleBuddy* buddy;
	Conversation* conversation;

	buddy = purple_find_buddy(conv->account, conv->name);
	//TODO what if buddy not in list yet?

	/* if we already have a window do nothing special */
	if ((conversation = Find(buddy)) != NULL) {
		 //TODO remove debug
		log->Write(Log::Level_error, "create_conversation_im(conversation): already have an object\n");

	/* otherwise create a conversation window first */
	} else {
		conversation = new ConversationIm(buddy);
		//TODO remove debug
		log->Write(Log::Type_cim, Log::Level_error, "new conversation: %p\n", conversation);
		AddConversation(conversation);
		windowmanager->Add(conversation);
	}

	/* assign the conversation to the window */
	conversation->SetConversation(conv);
}

void Conversations::create_conversation_chat(PurpleConversation *conv)
{
	PurpleChat *chat = NULL;
	Conversation* conversation;

	//TODO chats not really implemented
	//chat = purple_find_chat(purple_conversation_get_gc(conv), conv->id);
	//TODO what if buddy not in list yet?

	/* if we already have a window do nothing special */
	if ((conversation = Find(chat)) != NULL) {
		 //TODO remove debug
		log->Write(Log::Level_error, "create_conversation_chat(conversation): already have an object\n");

	/* otherwise create a conversation window first */
	} else {
		conversation = new ConversationChat(chat);
		//TODO remove debug
		log->Write(Log::Type_cim, Log::Level_error, "new conversation: %p\n", conversation);
		AddConversation(conversation);
		windowmanager->Add(conversation);
	}

	/* assign the conversation to the window */
	conversation->SetConversation(conv);
}

void Conversations::create_conversation_im(PurpleBlistNode *node)
{
	Conversation* conversation;
	PurpleBuddy* buddy;

	buddy = (PurpleBuddy*)node;
	if ((conversation = Find(buddy)) != NULL) {
		//TODO move window to the top and give focus, no debug
		log->Write(Log::Level_error, "create_conversation_im(buddy): already have an object\n");
	} else {
		conversation = new ConversationIm(buddy);
		//TODO remove debug
		log->Write(Log::Type_cim, Log::Level_error, "new conversation: %p\n", conversation);
		AddConversation(conversation);

		//TODO remove debug
		log->Write(Log::Type_cim, Log::Level_debug, "conversation: %p\n", conversation);

		/* If the account is connected, try to initiate a conversation conversation */
		if (purple_account_is_connected(purple_buddy_get_account(buddy))) {
			purple_conversation_new(PURPLE_CONV_TYPE_IM, purple_buddy_get_account(buddy), purple_buddy_get_name(buddy));
		}

		windowmanager->Add(conversation);
	}
}

void Conversations::create_conversation_chat(PurpleBlistNode *node)
{
	Conversation* conversation;
	PurpleChat* chat;

	chat = (PurpleChat*)node;
	if ((conversation = Find(chat)) != NULL) {
		//TODO move window to the top and give focus, no debug
		log->Write(Log::Level_debug, "create_conversation_chat(buddy): already have an object\n");
	} else {
		conversation = new ConversationChat(chat);
		//TODO remove debug
		log->Write(Log::Type_cim, Log::Level_error, "new conversation: %p\n", conversation);
		AddConversation(conversation);

		//TODO remove debug
		log->Write(Log::Type_cim, Log::Level_debug, "conversation: %p\n", conversation);

		/* If the account is connected, try to initiate a conversation conversation */
		//if (purple_account_is_connected(purple_chat_get_account(chat))) {
		//	purple_conversation_new(PURPLE_CONV_TYPE_IM, purple_chat_get_account(buddy), purple_chat_get_name(buddy));
		//}

		windowmanager->Add(conversation);
	}
}

void Conversations::destroy_conversation(PurpleConversation *conv)
{
	Conversation* conversation;

	if ((conversation = Find(conv)) != NULL) {
		conversation->UnsetConversation(NULL);
	} else {
		 //TODO should be debug() add some stuff to easily identify the conversation
		log->Write(Log::Type_cim, Log::Level_error, "ERROR: conversation without a window is being destroyed: %p, %s\n", conv, conv->name);
	}
}

void Conversations::write_conv(PurpleConversation *conv, const char *name,
	const char *alias, const char *message, PurpleMessageFlags flags, time_t mtime)
{
	Conversation *conversation = Find(conv);

	g_return_if_fail(conversation != NULL);//TODO create new conversation here or throw exception?
						// we should have created a conversation before, so this might not even happen
						// so add some error logging here if it does

	conversation->Receive(name, alias, message, flags, mtime);
}

//TODO move this struct inside the conversations object
static PurpleConversationUiOps centerim_conv_uiops =
{
        Conversations::create_conversation_,
        Conversations::destroy_conversation_,
        NULL,
        NULL,
        Conversations::write_conv_,
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
	//
	std::vector<Conversation*>::iterator i;
	Conversation* conv;

	while (conversations.size() > 0) {
		conv = conversations[0];
		Close(conv);
	}

	purple_conversations_set_ui_ops(NULL);
}

