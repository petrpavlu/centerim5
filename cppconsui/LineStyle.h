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

#ifndef __LINESTYLE_H__
#define __LINESTYLE_H__

#if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
//#define _XOPEN_SOURCE_EXTENDED
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include <glib.h>

typedef struct LineElements {
	gchar *h, *h_begin, *h_end, *h_up, *h_down;
	gchar *v, *v_begin, *v_end, *v_left, *v_right;
	gchar *cross, *corner_tl, *corner_tr, *corner_bl, *corner_br;
} LineElements;

//TODO add line drawing functions
class LineStyle
{
	public:
		LineStyle();
		LineStyle(const LineStyle *linestyle);
		LineStyle(const LineElements *elements);
		~LineStyle();

		//TODO should be able to set default linestyle
		#define LineStyleDefault LineStyleLight
		static LineStyle* LineStyleAscii(void);
		static LineStyle* LineStyleAsciiRounded(void);
		static LineStyle* LineStyleLight(void);
		static LineStyle* LineStyleLightRounded(void);
		static LineStyle* LineStyleHeavy(void);

		const cchar_t* H(void) { return h; }
		const cchar_t* HBegin(void) { return h_begin; }
		const cchar_t* HEnd(void) { return h_end; }
		const cchar_t* HUp(void) { return h_up; }
		const cchar_t* HDown(void) { return h_down; }
		const cchar_t* V(void) { return v; }
		const cchar_t* VBegin(void) { return v_begin; }
		const cchar_t* VEnd(void) { return v_end; }
		const cchar_t* VLeft(void) { return v_left; }
		const cchar_t* VRight(void) { return v_right; }
		const cchar_t* Cross(void) { return cross; }
		const cchar_t* CornerTL(void) { return corner_tl; }
		const cchar_t* CornerTR(void) { return corner_tr; }
		const cchar_t* CornerBL(void) { return corner_bl; }
		const cchar_t* CornerBR(void) { return corner_br; }

	protected:
		cchar_t *h;		// Horizontal line
		cchar_t *h_begin;	// "        " line begin
		cchar_t *h_end;		// "        " line end
		cchar_t *h_up;		// "             " and line up
		cchar_t *h_down;	// "             " and line down
		cchar_t *v;		// Vertical line
		cchar_t *v_begin;	// "      " line begin
		cchar_t *v_end;		// "      " line end
		cchar_t *v_left;	// "           " and line left
		cchar_t *v_right;	// "           " and line right
		cchar_t *cross;		// Horizontal and Vertical line crossed
		cchar_t *corner_tl;	// Top-left corner
		cchar_t *corner_tr;	// Top-right corner
		cchar_t *corner_bl;	// Bottom-left corner
		cchar_t *corner_br;	// Bottom-right corner

	private:
		cchar_t* MakeLineElement(const gchar *utf8, const gchar *fallback);
};

#endif /* __LINESTYLE_H__ */
