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

#include <cppconsui/TextBuffer.cpp>

#include <iostream>
#include <ostream>
#include <stdio.h>
#include <string.h>

/*
#define STR11 "I do not feel obliged to believe that the\n"
#define STR12 "same God who has endowed us with sense, reason,\n"
#define STR13 "and intellect has intended us to forgo their\n"
#define STR14 "use.\n"
*/
#define STR11 "1. Hello\n"
#define STR12 "2. World\n"
#define STR13 "3. Ｈｅｌｌｏ\n"
#define STR14 "4. ｗｏｒｌｄ\n"

#define STR2 "Just a test."
#define STR3 "Hello world, Καλημέρα κόσμε, コンニチハ"
#define STR4 "Ｊｕｓｔ ａ ｔｅｓｔ."

void printbuffer(TextBuffer* buffer);
void reverse_printbuffer(TextBuffer* buffer);

void printbuffer(TextBuffer* buffer)
{
	TextBuffer::char_iterator iter, end;
	TextBuffer::line_iterator line_iter;

	char *s;
	int j;

	end = buffer->end();
	for (iter = buffer->begin(); iter != end; iter++) {
		s = *iter;
		j = iter.char_bytes();
		for (int i = 0; i < j; i++) {
			printf("%c", s[i]);
		}
	}
}

void reverse_printbuffer(TextBuffer* buffer) 
{
	TextBuffer::char_iterator iter, end;
	TextBuffer::line_iterator line_iter;

	char *s;
	int j;

	for (line_iter = buffer->reverse_begin_line(); line_iter != buffer->reverse_end_line(); line_iter--) {
		end = line_iter;
		end.forward_lines(1);

		for (iter = line_iter; iter != end; iter++) {
			int i;

			s = *iter;
			j = iter.char_bytes();
			for (i = 0; i < j; i++) {
				printf("%c", s[i]);
			}
		}
	}
}

int main(int argc, char **argv)
{
	TextBuffer* buffer = NULL;
	TextBuffer::char_iterator iter, end;
	TextBuffer::line_iterator line_iter;

	/* Setup locale. */
	setlocale(LC_ALL, "");

	buffer = new TextBuffer();
	buffer->append(STR12, strlen(STR12));
	buffer->append(STR14, strlen(STR14));
	line_iter = buffer->begin();
	line_iter++;
	buffer->insert(line_iter, STR13, strlen(STR13));
	buffer->prefix(STR11, strlen(STR11));

	std::cout << "== Printing a 4 line buffer (char iter) ==" << std::endl;
	printbuffer(buffer);

	std::cout << "== Reversed == (line iter) ====" << std::endl;
	reverse_printbuffer(buffer);
	
	std::cout << "==============================" << std::endl;

	delete buffer;

	buffer = new TextBuffer();

	iter = buffer->begin();
	buffer->insert(iter, STR11, strlen(STR11));
	buffer->insert(iter, STR12, strlen(STR12));

	std::cout << "== 2 line buffer ====" << std::endl;
	printbuffer(buffer);

	iter = buffer->begin();
	iter.forward_chars(8);
	buffer->insert(iter, " world", strlen(" world"));
	
	std::cout << "==== insert ' world' =====" << std::endl;
	printbuffer(buffer);
	
	line_iter = buffer->reverse_begin_line();
	line_iter.forward_chars(3);
	buffer->insert(line_iter, "hello ", strlen("hello "));

	std::cout << "==== insert 'hello ' =====" << std::endl;
	printbuffer(buffer);
}
