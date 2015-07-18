/*
 * Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
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

namespace CppConsUI {

/**
 * This class should be used when the user must choose one value from several
 * options.
 */
class ComboBox : public Button {
public:
  ComboBox(int w, int h, const char *text_ = NULL);
  explicit ComboBox(const char *text_ = NULL);
  virtual ~ComboBox();

  /**
   * Removes all options.
   */
  virtual void clearOptions();

  /**
   * Appends a new option.
   */
  virtual int addOption(const char *text = NULL, intptr_t data = 0);
  virtual int addOptionPtr(const char *text = NULL, void *data = NULL)
  {
    return addOption(text, reinterpret_cast<intptr_t>(data));
  }

  /**
   * Returns last selected option.
   */
  virtual int getSelected() const { return selected_entry; };
  virtual const char *getSelectedTitle() const;
  virtual intptr_t getSelectedData() const;
  virtual void *getSelectedDataPtr() const;

  virtual int getOptionsCount() const { return options.size(); }

  virtual const char *getTitle(int entry) const;
  virtual intptr_t getData(int entry) const;

  virtual void setSelected(int new_entry);
  virtual void setSelectedByData(intptr_t data);
  virtual void setSelectedByDataPtr(void *data)
  {
    setSelectedByData(reinterpret_cast<intptr_t>(data));
  }

  sigc::signal<void, ComboBox &, int, const char *, intptr_t>
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

  /**
   * Prepares and displays the dropdown MenuWindow.
   */
  virtual void onDropDown(Button &activator);
  virtual void dropDownOk(Button &activator, int new_entry);
  virtual void dropDownClose(Window &window);

private:
  CONSUI_DISABLE_COPY(ComboBox);
};

} // namespace CppConsUI

#endif // __COMBOBOX_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
