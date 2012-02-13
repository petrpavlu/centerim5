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

#include "ColorScheme.h"

#include "gettext.h"

namespace CppConsUI
{

ColorPicker::ColorPicker(int fg, int bg, const char *text, bool sample_)
: HorizontalListBox(AUTOSIZE, 1)
, fg_combo(NULL), bg_combo(NULL), label(NULL), sample(NULL)
{
  fg_combo = new ColorPickerComboBox(10, fg);
  bg_combo = new ColorPickerComboBox(10, bg);

  label = new Label(1, 1, "");
  SetText(text);

  fg_combo->signal_color_changed.connect(
      sigc::mem_fun(this, &ColorPicker::OnColorChanged));
  bg_combo->signal_color_changed.connect(
      sigc::mem_fun(this, &ColorPicker::OnColorChanged));

  AppendWidget(*label);
  AppendWidget(*fg_combo);
  AppendWidget(*(new Label(1, 1, "")));
  AppendWidget(*bg_combo);

  if (sample_) {
    sample = new Sample(10, fg, bg);
    AppendWidget(*sample);
  }

  SetColorPair(fg, bg);
}

void ColorPicker::SetColorPair(int fg, int bg)
{
  fg_combo->SetColor(fg);
  bg_combo->SetColor(bg);

  if (sample)
    sample->SetColors(fg, bg);

  signal_colorpair_selected(*this, fg, bg);
}

void ColorPicker::SetText(const char *new_text)
{
  label->SetText(new_text);
  if (new_text)
    label->SetWidth(Curses::onscreen_width(new_text, NULL) + 1);
  else
    label->SetWidth(0);
}

void ColorPicker::OnColorChanged(ComboBox& activator, int new_color)
{
  int new_fg = fg_combo->GetColor();
  int new_bg = bg_combo->GetColor();

  if (&activator == fg_combo)
    new_fg = new_color;
  else
    new_bg = new_color;

  SetColorPair(new_fg, new_bg);
}

ColorPicker::Sample::Sample(int w, int fg, int bg)
: Widget(w, 1), c(fg, bg)
{
}

void ColorPicker::Sample::SetColors(int fg, int bg)
{
  c.foreground = fg;
  c.background = bg;
}

void ColorPicker::Sample::Draw()
{
  ProceedUpdateArea();

  if (!area)
    return;

  int colorpair = COLORSCHEME->GetColorPair(c);

  area->attron(colorpair);
  area->mvaddstring(1, 0, _(" SAMPLE "));
  area->attroff(colorpair);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 tw=78 expandtab : */
