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

#include <cstring>

#include <glib.h>
#include <glib/gprintf.h>

Label::Label(Widget& parent, int x, int y, int w, int h, const char *fmt, ...)
: Widget(parent, x, y, w, h)
, text(NULL)
, text_max_length(MAX_SIZE)
{
        va_list args;

        va_start(args, fmt);
	text_size = g_vasprintf(&text, fmt, args);
        va_end(args);

	n_bytes = text_size;
	text_length = g_utf8_strlen(text, text_size);
	//TODO  text may be no longer than text_max_length
}

Label::Label(Widget& parent, int x, int y, const char *fmt, ...)
: Widget(parent, x, y, 0, 0)
, text(NULL)
, text_max_length(MAX_SIZE)
{
        va_list args;

        va_start(args, fmt);
	text_size = g_vasprintf(&text, fmt, args);
        va_end(args);

	n_bytes = text_size;
	text_length = g_utf8_strlen(text, text_size);
	//TODO  text may be no longer than text_max_length

	Resize(width(text), 1);
}

Label::~Label()
{
	if (text)
		g_free(text);
}

void Label::Draw(void)
{
	//TODO unicode drawing
	mvwaddstr(area->w, 0, 0, text);

	Widget::Draw();
}

void Label::SetText(const gchar *str)
{
	//TODO may be no longer than text_max_length
	if (text)
		g_free(text);

	text = g_strdup(str);
	text_size = strlen(str);
	text_length = g_utf8_strlen(text, text_size);

	wclear(area->w);
	signal_redraw(this);
}

Glib::ustring Label::GetText(void)
{
	return text;
}
