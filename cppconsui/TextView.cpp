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

#include "CoreManager.h"
#include "CppConsUI.h"

namespace CppConsUI
{

const TextView::ScrollBarElements TextView::scrollbar_elements_ascii = {
  "-", " ", " "
};

const TextView::ScrollBarElements TextView::scrollbar_elements_extended = {
  "\302\267", "\342\206\221", "\342\206\223"
};

TextView::TextView(int w, int h, bool autoscroll_, bool scrollbar_,
    ScrollBarStyle style)
: Widget(w, h), view_top(0), autoscroll(autoscroll_)
, autoscroll_suspended(false), scroll(0), scrollbar(scrollbar_)
, scrollbar_style(style) , dirty_lines(false), recalculate_screen_lines(false)
{
  can_focus = true;
  DeclareBindables();
}

TextView::~TextView()
{
  Clear();
}

void TextView::Draw()
{
  int origw = area ? area->getmaxx() : 0;
  ProceedUpdateArea();

  if (!area)
    return;

  int realw = area->getmaxx();
  int realh = area->getmaxy();

  if (origw != realw)
    recalculate_screen_lines = true;

  if (recalculate_screen_lines) {
    // delete all screen lines
    screen_lines.clear();

    /// @todo Save and restore scroll afterwards.
    for (size_t i = 0, advice = 0; i < lines.size(); i++)
      advice = UpdateScreenLines(i, advice);
    recalculate_screen_lines = false;
  }
  else if (dirty_lines) {
    for (size_t i = 0, advice = 0; i < lines.size(); i++)
      if (lines[i]->dirty)
        advice = UpdateScreenLines(i, advice);
  }
  dirty_lines = false;

  area->erase();

  if (screen_lines.size() <= static_cast<unsigned>(realh)) {
    view_top = 0;
    autoscroll_suspended = false;
  }
  else if (view_top > screen_lines.size() - realh) {
    view_top = screen_lines.size() - realh;
    autoscroll_suspended = false;
  }

  if (screen_lines.size() > static_cast<unsigned>(realh)) {
    if (scroll) {
      unsigned s = abs(scroll) * ((realh + 1) / 2);
      if (scroll < 0) {
        if (view_top < s)
          view_top = 0;
        else
          view_top -= s;
      }
      else {
        if (view_top + s > screen_lines.size() - realh)
          view_top = screen_lines.size() - realh;
        else
          view_top += s;
      }

      autoscroll_suspended = screen_lines.size() > view_top + realh;
    }

    if (autoscroll && !autoscroll_suspended)
      view_top = screen_lines.size() - realh;
  }
  scroll = 0;

  int attrs = GetColorPair("textview", "text");
  area->attron(attrs);

  ScreenLines::iterator i;
  int j;
  for (i = screen_lines.begin() + view_top, j = 0; i != screen_lines.end()
      && j < realh; i++, j++) {
    int attrs2 = 0;
    if (i->parent->color) {
      char color[32];
      int w = g_snprintf(color, sizeof(color), "color%d", i->parent->color);
      g_assert(static_cast<int>(sizeof(color)) >= w); // just in case
      attrs2 = GetColorPair("textview", color);
      area->attroff(attrs);
      area->attron(attrs2);
    }

    const char *p = i->text;
    int w = 0;
    for (int k = 0; k < i->length; k++) {
      gunichar uc = g_utf8_get_char(p);
      if (uc == '\t') {
        int t = Curses::onscreen_width(uc, w);
        for (int l = 0; l < t; l++)
          area->mvaddchar(w + l, j, ' ');
        w += t;
      }
      else
        w += area->mvaddchar(w, j, uc);
      p = g_utf8_next_char(p);
    }

    if (i->parent->color) {
      area->attroff(attrs2);
      area->attron(attrs);
    }
  }

  area->attroff(attrs);

  // draw scrollbar
  if (scrollbar) {
    const ScrollBarElements *elems;
    if (scrollbar_style == ASCII || COREMANAGER->IsFallbackDrawMode())
      elems = &scrollbar_elements_ascii;
    else
      elems = &scrollbar_elements_extended;

    int x1, x2;
    if (screen_lines.size() <= static_cast<unsigned>(realh)) {
      x1 = 0;
      x2 = realh;
    }
    else {
      x2 = static_cast<float>(view_top + realh) * realh / screen_lines.size();
      /* Calculate x1 based on x2 (not based on view_top) to avoid jittering
       * during rounding. */
      x1 = x2 - realh * realh / screen_lines.size();
    }

    int attrs = GetColorPair("textview", "scrollbar") | Curses::Attr::REVERSE;
    area->attron(attrs);

    for (int i = x1 + 1; i < x2 - 1; i++)
      area->mvaddstring(realw - 1, i, " ");

    if (x2 - x1 < 2) {
      /* This is a special case when x1 is too close to x2, but we need to
       * draw at least two arrows. */
      if (realh - x1 < 2) {
        // we are close to bottom position
        area->mvaddstring(realw - 1, realh - 2, elems->upa);
        area->mvaddstring(realw - 1, realh - 1, elems->downa);
      }
      else if (x2 < 2) {
        // we are close to top position
        area->mvaddstring(realw - 1, 0, elems->upa);
        area->mvaddstring(realw - 1, 1, elems->downa);
      }
      else {
        // in between
        area->mvaddstring(realw - 1, x2 - 2, elems->upa);
        area->mvaddstring(realw - 1, x2 - 1, elems->downa);
      }
    }
    else {
      // scrollbar length is big enough to fit two arrows
      area->mvaddstring(realw - 1, x1, elems->upa);
      area->mvaddstring(realw - 1, x2 - 1, elems->downa);
    }

    // draw a dot to indicate "end of scrolling" for user
    if (view_top + realh >= screen_lines.size())
      area->mvaddstring(realw - 1, realh - 1, elems->mchar);
    if (view_top == 0)
      area->mvaddstring(realw - 1, 0, elems->mchar);

    area->attroff(attrs);
  }

  /*
  char pos[128];
  g_snprintf(pos, sizeof(pos), "%d/%d ", view_top, screen_lines.size());
  area->mvaddstring(0, 0, pos);
  */
}

void TextView::Append(const char *text, int color)
{
  Insert(lines.size(), text, color);
}

void TextView::Insert(size_t line_num, const char *text, int color)
{
  g_assert(text);
  g_assert(line_num <= lines.size());

  const char *p = text;
  const char *s = text;

  // parse lines
  while (*p) {
    if (*p == '\n') {
      Line *l = new Line(s, p - s, color);
      lines.insert(lines.begin() + line_num, l);
      line_num++;
      s = p = g_utf8_next_char(p);
      continue;
    }

    p = g_utf8_next_char(p);
  }

  if (s < p) {
    Line *l = new Line(s, p - s, color);
    lines.insert(lines.begin() + line_num, l);
    line_num++;
  }

  dirty_lines = true;
  Redraw();
}

void TextView::Erase(size_t line_num)
{
  g_assert(line_num < lines.size());

  EraseScreenLines(line_num, 0);
  delete lines[line_num];
  lines.erase(lines.begin() + line_num);

  Redraw();
}

void TextView::Erase(size_t start_line, size_t end_line)
{
  g_assert(start_line < lines.size());
  g_assert(end_line <= lines.size());
  g_assert(start_line <= end_line);

  size_t advice = 0;
  for (size_t i = start_line; i < end_line; i++)
    advice = EraseScreenLines(i, advice);
  for (size_t i = start_line; i < end_line; i++)
    delete lines[i];
  lines.erase(lines.begin() + start_line, lines.begin() + end_line);

  Redraw();
}

void TextView::Clear()
{
  for (Lines::iterator i = lines.begin(); i != lines.end(); i++)
    delete *i;
  lines.clear();

  screen_lines.clear();

  Redraw();
}

const char *TextView::GetLine(size_t line_num) const
{
  g_assert(line_num < lines.size());

  return lines[line_num]->text;
}

size_t TextView::GetLinesNumber() const
{
  return lines.size();
}

void TextView::SetAutoScroll(bool enabled)
{
  if (autoscroll == enabled)
    return;

  autoscroll = enabled;
  Redraw();
}

void TextView::SetScrollBar(bool enabled)
{
  if (scrollbar == enabled)
    return;

  scrollbar = enabled;
  recalculate_screen_lines = true;
  Redraw();
}

void TextView::SetScrollBarStyle(ScrollBarStyle style)
{
  if (scrollbar_style == style)
    return;

  scrollbar_style = style;
  if (scrollbar)
    Redraw();
}

TextView::Line::Line(const char *text_, size_t bytes, int color_, bool dirty_)
: color(color_), dirty(dirty_)
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

TextView::ScreenLine::ScreenLine(Line &parent_, const char *text_,
    int length_)
: parent(&parent_), text(text_), length(length_)
{
}

const char *TextView::ProceedLine(const char *text, int area_width,
    int *res_length) const
{
  g_assert(text);
  g_assert(area_width > 0);
  g_assert(res_length);

  const char *cur = text;
  const char *res = text;
  int prev_width = 0;
  int cur_width = 0;
  int cur_length = 0;
  bool space = false;
  *res_length = 0;

  while (*cur) {
    prev_width = cur_width;
    gunichar uc = g_utf8_get_char(cur);
    cur_width += Curses::onscreen_width(uc, cur_width);
    cur_length++;

    if (prev_width > area_width)
      break;

    // possibly too long word
    if (cur_width > area_width && !*res_length) {
      *res_length = cur_length - 1;
      res = cur;
    }

    if (g_unichar_isspace(uc))
      space = true;
    else if (space) {
      /* Found start of a word and everything before that can fit into
       * a screen line. */
      *res_length = cur_length - 1;
      res = cur;
      space = false;
    }

    cur = g_utf8_next_char(cur);
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
    res = g_utf8_next_char(res);

  return res;
}

size_t TextView::UpdateScreenLines(size_t line_num, size_t start)
{
  g_assert(line_num < lines.size());
  g_assert(start <= screen_lines.size());

  /* Find where new screen lines should be placed and remove previous screen
   * lines created for this line. */
  ScreenLines::iterator i = screen_lines.begin() + EraseScreenLines(line_num,
      start);

  lines[line_num]->dirty = false;

  if (!area)
    return 0;

  // parse line into screen lines
  ScreenLines new_lines;
  const char *p = lines[line_num]->text;
  const char *s;
  int realw = area->getmaxx();
  if (scrollbar && realw > 2)
    realw -= 2;
  int len;
  while (*p) {
    s = p;
    p = ProceedLine(p, realw, &len);
    new_lines.push_back(ScreenLine(*lines[line_num], s, len));
  }

  // empty line
  if (new_lines.empty())
    new_lines.push_back(ScreenLine(*lines[line_num], p, 0));

  size_t res = i - screen_lines.begin() + new_lines.size();
  screen_lines.insert(i, new_lines.begin(), new_lines.end());

  return res;
}

size_t TextView::EraseScreenLines(size_t line_num, size_t start,
    size_t *deleted)
{
  g_assert(line_num < lines.size());
  g_assert(start <= screen_lines.size());

  size_t i = start;
  bool begin_set = false, end_set = false;
  size_t begin, end;
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
    screen_lines.erase(screen_lines.begin() + begin, screen_lines.begin()
        + end);
    i -= (end - begin);
    if (deleted)
      *deleted = end - begin;
  }
  else if (deleted)
    deleted = 0;

  return i;
}

void TextView::ActionScroll(int direction)
{
  scroll += direction;
  Redraw();
}

void TextView::DeclareBindables()
{
  DeclareBindable("textview", "scroll-up",
      sigc::bind(sigc::mem_fun(this, &TextView::ActionScroll), -1),
      InputProcessor::BINDABLE_NORMAL);

  DeclareBindable("textview", "scroll-down",
      sigc::bind(sigc::mem_fun(this, &TextView::ActionScroll), 1),
      InputProcessor::BINDABLE_NORMAL);
}

} // namespace CppConsUI
