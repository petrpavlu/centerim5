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

/** @file TextEntry.h TextEntry class
 * @ingroup cppconsui
 */

#ifndef __TEXT_ENTRY_H__
#define __TEXT_ENTRY_H__

#include "Label.h"

/** Initial size of buffer, in bytes. */
#define MIN_SIZE 16

/** Maximum size of text buffer, in bytes. */
#define MAX_SIZE G_MAXUSHORT

class TextEntry
: public Label
{
	public:
		enum Flag {
			FlagAlphabetic = 1 << 0,
			FlagNumeric = 1 << 1,
			FlagNoSpace = 1 << 2,
			FlagNoPunctuation = 1 << 3,
			FlagHidden = 1 << 4
		};

		TextEntry(Widget &parent, int x_, int y_, int w_, int h_, const gchar *text_);
		TextEntry(Widget &parent, int x_, int y_, const gchar *text_);

		virtual ~TextEntry();

		// Widget
		virtual void Draw();
		virtual bool ProcessInputText(const TermKeyKey &key);

		// Label
		virtual void SetText(const gchar *text_);

		int GetFlags() { return flags; }
		void SetFlags(int flags);
		void SetPosition(int position);
		
		sigc::signal<void> signal_text_changed;

	protected:
		void InsertTextAtCursor(const gchar *new_text, int new_text_bytes = -1);
		void DeleteText(int start_pos, int end_pos);

		void DeleteFromCursor(DeleteType type, int direction);

		void MoveCursor(CursorMovement step, int direction);
		void ToggleOverwrite();

		int current_pos; ///< Current cursor position.

		bool editable;
		bool overwrite_mode;

		int flags; ///< Bitmask indicating which input we accept.

		int text_max_length; ///< Max char length.
		int text_size; ///< Allocated size, in bytes.
		int text_bytes; ///< Length in use, in bytes.
		int text_length; ///< Length in use, in chars.

		void RecalculateLengths();

	private:
		TextEntry();
		TextEntry(const TextEntry&);
		TextEntry& operator=(const TextEntry&);

		int MoveLogically(int start, int count);
		int MoveBackwardWord(int start);
		int MoveForwardWord(int start);

		void ActionMoveCursor(CursorMovement step, int direction);
		void ActionDelete(DeleteType type, int direction);
		void ActionToggleOverwrite();
		void ActionActivate();
	
		/** it handles the automatic registration of defined keys */
		DECLARE_SIG_REGISTERKEYS();
		static bool RegisterKeys();
		void DeclareBindables();

};

#endif /* __TEXT_ENTRY_H__ */
