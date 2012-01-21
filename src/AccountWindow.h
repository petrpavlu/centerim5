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
: public CppConsUI::SplitDialog
{
public:
  AccountWindow();
  virtual ~AccountWindow() {}

  // FreeWindow
  virtual void OnScreenResized();

protected:

private:
  class SplitOption;
  typedef std::list<SplitOption*> SplitWidgets;
  typedef std::vector<CppConsUI::Widget*> Widgets;

  struct AccountEntry
  {
    CppConsUI::Button *parent;
    CppConsUI::TreeView::NodeReference parent_reference;
    SplitWidgets split_widgets;
  };
  typedef std::map<PurpleAccount*, AccountEntry> AccountEntries;

  class BoolOption
  : public CppConsUI::CheckBox
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

    void OnToggle(CppConsUI::CheckBox& activator, bool new_state);

  private:
    BoolOption(const BoolOption&);
    BoolOption& operator=(const BoolOption&);
  };

  class StringOption
  : public CppConsUI::Button
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
    void OnActivate(CppConsUI::Button& activator);
    void ResponseHandler(CppConsUI::InputDialog& activator,
        CppConsUI::AbstractDialog::ResponseType response);

  private:
    StringOption(const StringOption&);
    StringOption& operator=(const StringOption&);
  };

  class IntOption
  : public CppConsUI::Button
  {
  public:
    IntOption(PurpleAccount *account, PurpleAccountOption *option);
    virtual ~IntOption() {}

  protected:
    PurpleAccount *account;
    PurpleAccountOption *option;

    void UpdateValue();
    void OnActivate(CppConsUI::Button& activator);
    void ResponseHandler(CppConsUI::InputDialog& activator,
        CppConsUI::AbstractDialog::ResponseType response);

  private:
    IntOption(const IntOption&);
    IntOption& operator=(const IntOption&);
  };

  class StringListOption
  : public CppConsUI::ComboBox
  {
  public:
    StringListOption(PurpleAccount *account, PurpleAccountOption *option);
    virtual ~StringListOption() {}

  protected:
    PurpleAccount *account;
    PurpleAccountOption *option;

    void OnSelectionChanged(CppConsUI::ComboBox& activator, int new_entry,
        const char *title, intptr_t data);

  private:
    StringListOption(const StringListOption&);
    StringListOption& operator=(const StringListOption&);
  };

  class SplitOption
  : public CppConsUI::Button
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
    void OnActivate(CppConsUI::Button& activator);
    void ResponseHandler(CppConsUI::InputDialog& activator,
        CppConsUI::AbstractDialog::ResponseType response);

  private:
    SplitOption(const SplitOption&);
    SplitOption& operator=(const SplitOption&);
  };

  class ProtocolOption
  : public CppConsUI::ComboBox
  {
  public:
    ProtocolOption(PurpleAccount *account, AccountWindow& account_window);
    virtual ~ProtocolOption() {}

  protected:
    AccountWindow *account_window;
    PurpleAccount *account;

    void OnProtocolChanged(CppConsUI::ComboBox& activator, size_t new_entry,
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

  void AddAccount(CppConsUI::Button& activator);
  void DropAccount(CppConsUI::Button& activator, PurpleAccount *account);
  void DropAccountResponseHandler(CppConsUI::MessageDialog& activator,
      CppConsUI::AbstractDialog::ResponseType response,
      PurpleAccount *account);

  CppConsUI::TreeView *accounts;

  AccountEntries account_entries;
};

#endif // __ACCOUNTSWINDOW_H__

/* vim: set tabstop=2 shiftwidth=2 expandtab */
