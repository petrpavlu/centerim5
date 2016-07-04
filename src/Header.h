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

#ifndef HEADER_H
#define HEADER_H

#include "Accounts.h"

#include <cppconsui/HorizontalListBox.h>
#include <cppconsui/Label.h>
#include <cppconsui/Window.h>
#include <libpurple/purple.h>
#include <map>

#define HEADER (Header::instance())

// Top "head"-area of the screen.
class Header : public CppConsUI::Window {
public:
  static Header *instance();

  // Window
  virtual void onScreenResized() override;

private:
  typedef std::multiset<std::string> ProtocolCount;
  typedef std::map<PurpleAccount *, CppConsUI::Label *> Statuses;

  CppConsUI::HorizontalListBox *container_;
  CppConsUI::Label *request_indicator_;
  ProtocolCount protocol_count_;
  Statuses statuses_;

  static Header *my_instance_;

  Header();
  virtual ~Header() override;
  CONSUI_DISABLE_COPY(Header);

  static void init();
  static void finalize();
  friend class CenterIM;

  // Signal from the Accounts singleton.
  void onRequestCountChange(Accounts &accounts, size_t request_count);

  static void account_signed_on_(PurpleAccount *account, gpointer data)
  {
    reinterpret_cast<Header *>(data)->account_signed_on(account);
  }
  static void account_signed_off_(PurpleAccount *account, gpointer data)
  {
    reinterpret_cast<Header *>(data)->account_signed_off(account);
  }
  static void account_status_changed_(
    PurpleAccount *account, PurpleStatus *old, PurpleStatus *cur, gpointer data)
  {
    reinterpret_cast<Header *>(data)->account_status_changed(account, old, cur);
  }
  static void account_alias_changed_(
    PurpleAccount *account, const char *old, gpointer data)
  {
    reinterpret_cast<Header *>(data)->account_alias_changed(account, old);
  }
  static void account_enabled_(PurpleAccount *account, gpointer data)
  {
    reinterpret_cast<Header *>(data)->account_enabled(account);
  }
  static void account_disabled_(PurpleAccount *account, gpointer data)
  {
    reinterpret_cast<Header *>(data)->account_disabled(account);
  }

  void account_signed_on(PurpleAccount *account);
  void account_signed_off(PurpleAccount *account);
  void account_status_changed(
    PurpleAccount *account, PurpleStatus *old, PurpleStatus *cur);
  void account_alias_changed(PurpleAccount *account, const char *old);
  void account_enabled(PurpleAccount *account);
  void account_disabled(PurpleAccount *account);
};

#endif // HEADER_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
