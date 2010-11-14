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

#include "Connections.h"

#include "Log.h"

#include <cstring>
#include "gettext.h"

Connections *Connections::Instance()
{
  static Connections instance;
  return &instance;
}

Connections::Connections()
{
  // set the purple connection callbacks
  memset(&centerim_connection_ui_ops, 0, sizeof(centerim_connection_ui_ops));
  centerim_connection_ui_ops.connect_progress = connect_progress_;
  centerim_connection_ui_ops.connected = connected_;
  centerim_connection_ui_ops.disconnected = disconnected_;
  centerim_connection_ui_ops.notice = notice_;
  // deprecated in favour of report_disconnect_reason()
  //centerim_connection_ui_ops.report_disconnect = report_disconnect_;
  centerim_connection_ui_ops.network_connected = network_connected_;
  centerim_connection_ui_ops.network_disconnected = network_disconnected_;
  centerim_connection_ui_ops.report_disconnect_reason = report_disconnect_reason_;
  purple_connections_set_ui_ops(&centerim_connection_ui_ops);
}

void Connections::connect_progress(PurpleConnection *gc, const char *text,
    size_t step, size_t step_count)
{
  PurpleAccount *account = purple_connection_get_account(gc);
  LOG->Message(_("+ [%s] %s: %s\n"),
      purple_account_get_protocol_name(account),
      purple_account_get_username(account), text);
}

void Connections::connected(PurpleConnection *gc)
{
  PurpleAccount *account = purple_connection_get_account(gc);
  LOG->Message(_("+ [%s] %s: Connected\n"),
      purple_account_get_protocol_name(account),
      purple_account_get_username(account));
}

void Connections::disconnected(PurpleConnection *gc)
{
  PurpleAccount *account = purple_connection_get_account(gc);
  LOG->Message(_("+ [%s] %s: Disconnected\n"),
      purple_account_get_protocol_name(account),
      purple_account_get_username(account));
}

void Connections::notice(PurpleConnection *gc, const char *text)
{
  PurpleAccount *account = purple_connection_get_account(gc);
  LOG->Message(_("+ [%s] %s: %s\n"),
      purple_account_get_protocol_name(account),
      purple_account_get_username(account), text);
}

void Connections::network_connected()
{
  LOG->Message(_("+ Network connected\n"));
}

void Connections::network_disconnected()
{
  LOG->Message(_("+ Network disconnected\n"));
}

void Connections::report_disconnect_reason(PurpleConnection *gc,
    PurpleConnectionError reason, const char *text)
{
  PurpleAccount *account = purple_connection_get_account(gc);
  LOG->Message(_("+ [%s] %s: %s\n"),
      purple_account_get_protocol_name(account),
      purple_account_get_username(account), text);
}
