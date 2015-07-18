/*
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
 * TextView class implementation.
 *
 * @ingroup cppconsui
 */

#include "TextView.h"

#include <cassert>
#include <cstdio>
#include <cstring>

namespace CppConsUI {

TextView::TextView(int w, int h, bool autoscroll_, bool scrollbar_)
  : Widget(w, h), view_top(0), autoscroll(autoscroll_),
    autoscroll_suspended(false), scrollbar(scrollbar_)
{
  can_focus = true;
  declareBindables();
}

TextView::~TextView()
{
  clear();
}

void TextView::draw(Curses::ViewPort area)
{
  area.erase();

  if (screen_lines.size() <= static_cast<unsigned>(real_height)) {
    view_top = 0;
    autoscroll_suspended = false;
  }
  else if (view_top > screen_lines.size() - real_height) {
    view_top = screen_lines.size() - real_height;
    autoscroll_suspended = false;
  }
  else if (autoscroll && !autoscroll_suspended)
    view_top = screen_lines.size() - real_height;

  int attrs = getColorPair("textview", "text");
  area.attrOn(attrs);

  ScreenLines::iterator i;
  int j;
  for (i = screen_lines.begin() + view_top, j = 0;
       i != screen_lines.end() && j < real_height; i++, j++) {
    int attrs2 = 0;
    if (i->parent->color) {
      char color[sizeof("color") + PRINTF_WIDTH(int)];
      std::sprintf(color, "color%d", i->parent->color);
      attrs2 = getColorPair("textview", color);
      area.attrOff(attrs);
      area.attrOn(attrs2);
    }

    const char *p = i->text;
    int w = 0;
    for (int k = 0; k < i->length; k++) {
      UTF8::UniChar uc = UTF8::getUniChar(p);
      int printed;
      if (uc == '\t') {
        printed = Curses::onScreenWidth(uc, w);
        for (int l = 0; l < printed; l++)
          area.addChar(w + l, j, ' ');
      }
      else {
        area.addChar(w, j, uc, &printed);
      }
      w += printed;
      p = UTF8::getNextChar(p);
    }

    if (i->parent->color) {
      area.attrOff(attrs2);
      area.attrOn(attrs);
    }
  }

  area.attrOff(attrs);

  // draw scrollbar
  if (scrollbar) {
    int x1, x2;
    if (screen_lines.size() <= static_cast<unsigned>(real_height)) {
      x1 = 0;
      x2 = real_height;
    }
    else {
      x2 = static_cast<float>(view_top + real_height) * real_height /
        screen_lines.size();
      /* Calculate x1 based on x2 (not based on view_top) to avoid jittering
       * during rounding. */
      x1 = x2 - real_height * real_height / screen_lines.size();
    }

    int attrs = getColorPair("textview", "scrollbar") | Curses::Attr::REVERSE;
    area.attrOn(attrs);

    for (int i = x1 + 1; i < x2 - 1; i++)
      area.addString(real_width - 1, i, " ");

    if (x2 - x1 < 2) {
      /* This is a special case when x1 is too close to x2, but we need to
       * draw at least two arrows. */
      if (real_height - x1 < 2) {
        // we are close to bottom position
        area.addLineChar(real_width - 1, real_height - 2, Curses::LINE_UARROW);
        area.addLineChar(real_width - 1, real_height - 1, Curses::LINE_DARROW);
      }
      else if (x2 < 2) {
        // we are close to top position
        area.addLineChar(real_width - 1, 0, Curses::LINE_UARROW);
        area.addLineChar(real_width - 1, 1, Curses::LINE_DARROW);
      }
      else {
        // in between
        area.addLineChar(real_width - 1, x2 - 2, Curses::LINE_UARROW);
        area.addLineChar(real_width - 1, x2 - 1, Curses::LINE_DARROW);
      }
    }
    else {
      // scrollbar length is big enough to fit two arrows
      area.addLineChar(real_width - 1, x1, Curses::LINE_UARROW);
      area.addLineChar(real_width - 1, x2 - 1, Curses::LINE_DARROW);
    }

    // draw a dot to indicate "end of scrolling" for user
    if (view_top + real_height >= screen_lines.size())
      area.addLineChar(real_width - 1, real_height - 1, Curses::LINE_BULLET);
    if (view_top == 0)
      area.addLineChar(real_width - 1, 0, Curses::LINE_BULLET);

    area.attrOff(attrs);
  }

  /*
  char pos[128];
  g_snprintf(pos, sizeof(pos), "%d/%d ", view_top, screen_lines.size());
  area.addString(0, 0, pos);
  */
}

void TextView::append(const char *text, int color)
{
  insert(lines.size(), text, color);
}

void TextView::insert(size_t line_num, const char *text, int color)
{
  if (!text)
    return;

  assert(line_num <= lines.size());

  const char *p = text;
  const char *s = text;
  size_t cur_line_num = line_num;

  // parse lines
  while (*p) {
    if (*p == '\n') {
      Line *l = new Line(s, p - s, color);
      lines.insert(lines.begin() + cur_line_num, l);
      cur_line_num++;
      s = p = UTF8::getNextChar(p);
      continue;
    }

    p = UTF8::getNextChar(p);
  }

  if (s < p) {
    Line *l = new Line(s, p - s, color);
    lines.insert(lines.begin() + cur_line_num, l);
    cur_line_num++;
  }

  // update screen lines
  for (size_t i = line_num, advice = 0; i < cur_line_num; i++)
    advice = updateScreenLines(i, advice);

  redraw();
}

void TextView::erase(size_t line_num)
{
  assert(line_num < lines.size());

  eraseScreenLines(line_num, 0);
  delete lines[line_num];
  lines.erase(lines.begin() + line_num);

  redraw();
}

void TextView::erase(size_t start_line, size_t end_line)
{
  assert(start_line < lines.size());
  assert(end_line <= lines.size());
  assert(start_line <= end_line);

  size_t advice = 0;
  for (size_t i = start_line; i < end_line; i++)
    advice = eraseScreenLines(i, advice);
  for (size_t i = start_line; i < end_line; i++)
    delete lines[i];
  lines.erase(lines.begin() + start_line, lines.begin() + end_line);

  redraw();
}

void TextView::clear()
{
  for (Lines::iterator i = lines.begin(); i != lines.end(); i++)
    delete *i;
  lines.clear();

  screen_lines.clear();

  redraw();
}

const char *TextView::getLine(size_t line_num) const
{
  assert(line_num < lines.size());

  return lines[line_num]->text;
}

size_t TextView::getLinesNumber() const
{
  return lines.size();
}

void TextView::setAutoScroll(bool new_autoscroll)
{
  if (new_autoscroll == autoscroll)
    return;

  autoscroll = new_autoscroll;
  redraw();
}

void TextView::setScrollBar(bool new_scrollbar)
{
  if (new_scrollbar == scrollbar)
    return;

  scrollbar = new_scrollbar;
  updateAllScreenLines();
  redraw();
}

TextView::Line::Line(const char *text_, size_t bytes, int color_)
  : color(color_)
{
  assert(text_);

  text = new char[bytes + 1];
  std::strncpy(text, text_, bytes);
  text[bytes] = '\0';

  length = 0;
  const char *p = text;
  while (p && *p) {
    length++;
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
  assert(text);
  assert(area_width > 0);
  assert(res_length);

  const char *cur = text;
  const char *res = text;
  int prev_width = 0;
  int cur_width = 0;
  int cur_length = 0;
  bool space = false;
  *res_length = 0;

  while (*cur) {
    prev_width = cur_width;
    UTF8::UniChar uc = UTF8::getUniChar(cur);
    cur_width += Curses::onScreenWidth(uc, cur_width);
    cur_length++;

    if (prev_width > area_width)
      break;

    // possibly too long word
    if (cur_width > area_width && !*res_length) {
      *res_length = cur_length - 1;
      res = cur;
    }

    if (UTF8::isUniCharSpace(uc))
      space = true;
    else if (space) {
      /* Found start of a word and everything before that can fit into
       * a screen line. */
      *res_length = cur_length - 1;
      res = cur;
      space = false;
    }

    cur = UTF8::getNextChar(cur);
  }

  // end of text
  if (!*cur && cur_width <= area_width) {
    *res_length = cur_length;
    res = cur;
  }

  /* Fix for very small area_width and characters wider that 1 cell. For
   * example, area_width = 1 and text = "W" where W is a wide character (2
   * cells width) (or simply for tabs). In that case we can not draw anything
   * but we want to skip to another character. */
  if (res == text)
    res = UTF8::getNextChar(res);

  return res;
}

size_t TextView::updateScreenLines(size_t line_num, size_t start)
{
  assert(line_num < lines.size());
  assert(start <= screen_lines.size());

  /* Find where new screen lines should be placed and remove previous screen
   * lines created for this line. */
  ScreenLines::iterator i =
    screen_lines.begin() + eraseScreenLines(line_num, start);

  // parse line into screen lines
  ScreenLines new_lines;
  const char *p = lines[line_num]->text;
  const char *s;

  int realw = real_width;
  if (scrollbar && realw > 2) {
    // scrollbar shrinks the width of the view area
    realw -= 2;
  }

  if (realw <= 0)
    return 0;

  int len;
  while (*p) {
    s = p;
    p = proceedLine(p, realw, &len);
    new_lines.push_back(ScreenLine(*lines[line_num], s, len));
  }

  // empty line
  if (new_lines.empty())
    new_lines.push_back(ScreenLine(*lines[line_num], p, 0));

  size_t res = i - screen_lines.begin() + new_lines.size();
  screen_lines.insert(i, new_lines.begin(), new_lines.end());

  return res;
}

void TextView::updateAllScreenLines()
{
  // delete all screen lines
  screen_lines.clear();

  /// @todo Save and restore scroll afterwards.
  for (size_t i = 0, advice = 0; i < lines.size(); i++)
    advice = updateScreenLines(i, advice);
}

size_t TextView::eraseScreenLines(
  size_t line_num, size_t start, size_t *deleted)
{
  assert(line_num < lines.size());
  assert(start <= screen_lines.size());

  size_t i = start;
  bool begin_set = false, end_set = false;
  /* Note, the assigment to the begin variable is only to silence a compiler
   * warning. The use of the variable is protected by the begin_set variable.
   * */
  size_t begin = 0, end;
  while (i < screen_lines.size()) {
    if (screen_lines[i].parent == lines[line_num]) {
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
    i++;
  }
  if (begin_set) {
    if (!end_set)
      end = screen_lines.size();
    screen_lines.erase(
      screen_lines.begin() + begin, screen_lines.begin() + end);
    i -= (end - begin);
    if (deleted)
      *deleted = end - begin;
  }
  else if (deleted)
    deleted = 0;

  return i;
}

void TextView::actionScroll(int direction)
{
  if (screen_lines.size() <= static_cast<unsigned>(real_height))
    return;

  unsigned s = abs(direction) * ((real_height + 1) / 2);
  if (direction < 0) {
    if (view_top < s)
      view_top = 0;
    else
      view_top -= s;
  }
  else {
    if (view_top + s > screen_lines.size() - real_height)
      view_top = screen_lines.size() - real_height;
    else
      view_top += s;
  }

  autoscroll_suspended = screen_lines.size() > view_top + real_height;
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

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
