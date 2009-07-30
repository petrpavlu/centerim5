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

#include "TextLine.h"

#include <cassert>

TextLine::TextLine()
{
}

TextLine::TextLine(const TextLine &line, unsigned int from, unsigned int to)
{
	/* Do we need to copy the line at all? */
	if (from == to)
		return;

	/* Make this a copy of line. */
	*this = line;

	/* Erase the back part of the line. */
	erase(MIN(to+1, chars()), chars());

	/* Erase everything upto the from'th character. */
	erase(0, MIN(chars(), from));
}

TextLine::~TextLine()
{
}

TextLine::TextLine(const TextLine &line)
{
	*this = line;
}

TextLine& TextLine::operator=(const TextLine& line)
{
	if (this == &line) return *this;

	tree = line.tree;

	return *this;
}

void TextLine::insert(unsigned int index, const char* str, unsigned int len)
{
	TextLine::char_iterator iter;

	iter = tree.get_iter_at_char_offset(index);
	tree.insert(iter, str, len);
}

void TextLine::erase(unsigned int from, unsigned int to)
{
	assert(from <= to);

	tree.erase(tree.get_iter_at_char_offset(from), tree.get_iter_at_char_offset(to));
}

unsigned int TextLine::bytes(void)
{
	return tree.bytes();
}

unsigned int TextLine::chars(void)
{
	return tree.chars();
}

unsigned int TextLine::columns(void)
{
	return tree.cols();
}

unsigned int TextLine::lines(void)
{
	return tree.lines();
}

gchar* TextLine::get_pointer_at_char_offset(unsigned int offset)
{
	TextLine::char_iterator iter;

	iter = tree.get_iter_at_char_offset(offset);

	return *iter;
}

unsigned int TextLine::byte_to_char_offset(unsigned int offset)
{
	TextLine::byte_iterator iter;

	iter = tree.begin();
	iter += offset;
	
	return iter.char_offset;
}

unsigned int TextLine::byte_to_col_offset(unsigned int offset)
{
	TextLine::byte_iterator iter;

	iter = tree.begin();
	iter += offset;
	
	return iter.col_offset;
}

unsigned int TextLine::col_to_byte_offset(unsigned int offset)
{
	TextLine::col_iterator iter;

	iter = tree.begin();
	iter += offset;
	
	return iter.byte_offset;
}

unsigned int TextLine::char_to_byte_offset(unsigned int offset)
{
	TextLine::char_iterator iter;

	iter = tree.begin();
	iter += offset;
	
	return iter.byte_offset;
}

TextLine::char_iterator TextLine::begin(void) const
{
	return tree.begin();
}

TextLine::char_iterator TextLine::back(void) const
{
	return tree.back();
}

TextLine::char_iterator TextLine::end(void) const
{
	return tree.end();
}

TextLine::char_iterator TextLine::reverse_begin(void) const
{
	return tree.reverse_begin();
}

TextLine::char_iterator TextLine::reverse_back(void) const
{
	return tree.reverse_back();
}

TextLine::char_iterator TextLine::reverse_end(void) const
{
	return tree.reverse_end();
}

TextLine::char_iterator TextLine::insert (const char_iterator iter, const char *text, int len)
{
	return tree.insert(iter, text, len);
}

/*TextLine::char_iterator TextLine::insert (const char *text, int len)
{
	return tree.insert(text, len);
}*/

TextLine::char_iterator TextLine::append (const char *text, int len)
{
	return tree.insert(tree.back(), text, len);
}
