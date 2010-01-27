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

/* Initial size of buffer, in bytes */
#define MIN_SIZE 16

/* Maximum size of text buffer, in bytes */
#define MAX_SIZE G_MAXUSHORT

class TextEntry
: public Label
{
	public:
		typedef enum {
			FlagAlphabetic = 1<<0,
			FlagNumeric = 1<<1,
			FlagNoSpace = 1<<2,
			FlagNoPunctuation = 1<<3,
			FlagHidden = 1<<4
		} Flag;

		TextEntry(Widget& parent, int x, int y, int w, int h, const gchar *text_);
		TextEntry(Widget& parent, int x, int y, const gchar *text_);

		virtual ~TextEntry();

		virtual void Draw(void);

		virtual int ProcessInputText(const char *input, const int bytes);

		int Flags(void) { return flags; }
		void Flags(int flags);
		
		/** @todo Obscured is not used nor implemented yet. Is it TextEntry specific ? Or is it Widget specific ? */
		bool Obscured(void) { return obscured; }
		void Obscured(bool obscure);

		sigc::signal<void> signal_text_changed;

		virtual void SetText(const gchar *text_);

	protected:
		void backspace(void);

		void insert_text (const gchar *new_text, gint new_text_length, gint *position);
		void delete_text (gint start_pos, gint end_pos);
		void delete_selection (void);
		void delete_whitespace (void);

		void insert_at_cursor (const gchar *str);
		void delete_from_cursor (DeleteType type, gint count);

		void set_selection_bounds (gint start, gint end);
		void set_positions (gint current_pos, gint selection_bound);
		void set_position (gint position);
		void recompute(void);
		void move_cursor (CursorMovement step, gint count, gboolean extend_selection);
		void toggle_overwrite (void);

		gint current_pos; /* cursor position */
		gint selection_bound;

		bool editable;
		bool overwrite_mode;
		bool obscured;

		int flags; /* Bitmask indicating which input we accept. */

		guint16 text_max_length;
		guint16 text_size;    /**< allocated size, in bytes */
		guint16 n_bytes;      /**< length in use, in bytes */
		guint16 text_length;  /**< length in use, in chars */

	private:
		TextEntry();
		TextEntry(const TextEntry&);

		TextEntry& operator=(const TextEntry&);

		void trash_area (gchar *area, gsize len);
		gint move_logically (gint start, gint count);
		gint move_visually (gint start, gint count);
		gint move_backward_word (gint start, gboolean allow_whitespace);
		gint move_forward_word (gint start, gboolean allow_whitespace);
		gboolean get_selection_bounds (gint *start, gint *end);

		void ActionMoveCursor(CursorMovement step, int count, bool extend_selection);
		void ActionSelectAll(bool select_all);
		void ActionDelete(DeleteType type, gint count);
		void ActionBackspace(void);
		void ActionToggleOverwrite(void);

		virtual void OnActivate(void);
	
		/** it handles the automatic registration of defined keys */
		DECLARE_SIG_REGISTERKEYS();
		static bool RegisterKeys();
		void DeclareBindables();

};

#endif /* __TEXT_ENTRY_H__ */
