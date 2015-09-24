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
/// Label class.
///
/// @ingroup cppconsui

#ifndef LABEL_H
#define LABEL_H

#include "Widget.h"

namespace CppConsUI {

/// A widget that displays a small to medium amount of text.
class Label : public Widget {
public:
  Label(int w, int h, const char *text = NULL);
  explicit Label(const char *text = NULL);
  virtual ~Label() override;

  // Widget
  virtual int draw(Curses::ViewPort area, Error &error) override;

  /// Sets a new label text and redraws the widget.
  virtual void setText(const char *new_text);

  /// Returns a current label text.
  virtual const char *getText() const { return text_; }

protected:
  char *text_;

private:
  CONSUI_DISABLE_COPY(Label);
};

} // namespace CppConsUI

#endif // LABEL_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
