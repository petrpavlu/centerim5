/*
 * Copyright (C) 2010-2012 by CenterIM developers
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
 * TextEdit class
 *
 * @ingroup cppconsui
 */

#ifndef __TEXTEDIT_H__
#define __TEXTEDIT_H__

#include "Widget.h"

#include <deque>

namespace CppConsUI
{

class TextEdit
: public Widget
{
public:
  enum Flag {
    FLAG_ALPHABETIC = 1 << 0,
    FLAG_NUMERIC = 1 << 1,
    FLAG_NOSPACE = 1 << 2,
    FLAG_NOPUNCTUATION = 1 << 3
  };

  TextEdit(int w, int h, const char *text_ = NULL, int flags_ = 0,
      bool single_line = false, bool accept_tabs_ = true);
  virtual ~TextEdit();

  // Widget
  virtual void Draw();
  virtual bool ProcessInputText(const TermKeyKey &key);

  /**
   * Sets new text.
   */
  virtual void SetText(const char *new_text);

  virtual void Clear();
  /**
   * Returns inserted text. Caller is responsible for freeing returned data.
   */
  virtual const char *GetText() const;

  virtual void SetFlags(int flags_, bool revalidate = true);
  virtual int GetFlags() const { return flags; }

  virtual void SetSingleLineMode(bool allowed);
  virtual bool IsSingleLineMode() const { return single_line_mode; }

  virtual void SetAcceptTabs(bool accept);
  virtual bool GetAcceptTabs() const { return accept_tabs; }

  sigc::signal<void, TextEdit&> signal_text_change;

protected:
  enum Direction {
    DIR_BACK,
    DIR_FORWARD
  };

  enum CursorMovement {
    MOVE_LOGICAL_POSITIONS,
    MOVE_VISUAL_POSITIONS,
    MOVE_WORDS,
    MOVE_DISPLAY_LINES,
    MOVE_DISPLAY_LINE_ENDS,
    MOVE_PARAGRAPHS,
    MOVE_PARAGRAPH_ENDS,
    MOVE_PAGES,
    MOVE_BUFFER_ENDS,
    MOVE_HORIZONTAL_PAGES
  };

  enum DeleteType {
    DELETE_CHARS,
    DELETE_WORD_ENDS
  };

  struct ScreenLine
  {
    /**
     * Pointer to the start of line (points into buffer).
     */
    const char *start;
    /**
     * Pointer to the first byte that is not part of this line.
     */
    const char *end;
    /**
     * Precalculated length.
     */
    size_t length;

    ScreenLine(const char *start_, const char *end_, size_t length_)
      : start(start_), end(end_), length(length_) {}
    bool operator==(const ScreenLine& other) const;
  };

  struct CmpScreenLineEnd
  {
    bool operator()(ScreenLine& sline, const char *tag);
  };

  typedef std::deque<ScreenLine> ScreenLines;

  ScreenLines screen_lines;

  /**
   * Bitmask indicating which input is accepted.
   */
  int flags;
  bool editable;
  bool overwrite_mode;
  bool single_line_mode;
  bool accept_tabs;

  /**
   * Character position from the start of buffer.
   */
  size_t current_pos;
  /**
   * Cursor location in the buffer.
   */
  mutable char *point;

  /**
   * Current cursor line (derived from current_pos and screen_lines).
   */
  size_t current_sc_line;
  /**
   * Current cursor character number (in the current line).
   */
  size_t current_sc_linepos;
  /**
   * Holds index into screen_lines that marks the first screen line.
   */
  size_t view_top;

  /**
   * Start of text buffer.
   */
  char *buffer;
  /**
   * First location outside buffer.
   */
  char *bufend;
  /**
   * Start of gap.
   */
  mutable char *gapstart;
  /**
   * First location after the gap end.
   */
  mutable char *gapend;
  /**
   * Length in use, in chars.
   */
  size_t text_length;

  mutable bool screen_lines_dirty;

  virtual void InitBuffer(size_t size);
  virtual size_t GetGapSize() const;
  virtual void ExpandGap(size_t size);
  virtual void MoveGapToCursor();

  size_t GetTextSize() const;
  virtual char *GetTextStart() const;
  virtual char *PrevChar(const char *p) const;
  virtual char *NextChar(const char *p) const;
  virtual int Width(const char *start, size_t chars) const;

  virtual char *GetScreenLine(const char *text, int max_width,
      size_t *res_length) const;
  /**
   * Recalculates all screen lines.
   */
  virtual void UpdateScreenLines();
  /**
   * Recalculates necessary amout of screen lines.
   */
  virtual void UpdateScreenLines(const char *begin, const char *end);
  virtual void AssertUpdatedScreenLines();

  /**
   * Recalculates screen cursor position based on current_pos and
   * screen_lines, sets current_sc_line and current_sc_linepos and handles
   * scrolling if necessary.
   */
  virtual void UpdateScreenCursor();

  /**
   * Inserts given text at the current cursor position.
   */
  virtual void InsertTextAtCursor(const char *new_text,
      size_t new_text_bytes);
  virtual void InsertTextAtCursor(const char *new_text);
  virtual void DeleteFromCursor(DeleteType type, Direction dir);
  virtual void MoveCursor(CursorMovement step, Direction dir);

  virtual void ToggleOverwrite();

  virtual size_t MoveLogicallyFromCursor(Direction dir) const;
  virtual size_t MoveWordFromCursor(Direction dir, bool word_end) const;

private:
  TextEdit(const TextEdit&);
  TextEdit& operator=(const TextEdit&);

  void ActionMoveCursor(CursorMovement step, Direction dir);
  void ActionDelete(DeleteType type, Direction dir);
  void ActionToggleOverwrite();

  void DeclareBindables();
};

} // namespace CppConsUI

#endif // __TEXTEDIT_H__

/* vim: set tabstop=2 shiftwidth=2 expandtab : */
