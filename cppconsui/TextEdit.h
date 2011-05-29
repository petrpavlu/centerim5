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
 * TextEdit class
 *
 * @ingroup cppconsui
 */

#ifndef __TEXTEDIT_H__
#define __TEXTEDIT_H__

#include "Widget.h"

#include <vector>

namespace CppConsUI
{

class TextEdit
: public Widget
{
public:
  TextEdit(int w, int h);
  virtual ~TextEdit();

  // Widget
  virtual void Draw();
  virtual bool ProcessInputText(const TermKeyKey &key);

  void Clear();
  /**
   * Returns inserted text. Lines are separated by a given separator. Caller
   * is responsible for freeing returned data.
   */
  char *AsString(const char *separator = "\n");

  sigc::signal<void, TextEdit&> signal_text_changed;

protected:
  void InitBuffer(int size);
  int SizeOfGap();
  int BufferSize();
  void ExpandGap(int size);
  void MoveGapToCursor();

  char *PrevChar(const char *p) const;
  char *NextChar(const char *p) const;
  int Width(const char *start, int chars) const;

  char *GetScreenLine(char *text, int max_width, int *res_width,
      int *res_length) const;
  void UpdateScreenLines();
  void ClearScreenLines();

  void UpdateScreenCursor();

  void InsertTextAtCursor(const char *new_text, int new_text_bytes = -1);
  void DeleteFromCursor(DeleteType type, int direction);
  void MoveCursor(CursorMovement step, int direction);

  void ToggleOverwrite();

  bool editable;
  bool overwrite_mode;

  int current_pos; ///< Character position from the start of buffer.
  char *point; ///< Cursor location in the buffer.

  int current_sc_line; ///< Current cursor line (derived from current_pos and screen_lines).
  int current_sc_linepos; ///< Current cursor character number (in the current line).
  int view_top;

  char *buffer; ///< Start of text buffer.
  char *bufend; ///< First location outside buffer.
  char *gapstart; ///< Start of gap.
  char *gapend; ///< First location after end of gap.
  int text_length; ///< Length in use, in chars.

  int gap_size; ///< Expand gap by this value.

  struct ScreenLine
  {
    const char *start; ///< Pointer to start of line (points into buffer).
    const char *end; ///< Pointer to first byte that is not part of line.
    int length; ///< Precalculated length.
    int width; ///< Precalculated on screen width.

    ScreenLine(const char *start, const char *end, int length, int width)
      : start(start), end(end), length(length), width(width) {}
  };

  std::vector<ScreenLine *> screen_lines;

private:
  TextEdit(const TextEdit&);
  TextEdit& operator=(const TextEdit&);

  int MoveLogically(int start, int direction);
  int MoveForwardWordFromCursor();
  int MoveBackwardWordFromCursor();

  void ActionMoveCursor(CursorMovement step, int direction);
  void ActionDelete(DeleteType type, int direction);
  void ActionToggleOverwrite();

  void DeclareBindables();
};

} // namespace CppConsUI

#endif // __TEXTEDIT_H__
