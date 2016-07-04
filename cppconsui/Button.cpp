// Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
// Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
//
// This file is part of CenterIM.
//
// CenterIM is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// CenterIM is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/// @file
/// Button class implementation.
///
/// @ingroup cppconsui

#include "Button.h"

#include "ColorScheme.h"

#include <cassert>
#include <cstdio>
#include <cstring>

namespace CppConsUI {

Button::Button(int w, int h, const char *text, int flags, bool masked)
  : Widget(w, h), flags_(flags), text_(nullptr), text_width_(0),
    text_height_(0), value_(nullptr), value_width_(0), unit_(nullptr),
    unit_width_(0), right_(nullptr), right_width_(0), masked_(masked)
{
  setText(text);
  setValue(static_cast<char *>(nullptr));
  setUnit(nullptr);
  setRight(nullptr);

  can_focus_ = true;
  declareBindables();
}

Button::Button(const char *text, int flags, bool masked)
  : Widget(AUTOSIZE, AUTOSIZE), flags_(flags), text_(nullptr), text_width_(0),
    text_height_(0), value_(nullptr), value_width_(0), unit_(nullptr),
    unit_width_(0), right_(nullptr), right_width_(0), masked_(masked)
{
  setText(text);
  setValue(static_cast<char *>(nullptr));
  setUnit(nullptr);
  setRight(nullptr);

  can_focus_ = true;
  declareBindables();
}

Button::Button(int w, int h, int flags, const char *text, const char *value,
  const char *unit, const char *right, bool masked)
  : Widget(w, h), flags_(flags), text_(nullptr), text_width_(0),
    text_height_(0), value_(nullptr), value_width_(0), unit_(nullptr),
    unit_width_(0), right_(nullptr), right_width_(0), masked_(masked)
{
  setText(text);
  setValue(value);
  setUnit(unit);
  setRight(right);

  can_focus_ = true;
  declareBindables();
}

Button::Button(int flags, const char *text, const char *value, const char *unit,
  const char *right, bool masked)
  : Widget(AUTOSIZE, AUTOSIZE), flags_(flags), text_(nullptr), text_width_(0),
    text_height_(0), value_(nullptr), value_width_(0), unit_(nullptr),
    unit_width_(0), right_(nullptr), right_width_(0), masked_(masked)
{
  setText(text);
  setValue(value);
  setUnit(unit);
  setRight(right);

  can_focus_ = true;
  declareBindables();
}

Button::~Button()
{
  delete[] text_;
  delete[] value_;
  delete[] unit_;
  delete[] right_;
}

int Button::draw(Curses::ViewPort area, Error &error)
{
  assert(text_ != nullptr);
  assert(value_ != nullptr);
  assert(unit_ != nullptr);
  assert(right_ != nullptr);

  int attrs;
  if (has_focus_) {
    DRAW(getAttributes(ColorScheme::PROPERTY_BUTTON_FOCUS, &attrs, error));
    attrs |= Curses::Attr::REVERSE;
  }
  else
    DRAW(getAttributes(ColorScheme::PROPERTY_BUTTON_NORMAL, &attrs, error));
  DRAW(area.attrOn(attrs, error));

  // Print text.
  DRAW(area.fill(attrs, 0, 0, text_width_, real_height_, error));
  int y = 0;
  const char *start, *end;
  start = end = text_;
  while (*end != '\0') {
    if (*end == '\n') {
      DRAW(area.addString(0, y, real_width_, start, end, error));
      ++y;
      start = end + 1;
    }
    ++end;
  }
  DRAW(area.addString(0, y, real_width_, start, end, error));

  int l = text_width_;
  int h = (text_height_ - 1) / 2;
  int printed;

  // Print value.
  if (flags_ & FLAG_VALUE) {
    DRAW(area.fill(attrs, l, 0, value_width_ + 2, real_height_, error));
    DRAW(area.addString(l, h, real_width_ - l, ": ", error, &printed));
    l += printed;

    if (masked_) {
      int count = value_width_;
      while (count--) {
        DRAW(area.addString(l, h, real_width_ - l, "*", error, &printed));
        l += printed;
      }
    }
    else {
      DRAW(area.addString(l, h, real_width_ - l, value_, error, &printed));
      l += printed;
    }
  }

  // Print unit text.
  if (flags_ & FLAG_UNIT) {
    DRAW(area.fill(attrs, l, 0, unit_width_ + 1, real_height_, error));
    DRAW(area.addString(l, h, real_width_ - l, " ", error, &printed));
    l += printed;
    DRAW(area.addString(l, h, real_width_ - l, unit_, error, &printed));
    l += printed;
  }

  DRAW(area.attrOff(attrs, error));

  // Print right area text.
  if (flags_ & FLAG_RIGHT) {
    const char *cur = right_;
    int width = right_width_;
    while (width > real_width_ - l - 1) {
      width -= Curses::onScreenWidth(UTF8::getUniChar(cur));
      cur = UTF8::getNextChar(cur);
    }
    DRAW(area.addString(real_width_ - width, h, cur, error));
  }

  return 0;
}

void Button::setFlags(int new_flags)
{
  if (new_flags == flags_)
    return;

  flags_ = new_flags;
  redraw();
}

void Button::setMasked(bool new_masked)
{
  if (new_masked == masked_)
    return;

  masked_ = new_masked;
  redraw();
}

void Button::setText(const char *new_text)
{
  size_t size = 1;
  if (new_text != nullptr)
    size += std::strlen(new_text);
  auto new_storage = new char[size];
  if (new_text != nullptr)
    std::strcpy(new_storage, new_text);
  else
    new_storage[0] = '\0';

  delete[] text_;
  text_ = new_storage;

  // Update text_width_, text_height_ and wish height.
  text_width_ = 0;
  text_height_ = 1;

  const char *start, *end;
  start = end = text_;
  int w;
  while (*end != '\0') {
    if (*end == '\n') {
      w = Curses::onScreenWidth(start, end);
      if (w > text_width_)
        text_width_ = w;
      ++text_height_;
      start = end + 1;
    }
    ++end;
  }
  w = Curses::onScreenWidth(start, end);
  if (w > text_width_)
    text_width_ = w;
  setWishHeight(text_height_);

  redraw();
}

void Button::setValue(const char *new_value)
{
  size_t size = 1;
  if (new_value != nullptr)
    size += std::strlen(new_value);
  auto new_storage = new char[size];
  if (new_value != nullptr)
    std::strcpy(new_storage, new_value);
  else
    new_storage[0] = '\0';

  delete[] value_;
  value_ = new_storage;

  value_width_ = Curses::onScreenWidth(value_);
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
  size_t size = 1;
  if (new_unit != nullptr)
    size += std::strlen(new_unit);
  auto new_storage = new char[size];
  if (new_unit != nullptr)
    std::strcpy(new_storage, new_unit);
  else
    new_storage[0] = '\0';

  delete[] unit_;
  unit_ = new_storage;

  unit_width_ = Curses::onScreenWidth(unit_);
  redraw();
}

void Button::setRight(const char *new_right)
{
  size_t size = 1;
  if (new_right != nullptr)
    size += std::strlen(new_right);
  auto new_storage = new char[size];
  if (new_right != nullptr)
    std::strcpy(new_storage, new_right);
  else
    new_storage[0] = '\0';

  delete[] right_;
  right_ = new_storage;

  right_width_ = Curses::onScreenWidth(right_);
  redraw();
}

void Button::actionActivate()
{
  signal_activate(*this);
}

void Button::declareBindables()
{
  declareBindable("button", "activate",
    sigc::mem_fun(this, &Button::actionActivate),
    InputProcessor::BINDABLE_NORMAL);
}

} // namespace CppConsUI

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
