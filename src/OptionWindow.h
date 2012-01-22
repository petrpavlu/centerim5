/*
 * Copyright (C) 2011-2012 by CenterIM developers
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

#ifndef __OPTIONWINDOW_H__
#define __OPTIONWINDOW_H__

#include <cppconsui/Button.h>
#include <cppconsui/CheckBox.h>
#include <cppconsui/ComboBox.h>
#include <cppconsui/InputDialog.h>
#include <cppconsui/SplitDialog.h>

class OptionWindow
: public CppConsUI::SplitDialog
{
public:
  OptionWindow();
  virtual ~OptionWindow() {}

  // FreeWindow
  virtual void OnScreenResized();

protected:
  class BooleanOption
  : public CppConsUI::CheckBox
  {
  public:
    BooleanOption(const char *text, const char *config);
    virtual ~BooleanOption();

  protected:
    char *pref;

    void OnToggle(CppConsUI::CheckBox& activator, bool new_state);

  private:
    BooleanOption(const BooleanOption&);
    BooleanOption& operator=(const BooleanOption&);
  };

  class StringOption
  : public CppConsUI::Button
  {
  public:
    StringOption(const char *text, const char *config);
    virtual ~StringOption();

  protected:
    char *pref;

    void OnActivate(CppConsUI::Button& activator);
    void ResponseHandler(CppConsUI::InputDialog& activator,
        CppConsUI::AbstractDialog::ResponseType response);

  private:
    StringOption(const StringOption&);
    StringOption& operator=(const StringOption&);
  };

  class IntegerOption
  : public CppConsUI::Button
  {
  public:
    IntegerOption(const char *text, const char *config);
    IntegerOption(const char *text, const char *config,
        sigc::slot<const char*, int> unit_fun_);
    virtual ~IntegerOption();

  protected:
    char *pref;
    bool unit;
    sigc::slot<const char*, int> unit_fun;

    void OnActivate(CppConsUI::Button& activator);
    void ResponseHandler(CppConsUI::InputDialog& activator,
        CppConsUI::AbstractDialog::ResponseType response);

  private:
    IntegerOption(const IntegerOption&);
    IntegerOption& operator=(const IntegerOption&);
  };

  class ChoiceOption
  : public CppConsUI::ComboBox
  {
  public:
    ChoiceOption(const char *text, const char *config);
    virtual ~ChoiceOption();

    void AddOption(const char *title, const char *value);

  protected:
    char *pref;

    void OnSelectionChanged(CppConsUI::ComboBox& activator, int new_entry,
        const char *title, intptr_t data);

  private:
    ChoiceOption(const ChoiceOption&);
    ChoiceOption& operator=(const ChoiceOption&);
  };

  const char *GetPercentUnit(int i) const;
  const char *GetMinUnit(int i) const;
  void ReloadKeybindingFile(CppConsUI::Button& activator) const;

private:
  OptionWindow(const OptionWindow&);
  OptionWindow& operator=(const OptionWindow&);
};

#endif // __OPTIONWINDOW_H__

/* vim: set tabstop=2 shiftwidth=2 expandtab : */
