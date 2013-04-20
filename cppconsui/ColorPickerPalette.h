/*
 * Copyright (C) 2012-2013 by CenterIM developers
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
 * ColorPickerPalette class.
 *
 * @ingroup cppconsui
 */

#ifndef __COLORPICKERPALETTE_H__
#define __COLORPICKERPALETTE_H__

#include "Button.h"
#include "Container.h"

namespace CppConsUI
{

class ColorPickerPalette
: public Container
{
public:
  enum Flag {
    FLAG_HIDE_ANSI = 1 << 0,
    FLAG_HIDE_GRAYSCALE = 1 << 1,
    FLAG_HIDE_COLORCUBE = 1 << 2
  };

  ColorPickerPalette(int defaultcolor, int flags = 0);
  virtual ~ColorPickerPalette() {};

  /**
   * Emited signal when a color is selected
   */
  sigc::signal<void, ColorPickerPalette&, int> signal_color_selected;

protected:
  class ColorPickerPaletteButton
  : public Button
  {
  friend class ColorPickerPalette;

  public:
    ColorPickerPaletteButton(int color);
    virtual ~ColorPickerPaletteButton() {};

    virtual void Draw();

  protected:
    int color;

  private:
    ColorPickerPaletteButton(const ColorPickerPaletteButton&);
    ColorPickerPaletteButton& operator=(const ColorPickerPaletteButton&);
  };

  virtual void OnSelectColor(Button& activator);

  virtual void AddButton(int x, int y, int color, int defaultcolor);

  virtual void AddAnsi(int defaultcolor);
  virtual void AddGrayscale(int defaultcolor);
  virtual void AddColorCube(int defaultcolor);

private:
  ColorPickerPalette(const ColorPickerPalette&);
  ColorPickerPalette& operator=(const ColorPickerPalette&);
};

} // namespace CppConsUI

#endif // __COLORPICKERPALETTE_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
