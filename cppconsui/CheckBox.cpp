/*
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
 * CheckBox class implementation.
 *
 * @ingroup cppconsui
 */

#include "CheckBox.h"

#include "Dialog.h"

#include "gettext.h"

namespace CppConsUI
{

CheckBox::CheckBox(int w, int h, const char *text_, bool default_state)
: Widget(w, h), text(NULL), text_width(0), text_height(0)
, state(default_state)
{
  SetText(text_);

  can_focus = true;
  DeclareBindables();
}

CheckBox::CheckBox(const char *text_, bool default_state)
: Widget(AUTOSIZE, AUTOSIZE), text(NULL), text_width(0), text_height(0)
, state(default_state)
{
  SetText(text_);

  can_focus = true;
  DeclareBindables();
}

CheckBox::~CheckBox()
{
  if (text)
    g_free(text);
}

void CheckBox::Draw()
{
  ProceedUpdateArea();

  if (!area || !text)
    return;

  int attrs;
  if (has_focus)
    attrs = GetColorPair("checkbox", "focus") | Curses::Attr::REVERSE;
  else
    attrs = GetColorPair("checkbox", "normal");
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
  const char *value = state ? YES_BUTTON_TEXT : NO_BUTTON_TEXT;
  int value_width = Curses::onscreen_width(value);
  area->fill(attrs, l, 0, value_width + 2, realh);
  if (h < realh) {
    l += area->mvaddstring(l, h, realw - l, ": ");
    l += area->mvaddstring(l, h, realw - l, value);
  }

  area->attroff(attrs);
}

void CheckBox::SetText(const char *new_text)
{
  if (text)
    g_free(text);

  text = g_strdup(new_text);

  // update text_width, text_height and wish height
  text_width = 0;
  text_height = 1;
  if (text) {
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
  }
  SetWishHeight(text_height);

  Redraw();
}

void CheckBox::SetState(bool new_state)
{
  bool old_state = state;
  state = new_state;

  if (state != old_state)
    signal_toggle(*this, state);
  Redraw();
}

void CheckBox::ActionToggle()
{
  state = !state;
  signal_toggle(*this, state);
  Redraw();
}

void CheckBox::DeclareBindables()
{
  DeclareBindable("checkbox", "toggle", sigc::mem_fun(this,
        &CheckBox::ActionToggle), InputProcessor::BINDABLE_NORMAL);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 expandtab : */
