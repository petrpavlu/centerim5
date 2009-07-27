/*
 * Copyright (C) 2008 by Mark Pustjens <pustjens@dds.nl>
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

#ifndef __TEXTLINE_H__
#define __TEXTLINE_H__

#include "config.h"

#include "TextLineRBTree.h"

class TextLine
{
	public:
		typedef TextLineRBTree::char_iterator char_iterator;
		typedef TextLineRBTree::col_iterator col_iterator;
		typedef TextLineRBTree::byte_iterator byte_iterator;

		TextLine();
		TextLine(const TextLine& line, unsigned int index, unsigned int num);
		~TextLine();

		TextLine(const TextLine &);

		TextLine& operator=(const TextLine&);

		void insert (unsigned int index, const char* str, unsigned int num);
		void erase (unsigned int from, unsigned int to);

		/* Return number of bytes/chars/columns in the line. */
		unsigned int bytes(void);
		unsigned int chars(void);
		unsigned int columns(void);
		unsigned int lines(void); /* Number of display lines. */

		gchar* get_pointer_at_char_offset(unsigned int offset);
		unsigned int byte_to_char_offset(unsigned int offset);
		unsigned int byte_to_col_offset(unsigned int offset);
		unsigned int col_to_byte_offset(unsigned int offset);
		unsigned int char_to_byte_offset(unsigned int offset);

		/* Count the number of bytes until we reach the n'th character. */
		unsigned int byte_count_to_char_offset(unsigned int n);
		/* Count the number of characters before the n'th byte. */
		unsigned int char_count_to_byte_offset(unsigned int n);

		/* Getting iterators into the buffer. */
		char_iterator begin(void) const;
		char_iterator back(void) const;
		char_iterator end(void) const;

		char_iterator reverse_begin(void) const;
		char_iterator reverse_back(void) const;
		char_iterator reverse_end(void) const;

		char_iterator insert (const char_iterator iter, const char *text, int len);
		//char_iterator insert (const char *text, int len);

		char_iterator append (const char *text, int len);

	protected:

	private:

		TextLineRBTree tree;
};

#endif /* __TEXTLINE_H__ */
