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

#define CONF_PLUGIN_SAVE_PREF	"/centerim/plugins/loaded"

#define EXCEPTION_NONE			0
#define EXCEPTION_PURPLE_CORE_INIT	100

#include "Accounts.h"

#include "Log.h"

#include <libpurple/account.h>
#include <libpurple/savedstatuses.h>

Accounts* Accounts::instance = NULL;

Accounts* Accounts::Instance(void)
{
	if (!instance) instance = new Accounts();
	return instance;
}

void Accounts::Delete(void)
{
	if (instance) {
		delete instance;
		instance = NULL;
	}
}

static PurpleAccountUiOps centerim_accounts_ui_ops =
{
	NULL, //TODO BuddyList::notify_added_,
	NULL, //Accounts::status_changed_,
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
	log = Log::Instance();
	accounts_handle = purple_accounts_get_handle();

	/* connect signal handlers */
	purple_signal_connect(purple_connections_get_handle(), "signed-on", this,
		PURPLE_CALLBACK(signed_on_), this);

	/* setup the callbacks for the buddylist */
	purple_accounts_set_ui_ops(&centerim_accounts_ui_ops);

	/* if the statuses are not known, set them all
	 * to the default */
	if (!purple_prefs_get_bool("/purple/savedstatus/startup_current_status"))
		purple_savedstatus_activate(purple_savedstatus_get_startup());
	
	/* restore last know status on all accounts. */
	purple_accounts_restore_current_statuses();
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
	log->Write(Log::Type_cim, Log::Level_info, "+ Account connected: %s %s\n", account->username, account->protocol_id);
}
