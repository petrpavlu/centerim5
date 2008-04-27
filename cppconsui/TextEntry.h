/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
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

#ifndef __TEXT_ENTRY_H__
#define __TEXT_ENTRY_H__

#include "Label.h"

#include <glibmm/ustring.h>
#include <vector>

class TextEntry
: public Label
{
	public:
		TextEntry(Widget& parent, int x, int y, int w, int h, const gchar *fmt, ...);
		TextEntry(Widget& parent, int x, int y, const gchar *fmt, ...);

		virtual ~TextEntry();

		virtual void Draw(void);

		virtual int ProcessInputText(const char *input, const int bytes);

		sigc::signal<void> signal_text_changed;

	protected:
		enum CursorMovement {
			MOVE_LOGICAL_POSITIONS,
			MOVE_VISUAL_POSITIONS,
			MOVE_WORDS,
			MOVE_DISPLAY_LINES,
			MOVE_DISPLAY_LINE_ENDS,
			MOVE_PARAGRAPHS,
			MOVE_PARAGRAPH_ENDS,
			MOVE_PAGES,
			MOVE_BUFFER_ENDS,
			MOVE_HORIZONTAL_PAGES
		};
		enum DeleteType {
			DELETE_CHARS,
			DELETE_WORDS,
			DELETE_WORD_ENDS,
			DELETE_DISPLAY_LINE_ENDS,
			DELETE_PARAGRAPH_ENDS,
			DELETE_DISPLAY_LINES,
			DELETE_PARAGRAPHS,
			DELETE_WHITESPACE
		};

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

		void DeclareBindables(void);
		void BindActions(void);
};

#endif /* __TEXT_ENTRY_H__ */
