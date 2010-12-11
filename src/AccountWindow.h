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

#ifndef __ACCOUNTSWINDOW_H__
#define __ACCOUNTSWINDOW_H__

#include <cppconsui/Button.h>
#include <cppconsui/CheckBox.h>
#include <cppconsui/ComboBox.h>
#include <cppconsui/InputDialog.h>
#include <cppconsui/SplitDialog.h>
#include <cppconsui/TreeView.h>

#include <libpurple/account.h>
#include <libpurple/accountopt.h>

class AccountWindow
: public SplitDialog
{
public:
  AccountWindow();

  // FreeWindow
  virtual void ScreenResized();

protected:

private:
  class AccountOptionSplit;
  typedef std::list<AccountOptionSplit*> SplitWidgets;
  typedef std::vector<Widget*> Widgets;

  struct AccountEntry {
    Button *parent;
    TreeView::NodeReference parent_reference;
    SplitWidgets split_widgets;
  };
  typedef std::map<PurpleAccount*, AccountEntry> AccountEntries;

  class AccountOptionBool
  : public CheckBox
  {
  public:
    enum Type {
      TYPE_PURPLE,
      TYPE_REMEMBER_PASSWORD,
      TYPE_ENABLE_ACCOUNT
    };

    AccountOptionBool(PurpleAccount *account, PurpleAccountOption *option);
    AccountOptionBool(PurpleAccount *account, Type type);
    virtual ~AccountOptionBool() {}

  protected:
    PurpleAccount *account;
    PurpleAccountOption *option;
    Type type;

    void OnToggle(CheckBox& activator, bool new_state);

  private:
    AccountOptionBool(const AccountOptionBool&);
    AccountOptionBool& operator=(const AccountOptionBool&);
  };

  class AccountOptionString
  : public Button
  {
  public:
    enum Type {
      TYPE_PURPLE,
      TYPE_PASSWORD,
      TYPE_ALIAS
    };

    AccountOptionString(PurpleAccount *account, PurpleAccountOption *option);
    AccountOptionString(PurpleAccount *account, Type type);
    virtual ~AccountOptionString() {}

  protected:
    PurpleAccount *account;
    PurpleAccountOption *option;
    Type type;
    const gchar *text;
    const gchar *value;

    void UpdateText();
    void OnActivate(Button& activator);
    void ResponseHandler(Dialog& activator, Dialog::ResponseType response);

  private:
    AccountOptionString(const AccountOptionString&);
    AccountOptionString& operator=(const AccountOptionString&);
  };

  class AccountOptionInt
  : public Button
  {
  public:
    AccountOptionInt(PurpleAccount *account, PurpleAccountOption *option);
    virtual ~AccountOptionInt() {}

  protected:
    PurpleAccount *account;
    PurpleAccountOption *option;
    const gchar *text;
    int value;

    void UpdateText();
    void OnActivate(Button& activator);
    void ResponseHandler(Dialog& activator, Dialog::ResponseType response);

  private:
    AccountOptionInt(const AccountOptionInt&);
    AccountOptionInt& operator=(const AccountOptionInt&);
  };

  class AccountOptionSplit
  : public Button
  {
  public:
    AccountOptionSplit(PurpleAccount *account, PurpleAccountUserSplit *split,
        AccountEntry *account_entry);
    virtual ~AccountOptionSplit();

    void SetValue(const gchar *new_value);
    const gchar *GetValue() const { return value; }

  protected:
    PurpleAccount *account;
    PurpleAccountUserSplit *split;
    AccountEntry *account_entry;
    const char *text;
    gchar *value;

    void UpdateSplits();
    void UpdateText();
    void OnActivate(Button& activator);
    void ResponseHandler(Dialog& activator, Dialog::ResponseType response);

  private:
    AccountOptionSplit(const AccountOptionSplit&);
    AccountOptionSplit& operator=(const AccountOptionSplit&);
  };

  class AccountOptionProtocol
  : public ComboBox
  {
  public:
    AccountOptionProtocol(PurpleAccount *account,
        AccountWindow& account_window);
    virtual ~AccountOptionProtocol() {}

  protected:
    AccountWindow *account_window;
    PurpleAccount *account;

    void OnProtocolChanged(Button& activator, size_t new_entry,
        const gchar *title, intptr_t data);

  private:
    AccountOptionProtocol(const AccountOptionProtocol&);
    AccountOptionProtocol& operator=(const AccountOptionProtocol&);
  };

  AccountWindow(const AccountWindow&);
  AccountWindow& operator=(const AccountWindow&);
  virtual ~AccountWindow() {}

  bool ClearAccount(PurpleAccount *account, bool full);

  void Populate();
  void PopulateAccount(PurpleAccount *account);

  void Add(Button& activator);
  void DropAccount(Button& activator, PurpleAccount *account);
  void DropAccountResponseHandler(Dialog& activator,
      Dialog::ResponseType response, PurpleAccount *account);

  TreeView *accounts;

  AccountEntries account_entries;
};

#endif // __ACCOUNTSWINDOW_H__
