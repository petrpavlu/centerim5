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

#include "AccountStatusMenu.h"
#include "CIMWindowManager.h"

#include <cppconsui/Label.h>
#include <cppconsui/HorizontalLine.h>
#include <cppconsui/LineStyle.h>
#include <cppconsui/WindowManager.h>

#include <libpurple/account.h>
#include <libpurple/savedstatuses.h>

AccountStatusMenu::AccountStatusMenu(int x, int y, int w, int h, LineStyle *linestyle)
: MenuWindow(x, y, w, h, linestyle)
{
	GList *iter;
	gchar *text;

	windowmanager = WindowManager::Instance();

	AddItem(_("All protocols"), sigc::mem_fun(this, &AccountStatusMenu::Dummy));
	AddItem(_("Already logged in only"), sigc::mem_fun(this, &AccountStatusMenu::Dummy));
	AddSeperator();

	for (iter = purple_accounts_get_all(); iter; iter = iter->next) {

		PurpleAccount *account = (PurpleAccount*)iter->data;

		text = g_strdup_printf("[%s] %s",
			purple_account_get_protocol_name(account),
			purple_account_get_username(account));

		AddItem(text, sigc::bind(sigc::mem_fun(this, &AccountStatusMenu::OpenStatusPopup), account));

		g_free(text);
	}
}

AccountStatusMenu::~AccountStatusMenu()
{
}

void AccountStatusMenu::OpenStatusPopup(PurpleAccount *account)
{
	AccountStatusMenu::StatusPopup *status_popup = new StatusPopup(x, y, w, h, LineStyle::LineStyleDefault(), account);
	windowmanager->Add(status_popup);
}

AccountStatusMenu::StatusPopup::StatusPopup(int x, int y, int w, int h, LineStyle *linestyle, PurpleAccount *account)
: MenuWindow(x, y, w, h, linestyle)
, account(account)
{
	GList *iter;
	PurpleStatusType *status_type;
	gchar *label;
	bool has_independents;
	bool active;

	has_independents = false;
	for (iter = purple_account_get_status_types(account); iter; iter = iter->next) {
		status_type = (PurpleStatusType*)iter->data;

		if (purple_status_type_is_independent(status_type)) {
			has_independents = true;
			continue;
		}

		active = purple_presence_is_status_active(
				purple_account_get_presence(account),
				purple_status_type_get_id(status_type));

		if (active) {
			label = g_strdup_printf("* %s", purple_status_type_get_name(status_type));
		} else {
			label = g_strdup(purple_status_type_get_name(status_type));
		}

		AddItem(label, sigc::bind(
					sigc::mem_fun(this, &AccountStatusMenu::StatusPopup::SetStatus),
					account,
					status_type,
					true));

		g_free(label);
	}

	if (!has_independents)
		return;

	AddSeperator();

	for (iter = purple_account_get_status_types(account); iter; iter = iter->next) {
		status_type = (PurpleStatusType*)iter->data;

		if (!purple_status_type_is_independent(status_type))
			continue;

		active = purple_presence_is_status_active(
				purple_account_get_presence(account),
				purple_status_type_get_id(status_type));

		if (active) {
			label = g_strdup_printf("* %s", purple_status_type_get_name(status_type));
		} else {
			label = g_strdup(purple_status_type_get_name(status_type));
		}

		AddItem(label, sigc::bind(
					sigc::mem_fun(this, &AccountStatusMenu::StatusPopup::SetStatus),
					account,
					status_type,
					!active));

		g_free(label);
	}



/*
	* Primitive (default) statuses (taken from gtksavedstatuses.c)*
	for (i = PURPLE_STATUS_UNSET + 1; i < PURPLE_STATUS_NUM_PRIMITIVES; i++) {
                if (i == PURPLE_STATUS_MOBILE || i == PURPLE_STATUS_TUNE)
                        *
                         * Special-case these.  They're intended to be independent
                         * status types, so don't show them in the list.
                         *
                        continue;
		PurpleStatusPrimitive primitive = (PurpleStatusPrimitive)i;

		AddItem(purple_primitive_get_name_from_type(primitive),
				sigc::bind(sigc::mem_fun(this, &AccountStatusMenu::StatusPopup::SetStatusPrimitive), account, primitive));
	}


	* Custom statuses *
	//TODO only use `popular' statuses here (like pidgin and finch do)?
	for (iter = purple_savedstatuses_get_all(); iter; iter = iter->next) {
		PurpleSavedStatus *status = (PurpleSavedStatus*)iter->data;

		AddItem(purple_savedstatus_get_title(status),
				sigc::bind(sigc::mem_fun(this, &AccountStatusMenu::StatusPopup::SetSavedStatus), account, status));
	}
*/
}

AccountStatusMenu::StatusPopup::~StatusPopup()
{
}

void AccountStatusMenu::StatusPopup::SetStatus(PurpleAccount *account, PurpleStatusType *status_type, bool active)
{
	Accounts *accounts = Accounts::Instance();
	accounts->SetStatus(account, status_type, active);
	Close();
}
/*
void AccountStatusMenu::StatusPopup::SetStatus(PurpleAccount *account, PurpleSavedStatus *status)
{
	accounts->SetStatus(account, status);
	Close();
}

void AccountStatusMenu::StatusPopup::SetStatusPrimitive(PurpleAccount *account, PurpleStatusPrimitive primitive)
{
	Accounts *accounts = Accounts::Instance();
	accounts->SetStatus(account, primitive);
	Close();
}*/

void AccountStatusMenu::ScreenResized()
{
	Rect chat = (CIMWindowManager::Instance())->ScreenAreaSize(CIMWindowManager::Chat);
	Move(chat.x, chat.y);
}
