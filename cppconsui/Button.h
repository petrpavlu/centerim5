// Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
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
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/// @file
/// Button class.
///
/// @ingroup cppconsui

#ifndef BUTTON_H
#define BUTTON_H

#include "Widget.h"

namespace CppConsUI {

/// This class implements a simple button behaviour.
///
/// The button does not keep states like pressed or not and it can call back one
/// (or more) functions when pressed.
class Button : public Widget {
public:
  enum Flag {
    FLAG_VALUE = 1 << 0,
    FLAG_UNIT = 1 << 1,
    FLAG_RIGHT = 1 << 2,
  };

  Button(
    int w, int h, const char *text = NULL, int flags = 0, bool masked = false);
  explicit Button(const char *text = NULL, int flags = 0, bool masked = false);
  Button(int w, int h, int flags = 0, const char *text = NULL,
    const char *value = NULL, const char *unit = NULL, const char *right = NULL,
    bool masked = false);
  Button(int flags, const char *text = NULL, const char *value = NULL,
    const char *unit = NULL, const char *right = NULL, bool masked = false);
  virtual ~Button();

  // Widget
  virtual int draw(Curses::ViewPort area, Error &error);

  virtual void setFlags(int new_flags);
  virtual int getFlags() const { return flags_; }

  /// Sets a new text and redraws itself.
  virtual void setText(const char *new_text);

  /// Returns previously set text.
  virtual const char *getText() const { return text_; }

  virtual void setValue(const char *new_value);
  virtual void setValue(int new_value);
  virtual const char *getValue() const { return value_; }

  virtual void setUnit(const char *new_unit);
  virtual const char *getUnit() const { return unit_; }

  virtual void setRight(const char *new_right);
  virtual const char *getRight() const { return right_; }

  virtual void setMasked(bool new_masked);
  virtual bool isMasked() const { return masked_; }

  /// Signal emitted when the button is pressed/activated.
  sigc::signal<void, Button &> signal_activate;

protected:
  int flags_;

  char *text_;
  int text_width_;
  int text_height_;

  char *value_;
  int value_width_;

  char *unit_;
  int unit_width_;

  char *right_;
  int right_width_;

  bool masked_;

private:
  CONSUI_DISABLE_COPY(Button);

  void actionActivate();

  void declareBindables();
};

} // namespace CppConsUI

#endif // BUTTON_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
