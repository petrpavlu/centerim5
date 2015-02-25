/*
 * Copyright (C) 2012-2015 by CenterIM developers
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

/**
 * @file
 * ColorPicker class.
 *
 * @ingroup cppconsui
 */

#ifndef __COLORPICKER_H__
#define __COLORPICKER_H__

#include "ColorPickerComboBox.h"
#include "HorizontalListBox.h"
#include "Label.h"

namespace CppConsUI
{

class ColorPicker
: public HorizontalListBox
{
public:
  ColorPicker(int fg, int bg, const char *text, bool sample = false);
  virtual ~ColorPicker() {}

  virtual void setColorPair(int fg, int bg);
  virtual void setText(const char *new_text);

  /**
   * Emited signal when the colorpair has been selected
   */
  sigc::signal<void, ColorPicker &, int, int> signal_colorpair_selected;

protected:
  class Sample
  : public Widget
  {
  public:
    Sample(int w, int fg = -1, int bg = -1);
    virtual ~Sample() {}

    // Widget
    virtual void draw(Curses::ViewPort area);

    virtual void setColors(int fg, int bg);

  protected:
    ColorScheme::Color c;

  private:
    CONSUI_DISABLE_COPY(Sample);
  };

  ColorPickerComboBox *fg_combo;
  ColorPickerComboBox *bg_combo;
  Label *label;
  Sample *sample;

  virtual void onColorChanged(ComboBox &activator, int new_color);

private:
  CONSUI_DISABLE_COPY(ColorPicker);
};

} // namespace CppConsUI

#endif // __COLORPICKER_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
