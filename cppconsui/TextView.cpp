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
 * TextView class implementation.
 *
 * @ingroup cppconsui
 */

#include "TextView.h"

#include "CppConsUI.h"

TextView::TextView(int w, int h, bool autoscroll_)
: Widget(w, h)
, view_top(0)
, autoscroll(autoscroll_)
{
}

TextView::~TextView()
{
  Clear();
}

void TextView::Append(const char *text, int color)
{
  Insert(lines.size(), text, color);
}

void TextView::Insert(int line_num, const char *text, int color)
{
  g_assert(text);
  g_assert(line_num >= 0);
  g_assert(line_num <= (int) lines.size());

  const char *p = text;
  const char *s = text;

  // parse lines
  int advice = 0;
  while (*p) {
    if (*p == '\n') {
      Line *l = new Line(s, p - s, color);
      lines.insert(lines.begin() + line_num, l);
      advice = UpdateScreenLines(line_num, advice);
      line_num++;
      s = p = g_utf8_next_char(p);
      continue;
    }

    p = g_utf8_next_char(p);
  }

  if (s < p) {
    Line *l = new Line(s, p - s, color);
    lines.insert(lines.begin() + line_num, l);
    advice = UpdateScreenLines(line_num, advice);
    line_num++;
  }

  Redraw();
}

void TextView::Erase(int line_num)
{
  g_assert(line_num >= 0);
  g_assert(line_num < (int) lines.size());

  /// @todo Untested...
  int deleted, pos;
  pos = EraseScreenLines(line_num, 0, &deleted);
  delete lines[line_num];
  lines.erase(lines.begin() + line_num);

  // adjust view_top
  if (view_top >= pos + deleted)
    view_top -= deleted;
  else if (view_top >= (int) screen_lines.size()) {
    if (screen_lines.size())
      view_top = screen_lines.size() - 1;
    else
      view_top = 0;
  }

  Redraw();
}

void TextView::Erase(int start_line, int end_line)
{
  g_assert(start_line >= 0);
  g_assert(start_line < (int) lines.size());
  g_assert(end_line >= 0);
  g_assert(end_line < (int) lines.size());
  g_assert(start_line <= end_line);

  /// @todo Untested...
  int deleted = 0;
  int advice = 0;
  for (int i = start_line; i < end_line; i++) {
    int d;
    advice = EraseScreenLines(i, advice, &d);
    deleted += d;
  }
  for (int i = start_line; i < end_line; i++)
    delete lines[i];
  lines.erase(lines.begin() + start_line, lines.begin() + end_line);

  // adjust view_top
  if (view_top >= advice + deleted)
    view_top -= deleted;
  else if (view_top >= (int) screen_lines.size()) {
    if (screen_lines.size())
      view_top = screen_lines.size() - 1;
    else
      view_top = 0;
  }

  Redraw();
}

void TextView::Clear()
{
  for (Lines::iterator i = lines.begin(); i != lines.end(); i++)
    delete *i;
  lines.clear();

  for (ScreenLines::iterator i = screen_lines.begin();
      i != screen_lines.end(); i++)
    delete *i;
  screen_lines.clear();

  view_top = 0;
  Redraw();
}

const char *TextView::GetLine(int line_num) const
{
  g_assert(line_num >= 0);
  g_assert(line_num < (int) lines.size());

  return lines[line_num]->text;
}

int TextView::GetLinesNumber() const
{
  return lines.size();
}

int TextView::ViewPosForLine(int line_num) const
{
  g_assert(line_num >= 0);
  g_assert(line_num < (int) lines.size());

  for (int i = 0; i < (int) screen_lines.size(); i++)
    if (screen_lines[i]->parent == lines[line_num])
      return i;

  return 0;
}

void TextView::SetViewPos(int viewy)
{
  g_assert(viewy >= 0);
  g_assert(viewy < (int) screen_lines.size());

  view_top = viewy;

  Redraw();
}

void TextView::Draw()
{
  int origw = area ? area->getmaxx() : 0;
  RealUpdateArea();

  if (!area || lines.empty())
    return;

  if (origw != area->getmaxx()) {
    // delete all screen lines
    for (ScreenLines::iterator i = screen_lines.begin();
        i != screen_lines.end(); i++)
      delete *i;
    screen_lines.clear();

    for (int i = 0, advice = 0; i < (int) lines.size(); i++)
      advice = UpdateScreenLines(i, advice);
  }

  area->erase();

  int realh = area->getmaxy();

  if (autoscroll && screen_lines.size()) {
    view_top = screen_lines.size() - area->getmaxy();
    if (view_top < 0)
      view_top = 0;
  }

  int attrs = GetColorPair("textview", "text");
  area->attron(attrs);

  ScreenLines::iterator i;
  int j;
  for (i = screen_lines.begin() + view_top, j = 0; i != screen_lines.end()
      && j < realh; i++, j++) {
    int attrs2 = 0;
    if ((*i)->parent->color) {
      char *color = g_strdup_printf("color%d", (*i)->parent->color);
      attrs2 = GetColorPair("textview", color);
      g_free(color);
      area->attroff(attrs);
      area->attron(attrs2);
    }

    area->mvaddstring(0, j, (*i)->width, (*i)->text);

    if (attrs2) {
      area->attroff(attrs2);
      area->attron(attrs);
    }
  }

  area->attroff(attrs);
}

const char *TextView::ProceedLine(const char *text, int area_width, int *res_width) const
{
  g_assert(text);
  g_assert(area_width > 0);
  g_assert(res_width);

  const char *cur = text;
  const char *res = text;
  int prev_width = 0;
  int cur_width = 0;
  gunichar uni;
  bool space = false;
  *res_width = 0;

  while (*cur) {
    prev_width = cur_width;
    uni = g_utf8_get_char(cur);
    cur_width += Curses::onscreen_width(uni);

    if (prev_width > area_width)
      break;

    // possibly too long word
    if (cur_width > area_width && !*res_width) {
      *res_width = prev_width;
      res = cur;
    }

    if (g_unichar_type(uni) == G_UNICODE_SPACE_SEPARATOR)
      space = true;
    else if (space) {
      /* Found start of a word and everything before that can fit into
       * a screen line. */
      *res_width = prev_width;
      res = cur;
      space = false;
    }

    cur = g_utf8_next_char(cur);
  }

  // end of text
  if (!*cur && cur_width <= area_width) {
    *res_width = cur_width;
    res = cur;
  }

  /* Fix for very small area_width and characters wider that 1 cell. For
   * example area_width = 1 and text = "W" where W is a wide character
   * (2 cells width). In that case we can not draw anything but we want to
   * skip to another character. */
  if (res == text)
    res = g_utf8_next_char(res);

  return res;
}

int TextView::UpdateScreenLines(int line_num, int start)
{
  g_assert(line_num >= 0);
  g_assert(line_num < (int) lines.size());
  g_assert(start >= 0);
  g_assert(start <= (int) screen_lines.size());

  /* Find where new screen lines should be placed and remove previous screen
   * lines created for this line. */
  ScreenLines::iterator i = screen_lines.begin() + EraseScreenLines(line_num,
      start);

  if (!area)
    return 0;

  // parse line into screen lines
  ScreenLines new_lines;
  const char *p = lines[line_num]->text;
  const char *s;
  int width;
  while (*p) {
    s = p;
    p = ProceedLine(p, area->getmaxx(), &width);
    new_lines.push_back(new ScreenLine(*lines[line_num], s, width));
  }

  // empty line
  if (new_lines.empty())
    new_lines.push_back(new ScreenLine(*lines[line_num], p, 0));

  int res = i - screen_lines.begin() + new_lines.size();
  screen_lines.insert(i, new_lines.begin(), new_lines.end());

  return res;
}

int TextView::EraseScreenLines(int line_num, int start, int *deleted)
{
  g_assert(line_num >= 0);
  g_assert(line_num < (int) lines.size());
  g_assert(start >= 0);
  g_assert(start <= (int) screen_lines.size());

  int i = start;
  int begin = -1;
  int end = -1;
  while (i < (int) screen_lines.size()) {
    if (screen_lines[i]->parent == lines[line_num]) {
      if (begin == -1)
        begin = i;
      delete screen_lines[i];
    }
    else if (begin != -1) {
      end = i;
      break;
    }
    i++;
  }
  if (begin != -1) {
    if (end == -1)
      end = screen_lines.size();
    screen_lines.erase(screen_lines.begin() + begin, screen_lines.begin()
        + end);
    i -= (end - begin);
  }

  if (deleted)
    *deleted = end - begin;

  return i;
}

TextView::Line::Line(const char *text_, int bytes, int color_)
: color(color_)
{
  g_assert(text_);
  g_assert(bytes >= 0);

  text = g_strndup(text_, bytes);
  length = g_utf8_strlen(text, -1);
}

TextView::Line::~Line()
{
  g_free(text);
}

TextView::ScreenLine::ScreenLine(Line &parent_, const char *text_, int width_)
: parent(&parent_), text(text_), width(width_)
{
}
