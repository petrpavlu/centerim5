/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2012 by CenterIM developers
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
 */

/**
 * @file
 * ColorPickerComboBox class implementation.
 *
 * @ingroup cppconsui
 */

#include "ColorPickerComboBox.h"
#include "AbstractDialog.h"

namespace CppConsUI
{

ColorPickerComboBox::ColorPickerComboBox(int w, int color)
: Button(w, 1, NULL, 0), dropdown(NULL), selected_color(color)
{
  signal_activate.connect(sigc::mem_fun(this, &ColorPickerComboBox::OnDropDown));

  if (w < 4) {
    Resize(4, 1);
  }
}

ColorPickerComboBox::~ColorPickerComboBox()
{
  if (dropdown)
    dropdown->Close();
}

void ColorPickerComboBox::Draw()
{
  ProceedUpdateArea();

  if (!area)
    return;

  int button_colorpair;
  if (has_focus)
    button_colorpair = GetColorPair("button", "focus") | Curses::Attr::REVERSE;
  else
    button_colorpair = GetColorPair("button", "normal");

  int selected_colorpair = Curses::getcolorpair(0, selected_color);

  area->attron(button_colorpair);
  area->fill(button_colorpair, 0, 0, area->getmaxx(), 1);
  area->mvaddchar(0, 0, '[');
  area->mvaddchar(area->getmaxx()-1, 0, ']');
  area->attroff(button_colorpair);

  area->attron(selected_colorpair);
  area->fill(selected_colorpair, 1, 0, area->getmaxx()-2, 1);
  area->attroff(selected_colorpair);
}

void ColorPickerComboBox::SetSelectedColor(int new_color)
{
  g_assert(new_color >= 0);
  g_assert(new_color < Curses::Color::Colors());

  if (new_color == selected_color)
    return;

  selected_color = new_color;

  Redraw();
  signal_color_changed(*this, new_color);
}

void ColorPickerComboBox::OnDropDown(Button& activator)
{
  //@todo use flags parameter
  dropdown = new CppConsUI::ColorPickerDialog("", selected_color, 0);

  dropdown->signal_response.connect(sigc::mem_fun(this,
      &ColorPickerComboBox::DropDownOk));
  dropdown->signal_close.connect(sigc::mem_fun(this,
      &ColorPickerComboBox::DropDownClose));

  dropdown->Show();
}

void ColorPickerComboBox::DropDownOk(
    ColorPickerDialog& activator,
    AbstractDialog::ResponseType response,
    int new_color)
{
  if (response != AbstractDialog::RESPONSE_OK)
      return;

  // selected option didn't change
  if (selected_color == new_color )
    return;

  selected_color = new_color;

  Redraw();
  signal_color_changed(*this, new_color);
}

void ColorPickerComboBox::DropDownClose(FreeWindow& window)
{
  dropdown = NULL;
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 expandtab : */
