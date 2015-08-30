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
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "Header.h"

#include "CenterIM.h"
#include "Utils.h"

#include <cstring>
#include "config.h"

Header *Header::my_instance_ = NULL;

Header *Header::instance()
{
  return my_instance_;
}

void Header::onScreenResized()
{
  moveResizeRect(CENTERIM->getScreenArea(CenterIM::HEADER_AREA));
}

Header::Header() : Window(0, 0, 80, 1, TYPE_NON_FOCUSABLE, false)
{
  setColorScheme(CenterIM::SCHEME_HEADER);

  container_ = new CppConsUI::HorizontalListBox(AUTOSIZE, AUTOSIZE);
  addWidget(*container_, 0, 0);

  // CenterIM name will always be top-left so we can "forget about" it.
  char *cimname = g_strdup_printf("CENTERIM %s", CenterIM::version_);
  container_->appendWidget(*(new CppConsUI::Label(cimname)));
  g_free(cimname);

  // Pending request indicator.
  request_indicator_ = new CppConsUI::Label(0, 1, "");
  request_indicator_->setColorScheme(CenterIM::SCHEME_HEADER_REQUEST);
  container_->appendWidget(*request_indicator_);

  for (GList *i = purple_accounts_get_all(); i != NULL; i = i->next) {
    PurpleAccount *account = reinterpret_cast<PurpleAccount *>(i->data);
    if (purple_account_get_enabled(account, PACKAGE_NAME))
      protocol_count_.insert(purple_account_get_protocol_id(account));
  }

  // Connect to the Accounts singleton.
  g_assert(ACCOUNTS != NULL);
  ACCOUNTS->signal_request_count_change.connect(
    sigc::mem_fun(this, &Header::onRequestCountChange));

  void *handle = purple_accounts_get_handle();
  purple_signal_connect(handle, "account-signed-on", this,
    PURPLE_CALLBACK(account_signed_on_), this);
  purple_signal_connect(handle, "account-signed-off", this,
    PURPLE_CALLBACK(account_signed_off_), this);
  purple_signal_connect(handle, "account-status-changed", this,
    PURPLE_CALLBACK(account_status_changed_), this);
  purple_signal_connect(handle, "account-alias-changed", this,
    PURPLE_CALLBACK(account_alias_changed_), this);
  purple_signal_connect(
    handle, "account-enabled", this, PURPLE_CALLBACK(account_enabled_), this);
  purple_signal_connect(
    handle, "account-disabled", this, PURPLE_CALLBACK(account_disabled_), this);

  onScreenResized();
}

Header::~Header()
{
  purple_signals_disconnect_by_handle(this);
}

void Header::init()
{
  g_assert(my_instance_ == NULL);

  my_instance_ = new Header;
  my_instance_->show();
}

void Header::finalize()
{
  g_assert(my_instance_ != NULL);

  delete my_instance_;
  my_instance_ = NULL;
}

void Header::onRequestCountChange(Accounts & /*accounts*/, size_t request_count)
{
  if (request_count > 0) {
    request_indicator_->setText("* ");
    request_indicator_->setWidth(2);
  }
  else {
    request_indicator_->setText("");
    request_indicator_->setWidth(0);
  }
}

void Header::account_signed_on(PurpleAccount *account)
{
  g_return_if_fail(account != NULL);

  if (statuses_.find(account) != statuses_.end())
    return;

  CppConsUI::Label *label = new CppConsUI::Label(0, 1, "");
  statuses_[account] = label;
  container_->appendWidget(*label);

  account_status_changed(
    account, NULL, purple_account_get_active_status(account));
}

void Header::account_signed_off(PurpleAccount *account)
{
  g_return_if_fail(account != NULL);

  if (statuses_.find(account) == statuses_.end())
    return;

  container_->removeWidget(*statuses_[account]);
  statuses_.erase(account);
}

void Header::account_status_changed(
  PurpleAccount *account, PurpleStatus * /*old*/, PurpleStatus *cur)
{
  g_return_if_fail(account != NULL);
  g_return_if_fail(cur != NULL);

  if (statuses_.find(account) == statuses_.end())
    return;

  const char *prid = purple_account_get_protocol_id(account);
  bool short_text = protocol_count_.count(prid) == 1;

  // Update label.
  CppConsUI::Label *label = statuses_[account];
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

void Header::account_alias_changed(PurpleAccount *account, const char * /*old*/)
{
  g_return_if_fail(account != NULL);

  account_status_changed(
    account, NULL, purple_account_get_active_status(account));
}

void Header::account_enabled(PurpleAccount *account)
{
  g_return_if_fail(account != NULL);

  const char *prid = purple_account_get_protocol_id(account);
  protocol_count_.insert(prid);

  if (protocol_count_.count(prid) == 2)
    for (Statuses::iterator i = statuses_.begin(); i != statuses_.end(); ++i) {
      PurpleAccount *acc = i->first;
      if (std::strcmp(purple_account_get_protocol_id(acc), prid) == 0)
        account_status_changed(
          acc, NULL, purple_account_get_active_status(acc));
    }
}

void Header::account_disabled(PurpleAccount *account)
{
  g_return_if_fail(account != NULL);

  const char *prid = purple_account_get_protocol_id(account);
  ProtocolCount::iterator i = protocol_count_.find(prid);
  protocol_count_.erase(i);

  if (protocol_count_.count(prid) == 1)
    for (Statuses::iterator i = statuses_.begin(); i != statuses_.end(); ++i) {
      PurpleAccount *acc = i->first;
      if (std::strcmp(purple_account_get_protocol_id(acc), prid) == 0)
        account_status_changed(
          acc, NULL, purple_account_get_active_status(acc));
    }
}

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
