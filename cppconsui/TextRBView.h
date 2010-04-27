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

#ifndef __TEXTVIEW_H__
#define __TEXTVIEW_H__

#include "Widget.h"
#include "TextBuffer.h"

class TextRBView
: public Widget 
{
	public:
		typedef TextBuffer::char_iterator char_iterator;
		typedef TextBuffer::line_iterator line_iterator;

		TextRBView(int w, int h);
		~TextRBView();

		char_iterator append (const char *text, int len);

		/* Widget */
		void Draw(void);

		Rect GetScrollSize(void);
		void SetScrollSize(const int width, const int height);
		void AdjustScroll(const int x, const int y);
		void AdjustScroll(const Rect);
		Rect GetScrollPosition(void);

		/* Getting iterators into the buffer. */
		char_iterator begin(void) const;
		char_iterator back(void) const;
		char_iterator end(void) const;

		char_iterator reverse_begin(void) const;
		char_iterator reverse_back(void) const;
		char_iterator reverse_end(void) const;


	protected:
		int view_left, view_top;
		WrapMode wrap_mode;

	private:
		TextRBView();
		TextRBView(const TextRBView &);
		TextRBView& operator=(const TextRBView&);

		TextBuffer buffer;
};

#endif /* __TEXTVIEW_H__ */
