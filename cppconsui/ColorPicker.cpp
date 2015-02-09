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
 * ColorPicker class implementation.
 *
 * @ingroup cppconsui
 */

#include "ColorPicker.h"

#include "ColorScheme.h"
#include "Spacer.h"

#include "gettext.h"

namespace CppConsUI
{

ColorPicker::ColorPicker(int fg, int bg, const char *text, bool sample_)
: HorizontalListBox(AUTOSIZE, 1), fg_combo(NULL), bg_combo(NULL), label(NULL)
, sample(NULL)
{
  fg_combo = new ColorPickerComboBox(10, fg);
  bg_combo = new ColorPickerComboBox(10, bg);

  label = new Label(1, 1, "");
  setText(text);

  fg_combo->signal_color_changed.connect(
      sigc::mem_fun(this, &ColorPicker::onColorChanged));
  bg_combo->signal_color_changed.connect(
      sigc::mem_fun(this, &ColorPicker::onColorChanged));

  appendWidget(*label);
  appendWidget(*fg_combo);
  appendWidget(*(new Spacer(1, 1)));
  appendWidget(*bg_combo);

  if (sample_) {
    sample = new Sample(10, fg, bg);
    appendWidget(*sample);
  }

  setColorPair(fg, bg);
}

void ColorPicker::setColorPair(int fg, int bg)
{
  fg_combo->setColor(fg);
  bg_combo->setColor(bg);

  if (sample)
    sample->setColors(fg, bg);

  signal_colorpair_selected(*this, fg, bg);
}

void ColorPicker::setText(const char *new_text)
{
  label->setText(new_text);
  if (new_text)
    label->setWidth(Curses::onScreenWidth(new_text) + 1);
  else
    label->setWidth(0);
}

void ColorPicker::onColorChanged(ComboBox& activator, int new_color)
{
  int new_fg = fg_combo->getColor();
  int new_bg = bg_combo->getColor();

  if (&activator == fg_combo)
    new_fg = new_color;
  else
    new_bg = new_color;

  setColorPair(new_fg, new_bg);
}

ColorPicker::Sample::Sample(int w, int fg, int bg)
: Widget(w, 1), c(fg, bg)
{
}

void ColorPicker::Sample::draw(Curses::ViewPort area)
{
  int colorpair = COLORSCHEME->getColorPair(c);

  area.attrOn(colorpair);
  area.addString(1, 0, _(" SAMPLE "));
  area.attrOff(colorpair);
}

void ColorPicker::Sample::setColors(int fg, int bg)
{
  c.foreground = fg;
  c.background = bg;
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
