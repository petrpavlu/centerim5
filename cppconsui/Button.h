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
  enum Type {
    TYPE_SIMPLE, // text
    TYPE_DOUBLE // text: value
    //TYPE_TRIPLE  // text: value [unit]
  };

  Button(int w, int h, const char *text_ = NULL);
  explicit Button(const char *text_ = NULL);
  Button(Type type_, int w, int h, const char *text_ = NULL,
      const char *value_ = NULL);
  Button(Type type_, const char *text_ = NULL,
      const char *value_ = NULL);
  virtual ~Button();

  // Widget
  virtual void Draw();

  virtual void SetType(Type new_type);
  virtual Type GetType() const { return type; }

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

  /**
   * Emited signal when the button is pressed/activated.
   */
  sigc::signal<void, Button&> signal_activate;

protected:
  Type type;
  char *text;
  char *value;

private:
  Button(const Button&);
  Button& operator=(const Button&);

  void ActionActivate();

  void DeclareBindables();
};

} // namespace CppConsUI

#endif // __BUTTON_H__
