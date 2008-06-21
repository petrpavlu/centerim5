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
#include <Keys.h>

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
, focus_cycle(false)
{
	const gchar *context = "container";
	can_focus = true;

	DeclareBindable(context, "focus-previous", sigc::mem_fun(this, &Container::FocusCyclePrevious),
		_("Focusses the previous widget"), InputProcessor::Bindable_Normal);
	DeclareBindable(context, "focus-next", sigc::mem_fun(this, &Container::FocusCycleNext),
		_("Focusses the next widget"), InputProcessor::Bindable_Normal);

	BindAction(context, "focus-previous", Keys::Instance()->Key_shift_tab(), false);
	BindAction(context, "focus-next", Keys::Instance()->Key_tab(), false);
}

Container::~Container()
{
	Clear();
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

	if (!focus_child) {
		focus_child = widget;
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
		if (child->first == focus_child) {
			i++;
			break;
		}
	}

	if (i == children.rend()) {
		if (focus_cycle)
			i = children.rbegin();
		else
			//TODO tell parent (signal) to move the focus to the next widget.
			//this means the container will lose focus.
			return;
	}

	if ((*i).first->GrabFocus())
		SetInputChild(focus_child);
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
		if (child->first == focus_child) {
			i++;
			break;
		}
	}

	if (i == children.end()) {
		if (focus_cycle)
			i = children.begin();
		else
			//TODO same as for focuscycleprevious
			return;
	}

	if ((*i).first->GrabFocus())
		SetInputChild(focus_child);
	Redraw();
}

void Container::OnChildRedraw(void)
{
	signal_redraw();
}

void Container::Clear(void)
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

void Container::GetFocusChain(FocusChain& focus_chain, FocusChain::iterator parent)
{
	Children::iterator i;
	Widget *widget;
	Container *container;
	FocusChain::pre_order_iterator iter;

	for (i = children.begin(); i != children.end(); i++) {
		widget = (*i).first;

		//TODO implement widget visibility.
		//if (!widget->Visible())
		//	/* invisible widgets dont need focus */
		//	continue;

		//TODO dont filter out widgets in this function (or overriding
		//functions in other classes. filter out somewhere else.
		if (!widget->CanFocus())
			/* widgets that dont want focus wont get it */
			continue;

		iter = focus_chain.append_child(parent, widget);

		if ((container = dynamic_cast<Container*>(widget))) {
			/* the widget is a container so add its widgets
			 * as well
			 * */
			container->GetFocusChain(focus_chain, iter);
		}
	}
}

void Container::MoveFocus(FocusDirection direction)
{
	FocusChain focus_chain;

	GetFocusChain(focus_chain, focus_chain.begin());

	
}
