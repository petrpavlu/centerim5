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

#include "Accounts.h"

#include "Log.h"

#include "gettext.h"

#define CONF_PLUGIN_SAVE_PREF	"/centerim/plugins/loaded"

Accounts *Accounts::Instance()
{
	static Accounts instance;
	return &instance;
}

//TODO move inside accounts class and make callbacks private
static PurpleAccountUiOps centerim_accounts_ui_ops =
{
	NULL, //TODO BuddyList::notify_added_,
	Accounts::status_changed_,
	NULL, //BuddyList::request_add_,
	NULL, //BuddyList::request_authorize_,
	NULL, //BuddyList::close_account_request_,
	NULL, /* reserverd */
	NULL,
	NULL,
	NULL
};

Accounts::Accounts()
{
	accounts_handle = purple_accounts_get_handle();

	/* connect signal handlers */
	purple_signal_connect(purple_connections_get_handle(), "signed-on", this,
		PURPLE_CALLBACK(signed_on_), this);

	/*TODO always set all accounts enabled
	GList *i;
	PurpleAccount *account;

	for (i = purple_accounts_get_all(); i; i = i->next) {
		account = (PurpleAccount*)i->data;
		purple_account_set_enabled(account, "centerim" , true);
	}*/

	/* if the statuses are not known, set them all
	 * to the default */
	if (!purple_prefs_get_bool("/purple/savedstatus/startup_current_status"))
		purple_savedstatus_activate(purple_savedstatus_get_startup());
	
	/* restore last know status on all accounts. */
	purple_accounts_restore_current_statuses();

	/* setup the callbacks for the buddylist */
	purple_accounts_set_ui_ops(&centerim_accounts_ui_ops);
}

Accounts::~Accounts()
{
	accounts_handle = NULL;
}

void Accounts::signed_on_(PurpleConnection *gc, gpointer p)
{
	Accounts *accounts = (Accounts*)p;
	accounts->signed_on(gc);
}

void Accounts::signed_on(PurpleConnection *gc)
{
	PurpleAccount *account = purple_connection_get_account(gc);
	LOG->Write(Log::Level_info, _("+ [%s] Logged in: %s\n"),
			account->protocol_id, account->username);
}

void Accounts::status_changed(PurpleAccount *account, PurpleStatus *status)
{
	LOG->Write(Log::Level_info, _("+ [%s] Status changed to: %s\n"),
			account->protocol_id, purple_status_get_name(status));
}

void Accounts::SetStatus(PurpleAccount *account, PurpleStatusType *status_type, bool active)
{
	purple_account_set_status(account,
			purple_status_type_get_id(status_type),
			active, NULL);
}
