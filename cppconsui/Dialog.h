// Copyright (C) 2008 Mark Pustjens <pustjens@dds.nl>
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
/// Dialog class.
///
/// @ingroup cppconsui

#ifndef DIALOG_H
#define DIALOG_H

#include "AbstractDialog.h"

namespace CppConsUI {

class Dialog : public AbstractDialog {
public:
  Dialog(int x, int y, int w, int h, const char *title = nullptr);
  explicit Dialog(const char *title = nullptr);
  virtual ~Dialog() override {}

  /// Signal emitted when user closes the dialog.
  sigc::signal<void, Dialog &, ResponseType> signal_response;

protected:
  // AbstractDialog
  virtual void emitResponse(ResponseType response) override;

private:
  CONSUI_DISABLE_COPY(Dialog);
};

} // namespace CppConsUI

#endif // DIALOG_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
