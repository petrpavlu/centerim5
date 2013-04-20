/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2013 by CenterIM developers
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
 * Button class.
 *
 * @ingroup cppconsui
 */

#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "Widget.h"

namespace CppConsUI
{

/**
 * This class implements a simple button behaviour.
 *
 * The button does not keep states like pressed or not and it can call back
 * one (or more) functions when pressed.
 */
class Button
: public Widget
{
public:
  enum Flag {
    FLAG_VALUE = 1 << 0,
    FLAG_UNIT = 1 << 1,
    FLAG_RIGHT = 1 << 2
  };

  Button(int w, int h, const char *text_ = NULL, int flags_ = 0,
      bool masked_ = false);
  explicit Button(const char *text_ = NULL, int flags_ = 0,
      bool masked_ = false);
  Button(int w, int h, int flags_ = 0, const char *text_ = NULL,
      const char *value_ = NULL, const char *unit_ = NULL,
      const char *right_ = NULL, bool masked_ = false);
  Button(int flags_, const char *text_ = NULL, const char *value_ = NULL,
      const char *unit_ = NULL, const char *right_ = NULL,
      bool masked_ = false);
  virtual ~Button();

  // Widget
  virtual void Draw();

  virtual void SetFlags(int new_flags);
  virtual int GetFlags() const { return flags; }

  /**
   * Sets a new text and redraws itself.
   */
  virtual void SetText(const char *new_text);
  /**
   * Returns previously set text.
   */
  virtual const char *GetText() const { return text; }

  virtual void SetValue(const char *new_value);
  virtual void SetValue(int new_value);
  virtual const char *GetValue() const { return value; }

  virtual void SetUnit(const char *new_unit);
  virtual const char *GetUnit() const { return unit; }

  virtual void SetRight(const char *new_right);
  virtual const char *GetRight() const { return right; }

  virtual void SetMasked(bool masked_);
  virtual bool GetMasked() const { return masked; }

  /**
   * Emited signal when the button is pressed/activated.
   */
  sigc::signal<void, Button&> signal_activate;

protected:
  int flags;
  char *text;
  int text_width;
  int text_height;
  char *value;
  int value_width;
  char *unit;
  int unit_width;
  char *right;
  int right_width;
  bool masked;

private:
  Button(const Button&);
  Button& operator=(const Button&);

  void ActionActivate();

  void DeclareBindables();
};

} // namespace CppConsUI

#endif // __BUTTON_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
