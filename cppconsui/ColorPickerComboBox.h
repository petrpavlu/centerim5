// Copyright (C) 2012 Mark Pustjens <pustjens@dds.nl>
// Copyright (C) 2012-2015 Petr Pavlu <setup@dagobah.cz>
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
// along with CenterIM.  If not, see <http://www.gnu.org/licenses/>.

/// @file
/// ColorPickerComboBox class.
///
/// @ingroup cppconsui

#ifndef COLORPICKERCOMBOBOX_H
#define COLORPICKERCOMBOBOX_H

#include "ColorPickerDialog.h"
#include "ComboBox.h"

#ifdef DEBUG
#define COLORPICKER_256COLOR
#endif

namespace CppConsUI {

class ColorPickerComboBox : public ComboBox {
public:
  ColorPickerComboBox(int w, int color);
  virtual ~ColorPickerComboBox() override;

  // Widget
  virtual int draw(Curses::ViewPort area, Error &error) override;

  virtual void setColor(int new_color);
  virtual int getColor() { return selected_color_; }

  sigc::signal<void, ColorPickerComboBox &, int> signal_color_changed;

protected:
  class ColorButton : public Button {
  public:
    ColorButton(int color = -1);
    virtual ~ColorButton() override {}

    // Widget
    virtual int draw(Curses::ViewPort area, Error &error) override;

  protected:
    int color_;

  private:
    CONSUI_DISABLE_COPY(ColorButton);
  };

  // ComboBox
  using ComboBox::clearOptions;
  using ComboBox::addOption;
  using ComboBox::addOptionPtr;
  using ComboBox::getSelectedTitle;
  using ComboBox::getTitle;
  using ComboBox::setSelectedByData;
  using ComboBox::setSelectedByDataPtr;

  // ComboBox
  virtual void onDropDown(Button &activator) override;
  virtual void dropDownOk(Button &activator, int new_entry) override;
  virtual void dropDownClose(Window &window) override
  {
    ComboBox::dropDownClose(window);
  }
#ifdef COLORPICKER_256COLOR
  virtual void colorPickerOk(ColorPickerDialog &activator,
    AbstractDialog::ResponseType response, int new_color);
  virtual void colorPickerClose(Window &window);
#endif // COLORPICKER_256COLOR
  virtual void setSelected(int new_entry) override;

  int selected_color_;

#ifdef COLORPICKER_256COLOR
  ColorPickerDialog *colorpicker_;
#endif // COLORPICKER_256COLOR

private:
  CONSUI_DISABLE_COPY(ColorPickerComboBox);
};

} // namespace CppConsUI

#endif // COLORPICKERCOMBOBOX_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
