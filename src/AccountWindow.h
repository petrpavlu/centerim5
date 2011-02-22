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

#ifndef __ACCOUNTSWINDOW_H__
#define __ACCOUNTSWINDOW_H__

#include <cppconsui/Button.h>
#include <cppconsui/CheckBox.h>
#include <cppconsui/ComboBox.h>
#include <cppconsui/InputDialog.h>
#include <cppconsui/MessageDialog.h>
#include <cppconsui/SplitDialog.h>
#include <cppconsui/TreeView.h>
#include <libpurple/purple.h>

class AccountWindow
: public SplitDialog
{
public:
  AccountWindow();
  virtual ~AccountWindow() {}

  // FreeWindow
  virtual void ScreenResized();

protected:

private:
  class SplitOption;
  typedef std::list<SplitOption*> SplitWidgets;
  typedef std::vector<Widget*> Widgets;

  struct AccountEntry {
    Button *parent;
    TreeView::NodeReference parent_reference;
    SplitWidgets split_widgets;
  };
  typedef std::map<PurpleAccount*, AccountEntry> AccountEntries;

  class BoolOption
  : public CheckBox
  {
  public:
    enum Type {
      TYPE_PURPLE,
      TYPE_REMEMBER_PASSWORD,
      TYPE_ENABLE_ACCOUNT
    };

    BoolOption(PurpleAccount *account, PurpleAccountOption *option);
    BoolOption(PurpleAccount *account, Type type);
    virtual ~BoolOption() {}

  protected:
    PurpleAccount *account;
    PurpleAccountOption *option;
    Type type;

    void OnToggle(CheckBox& activator, bool new_state);

  private:
    BoolOption(const BoolOption&);
    BoolOption& operator=(const BoolOption&);
  };

  class StringOption
  : public Button
  {
  public:
    enum Type {
      TYPE_PURPLE,
      TYPE_PASSWORD,
      TYPE_ALIAS
    };

    StringOption(PurpleAccount *account, PurpleAccountOption *option);
    StringOption(PurpleAccount *account, Type type);
    virtual ~StringOption() {}

  protected:
    PurpleAccount *account;
    PurpleAccountOption *option;
    Type type;

    void Initialize();
    void UpdateValue();
    void OnActivate(Button& activator);
    void ResponseHandler(InputDialog& activator,
        AbstractDialog::ResponseType response);

  private:
    StringOption(const StringOption&);
    StringOption& operator=(const StringOption&);
  };

  class IntOption
  : public Button
  {
  public:
    IntOption(PurpleAccount *account, PurpleAccountOption *option);
    virtual ~IntOption() {}

  protected:
    PurpleAccount *account;
    PurpleAccountOption *option;

    void UpdateValue();
    void OnActivate(Button& activator);
    void ResponseHandler(InputDialog& activator,
        AbstractDialog::ResponseType response);

  private:
    IntOption(const IntOption&);
    IntOption& operator=(const IntOption&);
  };

  class SplitOption
  : public Button
  {
  public:
    SplitOption(PurpleAccount *account, PurpleAccountUserSplit *split,
        AccountEntry *account_entry);
    virtual ~SplitOption() {}

  protected:
    PurpleAccount *account;
    PurpleAccountUserSplit *split;
    AccountEntry *account_entry;

    void UpdateSplits();
    void OnActivate(Button& activator);
    void ResponseHandler(InputDialog& activator,
        AbstractDialog::ResponseType response);

  private:
    SplitOption(const SplitOption&);
    SplitOption& operator=(const SplitOption&);
  };

  class ProtocolOption
  : public ComboBox
  {
  public:
    ProtocolOption(PurpleAccount *account, AccountWindow& account_window);
    virtual ~ProtocolOption() {}

  protected:
    AccountWindow *account_window;
    PurpleAccount *account;

    void OnProtocolChanged(ComboBox& activator, size_t new_entry,
        const char *title, intptr_t data);

  private:
    ProtocolOption(const ProtocolOption&);
    ProtocolOption& operator=(const ProtocolOption&);
  };

  AccountWindow(const AccountWindow&);
  AccountWindow& operator=(const AccountWindow&);

  bool ClearAccount(PurpleAccount *account, bool full);

  void Populate();
  void PopulateAccount(PurpleAccount *account);

  void Add(Button& activator);
  void DropAccount(Button& activator, PurpleAccount *account);
  void DropAccountResponseHandler(MessageDialog& activator,
      AbstractDialog::ResponseType response, PurpleAccount *account);

  TreeView *accounts;

  AccountEntries account_entries;
};

#endif // __ACCOUNTSWINDOW_H__
