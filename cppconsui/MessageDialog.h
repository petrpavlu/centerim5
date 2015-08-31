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
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/// @file
/// MessageDialog class.
///
/// @ingroup cppconsui

#ifndef MESSAGEDIALOG_H
#define MESSAGEDIALOG_H

#include "AbstractDialog.h"
#include "TextView.h"

namespace CppConsUI {

class MessageDialog : public AbstractDialog {
public:
  MessageDialog(const char *title, const char *text);
  virtual ~MessageDialog() {}

  /// Signal emitted when user closes the dialog.
  sigc::signal<void, MessageDialog &, ResponseType> signal_response;

protected:
  TextView *textview_;

  // AbstractDialog
  virtual void emitResponse(ResponseType response);

private:
  CONSUI_DISABLE_COPY(MessageDialog);
};

} // namespace CppConsUI

#endif // MESSAGEDIALOG_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
