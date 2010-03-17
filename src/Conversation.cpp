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

#define LOGS_DIR "clogs"
#define TIME_FORMAT _("%d.%m.%y %H:%M")

/// @todo Increase to higher value later.
//#define CONVERSATION_DESTROY_TIMEOUT 1800000
#define CONVERSATION_DESTROY_TIMEOUT 6000

static gboolean timeout_once_purple_conversation_destroy(gpointer data)
{
	purple_conversation_destroy(static_cast<PurpleConversation *>(data));
	return FALSE;
}

Conversation::Conversation(PurpleConversation *conv_)
: Window(0, 0, 80, 24)
, conv(conv_)
, logfile(NULL)
, destroy_id(0)
{
	g_assert(conv);

	SetColorScheme("conversation");

	conf = Conf::Instance();

	view = new TextView(*this, 1, 0, width - 2, height, true);
	input = new TextEdit(*this, 1, 1, width - 2, height);
	line = new HorizontalLine(*this, 0, height, width);
	AddWidget(*view);
	AddWidget(*input);
	AddWidget(*line);

	SetInputChild(*input);
	input->GrabFocus();

	SetPartitioning(conf->GetChatPartitioning());
	
	MoveResizeRect(conf->GetChatDimensions());

	// open logfile
	GError *err = NULL;
	const char *name = purple_conversation_get_name(conv);
	const char *acc = purple_account_get_username(purple_conversation_get_account(conv));

	gchar *dirname = g_build_filename(purple_user_dir(), LOGS_DIR, name, NULL);
	if (g_mkdir_with_parents(dirname, 0700) == -1)
		LOG->Write(Log::Level_error, _("Error creating directory `%s'.\n"), dirname);
	g_free(dirname);

	gchar *filename = g_build_filename(purple_user_dir(), LOGS_DIR, name, acc, NULL);
	if ((logfile = g_io_channel_new_file(filename, "a", &err)) == NULL) {
		if (err) {
			LOG->Write(Log::Level_error, _("Error opening conversation logfile `%s' (%s).\n"), filename, err->message);
			g_error_free(err);
			err = NULL;
		}
		else
			LOG->Write(Log::Level_error, _("Error opening conversation logfile `%s'.\n"), filename);
	}
	g_free(filename);

	DeclareBindables();
}

Conversation::~Conversation()
{
	if (logfile)
		g_io_channel_unref(logfile);
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

void Conversation::Close()
{
	WindowManager::Instance()->Remove(this);
	/* Let libpurple and Conversations know that this conversation should be
	 * destroyed after some time. */
	destroy_id = g_timeout_add(CONVERSATION_DESTROY_TIMEOUT, timeout_once_purple_conversation_destroy, conv);
}

void Conversation::ScreenResized()
{
	MoveResizeRect(CenterIM::Instance().ScreenAreaSize(CenterIM::ChatArea));
}

void Conversation::Show()
{
	if (destroy_id) {
		g_source_remove(destroy_id);
		destroy_id = 0;
	}
	WindowManager::Instance()->Add(this);
}

void Conversation::MoveResize(int newx, int newy, int neww, int newh)
{
	Window::MoveResize(newx, newy, neww, newh);

	SetPartitioning(conf->GetChatPartitioning());
}

void Conversation::Receive(const char *name, const char *alias, const char *message,
	PurpleMessageFlags flags, time_t mtime)
{
	// we currently don't support displaying HTML in any way
	char *nohtml = purple_markup_strip_html(message);

	int color = 0;
	char type = 'O'; // other
	if (flags & PURPLE_MESSAGE_SEND) {
		color = 1;
		type = 'S'; // sent
	}
	else if (flags & PURPLE_MESSAGE_RECV) {
		color = 2;
		type = 'R'; // recv
	}

	// write text into logfile
	// encode all newline characters as <br>
	char *html = purple_strdup_withhtml(nohtml);
	char *msg = g_strdup_printf("%c %d %s\n", type, static_cast<int>(mtime), html);
	g_free(html);
	if (logfile) {
		GError *err = NULL;
		if (g_io_channel_write_chars(logfile, msg, -1, NULL, &err) != G_IO_STATUS_NORMAL) {
			if (err) {
				LOG->Write(Log::Level_error, _("Error writing to conversation logfile (%s).\n"), err->message);
				g_error_free(err);
				err = NULL;
			}
			else
				LOG->Write(Log::Level_error, _("Error writing to conversation logfile.\n"));
		}
		if (g_io_channel_flush(logfile, &err) != G_IO_STATUS_NORMAL) {
			if (err) {
				LOG->Write(Log::Level_error, _("Error flushing conversation logfile (%s).\n"), err->message);
				g_error_free(err);
				err = NULL;
			}
			else
				LOG->Write(Log::Level_error, _("Error flushing conversation logfile.\n"));
		}
	}
	g_free(msg);

	// write text to the window
	msg = g_strdup_printf("%s %s", purple_utf8_strftime(TIME_FORMAT, localtime(&mtime)), nohtml);
	view->Append(msg, color);
	g_free(msg);

	g_free(nohtml);
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
	LOG->Write(Log::Level_debug, "%p constructor()\n", this);
}

ConversationIm::~ConversationIm()
{
	LOG->Write(Log::Level_debug, "%p destructor()\n", this);
}

void ConversationIm::LoadHistory()
{
	// open logfile
	GError *err = NULL;
	GIOChannel *chan;
	const char *name = purple_conversation_get_name(conv);
	const char *acc = purple_account_get_username(purple_conversation_get_account(conv));

	gchar *filename = g_build_filename(purple_user_dir(), LOGS_DIR, name, acc, NULL);
	if ((chan = g_io_channel_new_file(filename, "r", &err)) == NULL) {
		if (err) {
			LOG->Write(Log::Level_error, _("Error opening conversation logfile `%s' (%s).\n"), filename, err->message);
			g_error_free(err);
			err = NULL;
		}
		else
			LOG->Write(Log::Level_error, _("Error opening conversation logfile `%s'.\n"), filename);
	}

	GIOStatus st;
	gchar *line;
	// read conversation logfile line by line
	while ((st = g_io_channel_read_line(chan, &line, NULL, NULL, &err)) == G_IO_STATUS_NORMAL && line != NULL) {
		// parse type
		const gchar *cur = line;
		int color = 0;
		switch (*cur) {
			case 'O': // other
				break;
			case 'S': // sent
				color = 1;
				break;
			case 'R': // recv
				color = 2;
				break;
			default: // wrong format
				continue;
		}

		// skip to time mark
		cur++;
		if (*cur != ' ')
			continue;
		cur++;

		// parse time
		time_t time = 0;
		while (*cur >= '0' && *cur <= '9') {
			time = 10 * time + *cur - '0';
			cur++;
		}

		// sanity check
		if (*cur != ' ')
			continue;
		cur++;

		// write text to the window
		char *nohtml = purple_markup_strip_html(cur);
		char *msg = g_strdup_printf("%s %s", purple_utf8_strftime(TIME_FORMAT, localtime(&time)), nohtml);
		g_free(nohtml);
		view->Append(msg, color);
		g_free(msg);

		g_free(line);
	}
	if (st != G_IO_STATUS_EOF) {
		if (err) {
			LOG->Write(Log::Level_error, _("Error reading from conversation logfile `%s' (%s).\n"), filename, err->message);
			g_error_free(err);
			err = NULL;
		}
		else
			LOG->Write(Log::Level_error, _("Error reading from conversation logfile `%s'.\n"), filename);
	}
	g_free(filename);
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
