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
#include <cppconsui/ConsuiCurses.h>

#include "Conversation.h"
#include "CenterIM.h"

#include <cppconsui/HorizontalLine.h>
#include <cppconsui/TextView.h>
#include <cppconsui/LineStyle.h>
#include <cppconsui/Keys.h>

#define CONTEXT_CONVERSATION "conversation"

Conversation::Conversation(PurpleBlistNode* node)
: Window(0, 0, 80, 24)
, node(node)
, conv(NULL)
{
	conf = Conf::Instance();
	type = purple_blist_node_get_type(node);

	view = new TextView(*this, 1, 0, width - 2, height);
	input = new TextEdit(*this, 1, 1, width - 2, height);
	line = new HorizontalLine(*this, 0, view_height, width);
	AddWidget(*view);
	AddWidget(*input);
	AddWidget(*line);

	SetInputChild(*input);
	input->GrabFocus();

	SetPartitioning(conf->GetChatPartitioning());
	
	MoveResizeRect(conf->GetChatDimensions());
	DeclareBindables();
}

Conversation::~Conversation()
{
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

void Conversation::SetConversation(PurpleConversation* conv_)
{
	g_assert(conv == NULL);
	conv = conv_;
}

void Conversation::UnsetConversation(PurpleConversation* conv_)
{
	conv = NULL;
}

void Conversation::Receive(const char *name, const char *alias, const char *message,
	PurpleMessageFlags flags, time_t mtime)
{
	/* we should actually parse the html in the message to text attributes
	 * but i've not decided how to implement it.
	 **/
	char *html = purple_strdup_withhtml(message);
	message = purple_markup_strip_html(html);
	g_free(html);

	//TODO iconv, write to a window
	//printf("message from %s (%s) :\n%s\n", name, alias, message);
	view->Append(message);
}

//TODO if this remains empty, make it a pure virtual function
void Conversation::Send(void)
{
}

void Conversation::Close(void)
{
	/* close the open conversation, if any */
	Conversations::Instance()->Close(this);
	/* close the conversation window */
	Window::Close();
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

//TODO if this remains empty, make it a pure virtual function
void Conversation::CreatePurpleConv(void)
{
}

void Conversation::SetPartitioning(unsigned int percentage)
{
	int inputheight;

	//TODO check for rare condition that windowheight < 3
	// (in which case there is not enought room to draw anything)
	view_height = (height * percentage) / 100;
	if (view_height < 1) view_height = 1;

	inputheight = height - view_height - 1;
	if (inputheight < 1) {
		inputheight = 1;
		view_height = height - inputheight - 1;
	}

	view->MoveResize(1, 0, width - 2, view_height);
	input->MoveResize(1, view_height + 1, width - 2, inputheight);
	line->MoveResize(0, view_height, width, 1);
}

//TODO if this remains empty, make it a pure virtual function
void Conversation::LoadHistory(void)
{
}

ConversationChat::ConversationChat(PurpleChat* chat)
: Conversation(&chat->node)
, convchat(NULL)
, chat(chat)
{
	g_assert(chat != NULL);
	//CreatePurpleConv();
	LoadHistory();
}

ConversationChat::~ConversationChat()
{
}

void ConversationChat::SetConversation(PurpleConversation* conv)
{
	Conversation::SetConversation(conv);
	convchat = PURPLE_CONV_CHAT(conv);
}

void ConversationChat::UnsetConversation(PurpleConversation* conv)
{
	convchat = NULL;
	Conversation::UnsetConversation(conv);
}

void ConversationChat::CreatePurpleConv(void)
{
	//TODO adapt from conversationim::createpurpleconv
	//if (!convchat) {
	//	convchat = purple_conversation_new(PURPLE_CONV_TYPE_CHAT, purple_buddy_get_account(buddy), purple_buddy_get_name(buddy));
	//}
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

ConversationIm::ConversationIm(PurpleBuddy* buddy)
: Conversation(&buddy->node)
, convim(NULL)
, buddy(buddy)
{
	g_assert(buddy != NULL);
	//CreatePurpleConv();
	LoadHistory();
}

ConversationIm::~ConversationIm()
{
}

void ConversationIm::SetConversation(PurpleConversation* conv)
{
	Conversation::SetConversation(conv);
	convim = PURPLE_CONV_IM(conv);
}

void ConversationIm::UnsetConversation(PurpleConversation* conv)
{
	convim = NULL;
	Conversation::UnsetConversation(conv);
}

void ConversationIm::Send(void)
{
	if (!convim)
		CreatePurpleConv();

	gchar *str = input->AsString("<br/>");
	if (str) {
		purple_conv_im_send(convim, str);
		g_free(str);
		input->Clear();
	}
}

void ConversationIm::CreatePurpleConv(void)
{
	if (!convim) {
		conv = purple_conversation_new(PURPLE_CONV_TYPE_IM, purple_buddy_get_account(buddy), purple_buddy_get_name(buddy));
		convim = PURPLE_CONV_IM(conv);
	}


	if (!convim) //TODO only log an error if account is connected
		LOG->Write(Log::Level_error, "unable to open conversation with `%s'\n", buddy->name);
		//TODO add some info based on buddy
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

	header = g_strdup_printf("<b>Conversation with %s on %s:</b><br>\n", alias,
							 purple_date_format_full(localtime(&((PurpleLog *)logs->data)->time)));

	view->Append(header);

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
