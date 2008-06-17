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
#include "Widget.h"

#include <string>

Widget::Widget(Widget& parent, int x, int y, int w, int h)
: x(x)
, y(y)
, w(w)
, h(h)
, focus(false)
, canfocus(false)
, parent(&parent)
, colorscheme(NULL)
{
	area = new curses_imp_t(NULL);
	UpdateArea();
	colorscheme = ColorScheme::ColorSchemeNormal();
}

Widget::~Widget()
{
	delwin(area->w);
}

void Widget::Move(int newx, int newy)
{
	Point oldpos(x,y), newpos(newx, newy);

	x = newx;
	y = newy;

	UpdateArea();

	signal_move(oldpos, newpos);
}

void Widget::Resize(int neww, int newh)
{
	Rect oldsize(x, y, w, h), newsize(x, y, neww, newh);

	w = neww;
	h = newh;

	UpdateArea();

	signal_resize(oldsize, newsize);
}

void Widget::MoveResize(int newx, int newy, int neww, int newh)
{
	Rect oldsize(x, y, w, h), newsize(newx, newy, neww, newh);

	x = newx;
	y = newy;
	w = neww;
	h = newh;

	UpdateArea();

	signal_move(oldsize, newsize);
	signal_resize(oldsize, newsize);
}

void Widget::UpdateArea()
{
	if (area->w)
		delwin(area->w);

	area->w = subpad(parent->area->w, h, w, y, x);

	if (area->w == NULL)
		;//TODO throw an exception
		//actually, dont!
		//after a container resize, all widgets are updatearea()'d
		//which will probably result (unless resizing to bigger) in
		//area == null because no pad can be made
}

void Widget::Draw(void)
{
}

void Widget::Redraw(void)
{
	signal_redraw();
}

void Widget::GiveFocus(void)
{
	if (canfocus) focus = true;
}

void Widget::TakeFocus(void)
{
	focus = false;
}

void Widget::GetSubPad(curses_imp_t& a, const int x, const int y, const int w, const int h)
{
	a.w = subpad(area->w, h, w, y, x);
}
