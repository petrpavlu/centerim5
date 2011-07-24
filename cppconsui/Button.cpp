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

#include "Keys.h"

#include "gettext.h"

namespace CppConsUI
{

Button::Button(int w, int h, const char *text_, int flags_)
: Widget(w, h), flags(flags_), text(NULL), value(NULL), unit(NULL)
, right(NULL), right_width(0)
{
  SetText(text_);

  can_focus = true;
  DeclareBindables();
}

Button::Button(const char *text_, int flags_)
: Widget(AUTOSIZE, 1), flags(flags_), text(NULL), value(NULL), unit(NULL)
, right(NULL), right_width(0)
{
  SetText(text_);

  can_focus = true;
  DeclareBindables();
}

Button::Button(int w, int h, int flags_, const char *text_,
    const char *value_, const char *unit_, const char *right_)
: Widget(w, h), flags(flags_), text(NULL), value(NULL), unit(NULL)
, right(NULL), right_width(0)
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
: Widget(AUTOSIZE, 1), flags(flags_), text(NULL), value(NULL), unit(NULL)
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
  if (has_focus) {
    attrs = GetColorPair("button", "focus") | Curses::Attr::REVERSE;
    area->attron(attrs);
  }
  else {
    attrs = GetColorPair("button", "normal");
    area->attron(attrs);
  }

  /**
   * @todo Though this is not a widget for long text there are some cases in
   * cim where we use it for a short but multiline text, so we should threat
   * LF specially here.
   */

  int max = area->getmaxx() * area->getmaxy();
  int l = area->mvaddstring(0, 0, max, text);
  if (flags & FLAG_VALUE) {
    l += area->mvaddstring(l, 0, max - l, ": ");
    if (value)
      l += area->mvaddstring(l, 0, max - l, value);
  }
  if (flags & FLAG_UNIT && unit) {
    l += area->mvaddstring(l, 0, max - l, " ");
    l += area->mvaddstring(l, 0, max - l, unit);
  }
  if (flags & FLAG_RIGHT && right) {
    const char *cur = right;
    int width = right_width;
    while (width > max - l - 1) {
      width -= Curses::onscreen_width(g_utf8_get_char(cur));
      cur = g_utf8_next_char(cur);
    }
    area->mvaddstring(max - width, 0, cur);
  }

  area->attroff(attrs);
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
  Redraw();
}

void Button::SetValue(const char *new_value)
{
  if (value)
    g_free(value);

  value = g_strdup(new_value);
  Redraw();
}

void Button::SetValue(int new_value)
{
  if (value)
    g_free(value);

  value = g_strdup_printf("%d", new_value);
  Redraw();
}

void Button::SetUnit(const char *new_unit)
{
  if (unit)
    g_free(unit);

  unit = g_strdup(new_unit);
  Redraw();
}

void Button::SetRight(const char *new_right)
{
  if (right)
    g_free(right);

  right = g_strdup(new_right);
  right_width = right ? Curses::onscreen_width(right) : 0;
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
