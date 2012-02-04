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

ColorPicker::ColorPicker(int fg, int bg, bool sample_)
: HorizontalListBox(AUTOSIZE, 1)
, fg_combo(NULL), bg_combo(NULL), sample(NULL)
{
  fg_combo = new ColorPickerComboBox(10, fg);
  bg_combo = new ColorPickerComboBox(10, bg);

  fg_combo->signal_color_changed.connect(
      sigc::mem_fun(this, &ColorPicker::OnColorChanged));
  bg_combo->signal_color_changed.connect(
      sigc::mem_fun(this, &ColorPicker::OnColorChanged));

  AppendWidget(*fg_combo);
  AppendWidget(*(new Label(1, 1, "")));
  AppendWidget(*bg_combo);

  if (sample_) {
    sample = new Label(10, 1, _("  SAMPLE  "));

    colorscheme = g_strdup_printf("colorpicker %p", (void*)this);
    sample->SetColorScheme(colorscheme);

    AppendWidget(*(new Label(1, 1, "")));
    AppendWidget(*sample);
  }

  SetColorPair(fg, bg);
}

ColorPicker::~ColorPicker()
{
  COLORSCHEME->FreeScheme(colorscheme);
  g_free(colorscheme);
}

void ColorPicker::SetColorPair(int fg, int bg)
{
  fg_combo->SetColor(fg);
  bg_combo->SetColor(bg);

  if (sample) {
    COLORSCHEME->SetColorPair(colorscheme, "label", "text", fg, bg, 0, true);
  }

  signal_colorpair_selected(*this, fg, bg);
}

void ColorPicker::OnColorChanged(ComboBox& activator, int new_color)
{
  int new_fg, new_bg;

  new_fg = fg_combo->GetColor();
  new_bg = bg_combo->GetColor();

  if (&activator == fg_combo)
    new_fg = new_color;
  else
    new_bg = new_color;

  SetColorPair(new_fg, new_bg);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 tw=78 expandtab : */
