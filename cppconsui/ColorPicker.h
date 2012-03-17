/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2011 by CenterIM developers
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

#include "HorizontalListBox.h"
#include "ColorPickerComboBox.h"

namespace CppConsUI
{

/**
 * A color picker
 */
class ColorPicker
: public HorizontalListBox
{
public:
  ColorPicker(int fg, int bg, const char *text, bool sample = false);
  virtual ~ColorPicker() {}

  virtual void SetColorPair(int fg, int bg);
  virtual void SetText(const char *new_text);

  /**
   * Emited signal when the colorpair has been selected
   */
  sigc::signal<void, ColorPicker&, int, int> signal_colorpair_selected;

protected:
  class Sample
  : public Widget
  {
  public:
    Sample(int w, int fg = -1, int bg = -1);
    virtual ~Sample() {}

    virtual void SetColors(int fg, int bg);

    // Widget
    virtual void Draw();

  protected:
    ColorScheme::Color c;
  };

  virtual void OnColorChanged(ComboBox& activator, int new_color);

  ColorPickerComboBox *fg_combo;
  ColorPickerComboBox *bg_combo;
  Label *label;
  Sample *sample;

private:
  ColorPicker(const ColorPicker&);
  ColorPicker& operator=(const ColorPicker&);
};

} // namespace CppConsUI

#endif // __COLORPICKER_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
