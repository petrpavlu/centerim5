/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2011 by CenterIM developers
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

#include <string.h>
#include "gettext.h"

Accounts *Accounts::instance = NULL;

Accounts *Accounts::Instance()
{
  return instance;
}

Accounts::AddUserData::AddUserData(PurpleAccount *account_,
    const char *username_, const char *alias_)
: account(account_)
{
  username = g_strdup(username_);
  alias = g_strdup(alias_);
}

Accounts::AddUserData::~AddUserData()
{
  if (username)
    g_free(username);
  if (alias)
    g_free(alias);
}

Accounts::AuthAndAdd::AuthAndAdd(PurpleAccountRequestAuthorizationCb auth_cb_,
    PurpleAccountRequestAuthorizationCb deny_cb_, void *data_,
    const char *username_, const char *alias_, PurpleAccount *account_)
: auth_cb(auth_cb_), deny_cb(deny_cb_), data(data_), account(account_)
{
  username = g_strdup(username_);
  alias = g_strdup(alias_);
}

Accounts::AuthAndAdd::~AuthAndAdd()
{
  if (username)
    g_free(username);
  if (alias)
    g_free(alias);
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
  centerim_account_ui_ops.notify_added = notify_added_;
  centerim_account_ui_ops.status_changed = status_changed_;
  centerim_account_ui_ops.request_add = request_add_;
  centerim_account_ui_ops.request_authorize = request_authorize_;
  centerim_account_ui_ops.close_account_request = close_account_request_;
  purple_accounts_set_ui_ops(&centerim_account_ui_ops);
}

Accounts::~Accounts()
{
  purple_accounts_set_ui_ops(NULL);
}

void Accounts::Init()
{
  g_assert(!instance);

  instance = new Accounts;
}

void Accounts::Finalize()
{
  g_assert(instance);

  delete instance;
  instance = NULL;
}

char *Accounts::MakeInfo(PurpleAccount *account, const char *remote_user,
    const char *id, const char *alias, const char *msg)
{
  if (msg && !*msg)
    msg = NULL;

  char *ins = NULL;
  if (alias)
    ins = g_strdup_printf(" (%s)", alias);

  const char *name = id;
  if (!name)
    name = purple_account_get_name_for_display(account);

  char *res = g_strdup_printf(_("%s%s has made %s his or her buddy%s%s"),
      remote_user, ins ? ins : "",  name , msg ? ":\n" : ".",
      msg ? msg  : "");
  if (ins)
    g_free(ins);
  return res;
}

void Accounts::notify_added(PurpleAccount *account, const char *remote_user,
    const char *id, const char *alias, const char *message)
{
  char *buf = MakeInfo(account, remote_user, id, alias, message);
  LOG->Message("%s", buf);
  g_free(buf);
}

void Accounts::status_changed(PurpleAccount *account, PurpleStatus *status)
{
  if (!purple_account_get_enabled(account, PACKAGE_NAME))
    return;

  LOG->Message(_("+ [%s] %s: Status changed to: %s"),
      purple_account_get_protocol_name(account),
      purple_account_get_username(account),
      purple_status_get_name(status));
}

void Accounts::request_add(PurpleAccount *account, const char *remote_user,
    const char *id, const char *alias, const char *message)
{
  AddUserData *data = new AddUserData(account, remote_user, alias);
  char *buf = MakeInfo(account, remote_user, id, alias, message);
  purple_request_action(NULL, NULL, _("Add buddy to your list?"), buf,
      PURPLE_DEFAULT_ACTION_NONE, account, remote_user, NULL, data, 2,
      _("Add"), G_CALLBACK(add_user_cb_), _("Cancel"),
      G_CALLBACK(add_user_cancel_cb_));
  g_free(buf);
}

void *Accounts::request_authorize(PurpleAccount *account,
    const char *remote_user, const char *id, const char *alias,
    const char *message, gboolean on_list,
    PurpleAccountRequestAuthorizationCb authorize_cb,
    PurpleAccountRequestAuthorizationCb deny_cb,
    void *user_data)
{
  if (message && !*message)
    message = NULL;

  char *ins = NULL;
  if (alias)
    ins = g_strdup_printf(" (%s)", alias);

  const char *name = id;
  if (!name)
    name = purple_account_get_name_for_display(account);

  char *buf = g_strdup_printf(
      _("%s%s wants to add you (%s) to his or her buddy list%s%s"),
      remote_user, ins ? ins : "" , name, message ? ":\n" : ".",
      message ? message  : "");

  if (ins)
    g_free(ins);

  void *ui_handle;
  /* TODO Provide more information about the user that requested the
   * authorization. */
  if (on_list)
    ui_handle = purple_request_action(NULL, _("Authorize buddy?"), buf, NULL,
        PURPLE_DEFAULT_ACTION_NONE, account, remote_user, NULL, user_data, 2,
        _("Authorize"), authorize_cb, _("Deny"), deny_cb);
  else {
    AuthAndAdd *aa = new AuthAndAdd(authorize_cb, deny_cb, user_data,
        remote_user, alias, account);

    ui_handle = purple_request_action(NULL, _("Authorize buddy?"), buf, NULL,
        PURPLE_DEFAULT_ACTION_NONE, account, remote_user, NULL, aa, 2,
        _("Authorize"), authorize_and_add_cb_, _("Deny"), deny_no_add_cb_);
  }
  g_free(buf);

  // this is ugly
  CppConsUI::FreeWindow *win
    = reinterpret_cast<CppConsUI::FreeWindow*>(ui_handle);
  win->signal_close.connect(sigc::mem_fun(this,
        &Accounts::OnRequestAuthorizeClose));

  return ui_handle;
}

void Accounts::close_account_request(void *ui_handle)
{
}

void Accounts::add_user_cb(AddUserData *data)
{
  purple_blist_request_add_buddy(data->account, data->username, NULL,
      data->alias);
  delete data;
}

void Accounts::add_user_cancel_cb(AddUserData *data)
{
  delete data;
}

void Accounts::OnRequestAuthorizeClose(CppConsUI::FreeWindow& win)
{
  purple_account_request_close(&win);
}

void Accounts::authorize_and_add_cb(AuthAndAdd *aa)
{
  aa->auth_cb(aa->data);
  purple_blist_request_add_buddy(aa->account, aa->username, NULL, aa->alias);
  delete aa;
}

void Accounts::deny_no_add_cb(AuthAndAdd *aa)
{
  aa->deny_cb(aa->data);
  delete aa;
}

/* vim: set tabstop=2 shiftwidth=2 expandtab : */
