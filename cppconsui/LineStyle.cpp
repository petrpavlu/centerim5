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

#include "LineStyle.h"

#include <stdlib.h>

/* Put this macro around variable that will be used in the future,
 * remove macro when variables are used.
 * This macro is to get rid of compiler warnings
 */
#define VARIABLE_NOT_USED(x) x=x;

/* the following are UTF-8 encoded multibyte characters */

/* the *utf8 structures describe line drawing elements.
 * each element is a UTF-8 encoded multibyte character
 * */

//TODO add styles for double lines, block elements
static LineElements lineelementsascii = {
	"-", "-", "-", "+", "+",
	"|", "|", "|", "+", "+",
	"+", "+", "+", "+", "+",
};

static LineElements lineelementsasciirounded = {
	"-", "-", "-", "-", "-",
	"|", "|", "|", "|", "|",
	"+", "/", "\\", "\\", "/",
};

static LineElements lineelementslight = {
	"\342\224\200", "\342\225\266", "\342\225\264", "\342\224\264", "\342\224\254",
	"\342\224\202", "\342\225\267", "\342\225\265", "\342\224\244", "\342\224\234",
	"\342\224\274", "\342\224\214", "\342\224\220", "\342\224\224", "\342\224\230",
};

static LineElements lineelementslightrounded = {
	"\342\224\200", "\342\225\266", "\342\225\264", "\342\224\264", "\342\224\254",
	"\342\224\202", "\342\225\267", "\342\225\265", "\342\224\244", "\342\224\234",
	"\342\224\274", "\342\225\255", "\342\225\256", "\342\225\257", "\342\225\260",
};

static LineElements lineelementsheavy = {
	"\342\224\201", "\342\225\272", "\342\225\270", "\342\224\273", "\342\224\263",
	"\342\224\203", "\342\225\273", "\342\225\271", "\342\224\253", "\342\224\243",
	"\342\225\213", "\342\224\217", "\342\224\223", "\342\224\227", "\342\224\233",
};

LineStyle::LineStyle()
{
        VARIABLE_NOT_USED(lineelementsasciirounded)
#define MAKELINEELEMENT(elem,fallback) elem = MakeLineElement(lineelementsascii.elem, fallback);
	MAKELINEELEMENT(h, "-");
	MAKELINEELEMENT(h_begin, "-");
	MAKELINEELEMENT(h_end, "-");
	MAKELINEELEMENT(h_up, "+");
	MAKELINEELEMENT(h_down, "+");
	MAKELINEELEMENT(v, "-");
	MAKELINEELEMENT(v_begin, "-");
	MAKELINEELEMENT(v_end, "-");
	MAKELINEELEMENT(v_left, "+");
	MAKELINEELEMENT(v_right, "+");
	MAKELINEELEMENT(cross, "+");
	MAKELINEELEMENT(corner_tl, "+");
	MAKELINEELEMENT(corner_tr, "+");
	MAKELINEELEMENT(corner_bl, "+");
	MAKELINEELEMENT(corner_br, "+");
#undef MAKELINEELEMENT
}

LineStyle::LineStyle(const LineStyle *linestyle)
{
	h = (cchar_t*)g_memdup(linestyle->h, sizeof(cchar_t));
	h_begin = (cchar_t*)g_memdup(linestyle->h_begin, sizeof(cchar_t));
	h_end = (cchar_t*)g_memdup(linestyle->h_end, sizeof(cchar_t));
	h_up = (cchar_t*)g_memdup(linestyle->h_up, sizeof(cchar_t));
	h_down = (cchar_t*)g_memdup(linestyle->h_down, sizeof(cchar_t));
	v = (cchar_t*)g_memdup(linestyle->v, sizeof(cchar_t));
	v_begin = (cchar_t*)g_memdup(linestyle->v_begin, sizeof(cchar_t));
	v_end = (cchar_t*)g_memdup(linestyle->v_end, sizeof(cchar_t));
	v_left = (cchar_t*)g_memdup(linestyle->v_left, sizeof(cchar_t));
	v_right = (cchar_t*)g_memdup(linestyle->v_right, sizeof(cchar_t));
	cross = (cchar_t*)g_memdup(linestyle->cross, sizeof(cchar_t));
	corner_tl = (cchar_t*)g_memdup(linestyle->corner_tl, sizeof(cchar_t));
	corner_tr = (cchar_t*)g_memdup(linestyle->corner_tr, sizeof(cchar_t));
	corner_bl = (cchar_t*)g_memdup(linestyle->corner_bl, sizeof(cchar_t));
	corner_br = (cchar_t*)g_memdup(linestyle->corner_br, sizeof(cchar_t));
}

LineStyle::LineStyle(const LineElements *elements)
{
#define MAKELINEELEMENT(elem,fallback) elem = MakeLineElement(elements->elem, fallback);
	MAKELINEELEMENT(h, "-");
	MAKELINEELEMENT(h_begin, "-");
	MAKELINEELEMENT(h_end, "-");
	MAKELINEELEMENT(h_up, "+");
	MAKELINEELEMENT(h_down, "+");
	MAKELINEELEMENT(v, "-");
	MAKELINEELEMENT(v_begin, "-");
	MAKELINEELEMENT(v_end, "-");
	MAKELINEELEMENT(v_left, "+");
	MAKELINEELEMENT(v_right, "+");
	MAKELINEELEMENT(cross, "+");
	MAKELINEELEMENT(corner_tl, "+");
	MAKELINEELEMENT(corner_tr, "+");
	MAKELINEELEMENT(corner_bl, "+");
	MAKELINEELEMENT(corner_br, "+");
#undef MAKELINEELEMENT
}

LineStyle::~LineStyle(void)
{
	if (h) g_free(h);
	if (h_begin) g_free(h_begin);
	if (h_end) g_free(h_end);
	if (h_up) g_free(h_up);
	if (h_down) g_free(h_down);
	if (v) g_free(v);
	if (v_begin) g_free(v_begin);
	if (v_end) g_free(v_end);
	if (v_left) g_free(v_left);
	if (v_right) g_free(v_right);
	if (cross) g_free(cross);
	if (corner_tl) g_free(corner_tl);
	if (corner_tr) g_free(corner_tr);
	if (corner_bl) g_free(corner_bl);
	if (corner_br) g_free(corner_br);
}

LineStyle* LineStyle::LineStyleAscii(void)
{
	return (new LineStyle(&lineelementsascii));
}

LineStyle* LineStyle::LineStyleAsciiRounded(void)
{
	return (new LineStyle(&lineelementsascii));
}

LineStyle* LineStyle::LineStyleLight(void)
{
	return (new LineStyle(&lineelementslight));
}

LineStyle* LineStyle::LineStyleLightRounded(void)
{
	return (new LineStyle(&lineelementslightrounded));
}

LineStyle* LineStyle::LineStyleHeavy(void)
{
	return (new LineStyle(&lineelementsheavy));
}

cchar_t* LineStyle::MakeLineElement(const gchar *utf8, const gchar *fallback)
{
	gsize written;
	gchar *locale;
	cchar_t *element;
	wchar_t wide[2];
	//TODO: does cornersion fail if the current locale is UTF-8 encoded?

	element = g_new(cchar_t, 1);
	//TODO: from glib doc: some encodings may allow nul bytes to occur
	//inside strings n that case, using -1 for the len parameter is unsafe
	locale = g_locale_from_utf8(utf8, -1, NULL, &written, NULL);
	//TODO: perhaps check if error is set (last arg in prev func) and report?
	//probably a good idea, that way erorrs in theme files can be cought
	
	/* user supplied fallback */
	if (!locale) locale = (gchar*)fallback;

	/* cornert only one character */
	if (mbtowc(wide, locale, written) < 1) {
		/* real hard fallback character */
		wide[0] = L'@'; //TODO does @ exists in *all* charsets? lets hope so
	}
		wide[1] = L'\0';

	/* create a complex character, used in our line drawing functions */
	setcchar(element, wide, A_NORMAL, 0, NULL);

	return element;
}
