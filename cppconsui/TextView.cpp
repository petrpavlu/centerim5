/* -*- Mode: C; c-file-style: "gnu"; tab-width: 8 -*- */
/* GTK - The GIMP Toolkit
 * gtktextview.c Copyright (C) 2000 Red Hat, Inc.
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

#include "CppConsUI.h"
#include "TextView.h"

#include <string.h>

/* TODO implement copy operator for textbuffer and textrbtree
TextView::TextView(Widget& parent, int x, int y, int w, int h, TextBuffer *buffer)
: Widget(parent, x, y, w, h)
{
	this->buffer = buffer;
}*/

TextView::TextView(Widget& parent, int x, int y, int w, int h)
: Widget(parent, x, y, w, h)
, view_left(0)
, view_top(0)
, wrap_mode(WRAP_NONE)
{
}

TextView::~TextView(void)
{
}

TextView::char_iterator TextView::append (const char *text, int len)
{
	if (len < 0) {
		len = strlen(text);
	}

	return buffer.insert(buffer.back(), text, len);
}

void TextView::Draw(void)
{
	char_iterator line_iter;	/* Current line to be drawm. */
	char_iterator line_end;		/* Line just after the last line to be drawn. */
	char_iterator char_iter;	/* Current character to be drawn. */
	char_iterator char_end;		/* Char just after the lst char to be drawn. */
	unsigned int x, y;

	line_iter = line_end = begin();

	/* Move the begin and end iterators to their positions. */
	line_iter.forward_lines(view_top);
	line_end = line_iter;
	line_end.forward_lines(Height());

	y = 0;
	while (line_iter != line_end) {
		char_iter = line_iter;

		/* Move the line iterator to the next line. We also use this
		 * as guard for the character drawing loop, as this next line
		 * is where we should stop */
		line_iter.forward_lines(1);

		/* Skip view_left columns at the beginning of the string. */
		char_iter.forward_cols(view_left);
		char_end = char_iter;
		char_end.forward_cols(Width());

		/* We use char_end to indicate what the last char to draw is.
		 * Line_iter here is on the first character of the next line.
		 * Since the line starting from char_iter might not have Width()
		 * columns left, we set the limit at min(line_iter, char_end) */
		if (line_iter < char_end) {
			char_end = line_iter;
		}

		x = 0;

		/* After skipping columns we may have that we are at the
		 * second column of a 2-column character. In this case
		 * we need to draw an empty column. */
		if (!char_iter.valid_char()) {
			area->mvaddstr(y, x, " ");
			/* Move to the next valid char. */
			char_iter.forward_chars(1);
			x += 1;
		}

		/* Note that line_iter is at the next line at this point; eg
		 * where we should stop drawing characters. */
		while (char_iter < char_end) {
			area->mvaddnstr(y, x, *char_iter, char_iter.char_bytes());
			x += char_iter.char_cols();
			char_iter.forward_chars(1);
		}

		/* Clear until the end of the line. We
		 * can use this function since mvwaddnstr() also
		 * moves the cursor. */
		area->clrtoeol();

		y++;
	}

	Widget::Draw();
}

Rect TextView::GetScrollSize(void)
{
	Rect r;

	r.x = 0;
	r.y = 0;
	r.width = Width();
	r.height = Height();

	return r;
}

void TextView::SetScrollSize(const int width, const int height)
{
	//TODO omit warning that user should use Resize();
}

void TextView::AdjustScroll(const int newx, const int newy)
{
	if (newx < 0 || newy < 0 || newx > Width() || newy > Height())
		return;

	if (newx > view_left + w - 1) {
		view_left = newx - w + 1;
	} else if (newx < view_left) {
		view_left = newx;
	}

	if (newy > view_top + h - 1) {
		view_top = newy - h + 1;
	} else if (newy < view_top) {
		view_top = newy;
	}
}

Rect TextView::GetScrollPosition(void)
{
	Rect r;

	r.x = view_left;
	r.y = view_top;
	r.width = Width();
	r.height = Height();

	return r;
}

TextView::char_iterator TextView::begin(void) const
{
	return buffer.begin();
}

TextView::char_iterator TextView::back(void) const
{
	return buffer.back();
}

TextView::char_iterator TextView::end(void) const
{
	return buffer.end();
}

TextView::char_iterator TextView::reverse_begin(void) const
{
	return buffer.reverse_begin();
}

TextView::char_iterator TextView::reverse_back(void) const
{
	return buffer.reverse_back();
}

TextView::char_iterator TextView::reverse_end(void) const
{
	return buffer.reverse_end();
}
