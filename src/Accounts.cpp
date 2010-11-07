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

#include <libpurple/savedstatuses.h>
#include <cstring>
#include "gettext.h"

Accounts *Accounts::Instance()
{
  static Accounts instance;
  return &instance;
}

Accounts::Accounts()
{
  // if the statuses are not known, set them all to the default
  if (!purple_prefs_get_bool("/purple/savedstatus/startup_current_status"))
    purple_savedstatus_activate(purple_savedstatus_get_startup());

  // restore last know status on all accounts
  purple_accounts_restore_current_statuses();

  // set the purple account callbacks
  memset(&centerim_account_ui_ops, 0, sizeof(centerim_account_ui_ops));
  //centerim_account_ui_ops.notify_added = notify_added_;
  centerim_account_ui_ops.status_changed = status_changed_;
  //centerim_account_ui_ops.request_add = request_add_;
  //centerim_account_ui_ops.request_authorize = request_authorize_;
  //centerim_account_ui_ops.close_account_request = close_account_request_;
  purple_accounts_set_ui_ops(&centerim_account_ui_ops);
}

void Accounts::status_changed(PurpleAccount *account, PurpleStatus *status)
{
  LOG->Write(Log::LEVEL_MESSAGE, _("+ [%s] %s: Status changed to: %s\n"),
      purple_account_get_protocol_name(account),
      purple_account_get_username(account),
      purple_status_get_name(status));
}
