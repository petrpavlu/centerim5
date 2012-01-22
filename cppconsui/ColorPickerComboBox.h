/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2012 by CenterIM developers
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
 */

/**
 * @file
 * ColorPickerComboBox class.
 *
 * @ingroup cppconsui
 */

#ifndef __COLORPICKERCOMBOBOX_H__
#define __COLORPICKERCOMBOBOX_H__

#include "Button.h"
#include "ColorPickerDialog.h"

#include <vector>

namespace CppConsUI
{

/**
 * This class should be used when the user must choose one value from several
 * options.
 */
class ColorPickerComboBox
: public Button
{
public:
  ColorPickerComboBox(int w, int color = -1);
  virtual ~ColorPickerComboBox();

  // Button
  void Draw();

  void SetSelectedColor(int new_color);

  sigc::signal<void, ColorPickerComboBox&, int>
    signal_color_changed;

protected:
  ColorPickerDialog *dropdown;

  /**
   * Currently selected color.
   */
  int selected_color;

  /**
   * Prepares and displays the dropdown MenuWindow.
   */
  virtual void OnDropDown(Button& activator);
  virtual void DropDownOk( ColorPickerDialog& activator,
      AbstractDialog::ResponseType response, int new_color);
  virtual void DropDownClose(FreeWindow& window);

private:
  ColorPickerComboBox(const ColorPickerComboBox&);
  ColorPickerComboBox& operator=(const ColorPickerComboBox&);
};

} // namespace CppConsUI

#endif // __COLORPICKERCOMBOBOX_H__

/* vim: set tabstop=2 shiftwidth=2 expandtab : */
