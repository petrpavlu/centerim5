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
#include <cppconsui/MessageDialog.h>
#include <cppconsui/WindowManager.h>

#include "CIMWindowManager.h"

#include <libpurple/account.h>
#include <libpurple/accountopt.h>

#include <cstring>
#include <errno.h>

#include <glib.h>
#include <glibmm/main.h>

AccountWindow::AccountWindow()
: Window(0, 0, 80, 24, NULL)
, menu_index(1)
, accounts_index(1)
{
	//const gchar *context = "accountwindow";
	log = Log::Instance();
	conf = Conf::Instance();

	//TODO get linestyle from conf
	border = new Panel(*this, 0, 0, w, h, LineStyle::LineStyleDefault());
	AddWidget(border);

	accounts = new TreeView(*this, 1, 1, w-2, h-4, LineStyle::LineStyleDefault());
	menu = new HorizontalListBox(*this, 1, h-2, w-2, 1);
	line = new HorizontalLine(*this, 1, h-3, w-2);

	accounts->FocusCycle(Container::FocusCycleLocal);
	menu->FocusCycle(Container::FocusCycleLocal);

	menu->AddItem(_("Add"), sigc::mem_fun(this, &AccountWindow::Add));
	menu->AddItem(_("Done"), sigc::mem_fun(this, &AccountWindow::Close));

	AddWidget(accounts);
	AddWidget(menu);
	AddWidget(line);

	MoveResize(conf->GetAccountWindowDimensions());
	
	Populate();

	menu->GrabFocus();
}

void AccountWindow::Resize(int neww, int newh)
{
	Window::Resize(neww, newh);

	border->Resize(w, h);
	accounts->Resize(w-2, h-4);
	menu->MoveResize(1, h-2, w-2, 1);
	line->MoveResize(1, h-3, w-2, 1);
}

void AccountWindow::ScreenResized()
{
	Rect screen = (CIMWindowManager::Instance())->ScreenAreaSize(CIMWindowManager::Screen);
	Rect confSize = conf->GetAccountWindowDimensions();

	// Check against screen size
	if (screen.Width() < (confSize.Width()+4))
		confSize.width = screen.Width()-4;

	if (screen.Height() < (confSize.Height()+4))
		confSize.height = screen.Height()-4;

	// Center on screen
	confSize.x = (screen.Width() - confSize.Width()) / 2;
	confSize.y = (screen.Height() - confSize.Height()) / 2;
	
	MoveResize(confSize);
}

AccountWindow::~AccountWindow()
{
}

void AccountWindow::Add(void)
{
	GList *i;
	PurpleAccount *account;

        i = purple_plugins_get_protocols();
	account = purple_account_new(_("Username"),
			purple_plugin_get_id((PurplePlugin*)i->data));
	purple_accounts_add(account);

	PopulateAccount(account);
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
			Glib::signal_timeout().connect(
					sigc::bind(sigc::mem_fun(this,
					&AccountWindow::ClearAccount), 
					account, true), 0);
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

void AccountWindow::Clear(void)
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

	/* Move focus */
	account_entry->parent->GrabFocus();

        /* First remove the nodes from the tree, then
         * free() all the widgets. */
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

void AccountWindow::Populate(void)
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
		/* No entry for this account, so add one. */
		AccountEntry entry;
		entry.parent = NULL;
		account_entries[account] = entry;
	} else {
		/* The account exists, so clear all data. */
		ClearAccount(account, false);
	}

	account_entry = &(account_entries[account]);

	if (!account_entry->parent) {
		Button *button;
		TreeView::NodeReference parent_reference;

		//TODO proper autosizing for labels
		button = new Button(*accounts, 0, 0, width(label), 1, NULL,
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

		/* we cannot change the settings of an unknown account */
		label = new Label(*accounts, 0, 0, _("This account was configured, but the protocol plugin was not loaded"));
		accounts->AddNode(account_entry->parent_reference, label, account);
		account_entry->widgets.push_back(label);

	} else {

		prplinfo = PURPLE_PLUGIN_PROTOCOL_INFO(prpl);
	
		/* Protocols combobox */
		combobox = new AccountOptionProtocol(*accounts, account, this);
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

			/* Create widget for the username split and remember */
			widget_split = new AccountOptionSplit(*accounts, account, split, account_entry);
			widget_split->SetValue(value);
			widget_split->UpdateText();
			account_entry->split_widgets.push_front(widget_split);
			account_entry->widgets.push_back(widget_split);

			accounts->AddNode(account_entry->parent_reference, widget_split, NULL);
		}


		//TODO add this widget as the first widget in this subtree. Treeview needs to support this.
		widget_split = new AccountOptionSplit(*accounts, account, NULL, account_entry);
		widget_split->SetValue(username);
		widget_split->UpdateText();
		account_entry->split_widgets.push_front(widget_split);
		account_entry->widgets.push_back(widget_split);
		accounts->AddNode(account_entry->parent_reference, widget_split, NULL);
		g_free(username);

		/* Password */
		widget = new AccountOptionString(*accounts, account, true, false);
		accounts->AddNode(account_entry->parent_reference, widget, NULL);
		account_entry->widgets.push_back(widget);

		/* Remember Password */
		widget = new AccountOptionBool(*accounts, account, true);
		accounts->AddNode(account_entry->parent_reference, widget, NULL);
		account_entry->widgets.push_back(widget);

		/* Alias */
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

	}

	/* Drop account */
	widget = new Button(*accounts, 0, 0, _("Drop account"),
			sigc::bind(sigc::mem_fun(this, &AccountWindow::DropAccount), account));
	accounts->AddNode(account_entry->parent_reference, widget, NULL);
	account_entry->widgets.push_back(widget);
}

AccountWindow::AccountOption::AccountOption(Widget& parent,
	PurpleAccount *account, PurpleAccountOption *option)
: Button(parent, x, y, "")
, account(account)
, option(option)
{
	g_assert(account != NULL);

	if (option) {
		setting = purple_account_option_get_setting(option);
		text = purple_account_option_get_text(option);
	} else {
		setting = NULL;
	}

	SetFunction(sigc::mem_fun(this, 
		&AccountWindow::AccountOption::OnActivate));
}

AccountWindow::AccountOption::~AccountOption()
{
}

AccountWindow::AccountOptionBool::AccountOptionBool(Widget& parent,
	PurpleAccount *account, PurpleAccountOption *option)
: AccountWindow::AccountOption::AccountOption(parent, account, option)
, remember_password(false)
{
	UpdateText();
}

AccountWindow::AccountOptionBool::AccountOptionBool(Widget& parent,
	PurpleAccount *account, bool remember_password)
: AccountWindow::AccountOption::AccountOption(parent, account, NULL)
, remember_password(false)
{
	if (remember_password) {
		this->remember_password = remember_password;
		text = _("Remember password");
	}

	UpdateText();
}

AccountWindow::AccountOptionBool::~AccountOptionBool()
{
}

void AccountWindow::AccountOptionBool::UpdateText(void)
{
	gchar *str;
	
	if (remember_password) {
		value = purple_account_get_remember_password(account);
	} else {
		value = purple_account_get_bool(account, setting, 
				purple_account_option_get_default_bool(option));
	}

	//TODO create some DEFAULT_TEXT_YES etc, also for use in dialogs
	str = g_strdup_printf("%s: %s", text, value ? _("yes") : _("no"));
	SetText(str);
	g_free(str);
}


void AccountWindow::AccountOptionBool::OnActivate(void)
{
	if (remember_password) {
		purple_account_set_remember_password(account, !value);
	} else {
		purple_account_set_bool(account, setting, !value);
	}
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
, password(false)
, alias(false)
{
	if (password) {
		this->password = true;
		text = _("Password");
	} else if (alias) {
		this->alias = true;
		text = _("Alias");
	}

	UpdateText();
}

AccountWindow::AccountOptionString::~AccountOptionString()
{
}

void AccountWindow::AccountOptionString::UpdateText(void)
{
	gchar *str;
	
	if (password) {
		value = purple_account_get_password(account);
	} else if (alias) {
		value = purple_account_get_alias(account);
	} else {
		value = purple_account_get_string(account, setting, 
				purple_account_option_get_default_string(option));
	}

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

			if (password) {
				purple_account_set_password(account, dialog->GetText());
			} else if (alias) {
				purple_account_set_alias(account, dialog->GetText());
			} else {
				purple_account_set_string(account, setting, dialog->GetText());
			}

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

AccountWindow::AccountOptionProtocol::AccountOptionProtocol(Widget& parent,
        PurpleAccount *account, AccountWindow *account_window)
: ComboBox(parent, 0, 0, 15, 1, "") 
, account_window(account_window)
, account(account)
{
        g_assert(account != NULL);

        gchar *label;
        GList *i; 
        PurplePlugin *plugin;

        plugin = purple_plugins_find_with_id(purple_account_get_protocol_id(account));

        for (i = purple_plugins_get_protocols(); i; i = i->next){
                AddOption(((PurplePlugin*)i->data)->info->name, i->data);
        }   

        label = g_strdup_printf("Protocol: %s", plugin->info->name);
        SetText(label);
        g_free(label);

	signal_selection_changed.connect(
			sigc::mem_fun(this, &AccountWindow::AccountOptionProtocol::OnProtocolChanged));
}

AccountWindow::AccountOptionProtocol::~AccountOptionProtocol()
{
        //if (value)
        //      g_free(value);
}

void AccountWindow::AccountOptionProtocol::OnProtocolChanged(const ComboBox *combobox,
		ComboBox::ComboBoxEntry const old_entry, ComboBox::ComboBoxEntry const new_entry)
{
	Glib::signal_timeout().connect(
			sigc::bind(sigc::mem_fun(this,
			&AccountWindow::AccountOptionProtocol::OnProtocolChangedCallback), 
			new_entry), 0);
}

bool AccountWindow::AccountOptionProtocol::OnProtocolChangedCallback(const ComboBox::ComboBoxEntry entry)
{
	purple_account_set_protocol_id(account, purple_plugin_get_id((const PurplePlugin*)entry.data));
	account_window->PopulateAccount(account);
	return false;
}
