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
// along with CenterIM.  If not, see <http://www.gnu.org/licenses/>.

/// @file
/// TextEntry class implementation
///
/// @ingroup cppconsui

#include "TextEntry.h"

#include "Container.h"

namespace CppConsUI {

TextEntry::TextEntry(int w, int h, const char *text_, int flags_)
  : TextEdit(w, h, text_, flags_, true, false)
{
  declareBindables();
}

TextEntry::TextEntry(const char *text_, int flags_)
  : TextEdit(AUTOSIZE, 1, text_, flags_, true, false)
{
  declareBindables();
}

void TextEntry::actionActivate()
{
  if (parent_ != nullptr)
    parent_->moveFocus(Container::FOCUS_NEXT);
}

void TextEntry::declareBindables()
{
  // Non text editing bindables.
  declareBindable("textentry", "activate",
    sigc::mem_fun(this, &TextEntry::actionActivate),
    InputProcessor::BINDABLE_NORMAL);
}

} // namespace CppConsUI

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
