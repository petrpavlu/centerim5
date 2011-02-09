/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010 by CenterIM developers
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
  Button(int w, int h, const gchar *text_ = NULL, const gchar *value_ = NULL);
  explicit Button(const gchar *text_ = NULL, const gchar *value_ = NULL);
  virtual ~Button();

  // Widget
  virtual void Draw();

  /**
   * Sets a new text and redraws itself.
   */
  virtual void SetText(const gchar *new_text);
  /**
   * Returns previously set text.
   */
  virtual const gchar *GetText() const { return text; }

  virtual void SetValue(const gchar *new_value);
  virtual void SetValue(int new_value);
  virtual const gchar *GetValue() const { return value; }

  virtual void SetValueVisibility(bool visible);
  virtual bool GetValueVisibility() const { return value_visible; }

  /**
   * Emited signal when the button is pressed/activated.
   */
  sigc::signal<void, Button&> signal_activate;

protected:
  gchar *text;
  gchar *value;
  bool value_visible;

private:
  Button(const Button&);
  Button& operator=(const Button&);

  void ActionActivate();

  /**
   * Registration of defined keys.
   */
  DECLARE_SIG_REGISTERKEYS();
  static bool RegisterKeys();
  void DeclareBindables();
};

#endif // __BUTTON_H__
