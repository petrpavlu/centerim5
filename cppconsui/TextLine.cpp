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

#include "TextLine.h"

TextLine::TextLine()
{
}

TextLine::TextLine(const TextLine& line, size_t index, size_t num)
{
	str = line.str.substr(index, num);
}

TextLine::~TextLine()
{
}

void TextLine::insert(size_t index, const char* cstr, size_t num)
{
	str.insert(index, cstr, num);
}

size_t TextLine::byte_count(void)
{
	return str.bytes();
}

size_t TextLine::char_count(void)
{
	return str.size();
}

size_t TextLine::byte_count_to_char_offset(size_t n)
{
	return str.substr(0, n).bytes();
}

size_t TextLine::char_count_to_byte_offset(size_t n)
{
	Glib::ustring l(str.c_str(), n);

	return l.size();
}

const char* TextLine::c_str(void)
{
	return str.c_str();
}
