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

#include "Scrollable.h"

#include "Keys.h"

#if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include <panel.h>

Scrollable::Scrollable(WINDOW* parentarea, int x, int y, int w, int h, int scrollw, int scrollh)
: Widget(parentarea, x, y, w, h)
, scrollw(scrollw)
, scrollh(scrollh)
, scrollarea(NULL)
{
	//TODO scrollarea must be at least as largse as
	//widget size?? no
	scrollarea = newpad(scrollh, scrollw);
	if (scrollarea == NULL)
		;//TODO throw an exception?
}

Scrollable::~Scrollable()
{
	if (scrollarea)
		delwin(scrollarea);
}

void Scrollable::Draw(void)
{
	if (!scrollarea || ! area) return;

	copywin(scrollarea, area, ypos, xpos, 0, 0, h-1, w-1, 0);
	Widget::Draw();
}

void Scrollable::Scroll(const char *key)
{
	int deltay = 0, deltax = 0;

	if (Keys::Compare(CUI_KEY_UP, key)) deltay = -1;
	else if (Keys::Compare(CUI_KEY_DOWN, key)) deltay = 1;
	else if (Keys::Compare(CUI_KEY_LEFT, key)) deltax = -1;
	else if (Keys::Compare(CUI_KEY_RIGHT, key)) deltax = 1;
	else if (Keys::Compare(CUI_KEY_PGUP, key)) deltay = -h;
	else if (Keys::Compare(CUI_KEY_PGDOWN, key)) deltay = h;
	else if (Keys::Compare(CUI_KEY_HOME, key)) deltay = -scrollh;
	else if (Keys::Compare(CUI_KEY_END, key)) deltay = scrollh;
	//TODO more scroll posibilities?
	
	//TODO not overflow safe (probably not a problem)
	if (xpos + deltax > scrollw - w) xpos = scrollw - w;
	if (ypos + deltay > scrollh - h) ypos = scrollh - h;
	if (xpos + deltax < 0) xpos = 0;
	if (ypos + deltay < 0) ypos = 0;

	Redraw();
}

void Scrollable::ResizeScroll(int neww, int newh)
{
	int deltax, deltay;

	if (neww == scrollw && newh == scrollh)
		return;

	if (scrollarea)
		delwin(scrollarea);

	scrollarea = newpad(scrollh, scrollw);
	if (scrollarea == NULL)
		;//TODO throw an exception?

	//TODO not overflow safe (probably not a problem)
	if (xpos + deltax > scrollw - w) xpos = scrollw - w;
	if (ypos + deltay > scrollh - h) ypos = scrollh - h;
	if (xpos + deltax < 0) xpos = 0;
	if (ypos + deltay < 0) ypos = 0;
}
