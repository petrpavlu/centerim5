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

#ifndef __TEXTBUFFER_H__
#define __TEXTBUFFER_H__

#include "config.h"

#include "TextRBTree.h"

class TextBuffer
{
	public:
		typedef TextRBTree::char_iterator char_iterator;
		typedef TextRBTree::line_iterator line_iterator;

		TextBuffer();
		~TextBuffer();

		/* Getting iterators into the buffer. */
		char_iterator begin(void) const;
		char_iterator back(void) const;
		char_iterator end(void) const;

		char_iterator reverse_begin(void) const;
		char_iterator reverse_back(void) const;
		char_iterator reverse_end(void) const;

		char_iterator insert (const char_iterator iter, const char *text, int len);
		char_iterator insert (const char *text, int len);

		char_iterator append (const char *text, int len);

		char_iterator get_iter_at_cursor (void);
		char_iterator get_iter_at_line (int top);
	protected:

	private:
		TextBuffer(const TextBuffer &);

		TextBuffer& operator=(const TextBuffer&);

		TextRBTree tree;
		char_iterator cursor;
};

#endif /* __TEXTBUFFER_H__ */
