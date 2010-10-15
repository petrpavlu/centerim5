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

#include <cppconsui/Spacer.h>
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

void Request::OnDialogResponse(RequestDialog& dialog,
		Dialog::ResponseType response)
{
	requests.erase(&dialog);
	purple_request_close(dialog.GetRequestType(), &dialog);
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
	dialog->signal_response.connect(sigc::bind<0>(sigc::mem_fun(this,
					&Request::OnDialogResponse), sigc::ref(*dialog)));
	dialog->Show();
	return dialog;
}

void *Request::request_choice(const char *title, const char *primary,
		const char *secondary, int default_value, const char *ok_text,
		GCallback ok_cb, const char *cancel_text, GCallback cancel_cb,
		PurpleAccount *account, const char *who, PurpleConversation *conv,
		void *user_data, va_list choices)
{
	LOG->Write(Log::Level_debug, "request_choice\n");

	ChoiceDialog *dialog = new ChoiceDialog(title, primary, secondary,
			default_value, ok_text, ok_cb, cancel_text, cancel_cb, user_data,
			choices);
	dialog->signal_response.connect(sigc::bind<0>(sigc::mem_fun(this,
					&Request::OnDialogResponse), sigc::ref(*dialog)));
	dialog->Show();
	return dialog;
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
	LOG->Write(Log::Level_debug, "close_request\n");

	g_assert(ui_handle);

	InputDialog *dialog = reinterpret_cast<InputDialog *>(ui_handle);
	if (requests.find(dialog) != requests.end()) {
		requests.erase(dialog);
		delete dialog;
	}
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
: SplitDialog(title)
, ok_cb(ok_cb)
, cancel_cb(cancel_cb)
, user_data(user_data)
{
	lbox = new ListBox(AUTOSIZE, AUTOSIZE);
	if (primary)
		lbox->AppendWidget(*(new Label(primary)));
	if (primary && secondary)
		lbox->AppendWidget(*(new Spacer(AUTOSIZE, 1)));
	if (secondary)
		lbox->AppendWidget(*(new Label(secondary)));
	if (primary || secondary)
		lbox->AppendWidget(*(new HorizontalLine(AUTOSIZE)));
	SetContainer(*lbox);

	if (ok_text)
		AddButton(ok_text, RESPONSE_OK);
	if (ok_text && cancel_text)
		AddSeparator();
	if (cancel_text)
		AddButton(cancel_text, RESPONSE_CANCEL);
	signal_response.connect(sigc::mem_fun(this,
				&Request::RequestDialog::ResponseHandler));
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
	entry = new TextEntry(AUTOSIZE, AUTOSIZE, default_value);
	lbox->AppendWidget(*entry);
	entry->GrabFocus();
}

PurpleRequestType Request::InputDialog::GetRequestType()
{
	return PURPLE_REQUEST_INPUT;
}

void Request::InputDialog::ResponseHandler(ResponseType response)
{
	switch (response) {
		case Dialog::RESPONSE_OK:
			if (ok_cb)
				reinterpret_cast<PurpleRequestInputCb>(ok_cb)(user_data,
						entry->GetText());
			break;
		case Dialog::RESPONSE_CANCEL:
			if (cancel_cb)
				reinterpret_cast<PurpleRequestInputCb>(cancel_cb)(user_data,
						entry->GetText());
			break;
		default:
			g_assert_not_reached();
			break;
	}
}

Request::ChoiceDialog::ChoiceDialog(const gchar *title, const gchar *primary,
		const gchar *secondary, int default_value, const gchar *ok_text,
		GCallback ok_cb, const gchar *cancel_text, GCallback cancel_cb,
		void *user_data, va_list choices)
: RequestDialog(title, primary, secondary, ok_text, ok_cb, cancel_text,
		cancel_cb, user_data)
{
	combo = new ComboBox;
	lbox->AppendWidget(*combo);
	combo->GrabFocus();

	gchar *text;
	while ((text = va_arg(choices, gchar *))) {
		int resp = va_arg(choices, int);
		combo->AddOption(text, resp);
		if (resp == default_value)
			combo->SetSelectedByData(resp);
	}
}

PurpleRequestType Request::ChoiceDialog::GetRequestType()
{
	return PURPLE_REQUEST_CHOICE;
}

void Request::ChoiceDialog::ResponseHandler(ResponseType response)
{
	size_t selected = combo->GetSelected();
	int data = combo->GetData(selected);

	switch (response) {
		case Dialog::RESPONSE_OK:
			if (ok_cb)
				reinterpret_cast<PurpleRequestChoiceCb>(ok_cb)(user_data,
						data);
			break;
		case Dialog::RESPONSE_CANCEL:
			if (cancel_cb)
				reinterpret_cast<PurpleRequestChoiceCb>(cancel_cb)(user_data,
						data);
			break;
		default:
			g_assert_not_reached();
			break;
	}
}
