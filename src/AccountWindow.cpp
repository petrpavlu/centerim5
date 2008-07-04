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

#include "AccountWindow.h"

#include <cppconsui/Button.h>
#include <cppconsui/Keys.h>
#include <cppconsui/WindowManager.h>

#include <libpurple/account.h>
#include <libpurple/accountopt.h>

#include <cstring>
#include <errno.h>

AccountWindow::AccountWindow()
: Window(0, 0, 80, 24, NULL)
, menu_index(3)
, accounts_index(1)
{
	//const gchar *context = "accountwindow";
	log = Log::Instance();
	conf = Conf::Instance();

	MoveResize(conf->GetAccountWindowDimensions());

	//TODO get linestyle from conf
	border = new Panel(*this, 0, 0, w, h, LineStyle::LineStyleDefault());
	AddWidget(border);

	accounts = new TreeView(*this, 1, 1, w-2, h-4, LineStyle::LineStyleDefault());
	menu = new HorizontalListBox(*this, 1, h-2, w-2, 1);
	line = new HorizontalLine(*this, 1, h-3, w-2);

	accounts->FocusCycle(Container::FocusCycleLocal);
	menu->FocusCycle(Container::FocusCycleLocal);

	menu->AddItem(_("Add"), sigc::mem_fun(this, &AccountWindow::Add));
	menu->AddItem(_("Delete"), sigc::mem_fun(this, &AccountWindow::Delete));
	menu->AddItem(_("Change"), sigc::mem_fun(this, &AccountWindow::Change));
	menu->AddItem(_("Done"), sigc::mem_fun(this, &AccountWindow::Close));

	AddWidget(accounts);
	AddWidget(menu);
	AddWidget(line);

	Populate();

	SetInputChild(accounts);
}

AccountWindow::~AccountWindow()
{
}

void AccountWindow::Add(void)
{
}

void AccountWindow::Change(void)
{
}

void AccountWindow::Delete(void)
{
}

void AccountWindow::MoveFocus(FocusDirection direction)
{
	switch (direction) {
		case FocusLeft:
		case FocusRight:
			if (focus_child != menu) {
				accounts_index = accounts->GetActive();
				menu->SetActive(menu_index);
			}

			Window::MoveFocus(direction);

			break;
		case FocusUp:
		case FocusDown:
			if (focus_child != accounts) {
				menu_index = menu->GetActive();
				accounts->SetActive(accounts_index);
			}

			Window::MoveFocus(direction);

			break;
		default:
			Window::MoveFocus(direction);
			break;
	}
}

void AccountWindow::Populate(void)
{
	GList *iter, *jter, *pref;
	PurplePluginProtocolInfo *prplinfo;
	PurpleAccount *account;
	TreeView::NodeReference parent;
	char *username, *s;
	const char *value;
	char *label;
	AccountEntry *account_entry;
	AccountOptionSplit *widget_split;

	for (iter = purple_accounts_get_all(); iter; iter = iter->next) {
		account = (PurpleAccount*)iter->data;
		prplinfo = PURPLE_PLUGIN_PROTOCOL_INFO(purple_find_prpl(purple_account_get_protocol_id(account)));

		if (account_entries.find(account) == account_entries.end()) {
			/* No entry for this account, so add one. */
			AccountEntry entry;
			entry.parent = NULL;
			account_entries[account] = entry;
		} else {
			/* The account exists, so clear all data. */
			//TODO: ClearAccount(account);
		}

		account_entry = &(account_entries[account]);

		label = g_strdup_printf(" [%s] %s",
			purple_account_get_protocol_name(account),
			purple_account_get_username(account));
		parent = accounts->AddNode(accounts->Root(), new Button(*accounts, 0, 0, label), account);

		/* The username must be treated in a special way because it can contain
		 * multiple values. (eg user@server:port/resource) */
		username = g_strdup(purple_account_get_username(account));
		
		for (jter = g_list_last(prplinfo->user_splits); jter; jter = jter->prev) {
			PurpleAccountUserSplit *split = (PurpleAccountUserSplit*)jter->data;

			if(purple_account_user_split_get_reverse(split))
                                s = strrchr(username, purple_account_user_split_get_separator(split));
                        else
                                s = strchr(username, purple_account_user_split_get_separator(split));

                        if (s != NULL)
                        {
                                *s = '\0';
                                s++;
                                value = s;
                        } else {
				value = purple_account_user_split_get_default_value(split);
			}

			/* Create widget for the username split and remember */
			widget_split = new AccountOptionSplit(*accounts, account, split, account_entry);
			widget_split->SetValue(value);
			widget_split->UpdateText();
			account_entry->split_widgets.push_front(widget_split);

			accounts->AddNode(parent, widget_split, NULL);
		}


		//TODO add this widget as the first widget in this subtree. Treeview needs to support this.
		widget_split = new AccountOptionSplit(*accounts, account, NULL, account_entry);
		widget_split->SetValue(username);
		widget_split->UpdateText();
		account_entry->split_widgets.push_front(widget_split);
		accounts->AddNode(parent, widget_split, NULL);
		g_free(username);

		label = g_strdup_printf("%s: %s", _("Password"),purple_account_get_password(account));
		accounts->AddNode(parent, new Button(*accounts, 0, 0, label), NULL);
		g_free(label);

		label = g_strdup_printf("%s: %s", _("Alias"), purple_account_get_alias(account));
		accounts->AddNode(parent, new Button(*accounts, 0, 0, label), NULL);
		g_free(label);

		for (pref = prplinfo->protocol_options; pref; pref = pref->next) {
			PurpleAccountOption *option = (PurpleAccountOption*)pref->data;
			PurplePrefType type = purple_account_option_get_type(option);

			switch (type) {
			case PURPLE_PREF_STRING:
				accounts->AddNode(parent,
					new AccountOptionString(*accounts, account, option),
					NULL);
				break;
			case PURPLE_PREF_INT:
				accounts->AddNode(parent,
					new AccountOptionInt(*accounts, account, option),
					NULL);
				break;
			case PURPLE_PREF_BOOLEAN:
				accounts->AddNode(parent,
					new AccountOptionBool(*accounts, account, option),
					NULL);
				break;
			case PURPLE_PREF_STRING_LIST:
				//TODO implement, but for now, an error!
				break;
			default:
				//TODO error!
				break;
			}
		}
	}
}

AccountWindow::AccountOption::AccountOption(Widget& parent,
	PurpleAccount *account, PurpleAccountOption *option)
: Button(parent, x, y, "")
, account(account)
, option(option)
{
	g_assert(account != NULL && option != NULL);

	setting = purple_account_option_get_setting(option);
	text = purple_account_option_get_text(option);

	SetFunction(sigc::mem_fun(this, 
		&AccountWindow::AccountOption::OnActivate));
}

AccountWindow::AccountOption::~AccountOption()
{
}

AccountWindow::AccountOptionBool::AccountOptionBool(Widget& parent,
	PurpleAccount *account, PurpleAccountOption *option)
: AccountWindow::AccountOption::AccountOption(parent, account, option)
{
	UpdateText();
}

AccountWindow::AccountOptionBool::~AccountOptionBool()
{
}

void AccountWindow::AccountOptionBool::UpdateText(void)
{
	gchar *str;
	
	value = purple_account_get_bool(account, setting, 
			purple_account_option_get_default_bool(option));

	//TODO create some DEFAULT_TEXT_YES etc, also for use in dialogs
	str = g_strdup_printf("%s: %s", text, value ? _("yes") : _("no"));
	SetText(str);
	g_free(str);
}


void AccountWindow::AccountOptionBool::OnActivate(void)
{
	purple_account_set_bool(account, setting, !value);
	UpdateText();
}

AccountWindow::AccountOptionString::AccountOptionString(Widget& parent,
	PurpleAccount *account, PurpleAccountOption *option)
: AccountWindow::AccountOption::AccountOption(parent, account, option)
, value(NULL)
, dialog(NULL)
{
	UpdateText();
}

AccountWindow::AccountOptionString::~AccountOptionString()
{
}

void AccountWindow::AccountOptionString::UpdateText(void)
{
	gchar *str;
	
	value = purple_account_get_string(account, setting, 
			purple_account_option_get_default_string(option));
	if (value == NULL)
		value = "";

	str = g_strdup_printf("%s: %s", text, value);
	SetText(str);
	g_free(str);
}

void AccountWindow::AccountOptionString::OnActivate(void)
{
	WindowManager *wm = WindowManager::Instance();
	dialog = new InputDialog(text, value);
	sig_response = dialog->signal_response.connect(
		sigc::mem_fun(this, &AccountWindow::AccountOptionString::ResponseHandler));
	//TODO add something to dialog class to show it. this removes the need
	//for including windowmanager.h, and is easier to use.
	wm->Add(dialog);
}

void AccountWindow::AccountOptionString::ResponseHandler(Dialog::ResponseType response)
{
	switch (response) {
		case Dialog::ResponseOK:
			if (!dialog)
				/*TODO something is very wrong here */;

			purple_account_set_string(account, setting, dialog->GetText());
			UpdateText();

			break;
		default:
			break;
	}
}

AccountWindow::AccountOptionInt::AccountOptionInt(Widget& parent,
	PurpleAccount *account, PurpleAccountOption *option)
: AccountWindow::AccountOption::AccountOption(parent, account, option)
, value(NULL)
, dialog(NULL)
{
	UpdateText();
}

AccountWindow::AccountOptionInt::~AccountOptionInt()
{
	if (value)
		g_free(value);
}

void AccountWindow::AccountOptionInt::UpdateText(void)
{
	gchar *str;

	if (value)
		g_free(value);

	value = g_strdup_printf("%d", purple_account_get_int(account, setting, 
		purple_account_option_get_default_int(option)));

	str = g_strdup_printf("%s: %s", text, value);
	SetText(str);
	g_free(str);
}

void AccountWindow::AccountOptionInt::OnActivate(void)
{
	WindowManager *wm = WindowManager::Instance();
	dialog = new InputDialog(text, value);
	dialog->Flags(TextEntry::FlagNumeric);
	sig_response = dialog->signal_response.connect(
		sigc::mem_fun(this, &AccountWindow::AccountOptionInt::ResponseHandler));
	//TODO add something to dialog class to show it. this removes the need
	//for including windowmanager.h, and is easier to use.
	wm->Add(dialog);
}

void AccountWindow::AccountOptionInt::ResponseHandler(Dialog::ResponseType response)
{
	const gchar* text;
	long int i;

	switch (response) {
		case Dialog::ResponseOK:
			if (!dialog)
				/*TODO something is very wrong here */;

			text = dialog->GetText();
			i = strtol(text, NULL, 10);
			if (errno == ERANGE) 
				/*TODO handle error? */;

			purple_account_set_int(account, setting, i);
			UpdateText();

			break;
		default:
			break;
	}
}

AccountWindow::AccountOptionSplit::AccountOptionSplit(Widget& parent,
	PurpleAccount *account, PurpleAccountUserSplit *split,
	AccountEntry *account_entry)
: Button(parent, 0, 0, "")
, account(account)
, split(split)
, account_entry(account_entry)
{
	g_assert(account != NULL);

	if (split) {
		text = purple_account_user_split_get_text(split);
	} else {
		text = _("Username");
	}
	value = "";

	SetFunction(sigc::mem_fun(this, 
		&AccountWindow::AccountOptionSplit::OnActivate));

	UpdateText();
}

AccountWindow::AccountOptionSplit::~AccountOptionSplit()
{
	//if (value)
	//	g_free(value);
}

void AccountWindow::AccountOptionSplit::UpdateText(void)
{
	gchar *str;
	
	if (value == NULL)
		value =	g_strdup(purple_account_user_split_get_text(split));
	if (value == NULL)
		value = g_strdup("");

	str = g_strdup_printf("%s: %s", text, value);
	SetText(str);
	g_free(str);
}

void AccountWindow::AccountOptionSplit::UpdateSplits(void)
{
	AccountWindow::SplitWidgets::iterator split_widget;
	PurpleAccountUserSplit *split;
	AccountWindow::AccountOptionSplit *widget;
	PurplePluginProtocolInfo *prplinfo;
	SplitWidgets *split_widgets;
	GList *iter;
	const gchar *value;

	split_widgets = &(account_entry->split_widgets);
	split_widget = split_widgets->begin();
	widget = *split_widget;
	value = widget->GetValue();
	split_widget++;

	GString *username = g_string_new(value);
	prplinfo = PURPLE_PLUGIN_PROTOCOL_INFO(purple_find_prpl(purple_account_get_protocol_id(account)));

	for (iter = prplinfo->user_splits;
			iter && split_widget != split_widgets->end();
			iter = iter->next, split_widget++)
	{
		split = (PurpleAccountUserSplit*)iter->data;
		widget = *split_widget;

		value = widget->GetValue();
		if (value == NULL || *value == '\0')
			value = purple_account_user_split_get_default_value(split);
		g_string_append_printf(username, "%c%s",
				purple_account_user_split_get_separator(split),
				value);
	}

	purple_account_set_username(account, username->str);
}

void AccountWindow::AccountOptionSplit::SetValue(const gchar *value)
{
	this->value = g_strdup(value);
	UpdateText();
}

void AccountWindow::AccountOptionSplit::OnActivate(void)
{
	WindowManager *wm = WindowManager::Instance();
	dialog = new InputDialog(text, value);
	sig_response = dialog->signal_response.connect(
		sigc::mem_fun(this, &AccountWindow::AccountOptionSplit::ResponseHandler));
	//TODO add something to dialog class to show it. this removes the need
	//for including windowmanager.h, and is easier to use.
	wm->Add(dialog);
}

void AccountWindow::AccountOptionSplit::ResponseHandler(Dialog::ResponseType response)
{
	switch (response) {
		case Dialog::ResponseOK:
			if (!dialog)
				/*TODO something is very wrong here */;

			SetValue(dialog->GetText());
			UpdateSplits();
			break;
		default:
			break;
	}
}

