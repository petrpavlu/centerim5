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
 * Button class implementation.
 *
 * @ingroup cppconsui
 */

#include "Button.h"

namespace CppConsUI
{

Button::Button(int w, int h, const char *text_, int flags_)
: Widget(w, h), flags(flags_), text(NULL), text_width(0), text_height(0)
, value(NULL), value_width(0), unit(NULL) , unit_width(0), right(NULL)
, right_width(0)
{
  SetText(text_);

  can_focus = true;
  DeclareBindables();
}

Button::Button(const char *text_, int flags_)
: Widget(AUTOSIZE, AUTOSIZE), flags(flags_), text(NULL), text_width(0)
, text_height(0), value(NULL), value_width(0), unit(NULL), unit_width(0)
, right(NULL), right_width(0)
{
  SetText(text_);

  can_focus = true;
  DeclareBindables();
}

Button::Button(int w, int h, int flags_, const char *text_,
    const char *value_, const char *unit_, const char *right_)
: Widget(w, h), flags(flags_), text(NULL), text_width(0), text_height(0)
, value(NULL), value_width(0), unit(NULL), unit_width(0), right(NULL)
, right_width(0)
{
  SetText(text_);
  SetValue(value_);
  SetUnit(unit_);
  SetRight(right_);

  can_focus = true;
  DeclareBindables();
}

Button::Button(int flags_, const char *text_, const char *value_,
    const char *unit_, const char *right_)
: Widget(AUTOSIZE, AUTOSIZE), flags(flags_), text(NULL), text_width(0)
, text_height(0), value(NULL), value_width(0), unit(NULL), unit_width(0)
, right(NULL), right_width(0)
{
  SetText(text_);
  SetValue(value_);
  SetUnit(unit_);
  SetRight(right_);

  can_focus = true;
  DeclareBindables();
}

Button::~Button()
{
  if (text)
    g_free(text);
  if (value)
    g_free(value);
  if (unit)
    g_free(unit);
  if (right)
    g_free(right);
}

void Button::Draw()
{
  ProceedUpdateArea();

  if (!area || !text)
    return;

  int attrs;
  if (has_focus)
    attrs = GetColorPair("button", "focus") | Curses::Attr::REVERSE;
  else
    attrs = GetColorPair("button", "normal");
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
  if (flags & FLAG_VALUE) {
    area->fill(attrs, l, 0, value_width + 2, realh);
    if (h < realh) {
      l += area->mvaddstring(l, h, realw - l, ": ");
      if (value)
        l += area->mvaddstring(l, h, realw - l, value);
    }
  }

  // print unit text
  if (flags & FLAG_UNIT && unit) {
    area->fill(attrs, l, 0, unit_width + 1, realh);
    if (h < realh) {
      l += area->mvaddstring(l, h, realw - l, " ");
      l += area->mvaddstring(l, h, realw - l, unit);
    }
  }

  area->attroff(attrs);

  // print right area text
  if (flags & FLAG_RIGHT && right && h < realh) {
    const char *cur = right;
    int width = right_width;
    while (width > realw - l - 1) {
      width -= Curses::onscreen_width(g_utf8_get_char(cur));
      cur = g_utf8_next_char(cur);
    }
    area->mvaddstring(realw - width, h, cur);
  }
}

void Button::SetFlags(int new_flags)
{
  if (flags == new_flags)
    return;

  flags = new_flags;
  Redraw();
}

void Button::SetText(const char *new_text)
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

void Button::SetValue(const char *new_value)
{
  if (value)
    g_free(value);

  value = g_strdup(new_value);
  value_width = Curses::onscreen_width(value);
  Redraw();
}

void Button::SetValue(int new_value)
{
  if (value)
    g_free(value);

  value = g_strdup_printf("%d", new_value);
  value_width = Curses::onscreen_width(value);
  Redraw();
}

void Button::SetUnit(const char *new_unit)
{
  if (unit)
    g_free(unit);

  unit = g_strdup(new_unit);
  unit_width = Curses::onscreen_width(unit);
  Redraw();
}

void Button::SetRight(const char *new_right)
{
  if (right)
    g_free(right);

  right = g_strdup(new_right);
  right_width = Curses::onscreen_width(right);
  Redraw();
}

void Button::ActionActivate()
{
  signal_activate(*this);
}

void Button::DeclareBindables()
{
  DeclareBindable("button", "activate", sigc::mem_fun(this,
        &Button::ActionActivate), InputProcessor::BINDABLE_NORMAL);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 expandtab */
