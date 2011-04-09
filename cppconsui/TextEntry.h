/*
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

/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Ported to c++ and modified for cppconsui/centerim.
 */

/**
 * @file
 * TextEntry class
 *
 * @ingroup cppconsui
 */

#ifndef __TEXT_ENTRY_H__
#define __TEXT_ENTRY_H__

#include "Widget.h"

/** Initial size of buffer, in bytes. */
#define MIN_SIZE 16

/** Maximum size of text buffer, in bytes. */
#define MAX_SIZE G_MAXUSHORT

class TextEntry
: public Widget
{
public:
  enum Flag {
    FLAG_ALPHABETIC = 1 << 0,
    FLAG_NUMERIC = 1 << 1,
    FLAG_NOSPACE = 1 << 2,
    FLAG_NOPUNCTUATION = 1 << 3,
    FLAG_HIDDEN = 1 << 4
  };

  TextEntry(int w, int h, const char *text_ = NULL);
  explicit TextEntry(const char *text_ = NULL);
  virtual ~TextEntry();

  // Widget
  virtual void Draw();
  virtual bool ProcessInputText(const TermKeyKey &key);

  /**
   * Sets a new text and redraws itself.
   */
  virtual void SetText(const char *new_text);
  /**
   * Returns previously set text.
   */
  virtual const char *GetText() const { return text; }

  void SetFlags(int flags);
  int GetFlags() const { return flags; }

  void SetPosition(int position);

  sigc::signal<void, TextEntry&> signal_text_changed;

protected:
  char *text;

  int current_pos; ///< Current cursor position.

  bool editable;
  bool overwrite_mode;

  int flags; ///< Bitmask indicating which input we accept.

  int text_max_length; ///< Max char length.
  int text_size; ///< Allocated size, in bytes.
  int text_bytes; ///< Length in use, in bytes.
  int text_length; ///< Length in use, in chars.

  void InsertTextAtCursor(const char *new_text, int new_text_bytes = -1);
  void DeleteText(int start_pos, int end_pos);

  void DeleteFromCursor(DeleteType type, int direction);

  void MoveCursor(CursorMovement step, int direction);
  void ToggleOverwrite();

  void RecalculateLengths();

  int MoveLogically(int start, int count);
  int MoveForwardWord(int start);
  int MoveBackwardWord(int start);

private:
  TextEntry(const TextEntry&);
  TextEntry& operator=(const TextEntry&);

  void ActionMoveCursor(CursorMovement step, int direction);
  void ActionDelete(DeleteType type, int direction);
  void ActionToggleOverwrite();
  void ActionActivate();

  void DeclareBindables();
};

#endif // __TEXT_ENTRY_H__
