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
, scroll_xpos(0)
, scroll_ypos(0)
, scroll_width(scrollw)
, scroll_height(scrollh)
, scrollarea(NULL)
{
	//TODO scrollarea must be at least as largse as
	//widget size?? no
	area = Curses::Window::newpad(scrollw, scrollh);
	//UpdateArea();
	/*
	if (area->w == NULL)
		{}//TODO throw an exception?
		*/
}

ScrollPane::~ScrollPane()
{
	if (scrollarea)
		delete scrollarea;
}

void ScrollPane::UpdateArea()
{
	if (scrollarea)
		delete scrollarea;
	scrollarea = parent->GetSubPad(*this, xpos, ypos, width, height);
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
	copyw = MIN(scroll_width - 1, scrollarea->getmaxx() - 1);
	copyh = MIN(scroll_height - 1, scrollarea->getmaxy() - 1);

	if (area->copyto(scrollarea, scroll_xpos, scroll_ypos, 0, 0, copyw, copyh, 0) == Curses::C_ERR)
		g_assert(false); //TODO this is a big error
}

void ScrollPane::AdjustScroll(int newx, int newy)
{
	if (newx < 0 || newy < 0 || newx > scroll_width || newy > scroll_height)
		return;

	if (newx > scroll_xpos + width - 1) {
		scroll_xpos = newx - width + 1;
	} else if (newx < scroll_xpos) {
		scroll_xpos = newx;
	}

	if (newy > scroll_ypos + height - 1) {
		scroll_ypos = newy - height + 1;
	} else if (newy < scroll_ypos) {
		scroll_ypos = newy;
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

void ScrollPane::SetScrollSize(int swidth, int sheight)
{
	//TODO: deltax and deltay aren't used in this function

	if (swidth == scroll_width && sheight == scroll_height)
		return;

	scroll_width = swidth;
	scroll_height = sheight;

	if (area)
		delete area;
	area = Curses::Window::newpad(scroll_width, scroll_height);
	/*
	if (area->w == NULL)
		{}//TODO throw an exception?
		*/

	UpdateAreas();

	//TODO not overflow safe (probably not a problem. but fix anyway)
	if (scroll_xpos > scroll_width - width)
		scroll_xpos = scroll_width - width;
	if (scroll_ypos > scroll_height - height)
		scroll_ypos = scroll_height - height;
	if (scroll_xpos < 0)
		scroll_xpos = 0;
	if (scroll_ypos < 0)
		scroll_ypos = 0;
}
