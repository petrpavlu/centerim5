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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  Softwareee the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * */

/**
 * @file
 * ComboBox class.
 *
 * @ingroup cppconsui
 */

#ifndef __COMBOBOX_H__
#define __COMBOBOX_H__

#include "Button.h"
#include "MenuWindow.h"

#include <vector>

/**
 * This class should be used when the user must choose one value from several
 * options.
 */
class ComboBox
: public Button
{
public:
  ComboBox(int w, int h, const char *text_ = NULL);
  explicit ComboBox(const char *text_ = NULL);
  virtual ~ComboBox();

  /**
   * Removes all options.
   */
  void ClearOptions();

  /**
   * Appends a new option.
   */
  int AddOption(const char *text = NULL, intptr_t data = 0);
  int AddOptionPtr(const char *text = NULL, void *data = NULL)
    { return AddOption(text, reinterpret_cast<intptr_t>(data)); }

  /**
   * Returns last selected option.
   */
  int GetSelected() const { return selected_entry; };
  const char *GetSelectedTitle() const;

  int GetOptionsCount() const { return options.size(); }

  const char *GetTitle(int entry) const;
  intptr_t GetData(int entry) const;

  void SetSelected(int new_entry);
  void SetSelectedByData(intptr_t data);
  void SetSelectedByDataPtr(void *data)
    { SetSelectedByData(reinterpret_cast<intptr_t>(data)); }

  sigc::signal<void, ComboBox&, int, const char *, intptr_t>
    signal_selection_changed;

protected:
  /**
   * Keeps a pair of {display text, value}.
   */
  struct ComboBoxEntry {
    char *title;
    intptr_t data;
  };
  typedef std::vector<ComboBoxEntry> ComboBoxEntries;

  MenuWindow *dropdown;
  /**
   * Prepares and displays the dropdown MenuWindow.
   */
  void OnDropDown(Button& activator);
  void DropDownOk(Button& activator, int new_entry);
  void DropDownClose(FreeWindow& window);

  /**
   * Number of currently selected entry.
   */
  int selected_entry;
  /**
   * All options.
   */
  ComboBoxEntries options;

  /**
   * Maximal option width. Used for dropdown menu width.
   */
  int max_option_width;

private:
  ComboBox(const ComboBox&);
  ComboBox& operator=(const ComboBox&);
};

#endif // __COMBOBOX_H__
