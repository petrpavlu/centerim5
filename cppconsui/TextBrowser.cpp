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

#include "ConsuiCurses.h"
#include "TextBrowser.h"

#include "Widget.h"

#include <vector>

TextBrowser::TextBrowser(Widget& parent, int x, int y, int w, int h)
: Widget(parent, x, y, w, h)
, pos(0)
, follow(true)
{
}

TextBrowser::TextBrowser(Widget& parent, int x, int y, int w, int h, std::vector<Glib::ustring> &lines)
: Widget(parent, x, y, w, h)
{
	AddLines(lines);
}

TextBrowser::~TextBrowser()
{
}

void TextBrowser::SetLines(std::vector<Glib::ustring> &lines_)
{
	lines.assign(lines_.begin(), lines_.end());

	if (follow)
		pos = (height > (int)lines.size()) ? 0 : (int)lines.size()-height;
	else
		pos = 0;

	signal_redraw(*this);
}

//TODO remove code duplication for redraw on follow
void TextBrowser::AddLines(std::vector<Glib::ustring> &lines)
{
	std::vector<Glib::ustring>::iterator i;

	for (i = lines.begin(); i != lines.end(); i++)
		lines.push_back(*i);

	if (follow) {
                pos = (height > (int)lines.size()) ? 0 : (int)lines.size()-height;
		signal_redraw(*this);
	} else if ((int)lines.size() <= height) {
		signal_redraw(*this);
	}
}

void TextBrowser::AddLine(Glib::ustring line)
{
	lines.push_back(line);

	if (follow) {
	        pos = (height > (int)lines.size()) ? 0 : (int)lines.size()-height;
		signal_redraw(*this);
	} else if ((int)lines.size() <= height) {
		signal_redraw(*this);
	}
}

void TextBrowser::AddBytes(const char *s, int bytes)
{
	Glib::ustring *line = NULL;

	if (!lines.size()) {
		AddLine("");
		if (follow)
			pos = (height > (int)lines.size()) ? 0 : (int)lines.size()-height;
	}
	line = &lines.back();

	line->append(s, bytes);

	signal_redraw(*this);
}

void TextBrowser::Clear(void)
{
	lines.clear();
	area->erase();

	signal_redraw(*this);
}

int TextBrowser::Size(void)
{
	return (int)lines.size();
}

void TextBrowser::RemoveFront(void)
{
	if (lines.size()) {
		lines.erase(lines.begin());
		pos--;
		signal_redraw(*this);
	}
}

void TextBrowser::RemoveBack(void)
{
	if (lines.size() > 0) {
		lines.pop_back();
		pos--;
		signal_redraw(*this);
	}
}

//TODO can the return value be a reference?
Glib::ustring TextBrowser::AsString(Glib::ustring seperator)
{
	Glib::ustring str;

	std::vector<Glib::ustring>::iterator i;

	i = lines.begin();
	str.append(*i);
	i++;
	for (; i != lines.end(); i++) {
		str.append(seperator);
		str.append(*i);
	}

	return str;
}

void TextBrowser::Draw(void)
{
	int i, j; // i for current lines, j for the number of wrapped lines
	Glib::ustring line;

	for (i = pos, j = 0; (i+j-pos < height) && (i < (int)lines.size()); i++) {
		line = lines.at(i);
		//TODO prepare string for on-screen printing, or should Glib::ustring do this automatically?
		//probably not, as Glib::ustring doesn't know the output width
		//also should trim / wrap the string to fit the width of the widget
		area->mvaddstr(0, i - pos, line.c_str());
		/* clear until the end of the line */
		area->clrtoeol();
	}
}
