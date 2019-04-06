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
/// TextEdit class
///
/// @ingroup cppconsui

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include "Widget.h"

#include <deque>

namespace CppConsUI {

class TextEdit : public Widget {
public:
  enum Flag {
    FLAG_NUMERIC = 1 << 0,
    FLAG_NOSPACE = 1 << 1,
  };

  TextEdit(int w, int h, const char *text_ = nullptr, int flags_ = 0,
    bool single_line = false, bool accept_tabs_ = true, bool masked_ = false);
  virtual ~TextEdit() override;

  // InputProcessor
  virtual bool processInputText(const TermKeyKey &key) override;

  // Widget
  virtual int draw(Curses::ViewPort area, Error &error) override;

  /// Sets new text.
  virtual void setText(const char *new_text);

  /// Removes all text.
  virtual void clear();

  /// Returns inserted text.
  virtual const char *getText() const;

  virtual std::size_t getTextLength() const { return text_length_; }

  virtual void setFlags(int new_flags, bool revalidate = true);
  virtual int getFlags() const { return flags_; }

  virtual void setSingleLineMode(bool new_single_line_mode);
  virtual bool isSingleLineMode() const { return single_line_mode_; }

  virtual void setAcceptTabs(bool new_accept_tabs);
  virtual bool doesAcceptTabs() const { return accept_tabs_; }

  virtual void setMasked(bool new_masked);
  virtual bool isMasked() const { return masked_; }

  sigc::signal<void, TextEdit &> signal_text_change;

protected:
  enum Direction {
    DIR_BACK,
    DIR_FORWARD,
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
    MOVE_HORIZONTAL_PAGES,
  };

  enum DeleteType {
    DELETE_CHARS,
    DELETE_WORD_ENDS,
    DELETE_LINE_ENDS,
  };

  struct ScreenLine {
    /// Pointer to the start of line (points into buffer).
    const char *start;

    /// Pointer to the first byte that is not part of this line.
    const char *end;

    /// Precalculated length.
    std::size_t length;

    ScreenLine(const char *start, const char *end, std::size_t length)
      : start(start), end(end), length(length)
    {
    }
    bool operator==(const ScreenLine &other) const;
  };

  struct CmpScreenLineEnd {
    bool operator()(ScreenLine &sline, const char *tag);
  };

  typedef std::deque<ScreenLine> ScreenLines;

  ScreenLines screen_lines_;

  /// Bitmask indicating which input is accepted.
  int flags_;

  bool editable_;
  bool overwrite_mode_;
  bool single_line_mode_;
  bool accept_tabs_;
  bool masked_;

  /// Character position from the start of buffer.
  std::size_t current_pos_;

  /// Cursor location in the buffer.
  mutable char *point_;

  /// Current cursor line (derived from current_pos_ and screen_lines_).
  std::size_t current_sc_line_;

  /// Current cursor character number (in the current line).
  std::size_t current_sc_linepos_;

  /// Holds index into screen_lines_ that marks the first screen line.
  std::size_t view_top_;

  /// Start of text buffer.
  char *buffer_;

  /// First location outside buffer.
  char *bufend_;

  /// Start of gap.
  mutable char *gapstart_;

  /// First location after the gap end.
  mutable char *gapend_;

  /// Length in use, in chars.
  std::size_t text_length_;

  mutable bool screen_lines_dirty_;

  // Widget
  virtual void updateArea() override;

  virtual void initBuffer(std::size_t size);
  virtual std::size_t getGapSize() const;
  virtual void expandGap(std::size_t size);
  virtual void moveGapToCursor();

  virtual char *getTextStart() const;
  virtual char *prevChar(const char *p) const;
  virtual char *nextChar(const char *p) const;
  virtual int width(const char *start, std::size_t chars) const;

  /// Returns on-screen width of a given character in the same fashion as
  /// Curses::onscreen_width() does but handles the tab character and wide
  /// characters properly if the masked mode is active.
  virtual int onScreenWidth(UTF8::UniChar uc, int w = 0) const;

  virtual char *getScreenLine(
    const char *text, int max_width, std::size_t *res_length) const;

  /// Recalculates all screen lines.
  virtual void updateScreenLines();

  /// Recalculates necessary amout of screen lines.
  virtual void updateScreenLines(const char *begin, const char *end);

  virtual void assertUpdatedScreenLines();

  /// Recalculates screen cursor position based on current_pos_ and
  /// screen_lines_, sets current_sc_line_ and current_sc_linepos_ and handles
  /// scrolling if necessary.
  virtual void updateScreenCursor();

  /// Inserts given text at the current cursor position.
  virtual void insertTextAtCursor(
    const char *new_text, std::size_t new_text_bytes);
  virtual void insertTextAtCursor(const char *new_text);
  virtual void deleteFromCursor(DeleteType type, Direction dir);
  virtual void moveCursor(CursorMovement step, Direction dir);

  virtual void toggleOverwrite();

  virtual std::size_t moveLogicallyFromCursor(Direction dir) const;
  virtual std::size_t moveWordFromCursor(Direction dir, bool word_end) const;
  virtual std::size_t moveLineFromCursor(Direction dir) const;

private:
  CONSUI_DISABLE_COPY(TextEdit);

  void actionMoveCursor(CursorMovement step, Direction dir);
  void actionDelete(DeleteType type, Direction dir);
  void actionToggleOverwrite();

  void declareBindables();
};

} // namespace CppConsUI

#endif // TEXTEDIT_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
