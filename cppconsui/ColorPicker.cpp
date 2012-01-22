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
 * ColorPicker class implementation.
 *
 * @ingroup cppconsui
 */

#include "ColorPicker.h"

#include "ConsuiCurses.h"

namespace CppConsUI
{

ColorPicker::ColorPicker(int defaultcolor)
: Container(52, 4)
{
  int x;

  ColorPickerButton *btn;
  
  /* Default 16 colors */
  for (x = 0; x < 16; x++)
  {
    if (x >= 8)
      AddButton((x-8)*2, 1, x, defaultcolor);
    else
      AddButton(x*2, 0, x, defaultcolor);
  }

  /* Grayscale ladder */
  AddButton(0, 3, 0, defaultcolor);

  for (x = 232; x < 256; x++)
    AddButton((x-231)*2, 3, x, defaultcolor);

  AddButton(25*2, 3, 15, defaultcolor);

  /* 6x6x6 Color cube*/
  int y = 5;
  x = 0;

  for (int g = 0; g < 6; g++)
  {
    for (int r = 0; r < 6; r++)
    {
      for (int b = 0; b < 6; b++)
      {
        AddButton(x*2, y, 16 + (r * 36) + (g * 6) + b, defaultcolor);
        x++;
      }

      x++;
    }

    y++;
    x -= (6*7);
  }
}

void ColorPicker::OnSelectColor(Button& activator)
{
  ColorPickerButton *btn = dynamic_cast<ColorPickerButton*>(&activator);
  g_assert(btn);

  signal_color_selected (*this, btn->color);
}

void ColorPicker::AddButton(int x, int y, int color, int defaultcolor)
{
  ColorPickerButton *btn = new ColorPickerButton(color);

  btn->signal_activate.connect(sigc::mem_fun(this,
      &ColorPicker::OnSelectColor));
  AddWidget (*btn, x, y);

  if (color == defaultcolor)
    btn->GrabFocus();
}

ColorPicker::ColorPickerButton::ColorPickerButton (const int color)
: Button(2, 1, NULL, 0), color(color)
{
}

void ColorPicker::ColorPickerButton::Draw()
{
  ProceedUpdateArea();

  if (!area)
    return;

  int realw = area->getmaxx();
  int realh = area->getmaxy();

  //@todo get proper color for cursor in more cases
  int cursor = Curses::Color::BLACK;
  if (color >= 232 && color < 244)
    cursor = Curses::Color::WHITE;

  int colorpair = Curses::getcolorpair(cursor, color);

  area->fill(colorpair, 0, 0, 2, 1);

  // print cursor
  int y = 0;
  const char *start, *end;
  if (has_focus)
    area->mvaddstring(0, 0, "<>");
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 expandtab : */
