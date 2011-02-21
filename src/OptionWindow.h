/*
 * Copyright (C) 2011 by CenterIM developers
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

#ifndef __OPTIONWINDOW_H__
#define __OPTIONWINDOW_H__

#include <cppconsui/Button.h>
#include <cppconsui/CheckBox.h>
#include <cppconsui/ComboBox.h>
#include <cppconsui/InputDialog.h>
#include <cppconsui/SplitDialog.h>

class OptionWindow
: public SplitDialog
{
public:
  OptionWindow();
  virtual ~OptionWindow() {}

  // FreeWindow
  virtual void ScreenResized();

protected:
  class BooleanOption
  : public CheckBox
  {
  public:
    BooleanOption(const char *text, const char *config, bool default_value);
    virtual ~BooleanOption();

  protected:
    char *pref;

    void OnToggle(CheckBox& activator, bool new_state);

  private:
    BooleanOption(const BooleanOption&);
    BooleanOption& operator=(const BooleanOption&);
  };

  class StringOption
  : public Button
  {
  public:
    StringOption(const char *text, const char *config,
        const char *default_value);
    virtual ~StringOption();

  protected:
    char *pref;

    void OnActivate(Button& activator);
    void ResponseHandler(InputDialog& activator,
        AbstractDialog::ResponseType response);

  private:
    StringOption(const StringOption&);
    StringOption& operator=(const StringOption&);
  };

  class IntegerOption
  : public Button
  {
  public:
    IntegerOption(const char *text, const char *config, int default_value);
    IntegerOption(const char *text, const char *config, int default_value,
        int min_, int max_);
    virtual ~IntegerOption();

  protected:
    char *pref;
    bool bounds_check;
    int min;
    int max;

    void OnActivate(Button& activator);
    void ResponseHandler(InputDialog& activator,
        AbstractDialog::ResponseType response);

  private:
    IntegerOption(const IntegerOption&);
    IntegerOption& operator=(const IntegerOption&);
  };

  class ChoiceOption
  : public ComboBox
  {
  public:
    ChoiceOption(const char *text, const char *config,
        const char *default_value);
    virtual ~ChoiceOption();

    void AddOption(const char *title, const char *value);

  protected:
    char *pref;

    void OnSelectionChanged(ComboBox& activator, int new_entry,
        const char *title, intptr_t data);

  private:
    ChoiceOption(const ChoiceOption&);
    ChoiceOption& operator=(const ChoiceOption&);
  };

private:
  OptionWindow(const OptionWindow&);
  OptionWindow& operator=(const OptionWindow&);
};

#endif // __OPTIONWINDOW_H__
