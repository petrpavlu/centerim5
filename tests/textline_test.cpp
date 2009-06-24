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

int main(int argc, char **argv)
{
	char c;

	TextLine *line = new TextLine();
	TextLine::char_iterator iter, end;
	TextLine::char_iterator reverse_end;

	line->insert(0, STR1, strlen(STR1));

	std::cout << "The next two lines should be identical:" << std::endl;
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

	line = new TextLine();
	line->insert(0, STR2, strlen(STR2));

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
}
