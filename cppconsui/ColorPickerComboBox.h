/*
 * Copyright (C) 2012 by CenterIM developers
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
 * ColorPickerComboBox class.
 *
 * @ingroup cppconsui
 */

#ifndef __COLORPICKERCOMBOBOX_H__
#define __COLORPICKERCOMBOBOX_H__

#include "Button.h"
#include "ColorPickerDialog.h"
#include "ComboBox.h"

#ifdef DEBUG
#define COLORPICKER_256COLOR
#endif

namespace CppConsUI
{

class ColorPickerComboBox
: public ComboBox
{
public:
  ColorPickerComboBox(int w, int color);
  virtual ~ColorPickerComboBox();

  // Button
  virtual void Draw();

  virtual void SetColor(int new_color);
  virtual int GetColor() { return selected_color; }

  sigc::signal<void, ColorPickerComboBox&, int> signal_color_changed;

protected:
  class ColorButton
  : public Button
  {
  public:
    ColorButton(int color_ = -1);
    virtual ~ColorButton() {}

   // Button
   virtual void Draw();

  protected:
    int color;

  private:
    ColorButton(const ColorButton&);
    ColorButton& operator=(const ColorButton&);
  };

  // ComboBox
  ComboBox::ClearOptions;
  ComboBox::AddOption;
  ComboBox::AddOptionPtr;
  ComboBox::GetSelectedTitle;
  ComboBox::GetTitle;
  ComboBox::SetSelectedByData;
  ComboBox::SetSelectedByDataPtr;

  // ComboBox
  virtual void OnDropDown(Button& activator);
  virtual void DropDownOk(Button& activator, int new_entry);
  virtual void DropDownClose(FreeWindow& window)
    { ComboBox::DropDownClose(window); }
#ifdef COLORPICKER_256COLOR
  virtual void ColorPickerOk(ColorPickerDialog& activator,
      AbstractDialog::ResponseType response, int new_color);
  virtual void ColorPickerClose(FreeWindow& window);
#endif // COLORPICKER_256COLOR
  virtual void SetSelected(int new_entry);

  int selected_color;

#ifdef COLORPICKER_256COLOR
  ColorPickerDialog *colorpicker;
#endif // COLORPICKER_256COLOR

private:
  ColorPickerComboBox(const ColorPickerComboBox&);
  ColorPickerComboBox& operator=(const ColorPickerComboBox&);
};

} // namespace CppConsUI

#endif // __COLORPICKERCOMBOBOX_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
