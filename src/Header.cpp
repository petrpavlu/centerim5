/*
 * Copyright (C) 2010-2013 by CenterIM developers
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

#include "Header.h"

#include "CenterIM.h"
#include "Utils.h"

#include <string.h> // strcmp
#include "config.h"

Header *Header::my_instance = NULL;

Header *Header::instance()
{
  return my_instance;
}

void Header::onScreenResized()
{
  moveResizeRect(CENTERIM->getScreenArea(CenterIM::HEADER_AREA));
}

Header::Header()
: Window(0, 0, 80, 1, TYPE_NON_FOCUSABLE, false)
{
  setColorScheme("header");

  container = new CppConsUI::HorizontalListBox(AUTOSIZE, AUTOSIZE);
  addWidget(*container, 0, 0);

  // CenterIM name will always be top-left so we can "forget about" it
  char *cimname = g_strdup_printf("CENTERIM %s", CenterIM::version);
  container->appendWidget(*(new CppConsUI::Label(cimname)));
  g_free(cimname);

  // pending request indicator
  request_indicator = new CppConsUI::Label(0, 1, "");
  request_indicator->setColorScheme("header-request");
  container->appendWidget(*request_indicator);

  for (GList *i = purple_accounts_get_all(); i; i = i->next) {
    PurpleAccount *account = reinterpret_cast<PurpleAccount*>(i->data);
    if (purple_account_get_enabled(account, PACKAGE_NAME))
      protocol_count.insert(purple_account_get_protocol_id(account));
  }

  // connect to the Accounts singleton
  g_assert(ACCOUNTS);
  ACCOUNTS->signal_request_count_change.connect(sigc::mem_fun(this,
        &Header::onRequestCountChange));

  void *handle = purple_accounts_get_handle();
  purple_signal_connect(handle, "account-signed-on", this,
      PURPLE_CALLBACK(account_signed_on_), this);
  purple_signal_connect(handle, "account-signed-off", this,
      PURPLE_CALLBACK(account_signed_off_), this);
  purple_signal_connect(handle, "account-status-changed", this,
      PURPLE_CALLBACK(account_status_changed_), this);
  purple_signal_connect(handle, "account-alias-changed", this,
      PURPLE_CALLBACK(account_alias_changed_), this);
  purple_signal_connect(handle, "account-enabled", this,
      PURPLE_CALLBACK(account_enabled_), this);
  purple_signal_connect(handle, "account-disabled", this,
      PURPLE_CALLBACK(account_disabled_), this);

  onScreenResized();
}

Header::~Header()
{
  purple_signals_disconnect_by_handle(this);
}

void Header::init()
{
  g_assert(!my_instance);

  my_instance = new Header;
  my_instance->show();
}

void Header::finalize()
{
  g_assert(my_instance);

  delete my_instance;
  my_instance = NULL;
}

void Header::onRequestCountChange(Accounts& /*accounts*/,
    size_t request_count)
{
  if (request_count) {
    request_indicator->setText("* ");
    request_indicator->setWidth(2);
  }
  else {
    request_indicator->setText("");
    request_indicator->setWidth(0);
  }
}

void Header::account_signed_on(PurpleAccount *account)
{
  g_return_if_fail(account);

  if (statuses.find(account) != statuses.end())
    return;

  CppConsUI::Label *label = new CppConsUI::Label(0, 1, "");
  statuses[account] = label;
  container->appendWidget(*label);

  account_status_changed(account, NULL,
      purple_account_get_active_status(account));
}

void Header::account_signed_off(PurpleAccount *account)
{
  g_return_if_fail(account);

  if (statuses.find(account) == statuses.end())
    return;

  container->removeWidget(*statuses[account]);
  statuses.erase(account);
}

void Header::account_status_changed(PurpleAccount *account,
    PurpleStatus * /*old*/, PurpleStatus *cur)
{
  g_return_if_fail(account);
  g_return_if_fail(cur);

  if (statuses.find(account) == statuses.end())
    return;

  const char *prid = purple_account_get_protocol_id(account);
  bool short_text = protocol_count.count(prid) == 1;

  // update label
  CppConsUI::Label *label = statuses[account];
  char *text;
  if (short_text)
    text = g_strdup_printf(" %s [%s]", Utils::getStatusIndicator(cur),
        purple_account_get_protocol_name(account));
  else
    text = g_strdup_printf(" %s [%s|%s]", Utils::getStatusIndicator(cur),
        purple_account_get_protocol_name(account),
        purple_account_get_username(account));
  label->setText(text);
  label->setWidth(CppConsUI::Curses::onScreenWidth(text));
  g_free(text);
}

void Header::account_alias_changed(PurpleAccount *account,
    const char * /*old*/)
{
  g_return_if_fail(account);

  account_status_changed(account, NULL,
      purple_account_get_active_status(account));
}

void Header::account_enabled(PurpleAccount *account)
{
  g_return_if_fail(account);

  const char *prid = purple_account_get_protocol_id(account);
  protocol_count.insert(prid);

  if (protocol_count.count(prid) == 2)
    for (Statuses::iterator i = statuses.begin(); i != statuses.end(); i++) {
      PurpleAccount *acc = i->first;
      if (!strcmp(purple_account_get_protocol_id(acc), prid))
        account_status_changed(acc, NULL,
            purple_account_get_active_status(acc));
    }
}

void Header::account_disabled(PurpleAccount *account)
{
  g_return_if_fail(account);

  const char *prid = purple_account_get_protocol_id(account);
  ProtocolCount::iterator i = protocol_count.find(prid);
  protocol_count.erase(i);

  if (protocol_count.count(prid) == 1)
    for (Statuses::iterator i = statuses.begin(); i != statuses.end(); i++) {
      PurpleAccount *acc = i->first;
      if (!strcmp(purple_account_get_protocol_id(acc), prid))
        account_status_changed(acc, NULL,
            purple_account_get_active_status(acc));
    }
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
