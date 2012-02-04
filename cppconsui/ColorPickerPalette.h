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
 * ColorPickerPalette class.
 *
 * @ingroup cppconsui
 */

#ifndef __COLORPICKERPALETTE_H__
#define __COLORPICKERPALETTE_H__

#include "Container.h"
#include "Button.h"

#include <set>

namespace CppConsUI
{

/**
 * A color picker
 */
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
        ColorPickerPaletteButton (int color);
        virtual ~ColorPickerPaletteButton() {};

        void Draw();

      protected:

      private:
        int color;

        ColorPickerPaletteButton(const ColorPickerPaletteButton&);
        ColorPickerPaletteButton& operator=(const ColorPickerPaletteButton&);
    };

    void OnSelectColor(Button& activator);

  private:
    void AddButton(int x, int y, int color, int defaultcolor);

    void AddAnsi(int defaultcolor);
    void AddGrayscale(int defaultcolor);
    void AddColorCube(int defaultcolor);

    ColorPickerPalette(const ColorPickerPalette&);
    ColorPickerPalette& operator=(const ColorPickerPalette&);
};

} // namespace CppConsUI

#endif // __COLORPICKERPALETTE_H__

/* vim: set tabstop=2 shiftwidth=2 expandtab : */
