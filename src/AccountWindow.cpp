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

#include <cstring>

#include <libpurple/account.h>
#include <libpurple/accountopt.h>

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

	for (iter = purple_accounts_get_all(); iter; iter = iter->next) {
		account = (PurpleAccount*)iter->data;
		prplinfo = PURPLE_PLUGIN_PROTOCOL_INFO(purple_find_prpl(purple_account_get_protocol_id(account)));

		label = g_strdup_printf(" [%s] %s",
			purple_account_get_protocol_name(account),
			purple_account_get_username(account));
		parent = accounts->AddNode(accounts->Root(), new Button(*accounts, 0, 0, label), account);

		/* We need to process the user name in a special way */
		username = g_strdup(purple_account_get_username(account));

		for (jter = g_list_last(prplinfo->user_splits); jter; jter = jter->prev) {
			PurpleAccountUserSplit *split = (PurpleAccountUserSplit*)jter->data;

			if(purple_account_user_split_get_reverse(split))
                                s = strrchr(username, purple_account_user_split_get_separator(split));
                        else
                                s = strchr(username, purple_account_user_split_get_separator(split));
			log->Write(Log::Level_debug, s);

                        if (s != NULL)
                        {
                                *s = '\0';
                                s++;
                                value = s;
                        } else {
				value = NULL;
			}

			if (value == NULL)
				value = purple_account_user_split_get_default_value(split);

			label = g_strdup_printf("%s: %s", purple_account_user_split_get_text(split), value);
			accounts->AddNode(parent, new Button(*accounts, 0, 0, label), NULL);
			g_free(label);

		}

		//TODO add this widget as the first widget in this subtree. Treeview needs to support this.
		label = g_strdup_printf("%s: %s", _("Screen name"), username);
		accounts->AddNode(parent, new Button(*accounts, 0, 0, label), NULL);
		g_free(label);
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
			const char *setting = purple_account_option_get_setting(option);
			const char *text = purple_account_option_get_text(option);

			char *value;

			switch (type) {
			case PURPLE_PREF_STRING:
				accounts->AddNode(parent,
					new AccountOptionString(*accounts, 0, 0, account, option),
					NULL);
				break;
			case PURPLE_PREF_INT:
				value = g_strdup_printf("%s: %d", text,
						purple_account_get_int(account, setting, 0));
				accounts->AddNode(parent, new Button(*accounts, 0, 0, value), NULL);
				g_free(value);
				break;
			case PURPLE_PREF_BOOLEAN:
				accounts->AddNode(parent,
					new AccountOptionBool(*accounts, 0, 0, account, option),
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

AccountWindow::AccountOption::AccountOption(Widget& parent, int x, int y,
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

AccountWindow::AccountOptionBool::AccountOptionBool(Widget& parent, int x, int y,
	PurpleAccount *account, PurpleAccountOption *option)
: AccountWindow::AccountOption::AccountOption(parent, x, y, account, option)
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

AccountWindow::AccountOptionString::AccountOptionString(Widget& parent, int x, int y,
	PurpleAccount *account, PurpleAccountOption *option)
: AccountWindow::AccountOption::AccountOption(parent, x, y, account, option)
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
