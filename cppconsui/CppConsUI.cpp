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

#include "CppConsUI.h"

#include <glibmm/ustring.h>
#include <wchar.h>
#include <cstring>

/* NOTE: copied from pango/break.c, which has GNU GPL 2 or later
 * thank you pango team
 */
void find_paragraph_boundary (const gchar *text,
			       int         length,
			       int        *paragraph_delimiter_index,
			       int        *next_paragraph_start)
{
	const gchar *p = text;
	const gchar *end;
	const gchar *start = NULL;
	const gchar *delimiter = NULL;

	/* Only one character has type G_UNICODE_PARAGRAPH_SEPARATOR in
	 * Unicode 5.0; update the following code if that changes.
	 */

	/* prev_sep is the first byte of the previous separator.  Since
	 * the valid separators are \r, \n, and PARAGRAPH_SEPARATOR, the
	 * first byte is enough to identify it.
	 */
	gchar prev_sep;

	if (length < 0)
		length = strlen (text);

	end = text + length;

	if (paragraph_delimiter_index)
		*paragraph_delimiter_index = length;

	if (next_paragraph_start)
		*next_paragraph_start = length;

	if (length == 0)
		return;

	prev_sep = 0;

	while (p != end)
	{
		if (prev_sep == '\n' ||
				prev_sep == PARAGRAPH_SEPARATOR_STRING[0])
		{
			g_assert (delimiter);
			start = p;
			break;
		}
		else if (prev_sep == '\r')
		{
			/* don't break between \r and \n */
			if (*p != '\n')
			{
				g_assert (delimiter);
				start = p;
				break;
			}
		}

		if (*p == '\n' ||
				*p == '\r' ||
				!strncmp(p, PARAGRAPH_SEPARATOR_STRING,
					strlen(PARAGRAPH_SEPARATOR_STRING)))
		{
			if (delimiter == NULL)
				delimiter = p;
			prev_sep = *p;
		}
		else
			prev_sep = 0;

		p = g_utf8_next_char (p);
	}

	if (delimiter && paragraph_delimiter_index)
		*paragraph_delimiter_index = delimiter - text;

	if (start && next_paragraph_start)
		*next_paragraph_start = start - text;
}

Glib::ustring::size_type width(const Glib::ustring &string)
{
	return width(string.data(), string.data()+string.bytes());
}

//NOTE copied from libgnt/gntutils.c
/// @todo should g_unichar_iszerowidth be used?
/// @todo write a wrapper string class
/// if so, then also include drawing functions and a way to store colours
/// for a string.
Glib::ustring::size_type width(const char *start, const char *end)
{
	Glib::ustring::size_type width = 0;

	if (start == NULL)
		return 0;

	if (end == NULL)
		end = start + strlen(start);

	while (start < end) {
		width += g_unichar_iswide(g_utf8_get_char(start)) ? 2 : 1;
		start = g_utf8_next_char(start);
	}
	return width;
}

gchar* col_offset_to_pointer(gchar *str, glong offset)
{
	glong width = 0;

	while (width < offset) {
		width += g_unichar_iswide(g_utf8_get_char(str)) ? 2 : 1;
		str = g_utf8_next_char(str);
	}

	return str;
}

void mvwaddstring(WINDOW *win, int y, int x, int w, /* gboolean selected, */ const gchar *str)
{
	// @todo `\v' switch is not implemented yet

	int printed = 0;
	const gchar *u;

	wmove(win, y, x);
	//attrset(selection_color(selected, COLOR_STANDARD));

	for (u = str; *u && printed < w; u++) {
		/*
		if (*u == COLOR_SELECT_CHAR) {
			u++;
			if (*u & COLOR_SPECIAL) {
				attrset(selection_color_special(selected, *u & COLOR_MASK));
			} else if (*u & COLOR_BOLD) {
				attrset(selection_color(selected, *u & COLOR_MASK));
				attron(A_BOLD);
			} else {
				attrset(selection_color(selected, *u & COLOR_MASK));
			}
			continue;
		}
		*/

		if (((unsigned char) *u >= 0x7f && (unsigned char) *u < 0xa0)) {
			// filter out C1 (8-bit) control characters
			waddch(win, '?');
			printed++;
			continue;
		}

		// get a unicode character from the next few bytes
		wchar_t wch[2];
		cchar_t cc;

		wch[0] = g_utf8_get_char_validated(u, -1);
		wch[1] = L'\0';

		// invalid utf-8 sequence
		if (wch[0] < 0)
			continue;

		// control char symbols
		if (wch[0] < 32)
			wch[0] = 0x2400 + wch[0];

		setcchar(&cc, wch, A_NORMAL, 0, NULL);
		wadd_wch(win, &cc);
		printed += g_unichar_iswide(wch[0]) ? 2 : 1;
		u = g_utf8_next_char(u) - 1;
	}
	whline(win, ' ', w - printed);
}

void mvwaddstringf(WINDOW *win, int y, int x, int w, const gchar *fmt, ...)
{
	char *s;
	va_list ap;

	va_start(ap, fmt);
	s = g_strdup_vprintf(fmt, ap);
	va_end(ap);
	mvwaddstring(win, y, x, w, s);
	g_free(s);
}

Point::Point()
: x(0)
, y(0)
{
}

Point::Point(int x, int y)
: x(x)
, y(y)
{
}

Rect::Rect()
: Point(0, 0)
, width(0)
, height(0)
{
}

Rect::Rect(int x, int y, int w, int h)
: Point(x, y)
, width(w)
, height(h)
{
}
