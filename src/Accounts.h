/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2012 by CenterIM developers
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
 */

#ifndef __ACCOUNTS_H__
#define __ACCOUNTS_H__

#include <cppconsui/FreeWindow.h>
#include <libpurple/purple.h>

#define ACCOUNTS (Accounts::Instance())

class Accounts
{
public:
  static Accounts *Instance();

protected:

private:
  PurpleAccountUiOps centerim_account_ui_ops;

  struct AddUserData
  {
    PurpleAccount *account;
    char *username;
    char *alias;

    AddUserData(PurpleAccount *account_, const char *username_,
        const char *alias_);
    ~AddUserData();
  };

  struct AuthAndAdd
  {
    PurpleAccountRequestAuthorizationCb auth_cb;
    PurpleAccountRequestAuthorizationCb deny_cb;
    void *data;
    char *username;
    char *alias;
    PurpleAccount *account;

    AuthAndAdd(PurpleAccountRequestAuthorizationCb auth_cb_,
        PurpleAccountRequestAuthorizationCb deny_cb_, void *data_,
        const char *username_, const char *alias_, PurpleAccount *account_);
    ~AuthAndAdd();
  };

  static Accounts *instance;

  Accounts();
  Accounts(const Accounts&);
  Accounts& operator=(const Accounts&);
  ~Accounts();

  static void Init();
  static void Finalize();
  friend class CenterIM;

  char *MakeInfo(PurpleAccount *account, const char *remote_user,
      const char *id, const char *alias, const char *msg);

  static void notify_added_(PurpleAccount *account, const char *remote_user,
      const char *id, const char *alias, const char *message)
    { ACCOUNTS->notify_added(account, remote_user, id, alias, message); }
  static void status_changed_(PurpleAccount *account, PurpleStatus *status)
    { ACCOUNTS->status_changed(account, status); }
  static void request_add_(PurpleAccount *account, const char *remote_user,
      const char *id, const char *alias, const char *message)
    { ACCOUNTS->request_add(account, remote_user, id, alias, message); }
	static void *request_authorize_(PurpleAccount *account,
      const char *remote_user, const char *id, const char *alias,
      const char *message, gboolean on_list,
      PurpleAccountRequestAuthorizationCb authorize_cb,
      PurpleAccountRequestAuthorizationCb deny_cb, void *user_data)
    { return ACCOUNTS->request_authorize(account, remote_user, id, alias,
        message, on_list, authorize_cb, deny_cb, user_data); }
	static void close_account_request_(void *ui_handle)
    { ACCOUNTS->close_account_request(ui_handle); }

  void notify_added(PurpleAccount *account, const char *remote_user,
      const char *id, const char *alias, const char *message);
  void status_changed(PurpleAccount *account, PurpleStatus *status);
  void request_add(PurpleAccount *account, const char *remote_user,
      const char *id, const char *alias, const char *message);
  void *request_authorize(PurpleAccount *account, const char *remote_user,
      const char *id, const char *alias, const char *message,
      gboolean on_list, PurpleAccountRequestAuthorizationCb authorize_cb,
      PurpleAccountRequestAuthorizationCb deny_cb, void *user_data);
	void close_account_request(void *ui_handle);

  static void add_user_cb_(AddUserData *data)
    { ACCOUNTS->add_user_cb(data); }
  static void add_user_cancel_cb_(AddUserData *data)
    { ACCOUNTS->add_user_cancel_cb(data); }

  void add_user_cb(AddUserData *data);
  void add_user_cancel_cb(AddUserData *data);

  void OnRequestAuthorizeClose(CppConsUI::FreeWindow& win);

  static void authorize_and_add_cb_(AuthAndAdd *aa)
    { ACCOUNTS->authorize_and_add_cb(aa); }
  static void deny_no_add_cb_(AuthAndAdd *aa)
    { ACCOUNTS->deny_no_add_cb(aa); }

  void authorize_and_add_cb(AuthAndAdd *aa);
  void deny_no_add_cb(AuthAndAdd *aa);
};

#endif // __ACCOUNTS_H__

/* vim: set tabstop=2 shiftwidth=2 tw=78 expandtab : */
