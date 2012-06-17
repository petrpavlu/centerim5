/*
 * Copyright (C) 2012 by CenterIM developers
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

#include "gettext.h"

/* This is an invalid color number that is used for the "More..." button in
 * ColorPickerComboBox. */
#define COLOR_MORE (-2)

// width of the drop-down menu
#define MENU_WIDTH 12

namespace CppConsUI
{

ColorPickerComboBox::ColorPickerComboBox(int w, int color)
: ComboBox(w, 1), selected_color(color)
#ifdef COLORPICKER_256COLOR
, colorpicker(NULL)
#endif // COLORPICKER_256COLOR
{
  // add ANSI colors
  int colors = MIN(Curses::NUM_DEFAULT_COLORS, Curses::nrcolors());
  for (int i = 0; i < colors; i++)
    AddOption(NULL, i);

  // add options for default color and to open the 256 color dialog
  AddOption(NULL, Curses::Color::DEFAULT);
#ifdef COLORPICKER_256COLOR
  AddOption(_("More..."), COLOR_MORE);
#endif // COLORPICKER_256COLOR

  // set initial selection
  SetSelectedByData(color);
}

ColorPickerComboBox::~ColorPickerComboBox()
{
#ifdef COLORPICKER_256COLOR
  if (colorpicker)
    colorpicker->Close();
#endif // COLORPICKER_256COLOR
}

void ColorPickerComboBox::SetColor(int new_color)
{
  if (new_color < Curses::Color::DEFAULT || new_color >= Curses::nrcolors()) {
    // an invalid color was specified, use the default color
    new_color = Curses::Color::DEFAULT;
  }

  if (new_color == selected_color)
    return;

  selected_color = new_color;

#ifdef COLORPICKER_256COLOR
  if (selected_color >= Curses::Color::DEFAULT
      && selected_color < Curses::NUM_DEFAULT_COLORS)
    SetSelectedByData(selected_color);
  else
    SetSelectedByData(COLOR_MORE);
#else
  SetSelectedByData(selected_color);
#endif // COLORPICKER_256COLOR
}

void ColorPickerComboBox::Draw()
{
  ProceedUpdateArea();

  if (!area)
    return;

  int button_colorpair;
  if (has_focus)
    button_colorpair = GetColorPair("button", "focus")
      | Curses::Attr::REVERSE;
  else
    button_colorpair = GetColorPair("button", "normal");

  int realw = area->getmaxx();
  int color = selected_color;

  area->attron(button_colorpair);
  area->fill(button_colorpair, 0, 0, realw, 1);
  area->mvaddchar(0, 0, '[');
  area->mvaddchar(realw - 1, 0, ']');
  area->attroff(button_colorpair);

  if (selected_color == Curses::Color::DEFAULT)
    area->mvaddstring(1, 0, _("DEFAULT"));
  else {
    ColorScheme::Color c(Curses::Color::DEFAULT, color);
    int colorpair = COLORSCHEME->GetColorPair(c);
    area->attron(colorpair);
    area->fill(colorpair, 1, 0, realw - 2, 1);
    area->attroff(colorpair);
  }
}

void ColorPickerComboBox::OnDropDown(Button& /*activator*/)
{
  dropdown = new MenuWindow(*this, MENU_WIDTH, AUTOSIZE);
  dropdown->signal_close.connect(sigc::mem_fun(this,
        &ColorPickerComboBox::DropDownClose));

  int i;
  ComboBoxEntries::iterator j;
  for (i = 0, j = options.begin(); j != options.end(); i++, j++) {
    Button *b;
    if (j->data == COLOR_MORE) {
      // add the "More..." button
      b = dropdown->AppendItem(j->title, sigc::bind( sigc::mem_fun(this,
              &ColorPickerComboBox::DropDownOk), i));
    }
    else {
      // normal color button
      b = new ColorButton(j->data);
      dropdown->AppendWidget(*b);
      b->signal_activate.connect(sigc::bind(sigc::mem_fun(this,
              &ColorPickerComboBox::DropDownOk), i));
    }
    if (i == selected_entry)
      b->GrabFocus();
  }

  dropdown->Show();
}

void ColorPickerComboBox::DropDownOk(Button& /*activator*/, int new_entry)
{
  dropdown->Close();

#ifdef COLORPICKER_256COLOR
  if (options[new_entry].data != COLOR_MORE) {
    SetColor(options[new_entry].data);
    return;
  }

  // the "More..." button was selected, display the color picker dialog
  // @todo Initialize the color picker dialog with a selected color.
  colorpicker = new ColorPickerDialog("", 0, 0);
  colorpicker->signal_response.connect(sigc::mem_fun(this,
        &ColorPickerComboBox::ColorPickerOk));
  colorpicker->signal_close.connect(sigc::mem_fun(this,
        &ColorPickerComboBox::ColorPickerClose));
  colorpicker->Show();
#else
  SetColor(options[new_entry].data);
#endif // COLORPICKER_256COLOR
}

#ifdef COLORPICKER_256COLOR
void ColorPickerComboBox::ColorPickerOk(ColorPickerDialog& activator,
    AbstractDialog::ResponseType response, int new_color)
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
#endif // COLORPICKER_256COLOR

void ColorPickerComboBox::SetSelected(int new_entry)
{
  ComboBox::SetSelected(new_entry);

  selected_color = options[new_entry].data;
  signal_color_changed(*this, selected_color);
}

ColorPickerComboBox::ColorButton::ColorButton(int color_)
: Button(MENU_WIDTH - 2, 1, ""), color(color_)
{
}

void ColorPickerComboBox::ColorButton::Draw()
{
  ProceedUpdateArea();

  if (!area)
    return;

  int button_colorpair;
  if (has_focus)
    button_colorpair = GetColorPair("button", "focus")
      | Curses::Attr::REVERSE;
  else
    button_colorpair = GetColorPair("button", "normal");

  int realw = area->getmaxx();

  area->attron(button_colorpair);
  area->fill(button_colorpair, 0, 0, realw, 1);
  area->mvaddchar(0, 0, '[');
  area->mvaddchar(realw - 1, 0, ']');
  area->attroff(button_colorpair);

  if (color == Curses::Color::DEFAULT)
    area->mvaddstring(1, 0, _("DEFAULT "));
  else {
    ColorScheme::Color c(Curses::Color::DEFAULT, color);
    int colorpair = COLORSCHEME->GetColorPair(c);
    area->attron(colorpair);
    area->fill(colorpair, 1, 0, realw - 2, 1);
    area->attroff(colorpair);
  }
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
