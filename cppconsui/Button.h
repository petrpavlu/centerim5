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

#include "Label.h"

/**
 * This class implements a simple button behaviour.
 *
 * The button does not keep states like pressed or not and it can call back
 * one (or more) functions when pressed.
 */
class Button
: public Label
{
public:
  Button(int w, int h, const gchar *text = NULL);
  explicit Button(const gchar *text = NULL);
  virtual ~Button() {}

  // Widget
  virtual void Draw();

  /**
   * Emited signal when a button is pressed/activated.
   */
  sigc::signal<void, Button&> signal_activate;

protected:

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
