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

#ifndef __CONVERSATION_H__
#define __CONVERSATION_H__

#include "Log.h"
#include "Conf.h"
#include "Conversations.h"

#include <cppconsui/Window.h>
#include <cppconsui/TextView.h>
#include <cppconsui/TextEdit.h>
#include <cppconsui/LineStyle.h>
#include <cppconsui/HorizontalLine.h>

#include <libpurple/conversation.h>
#include <libpurple/blist.h>

class Conversation
: public Window
{
	friend class Conversations;

	public:
		Conversation(PurpleBlistNode* node);
		~Conversation();
	
		virtual void SetConversation(PurpleConversation* conv);
		virtual void UnsetConversation(PurpleConversation* conv);

		void Receive(const char *name, const char *alias, const char *message,
			PurpleMessageFlags flags, time_t mtime);
		virtual void Send(void);
		virtual void Close(void);

		virtual void MoveResize(int newx, int newy, int neww, int newh);
		virtual void ScreenResized();

	protected:
		virtual void CreatePurpleConv(void);

		void SetPartitioning(unsigned int percentage);
		virtual void LoadHistory(void);

		PurpleBlistNodeType type;
		PurpleBlistNode* node;

		Conf *conf;
		TextView *view;
		TextEdit *input;
		HorizontalLine *line;
		
		int view_height;
		PurpleConversation *conv;

	private:
		Conversation();

		void ConstructCommon(void);
	
		DECLARE_SIG_REGISTERKEYS();
		static bool RegisterKeys();
		void DeclareBindables();
};

class ConversationChat
: public Conversation
{
	friend class Conversations;

	public:
		ConversationChat(PurpleChat *chat);
		~ConversationChat();

		virtual void SetConversation(PurpleConversation* conv);
		virtual void UnsetConversation(PurpleConversation* conv);

		//void Send(void);

	protected:
		void CreatePurpleConv(void);

		void LoadHistory(void);

	private:
		ConversationChat();

		PurpleConvChat* convchat;
		PurpleChat* chat;
};

class ConversationIm
: public Conversation
{
	friend class Conversations;

	public:
		ConversationIm(PurpleBuddy *buddy);
		~ConversationIm();

		virtual void SetConversation(PurpleConversation* conv);
		virtual void UnsetConversation(PurpleConversation* conv);

		void Send(void);

	protected:
		void CreatePurpleConv(void);

		void LoadHistory(void);

	private:
		ConversationIm();

		PurpleConvIm* convim;
		PurpleBuddy* buddy;
};


#endif /* __CONVERSATION_H__ */
