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
/// TextEntry class
///
/// @ingroup cppconsui

#ifndef TEXT_ENTRY_H
#define TEXT_ENTRY_H

#include "TextEdit.h"

namespace CppConsUI {

class TextEntry : public TextEdit {
public:
  TextEntry(int w, int h, const char *text_ = nullptr, int flags_ = 0);
  explicit TextEntry(const char *text_ = nullptr, int flags_ = 0);
  virtual ~TextEntry() override {}

private:
  CONSUI_DISABLE_COPY(TextEntry);

  void actionActivate();

  void declareBindables();
};

} // namespace CppConsUI

#endif // TEXT_ENTRY_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
