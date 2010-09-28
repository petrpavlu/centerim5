/*
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

#include "Request.h"

#include "CenterIM.h"
#include "Log.h"

#include <cstring>

Request *Request::Instance()
{
	static Request instance;
	return &instance;
}

Request::Request()
{
	memset(&centerim_request_ui_ops, 0, sizeof(centerim_request_ui_ops));

	// set the purple request callbacks
	centerim_request_ui_ops.request_input = request_input_;
	centerim_request_ui_ops.request_choice = request_choice_;
	centerim_request_ui_ops.request_action = request_action_;
	centerim_request_ui_ops.request_fields = request_fields_;
	centerim_request_ui_ops.request_file = request_file_;
	centerim_request_ui_ops.close_request = close_request_;
	centerim_request_ui_ops.request_folder = request_folder_;
	centerim_request_ui_ops.request_action_with_icon
		= request_action_with_icon_;
	purple_request_set_ui_ops(&centerim_request_ui_ops);
}

Request::~Request()
{
}

void *Request::request_input(const char *title, const char *primary,
		const char *secondary, const char *default_value, gboolean multiline,
		gboolean masked, gchar *hint, const char *ok_text, GCallback ok_cb,
		const char *cancel_text, GCallback cancel_cb, PurpleAccount *account,
		const char *who, PurpleConversation *conv, void *user_data)
{
	LOG->Write(Log::Level_debug, "request_input\n");

	InputDialog *dialog = new InputDialog(title, primary, secondary,
			default_value, masked, ok_text, ok_cb, cancel_text, cancel_cb,
			user_data);
	dialog->Show();
	return dialog;
}

void *Request::request_choice(const char *title, const char *primary,
		const char *secondary, int default_value, const char *ok_text,
		GCallback ok_cb, const char *cancel_text, GCallback cancel_cb,
		PurpleAccount *account, const char *who, PurpleConversation *conv,
		void *user_data, va_list choices)
{
	return NULL;
}

void *Request::request_action(const char *title, const char *primary,
		const char *secondary, int default_action, PurpleAccount *account,
		const char *who, PurpleConversation *conv, void *user_data,
		size_t action_count, va_list actions)
{
	return NULL;
}

void *Request::request_fields(const char *title, const char *primary,
		const char *secondary, PurpleRequestFields *fields,
		const char *ok_text, GCallback ok_cb, const char *cancel_text,
		GCallback cancel_cb, PurpleAccount *account, const char *who,
		PurpleConversation *conv, void *user_data)
{
	return NULL;
}

void *Request::request_file(const char *title, const char *filename,
		gboolean savedialog, GCallback ok_cb, GCallback cancel_cb,
		PurpleAccount *account, const char *who, PurpleConversation *conv,
		void *user_data)
{
	return NULL;
}

void Request::close_request(PurpleRequestType type, void *ui_handle)
{
}

void *Request::request_folder(const char *title, const char *dirname,
		GCallback ok_cb, GCallback cancel_cb, PurpleAccount *account,
		const char *who, PurpleConversation *conv, void *user_data)
{
	return NULL;
}

void *Request::request_action_with_icon(const char *title,
		const char *primary, const char *secondary, int default_action,
		PurpleAccount *account, const char *who, PurpleConversation *conv,
		gconstpointer icon_data, gsize icon_size, void *user_data,
		size_t action_count, va_list actions)
{
	return NULL;
}

Request::RequestDialog::RequestDialog(const gchar *title,
		const gchar *primary, const gchar *secondary, const gchar *ok_text,
		GCallback ok_cb, const gchar *cancel_text, GCallback cancel_cb,
		void *user_data)
: SplitDialog()
, ok_cb(ok_cb)
, cancel_cb(cancel_cb)
, user_data(user_data)
{
	ListBox *l = new ListBox(AUTOSIZE, AUTOSIZE);
	if (title)
		l->AppendWidget(*(new Label(title)));
	if (primary)
		l->AppendWidget(*(new Label(primary)));
	if (secondary)
		l->AppendWidget(*(new Label(secondary)));
	SetContainer(*l);

	if (ok_text)
		AddButton(ok_text, RESPONSE_OK);
	if (ok_text && cancel_text)
		AddSeparator();
	if (cancel_text)
		AddButton(cancel_text, RESPONSE_CANCEL);
}

Request::RequestDialog::~RequestDialog()
{
}

void Request::RequestDialog::ScreenResized()
{
	Rect screen = CENTERIM->ScreenAreaSize(CenterIM::WholeArea);

	MoveResize(screen.Width() / 4, screen.Height() / 4, screen.Width() / 2,
			screen.Height() / 2);
}

Request::InputDialog::InputDialog(const gchar *title, const gchar *primary,
		const gchar *secondary, const gchar *default_value, bool masked,
		const gchar *ok_text, GCallback ok_cb, const gchar *cancel_text,
		GCallback cancel_cb, void *user_data)
: RequestDialog(title, primary, secondary, ok_text, ok_cb, cancel_text,
		cancel_cb, user_data)
{
}

Request::InputDialog::~InputDialog()
{
}
