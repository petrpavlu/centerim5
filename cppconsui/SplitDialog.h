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
/// SplitDialog class.
///
/// @ingroup cppconsui

#ifndef SPLITDIALOG_H
#define SPLITDIALOG_H

#include "AbstractDialog.h"

namespace CppConsUI {

class SplitDialog : public AbstractDialog {
public:
  SplitDialog(int x, int y, int w, int h, const char *title = nullptr);
  explicit SplitDialog(const char *title = nullptr);
  virtual ~SplitDialog() override;

  // Widget
  virtual void cleanFocus() override;

  // Container
  virtual void moveFocus(FocusDirection direction) override;

  virtual void setContainer(Container &cont);
  virtual Container *getContainer() const { return container_; }

  /// Signal emitted when user closes the dialog.
  sigc::signal<void, SplitDialog &, ResponseType> signal_response;

protected:
  Container *container_;

  Widget *cont_old_focus_;
  Widget *buttons_old_focus_;
  sigc::connection cont_old_focus_conn_;
  sigc::connection buttons_old_focus_conn_;

  // AbstractDialog
  virtual void emitResponse(ResponseType response) override;

  virtual void onOldFocusVisible(Widget &activator, bool visible);

private:
  CONSUI_DISABLE_COPY(SplitDialog);
};

} // namespace CppConsUI

#endif // SPLITDIALOG_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
