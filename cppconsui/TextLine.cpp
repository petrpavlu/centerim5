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

TextLine::TextLine()
{
}

TextLine::TextLine(const TextLine &line, unsigned int index, unsigned int len)
{
	*this = line;
	erase(index+len+1, chars());
	erase(0, index);
}

TextLine::~TextLine()
{
}

void TextLine::insert(unsigned int index, const char* str, unsigned int len)
{
	TextLine::char_iterator iter;

	iter = tree.get_iterator_at_char_offset(index);
	tree.insert(iter, str, len);
}

unsigned int TextLine::bytes(void)
{
	return begin().bytes();
}

unsigned int TextLine::chars(void)
{
	return begin().chars();
}

unsigned int TextLine::columns(void)
{
	return begin().columns();
}
