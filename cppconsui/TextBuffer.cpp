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

#include "config.h"

#include "TextBuffer.h"
#include "CppConsUI.h"

#include <cassert>

TextBuffer::TextBuffer()
: cursor(tree)
{
	cursor = tree.begin();
}

TextBuffer::~TextBuffer()
{
}

TextBuffer::char_iterator TextBuffer::begin(void) const
{
	return tree.begin();
}

TextBuffer::char_iterator TextBuffer::back(void) const
{
	return tree.back();
}

TextBuffer::char_iterator TextBuffer::end(void) const
{
	return tree.end();
}

TextBuffer::char_iterator TextBuffer::reverse_begin(void) const
{
	return tree.reverse_begin();
}

TextBuffer::char_iterator TextBuffer::reverse_back(void) const
{
	return tree.reverse_back();
}

TextBuffer::char_iterator TextBuffer::reverse_end(void) const
{
	return tree.reverse_end();
}

TextBuffer::line_iterator TextBuffer::begin_line(void) const
{
	return tree.begin_line();
}

TextBuffer::line_iterator TextBuffer::back_line(void) const
{
	return tree.back_line();
}

TextBuffer::line_iterator TextBuffer::end_line(void) const
{
	return tree.end_line();
}

TextBuffer::line_iterator TextBuffer::reverse_begin_line(void) const
{
	return tree.reverse_begin_line();
}

TextBuffer::line_iterator TextBuffer::reverse_back_line(void) const
{
	return tree.reverse_back_line();
}

TextBuffer::line_iterator TextBuffer::reverse_end_line(void) const
{
	return tree.reverse_end_line();
}

TextBuffer::char_iterator TextBuffer::insert(const TextBuffer::char_iterator iter, const char *text, int len) 
{
	return tree.insert(iter, text, len);
}

/*TextBuffer::char_iterator TextBuffer::insert(const TextBuffer::char_iterator _iter, const char *text, int len)
{
	int chunk_len; / number of characters in current chunk. /
	int sol; / start of line /
	int eol; / index of character just after last one in current chunk. /
	int delim; / index of paragraph delimiter /
	char_iterator line_iter(tree);
	TextLine *line;
	char_iterator iter(_iter);
	line_iterator tmp;

	eol = 0;
	sol = 0;
	line_iter = iter;

	while (eol < len)
	{
		sol = eol;

		find_paragraph_boundary (text + sol, len - sol, &delim, &eol);

		/ make these relative to the start of the text /
		delim += sol;
		eol += sol;

		assert (eol >= sol);
		assert (delim >= sol);
		assert (eol >= delim);
		assert (sol >= 0);
		assert (eol <= len);

		chunk_len = eol - sol;

		TODO assert (g_utf8_validate ((const gchar*)text[sol]+1, 1, NULL));

		/ insert the paragraph in the current line just after the cursor position /
		line_iter->insert(line_iter.byte_offset, &text[sol], chunk_len);

		/ advance the char_iterator by chunk_len bytes /
		iter.forward_bytes(chunk_len);

		if (delim == eol)
		{
			/ chunk didn't end with a paragraph separator /
			assert (eol == len);
			break;
		}

		/
		  The chunk ended with a newline, so create a new TextLine
		  and move the remainder of the old line to it.
		 /
		tmp = iter;
		line = new TextLine(
				*tmp,
				iter.byte_offset,
				iter->bytes());

		/ insert the new line after the current line. The returned
		  char_iterator points to the beginning of the new line.
		 /
		iter = tree.insert(++iter, *line);
	}

	return iter;
}*/

TextBuffer::char_iterator TextBuffer::insert (const char *text, int len)
{
	return tree.insert (cursor, text, len);
}

void TextBuffer::append (const char *text, int len)
{
	TextLine line;

	line.insert(0, text, len);

	tree.append (line);
}

void TextBuffer::prefix (const char *text, int len)
{
	TextLine line;

	line.insert(0, text, len);

	tree.prefix (line);
}

TextBuffer::char_iterator TextBuffer::get_iter_at_cursor (void)
{
	return cursor;
}
