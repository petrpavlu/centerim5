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
/// TextEdit class implementation
///
/// @ingroup cppconsui

// Gap buffer implementation based on code from Hsin Tsao
// (stsao@lazyhacker.com).

#include "TextEdit.h"

#include "ColorScheme.h"

#include <algorithm>
#include <cassert>
#include <cstring>

// Gap expand size when the gap becomes filled.
#define GAP_SIZE_EXPAND 4096

namespace CppConsUI {

TextEdit::TextEdit(int w, int h, const char *text, int flags, bool single_line,
  bool accept_tabs, bool masked)
  : Widget(w, h), flags_(flags), editable_(true), overwrite_mode_(false),
    single_line_mode_(single_line), accept_tabs_(accept_tabs), masked_(masked),
    buffer_(nullptr), screen_lines_dirty_(false)
{
  setText(text);

  can_focus_ = true;
  declareBindables();
}

TextEdit::~TextEdit()
{
  delete[] buffer_;
}

bool TextEdit::processInputText(const TermKeyKey &key)
{
  if (!editable_)
    return false;

  if (single_line_mode_ && key.code.codepoint == '\n')
    return false;

  if (!accept_tabs_ && key.code.codepoint == '\t')
    return false;

  // Filter out unwanted input.
  if (flags_ != 0) {
    if ((flags_ & FLAG_NUMERIC) && !UTF8::isUniCharDigit(key.code.codepoint))
      return false;
    if ((flags_ & FLAG_NOSPACE) && UTF8::isUniCharSpace(key.code.codepoint))
      return false;
  }

  insertTextAtCursor(key.utf8);
  return true;
}

int TextEdit::draw(Curses::ViewPort area, Error &error)
{
  assertUpdatedScreenLines();

  DRAW(area.erase(error));

  int attrs;
  DRAW(getAttributes(ColorScheme::PROPERTY_TEXTEDIT_TEXT, &attrs, error));
  DRAW(area.attrOn(attrs, error));

  ScreenLines::iterator i;
  int j;
  for (i = screen_lines_.begin() + view_top_, j = 0;
       i != screen_lines_.end() && j < real_height_; ++i, ++j) {
    const char *p = i->start;
    int w = 0;
    for (std::size_t k = 0; k < i->length && *p != '\n'; ++k) {
      int printed;
      if (masked_)
        DRAW(area.addChar(w, j, '*', error, &printed));
      else {
        UTF8::UniChar uc = UTF8::getUniChar(p);
        if (uc == '\t') {
          printed = onScreenWidth(uc, w);
          for (int l = 0; l < printed; ++l)
            DRAW(area.addChar(w + l, j, ' ', error));
        }
        else
          DRAW(area.addChar(w, j, uc, error, &printed));
      }
      w += printed;
      p = nextChar(p);
    }
  }

  DRAW(area.attrOff(attrs, error));

  if (has_focus_) {
    const char *line = screen_lines_[current_sc_line_].start;
    int sc_x = width(line, current_sc_linepos_);
    int sc_y = current_sc_line_ - view_top_;
    DRAW(area.changeAt(sc_x, sc_y, 1, Curses::Attr::REVERSE, 0, error));
  }

  return 0;
}

void TextEdit::setText(const char *new_text)
{
  if (new_text == nullptr) {
    clear();
    return;
  }

  // XXX should the text be validated (FLAG_*)?
  std::size_t size = strlen(new_text);
  initBuffer(size + GAP_SIZE_EXPAND);
  insertTextAtCursor(new_text, size);
}

void TextEdit::clear()
{
  initBuffer(GAP_SIZE_EXPAND);
  redraw();
}

const char *TextEdit::getText() const
{
  assert(gapend_ > gapstart_);

  screen_lines_dirty_ = true;

  // Move gap to the end.
  bool point_after_gap = point_ >= gapend_;

  // '-1' so the last '\n' is still in the end of the buffer.
  std::memmove(gapstart_, gapend_, bufend_ - gapend_ - 1);
  if (point_after_gap)
    point_ -= gapend_ - gapstart_;
  gapstart_ += bufend_ - gapend_ - 1;
  gapend_ = bufend_ - 1;

  *gapstart_ = '\0';

  return buffer_;
}

void TextEdit::setFlags(int new_flags, bool revalidate)
{
  if (new_flags == flags_)
    return;

  flags_ = new_flags;

  if (flags_ != 0 && revalidate) {
    bool valid = true;
    const char *p = getTextStart();
    while (p < bufend_ - 1) {
      UTF8::UniChar uc = UTF8::getUniChar(p);
      if ((flags_ & FLAG_NUMERIC) && !UTF8::isUniCharDigit(uc)) {
        valid = false;
        break;
      }
      if ((flags_ & FLAG_NOSPACE) && UTF8::isUniCharSpace(uc)) {
        valid = false;
        break;
      }
      p = nextChar(p);
    }
    if (!valid)
      clear();
  }
}

void TextEdit::setSingleLineMode(bool new_single_line_mode)
{
  if (new_single_line_mode == single_line_mode_)
    return;

  single_line_mode_ = new_single_line_mode;
}

void TextEdit::setAcceptTabs(bool new_accept_tabs)
{
  if (new_accept_tabs == accept_tabs_)
    return;

  accept_tabs_ = new_accept_tabs;
}

void TextEdit::setMasked(bool new_masked)
{
  if (new_masked == masked_)
    return;

  masked_ = new_masked;
  // In the masked mode, the tab character and wide characters lose their width
  // property, thus screen lines and cursor have to be updated.
  updateScreenLines();
  updateScreenCursor();
  redraw();
}

bool TextEdit::ScreenLine::operator==(const ScreenLine &other) const
{
  return start == other.start && end == other.end && length == other.length;
}

bool TextEdit::CmpScreenLineEnd::operator()(ScreenLine &sline, const char *tag)
{
  return sline.end < tag;
}

void TextEdit::updateArea()
{
  // Update screen lines and cursor position if the area width changed.
  updateScreenLines();
  updateScreenCursor();
}

void TextEdit::initBuffer(std::size_t size)
{
  assert(size > 0);

  delete[] buffer_;
  buffer_ = new char[size];

  current_pos_ = 0;
  point_ = gapstart_ = buffer_;

  bufend_ = gapend_ = buffer_ + size;
  gapend_ = bufend_ - 1;
  // Insert an empty line.
  *gapend_ = '\n';

  text_length_ = 0;
  current_pos_ = 0;
  current_sc_line_ = 0;
  current_sc_linepos_ = 0;

  view_top_ = 0;

  updateScreenLines();
}

std::size_t TextEdit::getGapSize() const
{
  // '-1' so '\0' character can be stored at the gapstart position in the
  // getText() method.
  return gapend_ - gapstart_ - 1;
}

void TextEdit::expandGap(std::size_t size)
{
  std::size_t gap_size = getGapSize();
  if (size <= gap_size)
    return;

  size += GAP_SIZE_EXPAND - gap_size;

  char *origbuffer = buffer_;
  bool point_after_gap = point_ >= gapend_;

  std::size_t alloc_size = (bufend_ - buffer_) + size;
  buffer_ = new char[alloc_size];
  std::memcpy(buffer_, origbuffer, alloc_size);

  point_ = buffer_ + (point_ - origbuffer);
  bufend_ = buffer_ + (bufend_ - origbuffer);
  gapstart_ = buffer_ + (gapstart_ - origbuffer);
  gapend_ = buffer_ + (gapend_ - origbuffer);

  delete[] origbuffer;

  std::memmove(gapend_ + size, gapend_, bufend_ - gapend_);

  if (point_after_gap) {
    // This should never happen because moveGapToCursor() is always called
    // before expandGap().
    point_ += size;
  }
  gapend_ += size;
  bufend_ += size;
}

void TextEdit::moveGapToCursor()
{
  if (point_ == gapstart_)
    return;

  if (point_ == gapend_) {
    point_ = gapstart_;
    return;
  }

  // Move gap towards the left.
  if (point_ < gapstart_) {
    // Move the point over by gapsize.
    std::memmove(point_ + (gapend_ - gapstart_), point_, gapstart_ - point_);
    gapend_ -= gapstart_ - point_;
    gapstart_ = point_;
  }
  else {
    // Since point is after the gap, find distance between gapend and point and
    // that is how much we move from gapend to gapstart.
    std::memmove(gapstart_, gapend_, point_ - gapend_);
    gapstart_ += point_ - gapend_;
    gapend_ = point_;
    point_ = gapstart_;
  }
}

char *TextEdit::getTextStart() const
{
  if (buffer_ == gapstart_)
    return const_cast<char *>(gapend_);
  return const_cast<char *>(buffer_);
}

char *TextEdit::prevChar(const char *p) const
{
  if (p >= gapend_) {
    if ((p = UTF8::findPrevChar(gapend_, p)))
      return const_cast<char *>(p);
    else
      p = gapstart_;
  }

  if ((p = UTF8::findPrevChar(buffer_, p)))
    return const_cast<char *>(p);
  else
    return const_cast<char *>(buffer_);
}

char *TextEdit::nextChar(const char *p) const
{
  // This happens when point_ == gapstart_.
  if (p == gapstart_)
    p = gapend_;

  if (p < gapstart_) {
    if ((p = UTF8::findNextChar(p, gapstart_)))
      return const_cast<char *>(p);
    else
      return const_cast<char *>(gapend_);
  }

  if ((p = UTF8::findNextChar(p, bufend_)))
    return const_cast<char *>(p);
  else
    return const_cast<char *>(bufend_);
}

int TextEdit::width(const char *start, std::size_t chars) const
{
  assert(start != nullptr);

  int width = 0;

  while (chars-- > 0) {
    UTF8::UniChar uc = UTF8::getUniChar(start);
    width += onScreenWidth(uc, width);
    start = nextChar(start);
  }
  return width;
}

int TextEdit::onScreenWidth(UTF8::UniChar uc, int w) const
{
  if (masked_)
    return 1;
  return Curses::onScreenWidth(uc, w);
}

char *TextEdit::getScreenLine(
  const char *text, int max_width, std::size_t *res_length) const
{
  assert(text != nullptr);
  assert(text < bufend_);
  assert(max_width > 0);
  assert(res_length != nullptr);

  const char *cur = text;
  const char *res = text;
  int prev_width = 0;
  int cur_width = 0;
  std::size_t cur_length = 0;
  bool space = false;
  *res_length = 0;

  while (cur < bufend_) {
    prev_width = cur_width;
    UTF8::UniChar uc = UTF8::getUniChar(cur);
    cur_width += onScreenWidth(uc, cur_width);
    ++cur_length;

    if (prev_width > max_width)
      break;

    // Possibly too long word.
    if (cur_width > max_width && !*res_length) {
      *res_length = cur_length - 1;
      res = cur;
    }

    // End of line (paragraph on screen) found.
    if (*cur == '\n') {
      *res_length = cur_length;
      return nextChar(cur);
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

    cur = nextChar(cur);
  }

  // Fix for very small max_width and characters wider that 1 cell. For example,
  // max_width = 1 and text = "W" where W is a wide character (2 cells width)
  // (or simply for tabs). In that case we can not draw anything but we want to
  // skip to another character.
  if (res == text) {
    *res_length = 1;
    res = nextChar(res);
  }

  return const_cast<char *>(res);
}

void TextEdit::updateScreenLines()
{
  screen_lines_.clear();

  if (real_width_ <= 1)
    return;

  const char *p = getTextStart();

  while (p < bufend_) {
    const char *s = p;
    std::size_t length;
    // Lower max width by one to make a room for the cursor.
    p = getScreenLine(p, real_width_ - 1, &length);
    screen_lines_.push_back(ScreenLine(s, p, length));
  }
}

void TextEdit::updateScreenLines(const char *begin, const char *end)
{
  assert(begin != nullptr);
  assert(end != nullptr);

  if (real_width_ <= 1)
    return;

  ScreenLines::iterator b, i;
  b = std::lower_bound(screen_lines_.begin(), screen_lines_.end(), begin,
    TextEdit::CmpScreenLineEnd());
  if (b != screen_lines_.begin()) {
    //  Initial      Correct final
    // situation      situation
    // ---------      ---------
    // |aaaa   |  ->  |aaaa b |
    // |bcdddd |      |cdddd  |
    //
    // User inserts a space in front of the 'c' character. The 'b' string can be
    // moved on the previous line thus one more extra line before has to be
    // recalculated to handle the situation correctly.
    --b;
  }
  i = b;

  ScreenLines new_screen_lines;

  const char *p = b->start;
  if (i == screen_lines_.begin())
    p = getTextStart();

  while (p < bufend_) {
    const char *s = p;
    std::size_t length;
    // Lower max width by one to make a room for the cursor.
    p = getScreenLine(p, real_width_ - 1, &length);
    ScreenLine sline(s, p, length);
    new_screen_lines.push_back(sline);
    while (
      i != screen_lines_.end() && (i->end <= end || i->start < s || i->end < p))
      ++i;
    if (i != screen_lines_.end() && sline == *i) {
      // Screen lines are same thus it is not necessary to recalculate more
      // of them.
      break;
    }
  }
  if (i != screen_lines_.end())
    ++i;

  // Replace old screen lines with new screen lines.
  ScreenLines::iterator j;
  for (j = new_screen_lines.begin(); j != new_screen_lines.end() && b != i;
       ++j, ++b)
    *b = *j;

  if (j != new_screen_lines.end()) {
    // b == i.
    screen_lines_.insert(b, j, new_screen_lines.end());
  }
  else {
    // b != i.
    screen_lines_.erase(b, i);
  }
}

void TextEdit::assertUpdatedScreenLines()
{
  if (!screen_lines_dirty_)
    return;

  updateScreenLines();
  screen_lines_dirty_ = false;
}

void TextEdit::updateScreenCursor()
{
  std::size_t acu_length = 0;
  current_sc_line_ = 0;
  current_sc_linepos_ = 0;

  assertUpdatedScreenLines();

  for (ScreenLine &line : screen_lines_) {
    std::size_t length = line.length;
    if (acu_length <= current_pos_ && current_pos_ < acu_length + length) {
      current_sc_linepos_ = current_pos_ - acu_length;
      break;
    }
    ++current_sc_line_;
    acu_length += length;
  }

  // Fix cursor visibility.
  if (view_top_ <= current_sc_line_ &&
    current_sc_line_ < view_top_ + real_height_)
    return;
  while (view_top_ > current_sc_line_)
    --view_top_;
  while (view_top_ + real_height_ <= current_sc_line_)
    ++view_top_;
}

void TextEdit::insertTextAtCursor(
  const char *new_text, std::size_t new_text_bytes)
{
  assert(new_text != nullptr);

  assertUpdatedScreenLines();

  // Move the gap if the point is not already at the start of the gap.
  char *min = gapstart_;
  char *max = gapend_;
  moveGapToCursor();
  char *min2 = gapstart_;

  // Make sure that the gap has enough room.
  bool full_screen_lines_update = false;
  if (new_text_bytes > getGapSize()) {
    expandGap(new_text_bytes);
    full_screen_lines_update = true;
  }

  std::size_t n_chars = 0;
  const char *p = new_text;
  while (p != nullptr && *p != '\0') {
    ++n_chars;
    p = UTF8::findNextChar(p, new_text + new_text_bytes);
  }
  text_length_ += n_chars;
  current_pos_ += n_chars;

  while (new_text_bytes) {
    *gapstart_++ = *new_text++;
    --new_text_bytes;
  }
  point_ = gapstart_;

  if (full_screen_lines_update)
    updateScreenLines();
  else
    updateScreenLines(std::min(min, min2), std::max(max, gapend_));
  updateScreenCursor();
  redraw();

  signal_text_change(*this);
}

void TextEdit::insertTextAtCursor(const char *new_text)
{
  assert(new_text != nullptr);

  insertTextAtCursor(new_text, strlen(new_text));
}

void TextEdit::deleteFromCursor(DeleteType type, Direction dir)
{
  if (!editable_)
    return;

  assertUpdatedScreenLines();

  int count = 0;

  switch (type) {
  case DELETE_CHARS:
    count = moveLogicallyFromCursor(dir) - current_pos_;
    break;
  case DELETE_WORD_ENDS:
    count = moveWordFromCursor(dir, true) - current_pos_;
    break;
  case DELETE_LINE_ENDS:
    count = moveLineFromCursor(dir) - current_pos_;
    break;
  default:
    assert(0);
  }

  if (count != 0) {
    char *min = gapstart_;
    char *max = gapend_;
    moveGapToCursor();

    while (count > 0) {
      gapend_ = nextChar(gapend_);
      --text_length_;
      --count;
    }

    while (count < 0) {
      gapstart_ = prevChar(gapstart_);
      --current_pos_;
      --text_length_;
      ++count;
    }
    point_ = gapstart_;

    updateScreenLines(std::min(min, gapstart_), std::max(max, gapend_));
    updateScreenCursor();
    redraw();

    signal_text_change(*this);
  }
}

void TextEdit::moveCursor(CursorMovement step, Direction dir)
{
  assertUpdatedScreenLines();

  std::size_t old_pos = current_pos_;
  switch (step) {
  case MOVE_LOGICAL_POSITIONS:
    current_pos_ = moveLogicallyFromCursor(dir);
    break;
  case MOVE_WORDS:
    current_pos_ = moveWordFromCursor(dir, false);
    break;
  case MOVE_DISPLAY_LINES:
    if (dir == DIR_FORWARD) {
      if (current_sc_line_ + 1 < screen_lines_.size()) {
        int oldw =
          width(screen_lines_[current_sc_line_].start, current_sc_linepos_);
        // First move to end of current line.
        current_pos_ +=
          screen_lines_[current_sc_line_].length - current_sc_linepos_;
        // Find a character close to the original position.
        const char *ch = screen_lines_[current_sc_line_ + 1].start;
        std::size_t i = 0;
        int w = 0;
        while (w < oldw && i < screen_lines_[current_sc_line_ + 1].length - 1) {
          UTF8::UniChar uc = UTF8::getUniChar(ch);
          w += onScreenWidth(uc, w);
          ch = nextChar(ch);
          ++i;
        }
        current_pos_ += i;
      }
    }
    else { // DIR_BACK
      if (current_sc_line_ > 0) {
        int oldw =
          width(screen_lines_[current_sc_line_].start, current_sc_linepos_);
        // First move to start of current line.
        current_pos_ -= current_sc_linepos_;
        // Move to the start of the previous line.
        current_pos_ -= screen_lines_[current_sc_line_ - 1].length;
        // Find a character close to the original position.
        const char *ch = screen_lines_[current_sc_line_ - 1].start;
        std::size_t i = 0;
        int w = 0;
        while (w < oldw && i < screen_lines_[current_sc_line_ - 1].length - 1) {
          UTF8::UniChar uc = UTF8::getUniChar(ch);
          w += onScreenWidth(uc, w);
          ch = nextChar(ch);
          ++i;
        }
        current_pos_ += i;
      }
    }
    break;
  case MOVE_DISPLAY_LINE_ENDS:
    if (dir == DIR_FORWARD)
      current_pos_ +=
        screen_lines_[current_sc_line_].length - current_sc_linepos_ - 1;
    else // DIR_BACK
      current_pos_ -= current_sc_linepos_;
    break;
  default:
    assert(0);
  }

  // Update point_.
  while (old_pos > current_pos_) {
    point_ = prevChar(point_);
    --old_pos;
  }
  while (old_pos < current_pos_) {
    point_ = nextChar(point_);
    ++old_pos;
  }

  updateScreenCursor();
  redraw();
}

void TextEdit::toggleOverwrite()
{
  overwrite_mode_ = !overwrite_mode_;
}

std::size_t TextEdit::moveLogicallyFromCursor(Direction dir) const
{
  if (dir == DIR_FORWARD && current_pos_ < text_length_)
    return current_pos_ + 1;
  else if (dir == DIR_BACK && current_pos_ > 0)
    return current_pos_ - 1;
  return current_pos_;
}

std::size_t TextEdit::moveWordFromCursor(Direction dir, bool word_end) const
{
  std::size_t new_pos = current_pos_;
  const char *cur = point_;
  if (cur == gapstart_)
    cur = gapend_;

  if (dir == DIR_FORWARD) {
    if (word_end) {
      // Search for the first white character after non-white characters.
      bool nonwhite = false;
      while (new_pos < text_length_) {
        if (!UTF8::isUniCharSpace(UTF8::getUniChar(cur)) && *cur != '\n')
          nonwhite = true;
        else if (nonwhite)
          break;
        cur = nextChar(cur);
        ++new_pos;
      }
      return new_pos;
    }
    else {
      // Search for the first nonwhite character after white characters.
      bool white = false;
      while (new_pos < text_length_) {
        if (UTF8::isUniCharSpace(UTF8::getUniChar(cur)) || *cur == '\n')
          white = true;
        else if (white)
          break;
        cur = nextChar(cur);
        ++new_pos;
      }
      return new_pos;
    }
  }
  else { // DIR_BACK
    if (new_pos == 0)
      return 0;

    // Always move at least one character back.
    cur = prevChar(cur);
    --new_pos;

    // Search for the first white character before nonwhite characters.
    bool nonwhite = false;
    while (new_pos != static_cast<std::size_t>(-1)) {
      if (!UTF8::isUniCharSpace(UTF8::getUniChar(cur)) && *cur != '\n')
        nonwhite = true;
      else if (nonwhite)
        break;
      if (new_pos > 0)
        cur = prevChar(cur);
      --new_pos;
    }
    return ++new_pos;
  }
}

std::size_t TextEdit::moveLineFromCursor(Direction dir) const
{
  std::size_t new_pos = current_pos_;
  const char *cur = point_;
  if (cur == gapstart_)
    cur = gapend_;

  if (dir == DIR_FORWARD) {
    if (new_pos == text_length_)
      return new_pos;

    // already at end of line?
    if( *cur == '\n' )
      return ++new_pos;

    while (new_pos < text_length_) {
      cur = nextChar(cur);
      ++new_pos;

      if (*cur == '\n')
        break;
    }
    return new_pos;
  }
  else { // DIR_BACK
    if (new_pos == 0)
      return 0;

    // already at start off line?
    cur = prevChar(cur);
    --new_pos;
    if (*cur == '\n')
      return new_pos;

    while (new_pos > 0) {
      cur = prevChar(cur);
      --new_pos;

      if (*cur == '\n')
        return ++new_pos;
    }
    return 0;
  }
}

void TextEdit::actionMoveCursor(CursorMovement step, Direction dir)
{
  moveCursor(step, dir);
}

void TextEdit::actionDelete(DeleteType type, Direction dir)
{
  deleteFromCursor(type, dir);
}

void TextEdit::actionToggleOverwrite()
{
  toggleOverwrite();
}

void TextEdit::declareBindables()
{
  // Cursor movement.
  declareBindable("textentry", "cursor-right",
    sigc::bind(sigc::mem_fun(this, &TextEdit::actionMoveCursor),
                    MOVE_LOGICAL_POSITIONS, DIR_FORWARD),
    InputProcessor::BINDABLE_NORMAL);

  declareBindable("textentry", "cursor-left",
    sigc::bind(sigc::mem_fun(this, &TextEdit::actionMoveCursor),
                    MOVE_LOGICAL_POSITIONS, DIR_BACK),
    InputProcessor::BINDABLE_NORMAL);

  declareBindable("textentry", "cursor-down",
    sigc::bind(sigc::mem_fun(this, &TextEdit::actionMoveCursor),
                    MOVE_DISPLAY_LINES, DIR_FORWARD),
    InputProcessor::BINDABLE_NORMAL);

  declareBindable("textentry", "cursor-up",
    sigc::bind(sigc::mem_fun(this, &TextEdit::actionMoveCursor),
                    MOVE_DISPLAY_LINES, DIR_BACK),
    InputProcessor::BINDABLE_NORMAL);

  declareBindable("textentry", "cursor-right-word",
    sigc::bind(sigc::mem_fun(this, &TextEdit::actionMoveCursor), MOVE_WORDS,
                    DIR_FORWARD),
    InputProcessor::BINDABLE_NORMAL);

  declareBindable("textentry", "cursor-left-word",
    sigc::bind(sigc::mem_fun(this, &TextEdit::actionMoveCursor), MOVE_WORDS,
                    DIR_BACK),
    InputProcessor::BINDABLE_NORMAL);

  declareBindable("textentry", "cursor-end",
    sigc::bind(sigc::mem_fun(this, &TextEdit::actionMoveCursor),
                    MOVE_DISPLAY_LINE_ENDS, DIR_FORWARD),
    InputProcessor::BINDABLE_NORMAL);

  declareBindable("textentry", "cursor-begin",
    sigc::bind(sigc::mem_fun(this, &TextEdit::actionMoveCursor),
                    MOVE_DISPLAY_LINE_ENDS, DIR_BACK),
    InputProcessor::BINDABLE_NORMAL);

  // Deleting text.
  declareBindable("textentry", "delete-char",
    sigc::bind(sigc::mem_fun(this, &TextEdit::actionDelete), DELETE_CHARS,
                    DIR_FORWARD),
    InputProcessor::BINDABLE_NORMAL);

  declareBindable("textentry", "backspace",
    sigc::bind(sigc::mem_fun(this, &TextEdit::actionDelete), DELETE_CHARS,
                    DIR_BACK),
    InputProcessor::BINDABLE_NORMAL);

  declareBindable("textentry", "delete-word-end",
    sigc::bind(sigc::mem_fun(this, &TextEdit::actionDelete), DELETE_WORD_ENDS,
                    DIR_FORWARD),
    InputProcessor::BINDABLE_NORMAL);

  declareBindable("textentry", "delete-word-begin",
    sigc::bind(sigc::mem_fun(this, &TextEdit::actionDelete), DELETE_WORD_ENDS,
                    DIR_BACK),
    InputProcessor::BINDABLE_NORMAL);

  declareBindable("textentry", "delete-line-end",
    sigc::bind(sigc::mem_fun(this, &TextEdit::actionDelete), DELETE_LINE_ENDS,
                    DIR_FORWARD),
    InputProcessor::BINDABLE_NORMAL);

  declareBindable("textentry", "delete-line-begin",
    sigc::bind(sigc::mem_fun(this, &TextEdit::actionDelete), DELETE_LINE_ENDS,
                    DIR_BACK),
    InputProcessor::BINDABLE_NORMAL);

  declareBindable("textentry", "newline",
    sigc::bind(sigc::mem_fun(
                 this, static_cast<void (TextEdit::*)(const char *)>(
                         &TextEdit::insertTextAtCursor)),
                    "\n"),
    InputProcessor::BINDABLE_NORMAL);

  /*
  // Overwrite.
  declareBindable("textentry", "toggle-overwrite", sigc::mem_fun(this,
        &TextEdit::actionToggleOverwrite), InputProcessor::BINDABLE_NORMAL);
  */
}

} // namespace CppConsUI

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
