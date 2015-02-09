/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2015 by CenterIM developers
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
 * Button class implementation.
 *
 * @ingroup cppconsui
 */

#include "Button.h"

#include <cassert>
#include <cstdio>
#include <cstring>

namespace CppConsUI
{

Button::Button(int w, int h, const char *text_, int flags_, bool masked_)
: Widget(w, h), flags(flags_), text(NULL), text_width(0), text_height(0)
, value(NULL), value_width(0), unit(NULL) , unit_width(0), right(NULL)
, right_width(0), masked(masked_)
{
  setText(text_);
  setValue((char*)NULL);
  setUnit(NULL);
  setRight(NULL);

  can_focus = true;
  declareBindables();
}

Button::Button(const char *text_, int flags_, bool masked_)
: Widget(AUTOSIZE, AUTOSIZE), flags(flags_), text(NULL), text_width(0)
, text_height(0), value(NULL), value_width(0), unit(NULL), unit_width(0)
, right(NULL), right_width(0), masked(masked_)
{
  setText(text_);
  setValue((char*)NULL);
  setUnit(NULL);
  setRight(NULL);

  can_focus = true;
  declareBindables();
}

Button::Button(int w, int h, int flags_, const char *text_,
    const char *value_, const char *unit_, const char *right_, bool masked_)
: Widget(w, h), flags(flags_), text(NULL), text_width(0), text_height(0)
, value(NULL), value_width(0), unit(NULL), unit_width(0), right(NULL)
, right_width(0), masked(masked_)
{
  setText(text_);
  setValue(value_);
  setUnit(unit_);
  setRight(right_);

  can_focus = true;
  declareBindables();
}

Button::Button(int flags_, const char *text_, const char *value_,
    const char *unit_, const char *right_, bool masked_)
: Widget(AUTOSIZE, AUTOSIZE), flags(flags_), text(NULL), text_width(0)
, text_height(0), value(NULL), value_width(0), unit(NULL), unit_width(0)
, right(NULL), right_width(0), masked(masked_)
{
  setText(text_);
  setValue(value_);
  setUnit(unit_);
  setRight(right_);

  can_focus = true;
  declareBindables();
}

Button::~Button()
{
  delete [] text;
  delete [] value;
  delete [] unit;
  delete [] right;
}

void Button::draw(Curses::ViewPort area)
{
  assert(text);
  assert(value);
  assert(unit);
  assert(right);

  int attrs;
  if (has_focus)
    attrs = getColorPair("button", "focus") | Curses::Attr::REVERSE;
  else
    attrs = getColorPair("button", "normal");
  area.attrOn(attrs);

  // print text
  area.fill(attrs, 0, 0, text_width, real_height);
  int y = 0;
  const char *start, *end;
  start = end = text;
  while (*end) {
    if (*end == '\n') {
      area.addString(0, y, real_width, start, end);
      ++y;
      start = end + 1;
    }
    ++end;
  }
  area.addString(0, y, real_width, start, end);

  int l = text_width;
  int h = (text_height - 1) / 2;
  int printed;

  // print value
  if (flags & FLAG_VALUE) {
    area.fill(attrs, l, 0, value_width + 2, real_height);
    area.addString(l, h, real_width - l, ": ", &printed);
    l += printed;

    if (masked) {
      int count = value_width;
      while (count--) {
        area.addString(l, h, real_width - l, "*", &printed);
        l += printed;
      }
    }
    else {
      area.addString(l, h, real_width - l, value, &printed);
      l += printed;
    }
  }

  // print unit text
  if (flags & FLAG_UNIT) {
    area.fill(attrs, l, 0, unit_width + 1, real_height);
    area.addString(l, h, real_width - l, " ", &printed);
    l += printed;
    area.addString(l, h, real_width - l, unit, &printed);
    l += printed;
  }

  area.attrOff(attrs);

  // print right area text
  if (flags & FLAG_RIGHT) {
    const char *cur = right;
    int width = right_width;
    while (width > real_width - l - 1) {
      width -= Curses::onScreenWidth(UTF8::getUniChar(cur));
      cur = UTF8::getNextChar(cur);
    }
    area.addString(real_width - width, h, cur);
  }
}

void Button::setFlags(int new_flags)
{
  if (new_flags == flags)
    return;

  flags = new_flags;
  redraw();
}

void Button::setMasked(bool new_masked)
{
  if (new_masked == masked)
    return;

  masked = new_masked;
  redraw();
}

void Button::setText(const char *new_text)
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

void Button::setValue(const char *new_value)
{
  delete [] value;

  size_t size = 1;
  if (new_value)
    size += std::strlen(new_value);
  value = new char[size];

  if (new_value)
    std::strcpy(value, new_value);
  else
    value[0] = '\0';
  value_width = Curses::onScreenWidth(value);
  redraw();
}

void Button::setValue(int new_value)
{
  char tmp[PRINTF_WIDTH(int) + 1];
  std::sprintf(tmp, "%d", new_value);
  setValue(tmp);
}

void Button::setUnit(const char *new_unit)
{
  delete [] unit;

  size_t size = 1;
  if (new_unit)
    size += std::strlen(new_unit);
  unit = new char[size];

  if (new_unit)
    std::strcpy(unit, new_unit);
  else
    unit[0] = '\0';
  unit_width = Curses::onScreenWidth(unit);
  redraw();
}

void Button::setRight(const char *new_right)
{
  delete [] right;

  size_t size = 1;
  if (new_right)
    size += std::strlen(new_right);
  right = new char[size];

  if (new_right)
    std::strcpy(right, new_right);
  else
    right[0] = '\0';
  right_width = Curses::onScreenWidth(right);
  redraw();
}

void Button::actionActivate()
{
  signal_activate(*this);
}

void Button::declareBindables()
{
  declareBindable("button", "activate", sigc::mem_fun(this,
        &Button::actionActivate), InputProcessor::BINDABLE_NORMAL);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
