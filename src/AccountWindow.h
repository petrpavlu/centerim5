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

  class AccountOption
  : public Button
  {
  public:
    AccountOption(PurpleAccount *account, PurpleAccountOption *option = NULL);
    virtual ~AccountOption() {}

  protected:
    PurpleAccount *account;
    PurpleAccountOption *option;

    const char *setting;
    const char *text;

  private:
    AccountOption(const AccountOption&);
    AccountOption& operator=(const AccountOption&);

    virtual void UpdateText() = 0;
    virtual void OnActivate(Button& activator) = 0;
  };

  class AccountOptionBool
  : public AccountOption
  {
  public:
    AccountOptionBool(PurpleAccount *account, PurpleAccountOption *option);
    AccountOptionBool(PurpleAccount *account, bool remember_password,
        bool enable_account);
    virtual ~AccountOptionBool() {}

  protected:
    gboolean value;

    bool remember_password, enable_account;

  private:
    AccountOptionBool(const AccountOptionBool&);
    AccountOptionBool& operator=(const AccountOptionBool&);

    virtual void UpdateText();
    virtual void OnActivate(Button& activator);
  };

  class AccountOptionString
  : public AccountOption
  {
  public:
    AccountOptionString(PurpleAccount *account, PurpleAccountOption *option);
    AccountOptionString(PurpleAccount *account, bool password, bool alias);
    virtual ~AccountOptionString() {}

  protected:
    const gchar *value;

    bool password, alias;

  private:
    AccountOptionString(const AccountOptionString&);
    AccountOptionString& operator=(const AccountOptionString&);

    virtual void UpdateText();
    virtual void OnActivate(Button& activator);

    void ResponseHandler(Dialog& activator, Dialog::ResponseType response);
  };

  class AccountOptionInt
  : public AccountOption
  {
  public:
    AccountOptionInt(PurpleAccount *account, PurpleAccountOption *option);
    virtual ~AccountOptionInt() {}

  protected:
    int value;

  private:
    AccountOptionInt(const AccountOptionInt&);
    AccountOptionInt& operator=(const AccountOptionInt&);

    virtual void UpdateText();
    virtual void OnActivate(Button& activator);

    void ResponseHandler(Dialog& activator, Dialog::ResponseType response);
  };

  class AccountOptionSplit
  : public Button
  {
  public:
    AccountOptionSplit(PurpleAccount *account, PurpleAccountUserSplit *split,
        AccountEntry *account_entry);
    virtual ~AccountOptionSplit();

    void SetValue(const gchar *new_value);
    const gchar* GetValue() { return value; }

  protected:
    PurpleAccount *account;
    PurpleAccountUserSplit *split;
    AccountEntry *account_entry;

    const char *text;
    gchar *value;

    void UpdateSplits();

  private:
    AccountOptionSplit(const AccountOptionSplit&);
    AccountOptionSplit& operator=(const AccountOptionSplit&);

    virtual void UpdateText();
    virtual void OnActivate(Button& activator);

    void ResponseHandler(Dialog& activator, Dialog::ResponseType response);
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

  private:
    AccountOptionProtocol(const AccountOptionProtocol&);
    AccountOptionProtocol& operator=(const AccountOptionProtocol&);

    void OnProtocolChanged(Button& activator, size_t new_entry,
        const gchar *title, intptr_t data);
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
