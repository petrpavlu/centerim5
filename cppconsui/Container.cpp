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

#include <algorithm>

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

	DeclareBindable(context, "focus-previous",
		sigc::bind(sigc::mem_fun(this, &Container::MoveFocus), Container::FocusPrevious),
		_("Focusses the previous widget"), InputProcessor::Bindable_Normal);
	DeclareBindable(context, "focus-next",
		sigc::bind(sigc::mem_fun(this, &Container::MoveFocus), Container::FocusNext),
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
		widget->GrabFocus();;
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

	MoveFocus(FocusNext);

	child->second.disconnect();
	children.erase(i);
}

void Container::OnChildRedraw(Widget* widget)
{
	signal_redraw(this);
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
	FocusChain search_chain;
	FocusChain::pre_order_iterator iter;
	FocusChain::pre_order_iterator focus_root = focus_chain.begin();
	//bool (*cmp)(Widget*, Widget*) = NULL;
	Widget *focus_widget;

	GetFocusChain(focus_chain, focus_chain.begin());

	/*TODO implement focus chain sorting functions
	switch (direction) {
		* Order by the focus_order property of the widget. *
		case FocusNext:
		case FocusPrevious:
			cmp = Container::FocusChainSortFocusOrder;
			break;
		case FocusDown:
		case FocusUp:
			cmp = Container::FocusChainSortVertical;
			break;
		case FocusRight:
		case FocusLeft:
			cmp = Container::FocusChainSortHorizontal;
			break;
		default:
			cmp = Container::FocusChainSortFocusOrder;
			break;
	}

	focus_chain.sort(focus_chain.begin(), focus_chain.end(), cmp);
	*/

	focus_widget = GetFocusWidget();
	if (!focus_widget) {
		/* there is no node assigned to receive so give focus to
		 * the first widget in the list (if there is a widget
		 * which accepts focus).
		 * */
		if (focus_chain.number_of_children(focus_chain.begin())) {
			iter = focus_chain.begin().begin();
			(*iter)->GrabFocus();
			return;
		} else {
			/* No children. */
		}
	}

	search_chain.set_head(focus_widget);
	iter = std::search(focus_root.begin(), focus_root.end(),
		search_chain.begin(), search_chain.end());

	if (iter == focus_chain.end()) {
		/* We have a focussed widget but we couldn't find it. */
		return;
	}

	/* This find the correct widget to focus */
	switch (direction) {
		case FocusPrevious:
		case FocusUp:
		case FocusLeft:
			if (iter == focus_root.begin()) {
				iter = focus_root.back();
			} else {
				iter--;
			}
			break;
		case FocusNext:
		case FocusDown:
		case FocusRight:
		default:
			if (iter == focus_root.back()) {
				iter = (focus_root.begin());
			} else {
				iter++;
			}
			break;
	}

	/* Make sure the widget is valid and the let it grab focus. */
	if ((*iter) != NULL) {
		(*iter)->GrabFocus();
	}
}
