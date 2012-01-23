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

//@todo replace magic constants

#include "ColorPickerComboBox.h"

#include "gettext.h"

namespace CppConsUI
{

ColorPickerComboBox::ColorPickerComboBox(int w, int color)
: ComboBox(w, 1, ""), selected_color(color)
{
  // Add ANSI colors
  for (int i = 0; i < 16; i++) {
    AddOption(NULL, i);
  }

  // Add options for default color and to open the 256 color dialog
  AddOption(NULL, -1);
  AddOption(_("More..."), -2);
}

void ColorPickerComboBox::SetColor(int new_color)
{
  g_assert(new_color >= -1 && new_color < Curses::Color::Colors());

  if (new_color == selected_color)
    return;

  selected_color = new_color;

  if (selected_color >= -1 && selected_color < 16)
    SetSelectedByData(selected_color);
  else
    SetSelectedByData(-2);
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

  int color = options[selected_entry].data;

  area->attron(button_colorpair);
  area->fill(button_colorpair, 0, 0, area->getmaxx(), 1);
  area->mvaddchar(0, 0, '[');
  area->mvaddchar(area->getmaxx()-1, 0, ']');
  area->attroff(button_colorpair);

  if (selected_color > -1) {
    int colorpair = Curses::getcolorpair(color, color);
    area->attron(colorpair);
    area->fill(colorpair, 1, 0, area->getmaxx()-2, 1);
    area->attroff(colorpair);
  } else {
    area->mvaddstring(1, 0, "DEFAULT");
  }
}

void ColorPickerComboBox::OnDropDown(Button& activator)
{
  if (options.empty())
    return;

  dropdown = new MenuWindow(*this, 12, AUTOSIZE);
  dropdown->signal_close.connect(sigc::mem_fun(this,
        &ColorPickerComboBox::DropDownClose));

  int i;
  ComboBoxEntries::iterator j;
  for (i = 0, j = options.begin(); j != options.end(); i++, j++) {
    if (options[i].data < -1) {
      Button *b = dropdown->AppendItem(j->title, sigc::bind(sigc::mem_fun(this,
          &ColorPickerComboBox::DropDownOk), i));
      if (selected_color < -1)
        b->GrabFocus();
    } else {
      ColorButton *b = new ColorButton(10, options[i].data);
      dropdown->AppendWidget(*b);
      b->signal_activate.connect(sigc::bind(sigc::mem_fun(this,
              &ColorPickerComboBox::DropDownOk), i));
      if (i == selected_entry)
        b->GrabFocus();
    }
  }

  dropdown->Show();
}

void ColorPickerComboBox::DropDownOk(Button& activator, int new_entry)
{
  dropdown->Close();

  if (options[new_entry].data != -2) {
    SetColor(options[new_entry].data);
    return;
  }

  // If `More...' button chosen, display color picker dialog
  // @todo initialize with selected color
  colorpicker = new CppConsUI::ColorPickerDialog("", 0, 0);

  colorpicker->signal_response.connect(sigc::mem_fun(this,
      &ColorPickerComboBox::ColorPickerOk));
  colorpicker->signal_close.connect(sigc::mem_fun(this,
      &ColorPickerComboBox::ColorPickerClose));

  colorpicker->Show();
}

void ColorPickerComboBox::ColorPickerOk(
    ColorPickerDialog& activator,
    AbstractDialog::ResponseType response,
    int new_color)
{
  if (response != AbstractDialog::RESPONSE_OK)
      return;

  // selected option didn't change
  if (selected_color == new_color)
    return;

  SetColor(new_color);
}

void ColorPickerComboBox::ColorPickerClose(FreeWindow& window)
{
  colorpicker = NULL;
}


ColorPickerComboBox::ColorButton::ColorButton(int w, int color)
: Button(w, 1, "", 0), color(color)
{
  if (w < 10)
    Resize(10, 1);
}

void ColorPickerComboBox::ColorButton::Draw()
{
  ProceedUpdateArea();

  if (!area)
    return;

  int button_colorpair;
  if (has_focus)
    button_colorpair = GetColorPair("button", "focus") | Curses::Attr::REVERSE;
  else
    button_colorpair = GetColorPair("button", "normal");

  area->attron(button_colorpair);
  area->fill(button_colorpair, 0, 0, area->getmaxx(), 1);
  area->mvaddchar(0, 0, '[');
  area->mvaddchar(area->getmaxx()-1, 0, ']');
  area->attroff(button_colorpair);

  if (color > -1) {
    int colorpair = Curses::getcolorpair(color, color);
    area->attron(colorpair);
    area->fill(colorpair, 1, 0, area->getmaxx()-2, 1);
    area->attroff(colorpair);
  } else {
    area->mvaddstring(1, 0, "DEFAULT");
  }

}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 expandtab : */
