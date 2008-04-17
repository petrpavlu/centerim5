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

#include "Curses.h"
#include "Label.h"

Label::Label(Widget& parent, int x, int y, int w, int h, Glib::ustring &text)
: Widget(parent, x, y, w, h)
, text(text)
{
}

Label::Label(Widget& parent, int x, int y, int w, int h, const char *fmt, ...)
: Widget(parent, x, y, w, h)
, text("")
{
        va_list args;
        char buf[1024]; //TODO lets hope this is enough!

        va_start(args, fmt);
        vsnprintf(buf, 1023, fmt, args);
        va_end(args);
	buf[1023] = '\0';
	text = buf;
}

Label::Label(Widget& parent, int x, int y, const char *fmt, ...)
: Widget(parent, x, y, 0, 0)
, text("")
{
        va_list args;
        char buf[1024]; //TODO lets hope this is enough!

        va_start(args, fmt);
        vsnprintf(buf, 1023, fmt, args);
        va_end(args);
	buf[1023] = '\0';
	text = buf;

	//TODO size() gives character count (taking into account wide chars
	//but not the number of cells a character takes.
	Resize(this->text.size(), 1);
}

Label::~Label()
{
}

void Label::Draw(void)
{
	//TODO unicode drawing
	mvwaddstr(area->w, 0, 0, text.c_str());

	Widget::Draw();
}

void Label::SetText(const Glib::ustring str)
{
	if (text == str) return;

	text = str;
	wclear(area->w);
	signal_redraw();
}

Glib::ustring Label::GetText(void)
{
	return text;
}
