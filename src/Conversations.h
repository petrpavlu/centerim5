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

#ifndef __CONVERSATIONS_H__
#define __CONVERSATIONS_H__

#include "Conversation.h"

#include <cppconsui/FreeWindow.h>
#include <cppconsui/HorizontalListBox.h>
#include <libpurple/conversation.h>
#include <vector>

#define CONVERSATIONS (Conversations::Instance())

class Conversations
: public FreeWindow
{
	public:
		static Conversations *Instance();

		// Widget
		virtual void MoveResize(int newx, int newy, int neww, int newh);

		// Window
		virtual void Close();
		virtual void ScreenResized();

		// force show the conversation, used by buddy list
		void ShowConversation(PurpleConversationType type,
				PurpleAccount *account, const char *name);
		// force hide the conversation
		void HideConversation(PurpleConversation *conv);

	protected:

	private:
		struct ConvChild
		{
			PurpleConversation *purple_conv;
			Conversation *conv;
			Label *label;
			sigc::connection sig_close;
		};

		typedef std::vector<ConvChild> ConversationsVector;

		ConversationsVector conversations;

		// active conversation, -1 if none
		int active;

		HorizontalListBox *list;

		PurpleConversationUiOps centerim_conv_ui_ops;

		Conversations();
		Conversations(const Conversations&);
		Conversations& operator=(const Conversations&);
		virtual ~Conversations();

		// find PurpleConversation in conversations
		int FindConversation(PurpleConversation *conv);

		int PrevActiveConversation(int current);
		int NextActiveConversation(int current);

		void ActivateConversation(int i);

		void OnConversationClose(Conversation& conv);

		static void write_conv_(PurpleConversation *conv, const char *name,
				const char *alias, const char *message, PurpleMessageFlags
				flags, time_t mtime)
			{ CONVERSATIONS->write_conv(conv, name, alias, message, flags,
					mtime); }
		static void create_conversation_(PurpleConversation *conv)
			{ CONVERSATIONS->create_conversation(conv); }
		static void destroy_conversation_(PurpleConversation *conv)
			{ CONVERSATIONS->destroy_conversation(conv); }

		void write_conv(PurpleConversation *conv, const char *name,
			const char *alias, const char *message, PurpleMessageFlags flags,
			time_t mtime);
		void create_conversation(PurpleConversation *conv);
		void destroy_conversation(PurpleConversation *conv);
};

#endif /* __CONVERSATIONS_H__ */
