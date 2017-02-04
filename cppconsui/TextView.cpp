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
// along with CenterIM.  If not, see <http://www.gnu.org/licenses/>.

/// @file
/// TextView class implementation.
///
/// @ingroup cppconsui

#include "TextView.h"

#include "ColorScheme.h"

#include <cassert>
#include <cstdio>
#include <cstring>

namespace CppConsUI {

TextView::TextView(int w, int h, bool autoscroll, bool scrollbar)
  : Widget(w, h), view_top_(0), autoscroll_(autoscroll),
    autoscroll_suspended_(false), scrollbar_(scrollbar)
{
  can_focus_ = true;
  declareBindables();
}

TextView::~TextView()
{
  clear();
}

int TextView::draw(Curses::ViewPort area, Error &error)
{
  DRAW(area.erase(error));

  if (screen_lines_.size() <= static_cast<unsigned>(real_height_)) {
    view_top_ = 0;
    autoscroll_suspended_ = false;
  }
  else if (view_top_ > screen_lines_.size() - real_height_) {
    view_top_ = screen_lines_.size() - real_height_;
    autoscroll_suspended_ = false;
  }
  else if (autoscroll_ && !autoscroll_suspended_)
    view_top_ = screen_lines_.size() - real_height_;

  int attrs;
  DRAW(getAttributes(ColorScheme::PROPERTY_TEXTVIEW_TEXT, &attrs, error));
  DRAW(area.attrOn(attrs, error));

  ScreenLines::iterator i;
  int j;
  for (i = screen_lines_.begin() + view_top_, j = 0;
       i != screen_lines_.end() && j < real_height_; ++i, ++j) {
    int attrs2 = 0;
    if (i->parent->color != 0) {
      DRAW(getAttributes(
        ColorScheme::PROPERTY_TEXTVIEW_TEXT, i->parent->color, &attrs2, error));
      DRAW(area.attrOff(attrs, error));
      DRAW(area.attrOn(attrs2, error));
    }

    const char *p = i->text;
    int w = 0;
    for (int k = 0; k < i->length; ++k) {
      UTF8::UniChar uc = UTF8::getUniChar(p);
      int printed;
      if (uc == '\t') {
        printed = Curses::onScreenWidth(uc, w);
        for (int l = 0; l < printed; ++l)
          DRAW(area.addChar(w + l, j, ' ', error));
      }
      else
        DRAW(area.addChar(w, j, uc, error, &printed));
      w += printed;
      p = UTF8::getNextChar(p);
    }

    if (i->parent->color != 0) {
      DRAW(area.attrOff(attrs2, error));
      DRAW(area.attrOn(attrs, error));
    }
  }

  DRAW(area.attrOff(attrs, error));

  // Draw scrollbar.
  if (scrollbar_) {
    int x1, x2;
    if (screen_lines_.size() <= static_cast<unsigned>(real_height_)) {
      x1 = 0;
      x2 = real_height_;
    }
    else {
      x2 = static_cast<float>(view_top_ + real_height_) * real_height_ /
        screen_lines_.size();
      // Calculate x1 based on x2 (not based on view_top_) to avoid jittering
      // during rounding.
      x1 = x2 - real_height_ * real_height_ / screen_lines_.size();
    }

    int attrs;
    DRAW(
      getAttributes(ColorScheme::PROPERTY_TEXTVIEW_SCROLLBAR, &attrs, error));
    attrs |= Curses::Attr::REVERSE;
    DRAW(area.attrOn(attrs, error));

    for (int i = x1 + 1; i < x2 - 1; ++i)
      DRAW(area.addChar(real_width_ - 1, i, ' ', error));

    if (x2 - x1 < 2) {
      // This is a special case when x1 is too close to x2, but we need to draw
      // at least two arrows.
      if (real_height_ - x1 < 2) {
        // We are close to bottom position.
        DRAW(area.addLineChar(
          real_width_ - 1, real_height_ - 2, Curses::LINE_UARROW, error));
        DRAW(area.addLineChar(
          real_width_ - 1, real_height_ - 1, Curses::LINE_DARROW, error));
      }
      else if (x2 < 2) {
        // We are close to top position.
        DRAW(area.addLineChar(real_width_ - 1, 0, Curses::LINE_UARROW, error));
        DRAW(area.addLineChar(real_width_ - 1, 1, Curses::LINE_DARROW, error));
      }
      else {
        // In between.
        DRAW(area.addLineChar(
          real_width_ - 1, x2 - 2, Curses::LINE_UARROW, error));
        DRAW(area.addLineChar(
          real_width_ - 1, x2 - 1, Curses::LINE_DARROW, error));
      }
    }
    else {
      // Scrollbar length is big enough to fit two arrows.
      DRAW(area.addLineChar(real_width_ - 1, x1, Curses::LINE_UARROW, error));
      DRAW(
        area.addLineChar(real_width_ - 1, x2 - 1, Curses::LINE_DARROW, error));
    }

    // Draw a dot to indicate "end of scrolling" for users.
    if (view_top_ + real_height_ >= screen_lines_.size())
      DRAW(area.addLineChar(
        real_width_ - 1, real_height_ - 1, Curses::LINE_BULLET, error));
    if (view_top_ == 0)
      DRAW(area.addLineChar(real_width_ - 1, 0, Curses::LINE_BULLET, error));

    DRAW(area.attrOff(attrs, error));
  }

  /*
  char pos[128];
  g_snprintf(pos, sizeof(pos), "%d/%d ", view_top_, screen_lines_.size());
  DRAW(area.addString(0, 0, pos));
  */

  return 0;
}

void TextView::append(const char *text, int color)
{
  insert(lines_.size(), text, color);
}

void TextView::insert(std::size_t line_num, const char *text, int color)
{
  if (text == nullptr)
    return;

  assert(line_num <= lines_.size());

  const char *p = text;
  const char *s = text;
  std::size_t cur_line_num = line_num;

  // Parse lines.
  while (*p != '\0') {
    if (*p == '\n') {
      auto l = new Line(s, p - s, color);
      lines_.insert(lines_.begin() + cur_line_num, l);
      ++cur_line_num;
      s = p = UTF8::getNextChar(p);
      continue;
    }

    p = UTF8::getNextChar(p);
  }

  if (s < p) {
    auto l = new Line(s, p - s, color);
    lines_.insert(lines_.begin() + cur_line_num, l);
    ++cur_line_num;
  }

  // Update screen lines.
  for (std::size_t i = line_num, advice = 0; i < cur_line_num; ++i)
    advice = updateScreenLines(i, advice);

  redraw();
}

void TextView::erase(std::size_t line_num)
{
  assert(line_num < lines_.size());

  eraseScreenLines(line_num, 0);
  delete lines_[line_num];
  lines_.erase(lines_.begin() + line_num);

  redraw();
}

void TextView::erase(std::size_t start_line, std::size_t end_line)
{
  assert(start_line < lines_.size());
  assert(end_line <= lines_.size());
  assert(start_line <= end_line);

  std::size_t advice = 0;
  for (std::size_t i = start_line; i < end_line; ++i)
    advice = eraseScreenLines(i, advice);
  for (std::size_t i = start_line; i < end_line; ++i)
    delete lines_[i];
  lines_.erase(lines_.begin() + start_line, lines_.begin() + end_line);

  redraw();
}

void TextView::clear()
{
  for (Line *line : lines_)
    delete line;
  lines_.clear();

  screen_lines_.clear();

  redraw();
}

const char *TextView::getLine(std::size_t line_num) const
{
  assert(line_num < lines_.size());

  return lines_[line_num]->text;
}

std::size_t TextView::getLinesNumber() const
{
  return lines_.size();
}

void TextView::setAutoScroll(bool new_autoscroll)
{
  if (new_autoscroll == autoscroll_)
    return;

  autoscroll_ = new_autoscroll;
  redraw();
}

void TextView::setScrollBar(bool new_scrollbar)
{
  if (new_scrollbar == scrollbar_)
    return;

  scrollbar_ = new_scrollbar;
  updateAllScreenLines();
  redraw();
}

TextView::Line::Line(const char *text_, std::size_t bytes, int color_)
  : color(color_)
{
  assert(text_ != nullptr);

  text = new char[bytes + 1];
  std::strncpy(text, text_, bytes);
  text[bytes] = '\0';

  length = 0;
  const char *p = text;
  while (p != nullptr && *p != '\0') {
    ++length;
    p = UTF8::getNextChar(p);
  }
}

TextView::Line::~Line()
{
  delete[] text;
}

TextView::ScreenLine::ScreenLine(Line &parent_, const char *text_, int length_)
  : parent(&parent_), text(text_), length(length_)
{
}

void TextView::updateArea()
{
  updateAllScreenLines();
}

const char *TextView::proceedLine(
  const char *text, int area_width, int *res_length) const
{
  assert(text != nullptr);
  assert(area_width > 0);
  assert(res_length != nullptr);

  const char *cur = text;
  const char *res = text;
  int prev_width = 0;
  int cur_width = 0;
  int cur_length = 0;
  bool space = false;
  *res_length = 0;

  while (*cur != '\0') {
    prev_width = cur_width;
    UTF8::UniChar uc = UTF8::getUniChar(cur);
    cur_width += Curses::onScreenWidth(uc, cur_width);
    ++cur_length;

    if (prev_width > area_width)
      break;

    // Possibly too long word.
    if (cur_width > area_width && *res_length == 0) {
      *res_length = cur_length - 1;
      res = cur;
    }

    if (UTF8::isUniCharSpace(uc))
      space = true;
    else if (space) {
      // Found start of a word and everything before that can fit into one
      // screen line.
      *res_length = cur_length - 1;
      res = cur;
      space = false;
    }

    cur = UTF8::getNextChar(cur);
  }

  // End of text.
  if (*cur == '\0' && cur_width <= area_width) {
    *res_length = cur_length;
    res = cur;
  }

  // Fix for very small area_width and characters wider that 1 cell. For
  // example, area_width = 1 and text = "W" where W is a wide character (2 cells
  // width) (or simply for tabs). In that case we can not draw anything but we
  // want to skip to another character.
  if (res == text)
    res = UTF8::getNextChar(res);

  return res;
}

std::size_t TextView::updateScreenLines(std::size_t line_num, std::size_t start)
{
  assert(line_num < lines_.size());
  assert(start <= screen_lines_.size());

  // Find where new screen lines should be placed and remove previous screen
  // lines created for this line.
  ScreenLines::iterator i =
    screen_lines_.begin() + eraseScreenLines(line_num, start);

  // Parse line into screen lines.
  ScreenLines new_lines;
  const char *p = lines_[line_num]->text;
  const char *s;

  int realw = real_width_;
  if (scrollbar_ && realw > 2) {
    // Scrollbar shrinks the width of the view area.
    realw -= 2;
  }

  if (realw <= 0)
    return 0;

  int len;
  while (*p != '\0') {
    s = p;
    p = proceedLine(p, realw, &len);
    new_lines.push_back(ScreenLine(*lines_[line_num], s, len));
  }

  // Empty line.
  if (new_lines.empty())
    new_lines.push_back(ScreenLine(*lines_[line_num], p, 0));

  std::size_t res = i - screen_lines_.begin() + new_lines.size();
  screen_lines_.insert(i, new_lines.begin(), new_lines.end());

  return res;
}

void TextView::updateAllScreenLines()
{
  // Delete all screen lines.
  screen_lines_.clear();

  /// @todo Save and restore scroll afterwards.
  for (std::size_t i = 0, advice = 0; i < lines_.size(); ++i)
    advice = updateScreenLines(i, advice);
}

std::size_t TextView::eraseScreenLines(
  std::size_t line_num, std::size_t start, std::size_t *deleted)
{
  assert(line_num < lines_.size());
  assert(start <= screen_lines_.size());

  std::size_t i = start;
  bool begin_set = false, end_set = false;
  // Note, the assigment to the begin variable is only to silence a compiler
  // warning. The use of the variable is protected by the begin_set variable.
  std::size_t begin = 0, end;
  while (i < screen_lines_.size()) {
    if (screen_lines_[i].parent == lines_[line_num]) {
      if (!begin_set) {
        begin = i;
        begin_set = true;
      }
    }
    else if (begin_set) {
      end = i;
      end_set = true;
      break;
    }
    ++i;
  }
  if (begin_set) {
    if (!end_set)
      end = screen_lines_.size();
    screen_lines_.erase(
      screen_lines_.begin() + begin, screen_lines_.begin() + end);
    i -= end - begin;
    if (deleted != nullptr)
      *deleted = end - begin;
  }
  else if (deleted != nullptr)
    deleted = nullptr;

  return i;
}

void TextView::actionScroll(int direction)
{
  if (screen_lines_.size() <= static_cast<unsigned>(real_height_))
    return;

  unsigned s = abs(direction) * ((real_height_ + 1) / 2);
  if (direction < 0) {
    if (view_top_ < s)
      view_top_ = 0;
    else
      view_top_ -= s;
  }
  else {
    if (view_top_ + s > screen_lines_.size() - real_height_)
      view_top_ = screen_lines_.size() - real_height_;
    else
      view_top_ += s;
  }

  autoscroll_suspended_ = screen_lines_.size() > view_top_ + real_height_;
  redraw();
}

void TextView::declareBindables()
{
  declareBindable("textview", "scroll-up",
    sigc::bind(sigc::mem_fun(this, &TextView::actionScroll), -1),
    InputProcessor::BINDABLE_NORMAL);

  declareBindable("textview", "scroll-down",
    sigc::bind(sigc::mem_fun(this, &TextView::actionScroll), 1),
    InputProcessor::BINDABLE_NORMAL);
}

} // namespace CppConsUI

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
