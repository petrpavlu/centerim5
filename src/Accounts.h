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

#ifndef __ACCOUNTS_H__
#define __ACCOUNTS_H__

#include <libpurple/account.h>

#define ACCOUNTS (Accounts::Instance())

class Accounts
{
public:
  static Accounts *Instance();

protected:

private:
  PurpleAccountUiOps centerim_account_ui_ops;

  Accounts();
  Accounts(const Accounts&);
  Accounts &operator=(const Accounts&);
  ~Accounts() {};

  static void status_changed_(PurpleAccount *account, PurpleStatus *status)
    { ACCOUNTS->status_changed(account, status); }

  void status_changed(PurpleAccount *account, PurpleStatus *status);
};

#endif // __ACCOUNTS_H__
