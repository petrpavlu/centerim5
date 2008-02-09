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

Container::Container(Widget& parent, const int x, const int y, const int w, const int h)
: Widget(parent, x, y, w, h)
, focuschild(NULL)
{
	const gchar *context = "container";
	canfocus = true;

	DeclareBindable(context, "focus-previous", sigc::mem_fun(this, &Container::FocusCyclePrevious),
		_("Focusses the previous widget"), InputProcessor::Bindable_Normal);
	DeclareBindable(context, "focus-next", sigc::mem_fun(this, &Container::FocusCycleNext),
		_("Focusses the next widget"), InputProcessor::Bindable_Normal);
}

Container::~Container()
{
	Children::iterator i;
	Widget *widget;

	while((i = children.begin()) != children.end()) {
		widget = (*i).first;
		//TODO should we do this???? (line below)
		delete widget;
		children.erase(i);
	}
}

void Container::UpdateAreas(void)
{
	Children::iterator i;

	for (i = children.begin(); i != children.end(); i++)
		((*i).first)->UpdateArea();
}

void Container::Move(const int newx, const int newy)
{
	Widget::Move(newx, newy);
	UpdateAreas();
}

void Container::Resize(const int neww, const int newh)
{
	Widget::Resize(neww, newh);
	UpdateAreas();
}

void Container::MoveResize(const int newx, const int newy, const int neww, const int newh)
{
	Widget::MoveResize(newx, newy, neww, newh);
	UpdateAreas();
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
	g_return_if_fail(focuschild != widget);

	if (focuschild)
		focuschild->TakeFocus();

	focuschild = widget;
	widget->GiveFocus();
	SetInputChild(widget);
}

Widget* Container::GetFocusChild(void)
{
	return focuschild;
}

void Container::AddWidget(Widget *widget)
{
	Child child;

	g_return_if_fail(widget != NULL);

	widget->UpdateArea();
	//TODO also other widget signals. maybe a descendant class would like
	//to do somethings. Eg a ListBox wants to undo move events.
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

	FocusCycleNext();

	child->second.disconnect();
	children.erase(i);
}

void Container::FocusCyclePrevious(void)
{
	Children::reverse_iterator i;
	Child *child = NULL;

	g_return_if_fail(children.size() > 1);

	//TODO take CanFocus() into account
	for (i = children.rbegin(); i != children.rend(); i++) {
		child = &(*i);
		if (child->first == focuschild) {
			i++;
			break;
		}
	}

	if (i == children.rend())
		i = children.rbegin();

	focuschild->TakeFocus();
	focuschild = (*i).first;
	focuschild->GiveFocus();
	SetInputChild(focuschild);
	Redraw();
}

void Container::FocusCycleNext(void)
{
	Children::iterator i;
	Child *child = NULL;

	g_return_if_fail(children.size() > 1);

	//TODO take CanFocus() into account
	for (i = children.begin(); i != children.end(); i++) {
		child = &(*i);
		if (child->first == focuschild) {
			i++;
			break;
		}
	}

	if (i == children.end())
		i = children.begin();

	focuschild->TakeFocus();
	focuschild = (*i).first;
	focuschild->GiveFocus();
	SetInputChild(focuschild);
	Redraw();
}

void Container::OnChildRedraw(void)
{
	signal_redraw();
}
