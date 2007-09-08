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

#include <cppconsui/Window.h>
#include <cppconsui/TextBrowser.h>

#include <libpurple/conversation.h>

class Conversation
: public Window
{
	public:
		Conversation(PurpleConversation *conv);
		~Conversation();
		void Receive(const char *name, const char *alias, const char *message,
			PurpleMessageFlags flags, time_t mtime);

	protected:
		void SetPartitioning(unsigned int percentage);

	private:
		Conversation();

		Log *log;
		Conf *conf;
		TextBrowser *browser;

		PurpleConversation *conv;
};

#endif /* __CONVERSATION_H__ */
