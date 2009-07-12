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

#include <cppconsui/TextLine.cpp>

#include <iostream>
#include <string.h>

#define STR1 "I do not feel obliged to believe that the same God who has endowed us with sense, reason, and intellect has intended us to forgo their use."
#define STR2 "Just a test."
#define STR3 "Hello world, Καλημέρα κόσμε, コンニチハ"
#define STR4 "Ｊｕｓｔ ａ ｔｅｓｔ."

int main(int argc, char **argv)
{
	char c;
	char *s, locale;

	TextLine *line = NULL;
	TextLine::char_iterator iter, end;
	TextLine::byte_iterator biter, bend;
	TextLine::char_iterator chariter, charend;
	TextLine::col_iterator coliter, colend;
	TextLine::char_iterator reverse_end;
	unsigned int bytecount, charcount, colcount;

	/* Setup locale. */
	setlocale(LC_ALL, "");

	line = new TextLine();
	line->insert(0, STR1, strlen(STR1));

	/* Printing strings character per charactor. */
	std::cout << "The next two lines should be identical (latin):" << std::endl;
	std::cout << STR1 << std::endl;

	end = line->end();
	for (iter = line->begin(); iter != end; iter++) {
		c = *(*iter);
		printf("%c", c);
	}
	printf("\n");

	std::cout << "Reversed:" << std::endl;
	reverse_end = line->reverse_end();
	for (iter = line->reverse_begin(); iter != reverse_end; iter--) {
		c = *(*iter);
		printf("%c", c);
	}
	printf("\n");

	delete line;

	std::cout << std::endl;

	line = new TextLine();
	line->insert(0, STR3, strlen(STR3));

	std::cout << "The next two lines should be identical: (utf8)" << std::endl;
	std::cout << STR3 << std::endl;

	end = line->end();
	for (iter = line->begin(); iter != end; iter++) {
		s = *iter;
		for (int i = 0; i < iter.char_bytes(); i++) {
			printf("%c", s[i]);
		}
	}
	printf("\n");

	std::cout << "Reversed:" << std::endl;
	reverse_end = line->reverse_end();
	for (iter = line->reverse_begin(); iter != reverse_end; iter--) {
		s = *iter;
		for (int i = 0; i < iter.char_bytes(); i++) {
			printf("%c", s[i]);
		}
	}
	printf("\n");

	delete line;

	/* String editing functions. */
	line = new TextLine();
	line->insert(0, STR2, strlen(STR2));

	std::cout << std::endl;

	std::cout <<"Text editing (latin):" << std::endl;

	std::cout << "Original        : ";
	for (iter = line->begin(); iter != end; iter++) { printf("%c", *(*iter)); }
	std::cout << std::endl;

	std::cout << "Insert \"very \"  : ";
	line->insert(8, "very ", strlen("very "));

	for (iter = line->begin(); iter != end; iter++) { printf("%c", *(*iter)); }
	std::cout << std::endl;

	std::cout << "Insert \" simple\": ";
	line->insert(12, " simple", strlen(" simple"));

	for (iter = line->begin(); iter != end; iter++) { printf("%c", *(*iter)); }
	std::cout << std::endl;

	std::cout << "Erase \" very simple\": ";
	line->erase(8, 19);

	for (iter = line->begin(); iter != end; iter++) { printf("%c", *(*iter)); }
	std::cout << std::endl;

	delete line;

	line = new TextLine();
	line->insert(0, STR4, strlen(STR4));

	std::cout << std::endl;

	std::cout <<"Text editing (utf8):" << std::endl;

	std::cout << "Original        : ";
	for (biter = line->begin(); biter != end; biter++) { printf("%c", *(*biter)); }
	std::cout << std::endl;

	std::cout << "Insert \"very \"  : ";
	line->insert(8, "ｖｅｒｙ ", strlen("ｖｅｒｙ "));

	for (biter = line->begin(); biter != end; biter++) { printf("%c", *(*biter)); }
	std::cout << std::endl;

	std::cout << "Insert \" simple\": ";
	line->insert(12, " ｓｉｍｐｌｅ", strlen(" ｓｉｍｐｌｅ"));

	for (biter = line->begin(); biter != end; biter++) { printf("%c", *(*biter)); }
	std::cout << std::endl;

	std::cout << "Erase \" very simple\": ";
	line->erase(8, 19);

	for (biter = line->begin(); biter != end; biter++) { printf("%c", *(*biter)); }
	std::cout << std::endl;

	delete line;

	std::cout << std::endl;
	/* Checking if the iterators work correctly. */

	line = new TextLine();
	line->insert(0, STR1, strlen(STR1));

	std::cout << "Counting the number of bytes, chars and colunms (latin, should be: 140, 140, 140):" << std::endl;
	bytecount = charcount = colcount = 0;

	bend = line->end();
	for (biter = line->begin(); biter != bend; biter++) {
		bytecount++;
	}

	charend = line->end();
	for (chariter = line->begin(); chariter != charend; chariter++) {
		charcount++;
	}

	colend = line->end();
	for (coliter = line->begin(); coliter != colend; coliter++) {
		colcount++;
	}

	std::cout << bytecount << ", " << charcount << ", " << colcount << std::endl;

	delete line;

	line = new TextLine();
	line->insert(0, STR3, strlen(STR3));

	std::cout << "Counting the number of bytes, chars and colunms (utf8, should be: 60, 35, 40):" << std::endl;
	bytecount = charcount = colcount = 0;

	bend = line->end();
	for (biter = line->begin(); biter != bend; biter++) {
		bytecount++;
	}

	charend = line->end();
	for (chariter = line->begin(); chariter != charend; chariter++) {
		charcount++;
	}

	colend = line->end();
	for (coliter = line->begin(); coliter != colend; coliter++) {
		colcount++;
	}

	std::cout << bytecount << ", " << charcount << ", " << colcount << std::endl;

	delete line;

}
