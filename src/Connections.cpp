// Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
// Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
//
// This file is part of CenterIM.
//
// CenterIM is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// CenterIM is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CenterIM.  If not, see <http://www.gnu.org/licenses/>.

#include "Connections.h"

#include "Log.h"

#include "gettext.h"
#include <cstring>

#define RECONNECTION_DELAY_MIN 60000
#define RECONNECTION_DELAY_MAX 120000

Connections *Connections::my_instance_ = nullptr;

Connections *Connections::instance()
{
  return my_instance_;
}

Connections::Connections()
{
  // Set the purple connection callbacks.
  std::memset(
    &centerim_connection_ui_ops, 0, sizeof(centerim_connection_ui_ops));
  centerim_connection_ui_ops.connect_progress = connect_progress_;
  centerim_connection_ui_ops.connected = connected_;
  centerim_connection_ui_ops.disconnected = disconnected_;
  centerim_connection_ui_ops.notice = notice_;
  // Deprecated in favour of report_disconnect_reason().
  // centerim_connection_ui_ops.report_disconnect = report_disconnect_;
  centerim_connection_ui_ops.network_connected = network_connected_;
  centerim_connection_ui_ops.network_disconnected = network_disconnected_;
  centerim_connection_ui_ops.report_disconnect_reason =
    report_disconnect_reason_;
  purple_connections_set_ui_ops(&centerim_connection_ui_ops);
}

Connections::~Connections()
{
  purple_connections_set_ui_ops(nullptr);
}

void Connections::init()
{
  g_assert(my_instance_ == nullptr);

  my_instance_ = new Connections;
}

void Connections::finalize()
{
  g_assert(my_instance_ != nullptr);

  delete my_instance_;
  my_instance_ = nullptr;
}

void Connections::reconnectAccount(PurpleAccount *account)
{
  g_return_if_fail(account != nullptr);

  if (!purple_account_is_disconnected(account) ||
    !purple_status_is_online(purple_account_get_active_status(account)))
    return;

  purple_account_connect(account);
}

void Connections::connect_progress(PurpleConnection *gc, const char *text,
  std::size_t /*step*/, std::size_t /*step_count*/)
{
  PurpleAccount *account = purple_connection_get_account(gc);
  LOG->message(_("+ [%s] %s: %s"), purple_account_get_protocol_name(account),
    purple_account_get_username(account), text);
}

void Connections::connected(PurpleConnection *gc)
{
  PurpleAccount *account = purple_connection_get_account(gc);
  LOG->message(_("+ [%s] %s: Connected"),
    purple_account_get_protocol_name(account),
    purple_account_get_username(account));
}

void Connections::disconnected(PurpleConnection *gc)
{
  PurpleAccount *account = purple_connection_get_account(gc);
  LOG->message(_("+ [%s] %s: Disconnected"),
    purple_account_get_protocol_name(account),
    purple_account_get_username(account));
}

void Connections::notice(PurpleConnection *gc, const char *text)
{
  PurpleAccount *account = purple_connection_get_account(gc);
  LOG->message(_("+ [%s] %s: %s"), purple_account_get_protocol_name(account),
    purple_account_get_username(account), text);
}

void Connections::network_connected()
{
  LOG->message(_("+ Network connected"));

  GList *list, *l;
  l = list = purple_accounts_get_all_active();
  while (l != nullptr) {
    PurpleAccount *account = reinterpret_cast<PurpleAccount *>(l->data);
    if (purple_account_is_disconnected(account))
      reconnectAccount(account);

    l = l->next;
  }
  g_list_free(list);
}

void Connections::network_disconnected()
{
  LOG->message(_("+ Network disconnected"));

  GList *list, *l;
  l = list = purple_accounts_get_all_active();
  while (l != nullptr) {
    PurpleAccount *a = reinterpret_cast<PurpleAccount *>(l->data);
    if (!purple_account_is_disconnected(a))
      purple_account_disconnect(a);

    l = l->next;
  }
  g_list_free(list);
}

void Connections::report_disconnect_reason(
  PurpleConnection *gc, PurpleConnectionError reason, const char *text)
{
  PurpleAccount *account = purple_connection_get_account(gc);
  const char *protocol = purple_account_get_protocol_name(account);
  const char *username = purple_account_get_username(account);

  LOG->message(_("+ [%s] %s: %s"), protocol, username, text);

  if (!purple_connection_error_is_fatal(reason)) {
    unsigned delay =
      g_random_int_range(RECONNECTION_DELAY_MIN, RECONNECTION_DELAY_MAX);
    CENTERIM->timeoutOnceConnect(
      sigc::bind(sigc::mem_fun(this, &Connections::reconnectAccount), account),
      delay);
    LOG->message(ngettext("+ [%s] %s: Auto-reconnection in %d second",
                   "+ [%s] %s: Auto-reconnection in %d seconds", delay / 1000),
      protocol, username, delay / 1000);
  }
  else {
    purple_account_set_enabled(account, PACKAGE_NAME, FALSE);
    LOG->message(_("+ [%s] %s: Account disabled"), protocol, username);
  }
}

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
