/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010 by CenterIM developers
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

/**
 * @file
 * ScrollPane class implementation.
 *
 * @ingroup cppconsui
 */

#include "ScrollPane.h"

ScrollPane::ScrollPane(int w, int h, int scrollw, int scrollh)
: Container(w, h)
, scroll_xpos(0)
, scroll_ypos(0)
, scroll_width(scrollw)
, scroll_height(scrollh)
, scrollarea(NULL)
{
	area = Curses::Window::newpad(scroll_width, scroll_height);
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

	// fix scroll position if necessary
	AdjustScroll(scroll_xpos, scroll_ypos);
}

void ScrollPane::Draw()
{
	int copyw, copyh;

	if (!area || !scrollarea)
		return;

	Container::Draw();

	/* If the defined scrollable area is smaller than the widget, make sure
	 * the copy works. */
	copyw = MIN(scroll_width - 1, scrollarea->getmaxx() - 1);
	copyh = MIN(scroll_height - 1, scrollarea->getmaxy() - 1);

	area->copyto(scrollarea, scroll_xpos, scroll_ypos, 0, 0, copyw, copyh, 0);
}

void ScrollPane::SetScrollSize(int swidth, int sheight)
{
	if (swidth == scroll_width && sheight == scroll_height)
		return;

	scroll_width = swidth;
	scroll_height = sheight;

	if (area)
		delete area;
	area = Curses::Window::newpad(scroll_width, scroll_height);

	UpdateAreas();

	// fix scroll position if necessary
	AdjustScroll(scroll_xpos, scroll_ypos);
}

void ScrollPane::AdjustScroll(int newx, int newy)
{
	if (scrollarea) {
		scroll_xpos = newx;
		scroll_ypos = newy;

		int real_width = scrollarea->getmaxx();
		int real_height = scrollarea->getmaxy();

		if (scroll_xpos + real_width > scroll_width)
			scroll_xpos = scroll_width - real_width;
		if (scroll_xpos < 0)
			scroll_xpos = 0;

		if (scroll_ypos + real_height > scroll_height)
			scroll_ypos = scroll_height - real_height;
		if (scroll_ypos < 0)
			scroll_ypos = 0;
	}
	else {
		scroll_xpos = 0;
		scroll_ypos = 0;
	}
	signal_redraw(*this);
}

void ScrollPane::MakeVisible(int x, int y)
{
	// fix parameters
	if (x < 0)
		x = 0;
	else if (x >= scroll_width)
		x = scroll_width - 1;
	if (y < 0)
		y = 0;
	else if (y >= scroll_height)
		y = scroll_height - 1;

	if (!scrollarea) {
		AdjustScroll(x, y);
		return;
	}

	int real_width = scrollarea->getmaxx();
	int real_height = scrollarea->getmaxy();

	if (x > scroll_xpos + real_width - 1)
		scroll_xpos = x - real_width + 1;
	else if (x < scroll_xpos)
		scroll_xpos = x;

	if (y > scroll_ypos + real_height - 1)
		scroll_ypos = y - real_height + 1;
	else if (y < scroll_ypos)
		scroll_ypos = y;

	signal_redraw(*this);
}
