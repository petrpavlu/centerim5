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

/* LoadHistory taken almost verbatim from pidgin/plugins/history.c */

//TODO remove this include
#include <cppconsui/Curses.h>

#include "Conversation.h"

#include <cppconsui/TextBrowser.h>
#include <cppconsui/LineStyle.h>

Conversation::Conversation(PurpleConversation *conv)
: Window(0, 0, 80, 24, NULL)
, conv(conv)
{
	if (conv)
		conv->ui_data = this;

	log = Log::Instance();
	conf = Conf::Instance();
	type = purple_conversation_get_type(conv);

	SetBorder(new Border());
	linestyle = LineStyle::LineStyleDefault();
	MoveResize(conf->GetChatDimensions());

	browser = new TextBrowser(*this, 2, 1, w-4, h-2);
	input = new TextInput(*this, 2, 1, w-4, h-2);
	SetPartitioning(conf->GetChatPartitioning());

	AddWidget(browser);
	AddWidget(input);
	SetInputChild(input);
}

void Conversation::Draw(void)
{
	mvwadd_wch(area->w, browserheight, 0, linestyle->HBegin());
	for (int i = 1; i+1 < w; i++) {
		mvwadd_wch(area->w, browserheight, i, linestyle->H());
	}
	mvwadd_wch(area->w, browserheight, w-1, linestyle->HEnd());

	Window::Draw();
}

Conversation::~Conversation()
{
	delete linestyle;
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
	int inputheight;

	//TODO check for rare condition that windowheight < 3
	// (in which case there is not enought room to draw anything)
	browserheight = (h * percentage) / 100;
	if (browserheight < 1) browserheight = 1;

	inputheight = h - browserheight - 2;
	if (inputheight < 1) {
		inputheight = 1;
		browserheight = h - inputheight - 1;
	}

	browser->Resize(w-4, browserheight-2);
	input->MoveResize(1, browserheight+1, w-4, inputheight);
}

void Conversation::LoadHistory(void)
{
}

ConversationChat::ConversationChat(PurpleConvChat* convchat)
: Conversation(convchat->conv)
, convchat(convchat)
{
	g_assert(conv != NULL);

	chat = purple_blist_find_chat(conv->account, conv->name);

	LoadHistory();
}

ConversationChat::ConversationChat(PurpleChat* chat)
: Conversation(NULL)
, convchat(NULL)
, chat(chat)
{
	LoadHistory();
}

ConversationChat::~ConversationChat()
{
}

void ConversationChat::LoadHistory(void)
{
	g_return_if_fail(conf->GetLogChats());

	PurpleAccount *account = chat->account;
	const char *name = purple_chat_get_name(chat);
	GList *logs = NULL;
	const char *alias = name;
	PurpleLogReadFlags flags;
	char *history;
	char *header;
	PurpleMessageFlags mflag;

	logs = purple_log_get_logs(PURPLE_LOG_CHAT, name, account);

	if (logs == NULL)
		return;

	mflag = (PurpleMessageFlags)(PURPLE_MESSAGE_NO_LOG | PURPLE_MESSAGE_SYSTEM | PURPLE_MESSAGE_DELAYED);
	history = purple_log_read((PurpleLog*)logs->data, &flags);

	header = g_strdup_printf("<b>Conversation with %s on %s:</b><br>", alias,
							 purple_date_format_full(localtime(&((PurpleLog *)logs->data)->time)));

	purple_conversation_write(conv, "", header, mflag, time(NULL));

	g_free(header);

	if (flags & PURPLE_LOG_READ_NO_NEWLINE)
		purple_str_strip_char(history, '\n');
	purple_conversation_write(conv, "", history, mflag, time(NULL));
	g_free(history);
	purple_conversation_write(conv, "", "<hr>", mflag, time(NULL));

	g_list_foreach(logs, (GFunc)purple_log_free, NULL);
	g_list_free(logs);
}
ConversationIm::ConversationIm(PurpleConvIm* convim)
: Conversation(convim->conv)
, convim(convim)
{
	g_assert(conv != NULL);

	buddy = purple_find_buddy(conv->account, conv->name);

	LoadHistory();
}

ConversationIm::ConversationIm(PurpleBuddy* buddy)
: Conversation(NULL)
, convim(NULL)
, buddy(buddy)
{
	LoadHistory();
}

ConversationIm::~ConversationIm()
{
}

void ConversationIm::LoadHistory(void)
{
	g_return_if_fail(conf->GetLogIms());

	PurpleAccount *account = purple_buddy_get_account(buddy);
	const char *name = purple_buddy_get_name(buddy);
	GList *logs = NULL;
	const char *alias = name;
	PurpleLogReadFlags flags;
	char *history;
	char *header;
	PurpleMessageFlags mflag;

	GSList *buddies;
	GSList *cur;

	/* Find buddies for this conversation. */
	buddies = purple_find_buddies(account, name);

	/* If we found at least one buddy, save the first buddy's alias. */
	if (buddies != NULL)
		alias = purple_buddy_get_contact_alias((PurpleBuddy *)buddies->data);

	for (cur = buddies; cur != NULL; cur = cur->next)
	{
		PurpleBlistNode *node = (PurpleBlistNode*)cur->data;
		if ((node != NULL) && ((node->prev != NULL) || (node->next != NULL)))
		{
			PurpleBlistNode *node2;

			alias = purple_buddy_get_contact_alias((PurpleBuddy *)node);

			/* We've found a buddy that matches this conversation.  It's part of a
			 * PurpleContact with more than one PurpleBuddy.  Loop through the PurpleBuddies
			 * in the contact and get all the logs. */
			for (node2 = node->parent->child ; node2 != NULL ; node2 = node2->next)
			{
				logs = g_list_concat(
					purple_log_get_logs(PURPLE_LOG_IM,
						purple_buddy_get_name((PurpleBuddy *)node2),
						purple_buddy_get_account((PurpleBuddy *)node2)),
					logs);
			}
			break;
		}
	}
	g_slist_free(buddies);

	if (logs == NULL)
		logs = purple_log_get_logs(PURPLE_LOG_IM, name, account);
	else
		logs = g_list_sort(logs, purple_log_compare);

	if (logs == NULL)
		return;

	mflag = (PurpleMessageFlags)(PURPLE_MESSAGE_NO_LOG | PURPLE_MESSAGE_SYSTEM | PURPLE_MESSAGE_DELAYED);
	history = purple_log_read((PurpleLog*)logs->data, &flags);

	header = g_strdup_printf("<b>Conversation with %s on %s:</b><br>", alias,
							 purple_date_format_full(localtime(&((PurpleLog *)logs->data)->time)));

	browser->AddLine(header);

	purple_conversation_write(conv, "", header, mflag, time(NULL));

	g_free(header);

	if (flags & PURPLE_LOG_READ_NO_NEWLINE)
		purple_str_strip_char(history, '\n');
	purple_conversation_write(conv, "", history, mflag, time(NULL));
	g_free(history);
	purple_conversation_write(conv, "", "<hr>", mflag, time(NULL));

	g_list_foreach(logs, (GFunc)purple_log_free, NULL);
	g_list_free(logs);
}
