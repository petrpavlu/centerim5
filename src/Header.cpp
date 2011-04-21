/*
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

#include "Header.h"

#include "CenterIM.h"
#include "Utils.h"
#include "config.h"

Header *Header::instance = NULL;

Header *Header::Instance()
{
  return instance;
}

void Header::ScreenResized()
{
  MoveResizeRect(CENTERIM->GetScreenAreaSize(CenterIM::HEADER_AREA));
}

Header::Header()
: FreeWindow(0, 0, 80, 1, TYPE_NON_FOCUSABLE)
{
  SetColorScheme("header");

  container = new HorizontalListBox(AUTOSIZE, AUTOSIZE);
  AddWidget(*container, 0, 0);

  // CenterIM name will always be top-left so we can "forget about" it
  char *cimname = g_strdup_printf("CENTERIM %s", PACKAGE_VERSION);
  container->AppendWidget(*(new Label(cimname)));
  g_free(cimname);

  void *handle = purple_accounts_get_handle();
  purple_signal_connect(handle, "account-signed-on", this,
      PURPLE_CALLBACK(account_signed_on_), this);
  purple_signal_connect(handle, "account-signed-off", this,
      PURPLE_CALLBACK(account_signed_off_), this);
  purple_signal_connect(handle, "account-status-changed", this,
      PURPLE_CALLBACK(account_status_changed_), this);
  purple_signal_connect(handle, "account-alias-changed", this,
      PURPLE_CALLBACK(account_alias_changed_), this);
}

Header::~Header()
{
  purple_signals_disconnect_by_handle(this);
}

void Header::Init()
{
  g_assert(!instance);

  instance = new Header;
  instance->Show();
}

void Header::Finalize()
{
  g_assert(instance);

  delete instance;
  instance = NULL;
}

void Header::account_signed_on(PurpleAccount *account)
{
  g_return_if_fail(account);

  if (statuses.find(account) == statuses.end())
    return;

  Label *label = new Label(0, 1, "");
  statuses[account] = label;
  container->AppendWidget(*label);

  account_status_changed(account, NULL,
      purple_account_get_active_status(account));
}

void Header::account_signed_off(PurpleAccount *account)
{
  g_return_if_fail(account);

  if (statuses.find(account) != statuses.end())
    return;

  container->RemoveWidget(*statuses[account]);
  statuses.erase(account);
}

void Header::account_status_changed(PurpleAccount *account, PurpleStatus *old,
    PurpleStatus *cur)
{
  g_return_if_fail(account);
  g_return_if_fail(cur);

  if (statuses.find(account) != statuses.end())
    return;

  // update label
  Label *label = statuses[account];
  char *text = g_strdup_printf(" %s [%s] %s", Utils::GetStatusIndicator(cur),
      purple_account_get_protocol_name(account),
      purple_account_get_username(account));
  label->SetText(text);
  label->SetWidth(Curses::onscreen_width(text));
  g_free(text);
}

void Header::account_alias_changed(PurpleAccount *account, const char *old)
{
  g_return_if_fail(account);

  account_status_changed(account, NULL,
      purple_account_get_active_status(account));
}
