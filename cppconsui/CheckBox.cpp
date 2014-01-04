/*
 * Copyright (C) 2010-2013 by CenterIM developers
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

#include "Dialog.h"

#include <cstring>
#include "gettext.h"

namespace CppConsUI
{

CheckBox::CheckBox(int w, int h, const char *text_, bool checked_)
: Widget(w, h), text(NULL), text_width(0), text_height(0)
, checked(checked_)
{
  setText(text_);

  can_focus = true;
  declareBindables();
}

CheckBox::CheckBox(const char *text_, bool checked_)
: Widget(AUTOSIZE, AUTOSIZE), text(NULL), text_width(0), text_height(0)
, checked(checked_)
{
  setText(text_);

  can_focus = true;
  declareBindables();
}

CheckBox::~CheckBox()
{
  delete [] text;
}

void CheckBox::draw()
{
  proceedUpdateArea();

  if (!area)
    return;

  int attrs;
  if (has_focus)
    attrs = getColorPair("checkbox", "focus") | Curses::Attr::REVERSE;
  else
    attrs = getColorPair("checkbox", "normal");
  area->attron(attrs);

  int realw = area->getmaxx();
  int realh = area->getmaxy();

  // print text
  area->fill(attrs, 0, 0, text_width, realh);
  int y = 0;
  const char *start, *end;
  start = end = text;
  while (*end) {
    if (*end == '\n') {
      if (y >= realh)
        break;

      area->mvaddstring(0, y, realw, start, end);
      y++;
      start = end + 1;
    }
    end++;
  }
  if (y < realh)
    area->mvaddstring(0, y, realw, start, end);

  int l = text_width;
  int h = (text_height - 1) / 2;

  // print value
  const char *value = checked ? YES_BUTTON_TEXT : NO_BUTTON_TEXT;
  int value_width = Curses::onscreen_width(value);
  area->fill(attrs, l, 0, value_width + 2, realh);
  if (h < realh) {
    l += area->mvaddstring(l, h, realw - l, ": ");
    l += area->mvaddstring(l, h, realw - l, value);
  }

  area->attroff(attrs);
}

void CheckBox::setText(const char *new_text)
{
  delete [] text;

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
      w = Curses::onscreen_width(start, end);
      if (w > text_width)
        text_width = w;
      text_height++;
      start = end + 1;
    }
    end++;
  }
  w = Curses::onscreen_width(start, end);
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
  declareBindable("checkbox", "toggle", sigc::mem_fun(this,
        &CheckBox::actionToggle), InputProcessor::BINDABLE_NORMAL);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
