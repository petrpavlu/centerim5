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
/// CheckBox class.
///
/// @ingroup cppconsui

#ifndef __CHECKBOX_H__
#define __CHECKBOX_H__

#include "Widget.h"

namespace CppConsUI {

/// CheckBox.
class CheckBox : public Widget {
public:
  CheckBox(int w, int h, const char *text = NULL, bool checked = false);
  explicit CheckBox(const char *text = NULL, bool checked = false);
  virtual ~CheckBox();

  // Widget
  virtual int draw(Curses::ViewPort area, Error &error);

  /// Sets new text label.
  virtual void setText(const char *new_text);

  /// Returns current text label.
  virtual const char *getText() const { return text_; }

  virtual void setChecked(bool new_checked);
  virtual bool isChecked() const { return checked_; }

  /// Signal emitted when the checkbox is pressed/activated.
  sigc::signal<void, CheckBox &, bool> signal_toggle;

protected:
  char *text_;
  int text_width_;
  int text_height_;

  bool checked_;

private:
  CONSUI_DISABLE_COPY(CheckBox);

  void actionToggle();

  void declareBindables();
};

} // namespace CppConsUI

#endif // __CHECKBOX_H__

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
