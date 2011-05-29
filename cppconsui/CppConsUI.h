/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
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
 * General classes, functions and enumerations.
 *
 * @ingroup cppconsui
 *
 * @todo CppConsUI namespace
 */

#ifndef __CPPCONSUI_H__
#define __CPPCONSUI_H__

namespace CppConsUI
{

/// @todo move into editable abstract class?
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
  DELETE_WORDS,
  DELETE_WORD_ENDS,
  DELETE_DISPLAY_LINE_ENDS,
  DELETE_PARAGRAPH_ENDS,
  DELETE_DISPLAY_LINES,
  DELETE_PARAGRAPHS,
  DELETE_WHITESPACE
};

enum WrapMode {
  WRAP_NONE,
  WRAP_CHAR,
  WRAP_WORD,
  WRAP_WORD_CHAR
};

enum DirectionType {
  DIR_TAB_FORWARD,
  DIR_TAB_BACKWARD,
  DIR_UP,
  DIR_DOWN,
  DIR_LEFT,
  DIR_RIGHT,
  DIR_NONE
};

enum ScrollStep {
  SCROLL_STEPS,
  SCROLL_PAGES,
  SCROLL_ENDS,
  SCROLL_HORIZONTAL_STEPS,
  SCROLL_HORIZONTAL_PAGES,
  SCROLL_HORIZONTAL_ENDS
};

class Point
{
public:
  Point();
  Point(int x, int y);

  int GetX() const { return x; }
  int GetY() const { return y; }

  int x, y;

protected:

private:
};

class Rect: public Point
{
public:
  Rect();
  Rect(int x, int y, int w, int h);

  int GetWidth() const { return width; }
  int GetHeight() const { return height; }
  int GetLeft() const { return x; }
  int GetTop() const { return y; }
  int GetRight() const { return x + width - 1; }
  int GetBottom() const { return y + height - 1; }

  int width, height;

protected:

private:
};

} // namespace CppConsUI

#endif // __CPPCONSUI_H__
