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

#include <cstring>

#include <libpurple/account.h>
#include <libpurple/accountopt.h>

AccountWindow::AccountWindow()
: Window(0, 0, 80, 24, NULL)
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
	menu->AddItem(_("Add"), sigc::mem_fun(this, &AccountWindow::Add));
	menu->AddItem(_("Delete"), sigc::mem_fun(this, &AccountWindow::Delete));
	menu->AddItem(_("Change"), sigc::mem_fun(this, &AccountWindow::Change));
	menu->AddItem(_("Done"), sigc::mem_fun(this, &AccountWindow::Close));

	AddWidget(accounts);
	AddWidget(menu);
	AddWidget(line);

	Populate();

	SetInputChild(accounts);

	can_focus = true;

	//DeclareBindable(context, "focus-previous-button", sigc::mem_fun(this, &AccountWindow::FocusCyclePrevious),
	//	_("Focusses the previous button"), InputProcessor::Bindable_Override);
	//DeclareBindable(context, "focus-next-button", sigc::mem_fun(this, &AccountWindow::FocusCycleNext),
	//	_("Focusses the next button"), InputProcessor::Bindable_Override);

	//BindAction(context, "focus-previous-button", "a", false);
	//BindAction(context, "focus-next-button", "s", false);
	//BindAction(context, "focus-previous-button", Keys::Instance()->Key_left(), false);
	//BindAction(context, "focus-next-button", Keys::Instance()->Key_right(), false);

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

void AccountWindow::FocusCyclePrevious(void)
{
	if (focus_child == accounts) {
		Window::FocusCyclePrevious();
	} else {
		menu->FocusCyclePrevious();
	}
}

void AccountWindow::FocusCycleNext(void)
{
	if (focus_child == accounts) {
		Window::FocusCycleNext();
	} else {
		menu->FocusCyclePrevious();
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

			char *value;
			gboolean bvalue;

			switch (type) {
			case PURPLE_PREF_STRING:
				value = g_strdup_printf("%s: %s", setting,
						purple_account_get_string(account, setting, NULL));
				accounts->AddNode(parent, new Button(*accounts, 0, 0, value), NULL);
				g_free(value);
				break;
			case PURPLE_PREF_INT:
				value = g_strdup_printf("%s: %d", setting,
						purple_account_get_int(account, setting, 0));
				accounts->AddNode(parent, new Button(*accounts, 0, 0, value), NULL);
				g_free(value);
				break;
			case PURPLE_PREF_BOOLEAN:
				bvalue = purple_account_get_bool(account, setting, false);
				value = g_strdup_printf("%s: %s", setting,
					bvalue ? _("yes") : _("no"));
				accounts->AddNode(parent, new Button(*accounts, 0, 0, value), NULL);
				g_free(value);
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
