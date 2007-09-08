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

#include "Conversation.h"

#include <cppconsui/TextBrowser.h>

Conversation::Conversation(PurpleConversation *conv)
: Window(0, 0, 80, 24, NULL)
, conv(conv)
{
	log = Log::Instance();
	conf = Conf::Instance();

	SetBorder(new Border());
	MoveResize(conf->GetChatDimensions());

        browser = new TextBrowser(area, 1, 0, w-2, h);
	AddWidget(browser);

	SetPartitioning(conf->GetChatPartitioning());

	focuschild = browser;
}

Conversation::~Conversation()
{
}

void Conversation::Receive(const char *name, const char *alias, const char *message,
	PurpleMessageFlags flags, time_t mtime)
{
	Glib::ustring text = message;
	//TODO iconv, write to a window
	//printf("message from %s (%s) :\n%s\n", name, alias, message);
	browser->AddLine(text);
}

void Conversation::SetPartitioning(unsigned int percentage)
{
	int browserheight, inputheight;

	browserheight = (h * percentage) / 100;
	if (browserheight < 1) browserheight = 1;

	inputheight = h - browserheight - 1;
	if (inputheight < 1) {
		inputheight = 1;
		browserheight = h - inputheight - 1;
	}

	browser->Resize(area, w-2, browserheight);
}
