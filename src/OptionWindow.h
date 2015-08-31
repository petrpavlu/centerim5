// Copyright (C) 2011-2015 Petr Pavlu <setup@dagobah.cz>
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

#ifndef OPTIONWINDOW_H
#define OPTIONWINDOW_H

#include <cppconsui/Button.h>
#include <cppconsui/CheckBox.h>
#include <cppconsui/ComboBox.h>
#include <cppconsui/InputDialog.h>
#include <cppconsui/SplitDialog.h>

class OptionWindow : public CppConsUI::SplitDialog {
public:
  OptionWindow();
  virtual ~OptionWindow() {}

  // FreeWindow
  virtual void onScreenResized();

protected:
  class BooleanOption : public CppConsUI::CheckBox {
  public:
    BooleanOption(const char *text, const char *config);
    virtual ~BooleanOption();

  protected:
    char *pref_;

    void onToggle(CppConsUI::CheckBox &activator, bool new_state);

  private:
    CONSUI_DISABLE_COPY(BooleanOption);
  };

  class StringOption : public CppConsUI::Button {
  public:
    StringOption(const char *text, const char *config);
    virtual ~StringOption();

  protected:
    char *pref_;

    void onActivate(CppConsUI::Button &activator);
    void responseHandler(CppConsUI::InputDialog &activator,
      CppConsUI::AbstractDialog::ResponseType response);

  private:
    CONSUI_DISABLE_COPY(StringOption);
  };

  class IntegerOption : public CppConsUI::Button {
  public:
    IntegerOption(const char *text, const char *config);
    IntegerOption(const char *text, const char *config,
      sigc::slot<const char *, int> unit_fun);
    virtual ~IntegerOption();

  protected:
    char *pref_;
    bool unit_;
    sigc::slot<const char *, int> unit_fun_;

    void onActivate(CppConsUI::Button &activator);
    void responseHandler(CppConsUI::InputDialog &activator,
      CppConsUI::AbstractDialog::ResponseType response);

  private:
    CONSUI_DISABLE_COPY(IntegerOption);
  };

  class ChoiceOption : public CppConsUI::ComboBox {
  public:
    ChoiceOption(const char *text, const char *config);
    virtual ~ChoiceOption();

    void addOption(const char *title, const char *value);

  protected:
    char *pref_;

    void onSelectionChanged(CppConsUI::ComboBox &activator, int new_entry,
      const char *title, intptr_t data);

  private:
    CONSUI_DISABLE_COPY(ChoiceOption);
  };

  const char *getPercentUnit(int i) const;
  const char *getMinUnit(int i) const;
  void reloadKeyBindings(CppConsUI::Button &activator) const;
  void reloadColorSchemes(CppConsUI::Button &activator) const;

private:
  CONSUI_DISABLE_COPY(OptionWindow);
};

#endif // OPTIONWINDOW_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
