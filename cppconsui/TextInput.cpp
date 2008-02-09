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

#include "TextInput.h"

#include <string.h>

TextInput::TextInput(Widget& parent, int x, int y, int w, int h)
: TextBrowser(parent, x, y, w, h)
{
}

TextInput::TextInput(Widget& parent, int x, int y, int w, int h, std::vector<Glib::ustring> &lines)
: TextBrowser(parent, x, y, w, h, lines)
{
}

TextInput::~TextInput()
{
}

void TextInput::Draw(void)
{
	//TODO draw cursor

	TextBrowser::Draw();
}

int TextInput::ProcessInputText(const char *input, const int bytes)
{
	const char *nl, *start;

	//TODO this is a hack!
	if (input[0] == '\033')
		return 0;

	//TODO strchr depends on terminating zero, which we cannot expect
	//(for now its hacked in in centerim.cpp) use g_strstr_len
	start = input;
	while (( nl = strchr(input, '\n') )) {
		AddBytes(start, nl - start);
		start = nl + 1;
		AddLine("");
	}
	if (*start == '\r')
		AddLine("");
	else
		AddBytes(start, input + bytes - start);
	return bytes;
}
