/*
 * Copyright (C) 2008 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010 by CenterIM developers
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
 * MessageDialog class.
 *
 * @ingroup cppconsui
 */

#ifndef __MESSAGEDIALOG_H__
#define __MESSAGEDIALOG_H__

#include "AbstractDialog.h"

class MessageDialog
: public AbstractDialog
{
public:
  MessageDialog(const gchar *title, const gchar *text);
  virtual ~MessageDialog() {}

  /**
   * Signal emitted when the user closes the dialog.
   */
  sigc::signal<void, MessageDialog&, ResponseType> signal_response;

protected:
  Label *label;

  // AbstractDialog
  virtual void EmitResponse(ResponseType response);

private:
  MessageDialog(const MessageDialog&);
  MessageDialog& operator=(const MessageDialog&);
};

#endif // __MESSAGEDIALOG_H__
