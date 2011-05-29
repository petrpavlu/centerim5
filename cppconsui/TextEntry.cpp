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

/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Ported to c++ and modified for cppconsui/centerim.
 */

/**
 * @file
 * TextEntry class implementation
 *
 * @ingroup cppconsui
 */

#include "TextEntry.h"

#include "Container.h"
#include "Keys.h"

#include <glib.h>
#include <string.h>
#include "gettext.h"

namespace CppConsUI
{

TextEntry::TextEntry(int w, int h, const char *text_)
: Widget(w, h)
, text(NULL)
, current_pos(0)
, editable(true)
, flags(0)
, text_max_length(MAX_SIZE)
{
  SetText(text_);

  can_focus = true;
  DeclareBindables();
}

TextEntry::TextEntry(const char *text_)
: Widget(AUTOSIZE, 1)
, text(NULL)
, current_pos(0)
, editable(true)
, flags(0)
, text_max_length(MAX_SIZE)
{
  SetText(text_);

  can_focus = true;
  DeclareBindables();
}

TextEntry::~TextEntry()
{
  if (text)
    g_free(text);
}

void TextEntry::Draw()
{
  RealUpdateArea();

  if (!area)
    return;

  /// @todo We can do better than this.

  int attrs = GetColorPair("textentry", "text");
  area->attron(attrs);

  int max = area->getmaxx() * area->getmaxy();
  area->mvaddstring(0, 0, max, text);

  area->attroff(attrs);

  if (has_focus) {
    int realw = area->getmaxx();
    char *ptr = g_utf8_offset_to_pointer(text, current_pos);
    int sc_x = Curses::onscreen_width(text, ptr);
    int sc_y = sc_x / realw;
    sc_x -= sc_y * realw;
    area->mvchgat(sc_x, sc_y, 1, Curses::Attr::REVERSE, 0, NULL);
  }
}

bool TextEntry::ProcessInputText(const TermKeyKey &key)
{
  // tab moves a focus
  if (key.code.codepoint == '\t')
    return false;

  // filter out unwanted input
  if (flags) {
    if (!(flags & FLAG_ALPHABETIC) && g_unichar_isalpha(key.code.codepoint))
      return false;
    if (!(flags & FLAG_NUMERIC) && g_unichar_isdigit(key.code.codepoint))
      return false;
    /// @todo move
    if (!(flags & FLAG_NOSPACE) && g_unichar_isspace(key.code.codepoint))
      return false;
    if (!(flags & FLAG_NOPUNCTUATION) && g_unichar_ispunct(key.code.codepoint))
      return false;
  }

  if (editable) {
    InsertTextAtCursor(key.utf8);
    return true;
  }

  return false;
}

void TextEntry::SetText(const char *new_text)
{
  if (text)
    g_free(text);

  text = g_strdup(new_text ? new_text : "");

  RecalculateLengths();
  current_pos = text_length;

  Redraw();
}

void TextEntry::SetFlags(int flags_)
{
  flags = flags_;

  /// @todo validate text using new flags?
}

void TextEntry::SetPosition(int position)
{
  if (position < 0)
    position = 0;
  else if (position > text_length)
    position = text_length;

  current_pos = position;
}

void TextEntry::InsertTextAtCursor(const char *new_text, int new_text_bytes)
{
  int index;
  int n_chars;

  g_assert(new_text);

  if (new_text_bytes == -1)
    new_text_bytes = strlen(new_text);

  n_chars = g_utf8_strlen(new_text, new_text_bytes);

  /* Test if adding new text wouldn't overflow text_max_length and if so
   * then add only characters that can still fit in. */
  if (n_chars + text_length > text_max_length) {
    /// @todo flash/blink display, or just beep? (a la screen)
    Curses::beep();

    n_chars = text_max_length - text_length;
    new_text_bytes = g_utf8_offset_to_pointer(new_text, n_chars) - new_text;
  }

  // extend text allocated memory if necessary
  if (new_text_bytes + text_bytes + 1 > text_size) {
    int prev_size = text_size;

    while (new_text_bytes + text_bytes + 1 > text_size) {
      if (text_size == 0)
        text_size = MIN_SIZE;
      else {
        if (2 * text_size < MAX_SIZE)
          text_size *= 2;
        else {
          text_size = MAX_SIZE;
          if (new_text_bytes > text_size - text_bytes - 1) {
            /// @todo flash/blink display, or just beep? (a la screen)
            Curses::beep();

            new_text_bytes = text_size - text_bytes - 1;
            new_text_bytes = g_utf8_find_prev_char(new_text, new_text + new_text_bytes + 1) - new_text;
            n_chars = g_utf8_strlen(new_text, new_text_bytes);
          }
          break;
        }
      }
    }

    // allocate a new buffer and copy old data into it
    char *et_new = (char *) g_malloc(text_size);
    memcpy(et_new, text, prev_size);

    // overwrite a memory that might contain sensitive information
    char *varea = text;
    while (prev_size-- > 0)
      *varea++ = 0;

    g_free(text);
    text = et_new;
  }

  index = g_utf8_offset_to_pointer(text, current_pos) - text;

  g_memmove(text + index + new_text_bytes, text + index, text_bytes - index);
  memcpy(text + index, new_text, new_text_bytes);

  text_bytes += new_text_bytes;
  text_length += n_chars;

  // NUL terminate for safety and convenience
  text[text_bytes] = '\0';

  current_pos = current_pos + n_chars;

  Redraw();

  signal_text_changed(*this);
}

void TextEntry::DeleteText(int start_pos, int end_pos)
{
  g_assert(start_pos >= 0 && start_pos <= text_length);
  g_assert(end_pos >= 0 && end_pos <= text_length);

  if (start_pos < end_pos) {
    int start_index = g_utf8_offset_to_pointer(text, start_pos) - text;
    int end_index = g_utf8_offset_to_pointer(text, end_pos) - text;

    g_memmove(text + start_index, text + end_index, text_bytes + 1 - end_index);
    text_length -= end_pos - start_pos;
    text_bytes -= end_index - start_index;

    if (current_pos > start_pos)
      current_pos -= MIN(current_pos, end_pos) - start_pos;

    signal_text_changed(*this);
  }
}

void TextEntry::DeleteFromCursor(DeleteType type, int direction)
{
  int start_pos = current_pos;
  int end_pos = current_pos;

  if (!editable)
    return;

  switch (type) {
    case DELETE_CHARS:
      end_pos = MoveLogically(current_pos, direction);
      DeleteText(MIN(start_pos, end_pos), MAX(start_pos, end_pos));
      break;
    case DELETE_WORD_ENDS:
      if (direction > 0)
        end_pos = MoveForwardWord(end_pos);
      else if (direction < 0)
        start_pos = MoveBackwardWord(start_pos);
      DeleteText(start_pos, end_pos);
      break;
    default:
      g_assert_not_reached();
  }
}

void TextEntry::MoveCursor(CursorMovement step, int direction)
{
  switch (step) {
    case MOVE_LOGICAL_POSITIONS:
      current_pos = MoveLogically(current_pos, direction);
      break;
    case MOVE_WORDS:
      if (direction > 0)
        current_pos = MoveForwardWord(current_pos);
      else if (direction < 0)
        current_pos = MoveBackwardWord(current_pos);
      break;
    case MOVE_DISPLAY_LINE_ENDS:
      current_pos = direction < 0 ? 0 : text_length;
      break;
    default:
      g_assert_not_reached();
  }
}

void TextEntry::ToggleOverwrite()
{
  overwrite_mode = !overwrite_mode;
}

void TextEntry::RecalculateLengths()
{
  text_size = strlen(text);
  text_bytes = text_size;
  text_length = g_utf8_strlen(text, text_size);
}

int TextEntry::MoveLogically(int start, int direction)
{
  if (direction > 0 && start < text_length)
    return start + 1;
  else if (direction < 0 && start > 0)
    return start - 1;
  return start;
}

int TextEntry::MoveForwardWord(int start)
{
  int new_pos = start;
  char *cur = g_utf8_offset_to_pointer(text, start);
  bool white = false;

  // search for the first nonwhite character after white characters
  while (new_pos < text_length) {
    if (g_unichar_type(g_utf8_get_char(cur)) == G_UNICODE_SPACE_SEPARATOR)
      white = true;
    else if (white)
      break;
    new_pos++;
    cur = g_utf8_next_char(cur);
  }

  return new_pos;
}

int TextEntry::MoveBackwardWord(int start)
{
  int new_pos = start;
  char *cur = g_utf8_offset_to_pointer(text, start);
  bool nonwhite = false;

  if (start <= 0)
    return 0;

  // always move at least one character back
  cur = g_utf8_prev_char(cur);
  new_pos--;

  // search for the first white character before nonwhite characters
  while (new_pos >= 0) {
    if (g_unichar_type(g_utf8_get_char(cur)) != G_UNICODE_SPACE_SEPARATOR)
      nonwhite = true;
    else if (nonwhite)
      break;

    if (new_pos > 0)
      cur = g_utf8_prev_char(cur);
    new_pos--;
  }

  return ++new_pos;
}

void TextEntry::ActionMoveCursor(CursorMovement step, int direction)
{
  MoveCursor(step, direction);
  Redraw();
}

void TextEntry::ActionDelete(DeleteType type, int direction)
{
  DeleteFromCursor(type, direction);
  Redraw();
}

void TextEntry::ActionToggleOverwrite()
{
  ToggleOverwrite();
}

//TODO custom handlers?
void TextEntry::ActionActivate()
{
  if (parent)
    parent->MoveFocus(Container::FOCUS_NEXT);
}

void TextEntry::DeclareBindables()
{
  // cursor movement
  DeclareBindable("textentry", "cursor-right",
      sigc::bind(sigc::mem_fun(this, &TextEntry::ActionMoveCursor),
        MOVE_LOGICAL_POSITIONS, 1), InputProcessor::BINDABLE_NORMAL);

  DeclareBindable("textentry", "cursor-left",
      sigc::bind(sigc::mem_fun(this, &TextEntry::ActionMoveCursor),
        MOVE_LOGICAL_POSITIONS, -1), InputProcessor::BINDABLE_NORMAL);

  DeclareBindable("textentry", "cursor-right-word",
      sigc::bind(sigc::mem_fun(this, &TextEntry::ActionMoveCursor),
        MOVE_WORDS, 1), InputProcessor::BINDABLE_NORMAL);

  DeclareBindable("textentry", "cursor-left-word",
      sigc::bind(sigc::mem_fun(this, &TextEntry::ActionMoveCursor),
        MOVE_WORDS, -1), InputProcessor::BINDABLE_NORMAL);

  DeclareBindable("textentry", "cursor-end",
      sigc::bind(sigc::mem_fun(this, &TextEntry::ActionMoveCursor),
        MOVE_DISPLAY_LINE_ENDS, 1), InputProcessor::BINDABLE_NORMAL);

  DeclareBindable("textentry", "cursor-begin",
      sigc::bind(sigc::mem_fun(this, &TextEntry::ActionMoveCursor),
        MOVE_DISPLAY_LINE_ENDS, -1), InputProcessor::BINDABLE_NORMAL);

  // deleting text
  DeclareBindable("textentry", "delete-char",
      sigc::bind(sigc::mem_fun(this, &TextEntry::ActionDelete),
        DELETE_CHARS, 1), InputProcessor::BINDABLE_NORMAL);

  DeclareBindable("textentry", "backspace",
      sigc::bind(sigc::mem_fun(this, &TextEntry::ActionDelete),
        DELETE_CHARS, -1), InputProcessor::BINDABLE_NORMAL);

  // tabifying
  DeclareBindable("textentry", "tab",
      sigc::bind(sigc::mem_fun(this, &TextEntry::InsertTextAtCursor),
        "    ", 4), InputProcessor::BINDABLE_NORMAL);

  /*
  DeclareBindable("textentry", "delete-word-end",
      sigc::bind(sigc::mem_fun(this, &TextEntry::ActionDelete),
        DELETE_WORD_ENDS, 1), InputProcessor::BINDABLE_NORMAL);

  DeclareBindable("textentry", "delete-word-begin",
      sigc::bind(sigc::mem_fun(this, &TextEntry::ActionDelete),
        DELETE_WORD_ENDS, -1), InputProcessor::BINDABLE_NORMAL);

  // overwrite
  DeclareBindable("textentry", "toggle-overwrite", sigc::mem_fun(this,
        &TextEntry::ActionToggleOverwrite), InputProcessor::BINDABLE_NORMAL);
  */

  // non text editing bindables
  DeclareBindable("textentry", "activate", sigc::mem_fun(this,
        &TextEntry::ActionActivate), InputProcessor::BINDABLE_NORMAL);
}

} // namespace CppConsUI
