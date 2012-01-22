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

#include "Container.h"
#include "Button.h"

#include <set>

namespace CppConsUI
{

/**
 * A color picker
 */
class ColorPicker
: public Container
{
  public:
    ColorPicker(int defaultcolor);
    virtual ~ColorPicker() {};

    /**
     * Emited signal when a color is selected
     */
    sigc::signal<void, ColorPicker&, int> signal_color_selected;

  protected:
    class ColorPickerButton
    : public Button
    {
      friend class ColorPicker;

      public:
        ColorPickerButton (const int color);
        virtual ~ColorPickerButton() {};

        void Draw();

      protected:

      private:
        const int color;

        ColorPickerButton(const ColorPickerButton&);
        ColorPickerButton& operator=(const ColorPickerButton&);
    };

    void OnSelectColor(Button& activator);

  private:
    void AddButton(int x, int y, int color, int defaultcolor);

    void AddAnsi(int defaultcolor);
    void AddGrayscale(int defaultcolor);
    void AddColorCube(int defaultcolor);

    ColorPicker(const ColorPicker&);
    ColorPicker& operator=(const ColorPicker&);
};

} // namespace CppConsUI

#endif // __COLORPICKER_H__

/* vim: set tabstop=2 shiftwidth=2 expandtab : */
