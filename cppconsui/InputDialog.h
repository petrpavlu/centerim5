/*
 * Copyright (C) 2008 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2013 by CenterIM developers
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
 */

/**
 * @file
 * InputDialog class.
 *
 * @ingroup cppconsui
 */

#ifndef __INPUTDIALOG_H__
#define __INPUTDIALOG_H__

#include "AbstractDialog.h"
#include "TextEntry.h"

namespace CppConsUI
{

class InputDialog
: public AbstractDialog
{
public:
  InputDialog(const char *title, const char *defaultvalue);
  virtual ~InputDialog() {}

  virtual void setText(const char *new_text) { entry->setText(new_text); }
  virtual const char *getText() const { return entry->getText(); }

  virtual void setFlags(int new_flags) { entry->setFlags(new_flags); }
  virtual int getFlags() const { return entry->getFlags(); }

  virtual void setMasked(bool new_masked) { entry->setMasked(new_masked); }
  virtual bool isMasked() const { return entry->isMasked(); }

  /**
   * Signal emitted when the user closes the dialog.
   */
  sigc::signal<void, InputDialog&, ResponseType> signal_response;

protected:
  TextEntry *entry;

  // AbstractDialog
  virtual void emitResponse(ResponseType response);

private:
  InputDialog(const InputDialog&);
  InputDialog& operator=(const InputDialog&);
};

} // namespace CppConsUI

#endif // __INPUTDIALOG_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
