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

#include "Conversation.h"
#include "CenterIM.h"

#include <cppconsui/HorizontalLine.h>
#include <cppconsui/TextView.h>
#include <cppconsui/LineStyle.h>
#include <cppconsui/Keys.h>
#include "gettext.h"

#define CONTEXT_CONVERSATION "conversation"

Conversation::Conversation(PurpleConversation *conv_)
: Window(0, 0, 80, 24)
, conv(conv_)
{
	g_assert(conv);

	SetColorScheme("conversation");

	conf = Conf::Instance();

	view = new TextView(*this, 1, 0, width - 2, height);
	input = new TextEdit(*this, 1, 1, width - 2, height);
	line = new HorizontalLine(*this, 0, height, width);
	AddWidget(*view);
	AddWidget(*input);
	AddWidget(*line);

	SetInputChild(*input);
	input->GrabFocus();

	SetPartitioning(conf->GetChatPartitioning());
	
	MoveResizeRect(conf->GetChatDimensions());
	DeclareBindables();
}

void Conversation::DeclareBindables()
{
	DeclareBindable(CONTEXT_CONVERSATION, "send",
			sigc::mem_fun(this, &Conversation::Send),
			InputProcessor::Bindable_Override);
}

DEFINE_SIG_REGISTERKEYS(Conversation, RegisterKeys);
bool Conversation::RegisterKeys()
{
	RegisterKeyDef(CONTEXT_CONVERSATION, "send", _("Send the message."),
			Keys::UnicodeTermKey("x", TERMKEY_KEYMOD_CTRL));
	return true;
}

void Conversation::Receive(const char *name, const char *alias, const char *message,
	PurpleMessageFlags flags, time_t mtime)
{
	// we currently don't support displaying HTML in any way
	char *nohtml = purple_markup_strip_html(message);

	int color = 0;
	if (flags & PURPLE_MESSAGE_SEND)
		color = 1;
	else if (flags & PURPLE_MESSAGE_RECV)
		color = 2;

	char *msg = g_strdup_printf("%s %s", purple_date_format_long(localtime(&mtime)), nohtml);
	g_free(nohtml);
	view->Append(msg, color);
	g_free(msg);

}

void Conversation::Close()
{
	/* Let libpurple and Conversations know that this conversation should be
	 * destroyed. */
	purple_conversation_destroy(conv);
}

void Conversation::MoveResize(int newx, int newy, int neww, int newh)
{
	Window::MoveResize(newx, newy, neww, newh);

	SetPartitioning(conf->GetChatPartitioning());
}

void Conversation::ScreenResized()
{
	MoveResizeRect(CenterIM::Instance().ScreenAreaSize(CenterIM::ChatArea));
}

void Conversation::SetPartitioning(unsigned percentage)
{
	int input_height;
	int view_height;

	//TODO check for rare condition that windowheight < 3
	// (in which case there is not enough room to draw anything)
	view_height = (height * percentage) / 100;
	if (view_height < 1) view_height = 1;

	input_height = height - view_height - 1;
	if (input_height < 1) {
		input_height = 1;
		view_height = height - input_height - 1;
	}

	view->MoveResize(1, 0, width - 2, view_height);
	input->MoveResize(1, view_height + 1, width - 2, input_height);
	line->MoveResize(0, view_height, width, 1);
}

ConversationChat::ConversationChat(PurpleConversation *conv)
: Conversation(conv)
{
	convchat = PURPLE_CONV_CHAT(conv);
	LoadHistory();
}

void ConversationChat::LoadHistory()
{
	g_return_if_fail(conf->GetLogChats());

	PurpleAccount *account = purple_conversation_get_account(conv);
	const char *name = purple_conversation_get_name(conv);
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
	view->Append(header);
	g_free(header);

	if (flags & PURPLE_LOG_READ_NO_NEWLINE)
		purple_str_strip_char(history, '\n');
	char *nohtml = purple_markup_strip_html(history);
	g_free(history);
	view->Append(nohtml);
	g_free(nohtml);

	g_list_foreach(logs, (GFunc)purple_log_free, NULL);
	g_list_free(logs);
}

void ConversationChat::Send()
{
}

ConversationIm::ConversationIm(PurpleConversation *conv)
: Conversation(conv)
{
	convim = PURPLE_CONV_IM(conv);
	LoadHistory();
}

void ConversationIm::LoadHistory()
{
	g_return_if_fail(conf->GetLogIms());

	PurpleAccount *account = purple_conversation_get_account(conv);
	const char *name = purple_conversation_get_name(conv);
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

	header = g_strdup_printf("Conversation with %s on %s:\n", alias,
							 purple_date_format_full(localtime(&((PurpleLog *)logs->data)->time)));
	view->Append(header);
	g_free(header);

	if (flags & PURPLE_LOG_READ_NO_NEWLINE)
		purple_str_strip_char(history, '\n');
	char *nohtml = purple_markup_strip_html(history);
	g_free(history);
	view->Append(nohtml);
	g_free(nohtml);

	g_list_foreach(logs, (GFunc)purple_log_free, NULL);
	g_list_free(logs);
}

void ConversationIm::Send()
{
	gchar *str = input->AsString("<br/>");
	if (str) {
		purple_conv_im_send(convim, str);
		g_free(str);
		input->Clear();
	}
}
