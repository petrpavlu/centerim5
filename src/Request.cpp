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
	return NULL;
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
