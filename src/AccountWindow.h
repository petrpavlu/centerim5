// Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
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

#ifndef ACCOUNTWINDOW_H
#define ACCOUNTWINDOW_H

#include <cppconsui/Button.h>
#include <cppconsui/CheckBox.h>
#include <cppconsui/ColorPicker.h>
#include <cppconsui/ComboBox.h>
#include <cppconsui/InputDialog.h>
#include <cppconsui/MessageDialog.h>
#include <cppconsui/SplitDialog.h>
#include <cppconsui/TreeView.h>
#include <libpurple/purple.h>

class AccountWindow : public CppConsUI::SplitDialog {
public:
  AccountWindow();
  virtual ~AccountWindow() override {}

  // FreeWindow
  virtual void onScreenResized() override;

private:
  class SplitOption;
  typedef std::list<SplitOption *> SplitWidgets;
  typedef std::vector<CppConsUI::Widget *> Widgets;

  struct AccountEntry {
    CppConsUI::Button *parent;
    CppConsUI::TreeView::NodeReference parent_reference;
    SplitWidgets split_widgets;
  };
  typedef std::map<PurpleAccount *, AccountEntry> AccountEntries;

  class BoolOption : public CppConsUI::CheckBox {
  public:
    enum Type {
      TYPE_PURPLE,
      TYPE_REMEMBER_PASSWORD,
      TYPE_ENABLE_ACCOUNT,
    };

    BoolOption(PurpleAccount *account, PurpleAccountOption *option);
    BoolOption(PurpleAccount *account, Type type);
    virtual ~BoolOption() override {}

  protected:
    PurpleAccount *account_;
    PurpleAccountOption *option_;
    Type type_;

    void onToggle(CppConsUI::CheckBox &activator, bool new_state);

  private:
    CONSUI_DISABLE_COPY(BoolOption);
  };

  class StringOption : public CppConsUI::Button {
  public:
    enum Type {
      TYPE_PURPLE,
      TYPE_PASSWORD,
      TYPE_ALIAS,
    };

    StringOption(PurpleAccount *account, PurpleAccountOption *option);
    StringOption(PurpleAccount *account, Type type);
    virtual ~StringOption() override {}

  protected:
    PurpleAccount *account_;
    PurpleAccountOption *option_;
    Type type_;

    void initialize();
    void updateValue();
    void onActivate(CppConsUI::Button &activator);
    void responseHandler(CppConsUI::InputDialog &activator,
      CppConsUI::AbstractDialog::ResponseType response);

  private:
    CONSUI_DISABLE_COPY(StringOption);
  };

  class IntegerOption : public CppConsUI::Button {
  public:
    IntegerOption(PurpleAccount *account, PurpleAccountOption *option);
    virtual ~IntegerOption() override {}

  protected:
    PurpleAccount *account_;
    PurpleAccountOption *option_;

    void updateValue();
    void onActivate(CppConsUI::Button &activator);
    void responseHandler(CppConsUI::InputDialog &activator,
      CppConsUI::AbstractDialog::ResponseType response);

  private:
    CONSUI_DISABLE_COPY(IntegerOption);
  };

  class StringListOption : public CppConsUI::ComboBox {
  public:
    StringListOption(PurpleAccount *account, PurpleAccountOption *option);
    virtual ~StringListOption() override {}

  protected:
    PurpleAccount *account_;
    PurpleAccountOption *option_;

    void onSelectionChanged(CppConsUI::ComboBox &activator, int new_entry,
      const char *title, intptr_t data);

  private:
    CONSUI_DISABLE_COPY(StringListOption);
  };

  class SplitOption : public CppConsUI::Button {
  public:
    SplitOption(PurpleAccount *account, PurpleAccountUserSplit *split,
      AccountEntry *account_entry);
    virtual ~SplitOption() override {}

  protected:
    PurpleAccount *account_;
    PurpleAccountUserSplit *split_;
    AccountEntry *account_entry_;

    void updateSplits();
    void onActivate(CppConsUI::Button &activator);
    void responseHandler(CppConsUI::InputDialog &activator,
      CppConsUI::AbstractDialog::ResponseType response);

  private:
    CONSUI_DISABLE_COPY(SplitOption);
  };

  class ProtocolOption : public CppConsUI::ComboBox {
  public:
    ProtocolOption(PurpleAccount *account, AccountWindow &account_window);
    virtual ~ProtocolOption() override {}

  protected:
    AccountWindow *account_window_;
    PurpleAccount *account_;

    void onProtocolChanged(CppConsUI::ComboBox &activator,
      std::size_t new_entry, const char *title, intptr_t data);

  private:
    CONSUI_DISABLE_COPY(ProtocolOption);
  };

  class ColorOption : public CppConsUI::ColorPicker {
  public:
    ColorOption(PurpleAccount *account);
    virtual ~ColorOption() override {}

  protected:
    PurpleAccount *account_;

    void onColorChanged(
      CppConsUI::ColorPicker &activator, int new_fg, int new_bg);

  private:
    CONSUI_DISABLE_COPY(ColorOption);
  };

  CppConsUI::TreeView *treeview_;
  AccountEntries account_entries_;

  CONSUI_DISABLE_COPY(AccountWindow);

  void clearAccount(PurpleAccount *account, bool full);
  void populateAccount(PurpleAccount *account);

  void addAccount(CppConsUI::Button &activator);
  void dropAccount(CppConsUI::Button &activator, PurpleAccount *account);
  void dropAccountResponseHandler(CppConsUI::MessageDialog &activator,
    CppConsUI::AbstractDialog::ResponseType response, PurpleAccount *account);
};

#endif // ACCOUNTWINDOW_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
