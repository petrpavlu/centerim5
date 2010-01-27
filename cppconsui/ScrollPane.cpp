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
#include "ScrollPane.h"

ScrollPane::ScrollPane(Widget& parent, int x, int y, int w, int h, int scrollw, int scrollh)
: Container(parent, x, y, w, h)
, scrollw(scrollw)
, scrollh(scrollh)
, xpos(0)
, ypos(0)
, scrollarea(NULL)
{
	//TODO scrollarea must be at least as largse as
	//widget size?? no
	area = Curses::newpad(scrollh, scrollw);
	UpdateArea();
	/*
	if (area->w == NULL)
		{}//TODO throw an exception?
		*/
}

ScrollPane::~ScrollPane()
{
	if (scrollarea)
		Curses::delwin(scrollarea);
}

void ScrollPane::UpdateArea()
{
	if (scrollarea)
		Curses::delwin(scrollarea);

	scrollarea = parent->GetSubPad(h, w, y, x);

	/*
	if (scrollarea == NULL)
		{}//TODO throw an exception
		//actually, dont!
		//after a container resize, all widgets are updatearea()'d
		//which will probably result (unless resizing to bigger) in
		//area == null because no pad can be made
		*/
}

void ScrollPane::Draw(void)
{
	int copyw, copyh;

	if (!scrollarea || !area)
		return;

	Container::Draw();

	/* if the defined scrollable area is smaller than the 
	 * widget, make sure the copy works.
	 * */
	copyw = (w > scrollw) ? scrollw-1 : w-1;
	copyh = (h > scrollh) ? scrollh-1 : h-1;

	if (Curses::copywin(area, scrollarea, ypos, xpos, 0, 0, copyh, copyw, 0) == ERR)
		g_assert(false); //TODO this is a big error

	Widget::Draw();
}

void ScrollPane::AdjustScroll( const int newx, const int newy)
{
	if (newx < 0 || newy < 0 || newx > scrollw || newy > scrollh)
		return;

	if (newx > xpos + w - 1) {
		xpos = newx - w + 1;
	} else if (newx < xpos) {
		xpos = newx;
	}

	if (newy > ypos + h - 1) {
		ypos = newy - h + 1;
	} else if (newy < ypos) {
		ypos = newy;
	}
}

/*void ScrollPane::Scroll(const int deltax, const int deltay)
{
 TODO do this with key combos
	int deltay = 0, deltax = 0;


	if (Keys::Compare(Key_up, key)) deltay = -1;
	else if (Keys::Compare(CUI_KEY_DOWN, key)) deltay = 1;
	else if (Keys::Compare(CUI_KEY_LEFT, key)) deltax = -1;
	else if (Keys::Compare(CUI_KEY_RIGHT, key)) deltax = 1;
	else if (Keys::Compare(CUI_KEY_PGUP, key)) deltay = -h;
	else if (Keys::Compare(CUI_KEY_PGDOWN, key)) deltay = h;
	else if (Keys::Compare(CUI_KEY_HOME, key)) deltay = -scrollh;
	else if (Keys::Compare(CUI_KEY_END, key)) deltay = scrollh;
	//TODO more scroll posibilities?
	
	//TODO not overflow safe
	if (xpos + deltax > scrollw - w) xpos = scrollw - w;
	if (ypos + deltay > scrollh - h) ypos = scrollh - h;
	if (xpos + deltax < 0) xpos = 0;
	if (ypos + deltay < 0) ypos = 0;

	Redraw();
}*/

void ScrollPane::SetScrollSize(const int width, const int height)
{
	//TODO: deltax and deltay aren't used in this function

	if (width == scrollw && height == scrollh)
		return;

	scrollw = width;
	scrollh = height;

	if (area)
		Curses::delwin(area);

	area = Curses::newpad(scrollh, scrollw);
	/*
	if (area->w == NULL)
		{}//TODO throw an exception?
		*/

	UpdateAreas();

	//TODO not overflow safe (probably not a problem. but fix anyway)
	if (xpos > scrollw - w) xpos = scrollw - w;
	if (ypos > scrollh - h) ypos = scrollh - h;
	if (xpos < 0) xpos = 0;
	if (ypos < 0) ypos = 0;
}
