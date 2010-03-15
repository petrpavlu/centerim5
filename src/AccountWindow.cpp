/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
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

#include "AccountWindow.h"
#include "CenterIM.h"
#include "config.h"

#include <cppconsui/Button.h>
#include <cppconsui/Keys.h>
#include <cppconsui/MessageDialog.h>
#include <cppconsui/WindowManager.h>

#include <libpurple/account.h>
#include <libpurple/accountopt.h>

#include <cstring>
#include <errno.h>

#include <glib.h>
#include "gettext.h"

AccountWindow::AccountWindow()
: Window(0, 0, 80, 24)
, menu_index(2)
, accounts_index(1)
{
	SetColorScheme("accountwindow");

	conf = Conf::Instance();

	accounts = new TreeView(*this, 0, 0, width, height - 2);
	menu = new HorizontalListBox(*this, 1, height - 1, width, 1);
	line = new HorizontalLine(*this, 1, height - 2, width);

	accounts->FocusCycle(Container::FocusCycleLocal);
	menu->FocusCycle(Container::FocusCycleLocal);

	menu->AddItem(_("Add"), sigc::mem_fun(this, &AccountWindow::Add));
	menu->AddSeparator();
	menu->AddItem(_("Done"), sigc::mem_fun(this, &AccountWindow::Close));

	AddWidget(*accounts);
	AddWidget(*menu);
	AddWidget(*line);

	MoveResizeRect(conf->GetAccountWindowDimensions());
	
	Populate();

	menu->GrabFocus();
}

void AccountWindow::MoveResize(int newx, int newy, int neww, int newh)
{
	Window::MoveResize(newx, newy, neww, newh);

	accounts->MoveResize(0, 0, width, height - 2);
	menu->MoveResize(0, height - 1, width, 1);
	line->MoveResize(0, height - 2, width, 1);
}

void AccountWindow::ScreenResized()
{
	Rect screen = CenterIM::Instance().ScreenAreaSize(CenterIM::WholeArea);
	Rect confSize = conf->GetAccountWindowDimensions();

	// Check against screen size
	if (screen.Width() < (confSize.Width()+4))
		confSize.width = screen.Width()-4;

	if (screen.Height() < (confSize.Height()+4))
		confSize.height = screen.Height()-4;

	// Center on screen
	confSize.x = (screen.Width() - confSize.Width()) / 2;
	confSize.y = (screen.Height() - confSize.Height()) / 2;
	
	MoveResizeRect(confSize);
}

void AccountWindow::Add()
{
	GList *i;
	PurpleAccount *account;

	i = purple_plugins_get_protocols();
	account = purple_account_new(_("Username"),
			purple_plugin_get_id((PurplePlugin*)i->data));

	/* Stop here if libpurple returned an already created account. This
	 * happens when user pressed Add button twice in a row. In that case there
	 * is already one "empty" account that user can edit. */
	if (account_entries.find(account) == account_entries.end()) {
		purple_accounts_add(account);

		PopulateAccount(account);
	}
}

//TODO move to Accounts class
void AccountWindow::DropAccount(PurpleAccount *account)
{
	WindowManager *wm = WindowManager::Instance();
	MessageDialog *dialog = new MessageDialog(_("Are you sure you want to delete this account?"));
	dialog->signal_response.connect(
		sigc::bind(sigc::mem_fun(this, &AccountWindow::DropAccountResponseHandler), account));
	//TODO add something to dialog class to show it. this removes the need
	//for including windowmanager.h, and is easier to use.
	wm->Add(dialog);
}

//TODO move to Accounts.cpp
void AccountWindow::DropAccountResponseHandler(Dialog::ResponseType response, PurpleAccount *account)
{
	switch (response) {
		case Dialog::ResponseOK:
			purple_accounts_remove(account);
			ClearAccount(account, true);
			break;
		default:
			break;
	}
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

void AccountWindow::Clear()
{
	GList *i;
	PurpleAccount *account;

	for (i = purple_accounts_get_all(); i; i = i->next) {
		account = (PurpleAccount*)i->data;
		ClearAccount(account, true);
	}
}

bool AccountWindow::ClearAccount(PurpleAccount *account, bool full)
{
	AccountEntry* account_entry = &(account_entries[account]);
	std::vector<Widget*>::iterator i;
	std::list<AccountOptionSplit*>::iterator j;

	// move focus
	account_entry->parent->GrabFocus();

	// first remove the nodes from the tree, then free() all the widgets
	accounts->DeleteChildren(account_entry->parent_reference, false);
	if (full)
		accounts->DeleteNode(account_entry->parent_reference, false);

	for (i = account_entry->widgets.begin(); i != account_entry->widgets.end(); i++) {
		delete *i;
	}
	if (full)
		delete account_entry->parent;

	account_entry->widgets.clear();
	account_entry->split_widgets.clear();
	if (full)
		account_entries.erase(account);

	return false;
}

void AccountWindow::Populate()
{
	GList *i;
	PurpleAccount *account;

	for (i = purple_accounts_get_all(); i; i = i->next) {
		account = (PurpleAccount*)i->data;
		PopulateAccount(account);
	}
}

void AccountWindow::PopulateAccount(PurpleAccount *account)
{
	GList *iter, *pref;
	PurplePlugin *prpl;
	PurplePluginProtocolInfo *prplinfo;
	char *username, *s;
	const char *value;
	const char *protocol_id;
	char *label;
	AccountEntry *account_entry;
	AccountOptionSplit *widget_split;
	Widget *widget;
	ComboBox *combobox;

	label = g_strdup_printf(" [%s] %s",
			purple_account_get_protocol_name(account),
			purple_account_get_username(account));

	if (account_entries.find(account) == account_entries.end()) {
		// no entry for this account, so add one
		AccountEntry entry;
		entry.parent = NULL;
		account_entries[account] = entry;
	} else {
		// the account exists, so clear all data
		ClearAccount(account, false);
	}

	account_entry = &(account_entries[account]);

	if (!account_entry->parent) {
		Button *button;
		TreeView::NodeReference parent_reference;

		button = new Button(*accounts, 0, 0, "",
				sigc::mem_fun(accounts, &TreeView::ActionToggleCollapsed));
		parent_reference = accounts->AddNode(accounts->Root(), button, account);
		accounts->Collapse(parent_reference);
		account_entry->parent = button;
		account_entry->parent_reference = parent_reference;
	}

	account_entry->parent->SetText(label);
	g_free(label);

	protocol_id = purple_account_get_protocol_id(account);
	prpl = purple_find_prpl(protocol_id);

	if (prpl == NULL) {
		Label *label;

		// we cannot change the settings of an unknown account
		label = new Label(*accounts, 0, 0, _("Invalid account or protocol plugin not loaded"));
		accounts->AddNode(account_entry->parent_reference, label, account);
		account_entry->widgets.push_back(label);

	}
	else {
		prplinfo = PURPLE_PLUGIN_PROTOCOL_INFO(prpl);
	
		// protocols combobox
		combobox = new AccountOptionProtocol(*accounts, account, *this);
		accounts->AddNode(account_entry->parent_reference, combobox, account);
		account_entry->widgets.push_back(combobox);

		/* The username must be treated in a special way because it can contain
		 * multiple values. (eg user@server:port/resource) */
		username = g_strdup(purple_account_get_username(account));
		
		for (iter = g_list_last(prplinfo->user_splits); iter; iter = iter->prev) {
			PurpleAccountUserSplit *split = (PurpleAccountUserSplit*)iter->data;

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

			// create widget for the username split and remember
			widget_split = new AccountOptionSplit(*accounts, account, split, account_entry);
			widget_split->SetValue(value);
			account_entry->split_widgets.push_front(widget_split);
			account_entry->widgets.push_back(widget_split);

			accounts->AddNode(account_entry->parent_reference, widget_split, NULL);
		}


		//TODO add this widget as the first widget in this subtree. Treeview needs to support this.
		widget_split = new AccountOptionSplit(*accounts, account, NULL, account_entry);
		widget_split->SetValue(username);
		account_entry->split_widgets.push_front(widget_split);
		account_entry->widgets.push_back(widget_split);
		accounts->AddNode(account_entry->parent_reference, widget_split, NULL);
		g_free(username);

		// password
		widget = new AccountOptionString(*accounts, account, true, false);
		accounts->AddNode(account_entry->parent_reference, widget, NULL);
		account_entry->widgets.push_back(widget);

		// remember password
		widget = new AccountOptionBool(*accounts, account, true, false);
		accounts->AddNode(account_entry->parent_reference, widget, NULL);
		account_entry->widgets.push_back(widget);

		// alias
		widget = new AccountOptionString(*accounts, account, false, true);
		accounts->AddNode(account_entry->parent_reference, widget, NULL);
		account_entry->widgets.push_back(widget);

		for (pref = prplinfo->protocol_options; pref; pref = pref->next) {
			PurpleAccountOption *option = (PurpleAccountOption*)pref->data;
			PurplePrefType type = purple_account_option_get_type(option);

			switch (type) {
			case PURPLE_PREF_STRING:
				widget = new AccountOptionString(*accounts, account, option);
				accounts->AddNode(account_entry->parent_reference, widget, NULL);
				account_entry->widgets.push_back(widget);
				break;
			case PURPLE_PREF_INT:
				widget = new AccountOptionInt(*accounts, account, option);
				accounts->AddNode(account_entry->parent_reference, widget, NULL);
				account_entry->widgets.push_back(widget);
				break;
			case PURPLE_PREF_BOOLEAN:
				widget = new AccountOptionBool(*accounts, account, option);
				accounts->AddNode(account_entry->parent_reference, widget, NULL);
				account_entry->widgets.push_back(widget);
				break;
			case PURPLE_PREF_STRING_LIST:
				//TODO implement, but for now, an error!
				break;
			default:
				//TODO error!
				break;
			}
		}

		// enable/disable account
		widget = new AccountOptionBool(*accounts, account, false, true);
		accounts->AddNode(account_entry->parent_reference, widget, NULL);
		account_entry->widgets.push_back(widget);
	}

	// drop account
	widget = new Button(*accounts, 0, 0, _("Drop account"),
			sigc::bind(sigc::mem_fun(this, &AccountWindow::DropAccount), account));
	accounts->AddNode(account_entry->parent_reference, widget, NULL);
	account_entry->widgets.push_back(widget);
}

AccountWindow::AccountOption::AccountOption(Widget& parent,
		PurpleAccount *account, PurpleAccountOption *option)
: Button(parent, 0, 0, "")
, account(account)
, option(option)
{
	g_assert(account);

	if (option) {
		setting = purple_account_option_get_setting(option);
		text = purple_account_option_get_text(option);
	}
	else
		setting = NULL;

	signal_activate.connect(sigc::mem_fun(this,
				&AccountWindow::AccountOption::OnActivate));
}

AccountWindow::AccountOptionBool::AccountOptionBool(Widget& parent,
	PurpleAccount *account, PurpleAccountOption *option)
: AccountWindow::AccountOption::AccountOption(parent, account, option)
, remember_password(false), enable_account(false)
{
	value = purple_account_get_bool(account, setting,
			purple_account_option_get_default_bool(option));

	UpdateText();
}

AccountWindow::AccountOptionBool::AccountOptionBool(Widget& parent,
		PurpleAccount *account, bool remember_password, bool enable_account)
: AccountWindow::AccountOption::AccountOption(parent, account)
, remember_password(remember_password), enable_account(enable_account)
{
	if (remember_password)
		text = _("Remember password");
	else if (enable_account)
		text = _("Account enabled");

	UpdateText();
}

void AccountWindow::AccountOptionBool::UpdateText()
{
	if (remember_password)
		value = purple_account_get_remember_password(account);
	else if (enable_account)
		value = purple_account_get_enabled(account, PACKAGE_NAME);
	else
		value = purple_account_get_bool(account, setting,
				purple_account_option_get_default_bool(option));

	/// @todo Create some DEFAULT_TEXT_YES etc, also for use in dialogs.
	gchar *str = g_strdup_printf("%s: %s", text, value ? _("yes") : _("no"));
	SetText(str);
	g_free(str);
}

void AccountWindow::AccountOptionBool::OnActivate()
{
	if (remember_password)
		purple_account_set_remember_password(account, !value);
	else if (enable_account)
		purple_account_set_enabled(account, PACKAGE_NAME, !value);
	else
		purple_account_set_bool(account, setting, !value);
	UpdateText();
}

AccountWindow::AccountOptionString::AccountOptionString(Widget& parent,
	PurpleAccount *account, PurpleAccountOption *option)
: AccountWindow::AccountOption::AccountOption(parent, account, option)
, value(NULL)
, dialog(NULL)
, password(false)
, alias(false)
{
	UpdateText();
}

AccountWindow::AccountOptionString::AccountOptionString(Widget& parent,
	PurpleAccount *account, bool password, bool alias)
: AccountWindow::AccountOption::AccountOption(parent, account, NULL)
, value(NULL)
, dialog(NULL)
, password(password)
, alias(alias)
{
	if (password)
		text = _("Password");
	else if (alias)
		text = _("Alias");

	UpdateText();
}

void AccountWindow::AccountOptionString::UpdateText()
{
	if (password)
		value = purple_account_get_password(account);
	else if (alias)
		value = purple_account_get_alias(account);
	else
		value = purple_account_get_string(account, setting,
				purple_account_option_get_default_string(option));

	gchar *str = g_strdup_printf("%s: %s", text, value ? value : "");
	SetText(str);
	g_free(str);
}

void AccountWindow::AccountOptionString::OnActivate()
{
	WindowManager *wm = WindowManager::Instance();
	dialog = new InputDialog(text, value);
	dialog->signal_response.connect(
			sigc::mem_fun(this, &AccountWindow::AccountOptionString::ResponseHandler));
	//TODO add something to dialog class to show it. this removes the need
	//for including windowmanager.h, and is easier to use.
	wm->Add(dialog);
}

void AccountWindow::AccountOptionString::ResponseHandler(Dialog::ResponseType response)
{
	g_assert(dialog);

	switch (response) {
		case Dialog::ResponseOK:
			if (password)
				purple_account_set_password(account, dialog->GetText());
			else if (alias)
				purple_account_set_alias(account, dialog->GetText());
			else
				purple_account_set_string(account, setting, dialog->GetText());

			UpdateText();
			break;
		default:
			break;
	}
	dialog = NULL;
}

AccountWindow::AccountOptionInt::AccountOptionInt(Widget& parent,
	PurpleAccount *account, PurpleAccountOption *option)
: AccountWindow::AccountOption::AccountOption(parent, account, option)
, value(NULL)
, dialog(NULL)
{
	UpdateText();
}

void AccountWindow::AccountOptionInt::UpdateText()
{
	gchar *str = g_strdup_printf("%s: %d", text,
			purple_account_get_int(account, setting,
				purple_account_option_get_default_int(option)));
	SetText(str);
	g_free(str);
}

void AccountWindow::AccountOptionInt::OnActivate()
{
	WindowManager *wm = WindowManager::Instance();
	dialog = new InputDialog(text, value);
	dialog->SetFlags(TextEntry::FlagNumeric);
	dialog->signal_response.connect(
			sigc::mem_fun(this, &AccountWindow::AccountOptionInt::ResponseHandler));
	//TODO add something to dialog class to show it. this removes the need
	//for including windowmanager.h, and is easier to use.
	wm->Add(dialog);
}

void AccountWindow::AccountOptionInt::ResponseHandler(Dialog::ResponseType response)
{
	g_assert(dialog);

	const gchar* text;
	long int i;

	switch (response) {
		case Dialog::ResponseOK:
			text = dialog->GetText();
			errno = 0;
			i = strtol(text, NULL, 10);
			if (errno == ERANGE)
				Log::Instance()->Write(Log::Level_warning, _("Value out of range.\n"));
			purple_account_set_int(account, setting, i);

			UpdateText();
			break;
		default:
			break;
	}
	dialog = NULL;
}

AccountWindow::AccountOptionSplit::AccountOptionSplit(Widget& parent,
	PurpleAccount *account, PurpleAccountUserSplit *split,
	AccountEntry *account_entry)
: Button(parent, 0, 0, "")
, account(account)
, split(split)
, account_entry(account_entry)
, value(NULL)
{
	g_assert(account);

	if (split)
		text = purple_account_user_split_get_text(split);
	else
		text = _("Username");
	value =	g_strdup(purple_account_user_split_get_text(split));

	signal_activate.connect(sigc::mem_fun(this,
				&AccountWindow::AccountOptionSplit::OnActivate));

	UpdateText();
}

AccountWindow::AccountOptionSplit::~AccountOptionSplit()
{
	if (value)
		g_free(value);
}

void AccountWindow::AccountOptionSplit::UpdateText()
{
	gchar *str;
	
	str = g_strdup_printf("%s: %s", text, value ? value : "");
	SetText(str);
	g_free(str);
}

void AccountWindow::AccountOptionSplit::UpdateSplits()
{
	AccountWindow::SplitWidgets::iterator split_widget;
	PurpleAccountUserSplit *user_split;
	AccountWindow::AccountOptionSplit *widget;
	PurplePluginProtocolInfo *prplinfo;
	SplitWidgets *split_widgets;
	GList *iter;
	const gchar *val;

	split_widgets = &(account_entry->split_widgets);
	split_widget = split_widgets->begin();
	widget = *split_widget;
	val = widget->GetValue();
	split_widget++;

	GString *username = g_string_new(val);
	prplinfo = PURPLE_PLUGIN_PROTOCOL_INFO(purple_find_prpl(purple_account_get_protocol_id(account)));

	for (iter = prplinfo->user_splits;
			iter && split_widget != split_widgets->end();
			iter = iter->next, split_widget++)
	{
		user_split = (PurpleAccountUserSplit*)iter->data;
		widget = *split_widget;

		val = widget->GetValue();
		if (val == NULL || *val == '\0')
			val = purple_account_user_split_get_default_value(user_split);
		g_string_append_printf(username, "%c%s",
				purple_account_user_split_get_separator(user_split), val);
	}

	purple_account_set_username(account, username->str);
	g_string_free(username, TRUE);
}

void AccountWindow::AccountOptionSplit::SetValue(const gchar *new_value)
{
	if (value)
		g_free(value);
	value = g_strdup(new_value);
	UpdateText();
}

void AccountWindow::AccountOptionSplit::OnActivate()
{
	WindowManager *wm = WindowManager::Instance();
	dialog = new InputDialog(text, value);
	dialog->signal_response.connect(
			sigc::mem_fun(this, &AccountWindow::AccountOptionSplit::ResponseHandler));
	//TODO add something to dialog class to show it. this removes the need
	//for including windowmanager.h, and is easier to use.
	wm->Add(dialog);
}

void AccountWindow::AccountOptionSplit::ResponseHandler(Dialog::ResponseType response)
{
	g_assert(dialog);

	switch (response) {
		case Dialog::ResponseOK:
			SetValue(dialog->GetText());
			UpdateSplits();
			break;
		default:
			break;
	}
	dialog = NULL;
}

AccountWindow::AccountOptionProtocol::AccountOptionProtocol(Widget& parent,
		PurpleAccount *account, AccountWindow &account_window)
: ComboBox(parent, 0, 0, "")
, account_window(&account_window)
, account(account)
{
	g_assert(account);

	GList *i;
	PurplePlugin *plugin;

	plugin = purple_plugins_find_with_id(purple_account_get_protocol_id(account));

	for (i = purple_plugins_get_protocols(); i; i = i->next)
		AddOption(((PurplePlugin*)i->data)->info->name, i->data);

	gchar *label = g_strdup_printf("Protocol: %s", plugin->info->name);
	SetText(label);
	g_free(label);

	signal_selection_changed.connect(
		sigc::mem_fun(this, &AccountWindow::AccountOptionProtocol::OnProtocolChanged));
}

void AccountWindow::AccountOptionProtocol::OnProtocolChanged(const ComboBox::ComboBoxEntry& new_entry)
{
	purple_account_set_protocol_id(account, purple_plugin_get_id((const PurplePlugin*)new_entry.data));
	// this deletes us so don't touch any instance variable after
	account_window->PopulateAccount(account);
}
