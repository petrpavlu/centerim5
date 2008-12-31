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

#include <cppconsui/HorizontalLine.h>
#include <cppconsui/TextBrowser.h>
#include <cppconsui/LineStyle.h>
#include <cppconsui/Keys.h>

Conversation::Conversation(PurpleBlistNode* node)
: Window(0, 0, 80, 24, NULL)
, node(node)
, conv(NULL)
{
	const gchar *context = "conversation";
	log = Log::Instance();
	conf = Conf::Instance();
	type = purple_blist_node_get_type(node);

	SetBorder(new Border());
	linestyle = LineStyle::LineStyleDefault();
	MoveResize(conf->GetChatDimensions());

	browser = new TextBrowser(*this, 2, 1, w-4, h-2);
	input = new TextInput(*this, 2, 1, w-4, h-2);
	line = new HorizontalLine(*this, linestyle, 1, browserheight, w-2);
	SetPartitioning(conf->GetChatPartitioning());

	AddWidget(browser);
	AddWidget(input);
	AddWidget(line);

	SetInputChild(input);
	input->GrabFocus();

	DeclareBindable(context, "send",  sigc::mem_fun(this, &Conversation::Send),
		_("Send the message."), InputProcessor::Bindable_Override);

	//TODO get real binding from config
	BindAction(context, "send", Keys::Instance()->Key_ctrl_x(), false);
}

Conversation::~Conversation()
{
	delete linestyle;
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

	Glib::ustring text = message;
	//TODO iconv, write to a window
	//printf("message from %s (%s) :\n%s\n", name, alias, message);
	browser->AddLine(text);
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

//TODO if this remains empty, make it a pure virtual function
void Conversation::CreatePurpleConv(void)
{
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
	line->Move(1, browserheight);
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

	purple_conv_im_send(convim, input->AsString("<br/>").c_str());

	input->Clear();
}

void ConversationIm::CreatePurpleConv(void)
{
	if (!convim) {
		conv = purple_conversation_new(PURPLE_CONV_TYPE_IM, purple_buddy_get_account(buddy), purple_buddy_get_name(buddy));
		convim = PURPLE_CONV_IM(conv);
	}


	if (!convim) //TODO only log an error if account is connected
		log->Write(Log::Type_cim, Log::Level_error, "unable to open conversation with `%s'", buddy->name);
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
