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
/// CheckBox class implementation.
///
/// @ingroup cppconsui

#include "CheckBox.h"

#include "ColorScheme.h"
#include "Dialog.h"

#include <cassert>
#include <cstring>
#include "gettext.h"

namespace CppConsUI {

CheckBox::CheckBox(int w, int h, const char *text, bool checked)
  : Widget(w, h), text_(nullptr), text_width_(0), text_height_(0),
    checked_(checked)
{
  setText(text);

  can_focus_ = true;
  declareBindables();
}

CheckBox::CheckBox(const char *text, bool checked)
  : Widget(AUTOSIZE, AUTOSIZE), text_(nullptr), text_width_(0), text_height_(0),
    checked_(checked)
{
  setText(text);

  can_focus_ = true;
  declareBindables();
}

CheckBox::~CheckBox()
{
  delete[] text_;
}

int CheckBox::draw(Curses::ViewPort area, Error &error)
{
  assert(text_ != nullptr);

  int attrs;
  if (has_focus_) {
    DRAW(getAttributes(ColorScheme::PROPERTY_CHECKBOX_FOCUS, &attrs, error));
    attrs |= Curses::Attr::REVERSE;
  }
  else
    DRAW(getAttributes(ColorScheme::PROPERTY_CHECKBOX_NORMAL, &attrs, error));
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
  const char *value = checked_ ? YES_BUTTON_TEXT : NO_BUTTON_TEXT;
  int value_width = Curses::onScreenWidth(value);
  DRAW(area.fill(attrs, l, 0, value_width + 2, real_height_, error));
  DRAW(area.addString(l, h, real_width_ - l, ": ", error, &printed));
  l += printed;
  DRAW(area.addString(l, h, real_width_ - l, value, error));

  DRAW(area.attrOff(attrs, error));

  return 0;
}

void CheckBox::setText(const char *new_text)
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

void CheckBox::setChecked(bool new_checked)
{
  if (new_checked == checked_)
    return;

  checked_ = new_checked;
  signal_toggle(*this, checked_);
  redraw();
}

void CheckBox::actionToggle()
{
  setChecked(!checked_);
}

void CheckBox::declareBindables()
{
  declareBindable("checkbox", "toggle",
    sigc::mem_fun(this, &CheckBox::actionToggle),
    InputProcessor::BINDABLE_NORMAL);
}

} // namespace CppConsUI

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
