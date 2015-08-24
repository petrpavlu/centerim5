/*
 * Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
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
 * CheckBox class implementation.
 *
 * @ingroup cppconsui
 */

#include "CheckBox.h"

#include "ColorScheme.h"
#include "Dialog.h"

#include <cassert>
#include <cstring>
#include "gettext.h"

namespace CppConsUI {

CheckBox::CheckBox(int w, int h, const char *text_, bool checked_)
  : Widget(w, h), text(NULL), text_width(0), text_height(0), checked(checked_)
{
  setText(text_);

  can_focus = true;
  declareBindables();
}

CheckBox::CheckBox(const char *text_, bool checked_)
  : Widget(AUTOSIZE, AUTOSIZE), text(NULL), text_width(0), text_height(0),
    checked(checked_)
{
  setText(text_);

  can_focus = true;
  declareBindables();
}

CheckBox::~CheckBox()
{
  delete[] text;
}

int CheckBox::draw(Curses::ViewPort area, Error &error)
{
  assert(text != NULL);

  int attrs;
  if (has_focus) {
    DRAW(getAttributes(ColorScheme::PROPERTY_CHECKBOX_FOCUS, &attrs, error));
    attrs |= Curses::Attr::REVERSE;
  }
  else
    DRAW(getAttributes(ColorScheme::PROPERTY_CHECKBOX_NORMAL, &attrs, error));
  DRAW(area.attrOn(attrs, error));

  // Print text.
  DRAW(area.fill(attrs, 0, 0, text_width, real_height, error));
  int y = 0;
  const char *start, *end;
  start = end = text;
  while (*end != '\0') {
    if (*end == '\n') {
      DRAW(area.addString(0, y, real_width, start, end, error));
      ++y;
      start = end + 1;
    }
    ++end;
  }
  DRAW(area.addString(0, y, real_width, start, end, error));

  int l = text_width;
  int h = (text_height - 1) / 2;
  int printed;

  // Print value.
  const char *value = checked ? YES_BUTTON_TEXT : NO_BUTTON_TEXT;
  int value_width = Curses::onScreenWidth(value);
  DRAW(area.fill(attrs, l, 0, value_width + 2, real_height, error));
  DRAW(area.addString(l, h, real_width - l, ": ", error, &printed));
  l += printed;
  DRAW(area.addString(l, h, real_width - l, value, error));

  DRAW(area.attrOff(attrs, error));

  return 0;
}

void CheckBox::setText(const char *new_text)
{
  delete[] text;

  size_t size = 1;
  if (new_text)
    size += std::strlen(new_text);
  text = new char[size];
  if (new_text)
    std::strcpy(text, new_text);
  else
    text[0] = '\0';

  // update text_width, text_height and wish height
  text_width = 0;
  text_height = 1;

  const char *start, *end;
  start = end = text;
  int w;
  while (*end) {
    if (*end == '\n') {
      w = Curses::onScreenWidth(start, end);
      if (w > text_width)
        text_width = w;
      ++text_height;
      start = end + 1;
    }
    ++end;
  }
  w = Curses::onScreenWidth(start, end);
  if (w > text_width)
    text_width = w;
  setWishHeight(text_height);

  redraw();
}

void CheckBox::setChecked(bool new_checked)
{
  if (new_checked == checked)
    return;

  checked = new_checked;
  signal_toggle(*this, checked);
  redraw();
}

void CheckBox::actionToggle()
{
  setChecked(!checked);
}

void CheckBox::declareBindables()
{
  declareBindable("checkbox", "toggle",
    sigc::mem_fun(this, &CheckBox::actionToggle),
    InputProcessor::BINDABLE_NORMAL);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
