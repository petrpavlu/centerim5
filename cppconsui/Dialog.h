/*
 * Copyright (C) 2008 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2011 by CenterIM developers
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
 * Dialog class.
 *
 * @ingroup cppconsui
 */

#ifndef __DIALOG_H__
#define __DIALOG_H__

#include "AbstractDialog.h"

namespace CppConsUI
{

class Dialog
: public AbstractDialog
{
public:
  Dialog(int x, int y, int w, int h, const char *title = NULL,
      LineStyle::Type ltype = LineStyle::DEFAULT);
  explicit Dialog(const char *title = NULL,
      LineStyle::Type ltype = LineStyle::DEFAULT);
  virtual ~Dialog() {}

  /**
   * Signal emitted when the user closes the dialog.
   */
  sigc::signal<void, Dialog&, ResponseType> signal_response;

protected:
  // AbstractDialog
  virtual void EmitResponse(ResponseType response);

private:
  Dialog(const Dialog&);
  Dialog& operator=(const Dialog&);
};

} // namespace CppConsUI

#endif // __DIALOG_H__
