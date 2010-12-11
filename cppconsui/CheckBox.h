/*
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
 * CheckBox class.
 *
 * @ingroup cppconsui
 */

#ifndef __CHECKBOX_H__
#define __CHECKBOX_H__

#include "Widget.h"

/**
 * CheckBox.
 */
class CheckBox
: public Widget
{
public:
  enum Style {
    STYLE_DEFAULT,
    STYLE_YES_NO = STYLE_DEFAULT,
    STYLE_BOX
  };

  CheckBox(int w, int h, const gchar *text_ = NULL,
      bool default_state = false);
  explicit CheckBox(const gchar *text_ = NULL, bool default_state = false);
  virtual ~CheckBox();

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

  virtual void SetState(bool new_state);
  virtual bool GetState() const { return state; }

  virtual void SetStyle(Style new_style);
  virtual Style GetStyle() const { return style; }

  /**
   * Emited signal when a checkbox is pressed/activated.
   */
  sigc::signal<void, CheckBox&, bool> signal_toggle;

protected:
  gchar *text;
  bool state;
  Style style;

private:
  CheckBox(const CheckBox&);
  CheckBox& operator=(const CheckBox&);

  void ActionToggle();

  /**
   * Registration of defined keys.
   */
  DECLARE_SIG_REGISTERKEYS();
  static bool RegisterKeys();
  void DeclareBindables();
};

#endif // __CHECKBOX_H__
