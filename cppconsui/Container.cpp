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

#include <Curses.h>
#include <Container.h>

/* NOTES:
 * Widgets added to a container will be deleted by the
 * container.
 * */

//TODO when adding/removing child widgets
//connect to signals
//this also means making the children vector
//private (protected?>

Container::Container(WINDOW* parentarea, int x, int y, int w, int h)
: Widget(parentarea, x, y, w, h)
, focuschild(NULL)
{
	canfocus = true;
}

Container::~Container()
{
	Children::iterator i;
	Widget *widget;

	while((i = children.begin()) != children.end()) {
		widget = (*i).first;
		delete widget;
		children.erase(i);
	}
}

void Container::Move(WINDOW* parentarea, int newx, int newy)
{
	Children::iterator i;

	Widget::Move(parentarea, newx, newy);

	for (i = children.begin(); i != children.end(); i++)
		((*i).first)->UpdateArea(area->w);
}

void Container::Resize(WINDOW* parentarea, int neww, int newh)
{
	Children::iterator i;

	Widget::Resize(parentarea, neww, newh);

	for (i = children.begin(); i != children.end(); i++)
		((*i).first)->UpdateArea(area->w);
}

void Container::MoveResize(WINDOW* parentarea, int newx, int newy, int neww, int newh)
{
	Children::iterator i;

	Widget::MoveResize(parentarea, newx, newy, neww, newh);

	for (i = children.begin(); i != children.end(); i++)
		((*i).first)->UpdateArea(area->w);
}

void Container::Draw(void)
{
	Children::iterator i;

	for (i = children.begin(); i != children.end(); i++)
		((*i).first)->Draw();
}

void Container::GiveFocus(void)
{
	focus = true;
	if (focuschild)
		focuschild->GiveFocus();
}

void Container::TakeFocus(void)
{
	focus = false;
	if (focuschild)
		focuschild->TakeFocus();
}

void Container::SetFocusChild(Widget* widget)
{
	focuschild = widget;
}

Widget* Container::GetFocusChild(void)
{
	return focuschild;
}

void Container::AddWidget(Widget *widget)
{
	Child child;

	g_return_if_fail(widget != NULL);

	widget->UpdateArea(area->w);
	child.second = widget->signal_redraw.connect(sigc::mem_fun(this, &Container::OnChildRedraw));
	child.first = widget;
	children.push_back(child);

	if (!focuschild) {
		focuschild = widget;
		SetInputChild(widget);
	}
}

void Container::RemoveWidget(Widget *widget)
{
	Children::iterator i;
	Child *child = NULL;

	g_return_if_fail(widget != NULL);

	for (i = children.begin(); i != children.end(); i++) {
		child = &(*i);
		if (child->first == widget)
			break;
	}

	if (!child) return; //TODO a warning also?

	child->first->TakeFocus();
	child->second.disconnect();
	children.erase(i);

	if (focuschild == widget) {
		focuschild = NULL;
		SetInputChild(NULL);
	}
}

void Container::OnChildRedraw(void)
{
	signal_redraw();
}
