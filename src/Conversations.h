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

#ifndef __CONVERSATIONS_H__
#define __CONVERSATIONS_H__

#include <libpurple/conversation.h>

#include <cppconsui/WindowManager.h>

class Conversations
{
	public:
		static Conversations* Instance(void);
		static void Delete(void);

		static void write_conv(PurpleConversation *conv, const char *name,
			const char *alias, const char *message, PurpleMessageFlags flags, time_t mtime);
		static void create_conversation_(PurpleConversation *conv)
			{ Conversations::Instance()->create_conversation(conv); }
		static void destroy_conversation_(PurpleConversation *conv)
			{ Conversations::Instance()->destroy_conversation(conv); }

		void create_conversation(PurpleConversation *conv);
		void destroy_conversation(PurpleConversation *conv);

	protected:

	private:
		Conversations();
		~Conversations();

		WindowManager *windowmanager;
		static Conversations *instance;
};

#endif /* __CONVERSATIONS_H__ */
