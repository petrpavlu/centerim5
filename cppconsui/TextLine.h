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

#ifndef __TEXTLINE_H__
#define __TEXTLINE_H__

#include "config.h"

#include <glibmm/ustring.h>

#include <cstddef>

class TextLine
{
	public:
		TextLine();
		TextLine(const TextLine& line, size_t index, size_t num);
		~TextLine();

		void insert (size_t index, const char* str, size_t num);

		/* Return number of bytes/chars in the line. */
		size_t byte_count(void);
		size_t char_count(void);

		/* Count the number of bytes until we reach the n'th character. */
		size_t byte_count_to_char_offset(size_t n);
		/* Count the number of characters before the n'th byte. */
		size_t char_count_to_byte_offset(size_t n);

		const char* c_str(void);

		/* Iterator for the line. */
		class iterator
		{
			public:
				iterator(void);
				iterator(const iterator&);

				char*& operator*() const;
				char* operator->() const;

				unsigned int byte_offset;
				unsigned int char_offset;

				bool operator==(const iterator&) const;
				bool operator!=(const iterator&) const;
				iterator& operator++();
				iterator& operator--();
				iterator operator++(int);
				iterator operator--(int);
				iterator& operator+=(unsigned int);
				iterator& operator-=(unsigned int);
		};

	protected:

	private:
		TextLine(const TextLine &);

		TextLine& operator=(const TextLine&);

		Glib::ustring str;
};

#endif /* __TEXTLINE_H__ */
