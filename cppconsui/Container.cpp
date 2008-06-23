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
, focus_cycle_scope(FocusCycleNone)
{
	const gchar *context = "container";

	DeclareBindable(context, "focus-previous",
		sigc::bind(sigc::mem_fun(this, &Container::MoveFocus), Container::FocusPrevious),
		_("Focusses the previous widget"), InputProcessor::Bindable_Normal);
	DeclareBindable(context, "focus-next",
		sigc::bind(sigc::mem_fun(this, &Container::MoveFocus), Container::FocusNext),
		_("Focusses the next widget"), InputProcessor::Bindable_Normal);
	DeclareBindable(context, "focus-left",
		sigc::bind(sigc::mem_fun(this, &Container::MoveFocus), Container::FocusLeft),
		_("Focus the next widget to the left."), InputProcessor::Bindable_Normal);
	DeclareBindable(context, "focus-right",
		sigc::bind(sigc::mem_fun(this, &Container::MoveFocus), Container::FocusRight),
		_("Focus the next widget to the right."), InputProcessor::Bindable_Normal);
	DeclareBindable(context, "focus-up",
		sigc::bind(sigc::mem_fun(this, &Container::MoveFocus), Container::FocusUp),
		_("Focus the next widget above."), InputProcessor::Bindable_Normal);
	DeclareBindable(context, "focus-down",
		sigc::bind(sigc::mem_fun(this, &Container::MoveFocus), Container::FocusDown),
		_("Focus the next widget below."), InputProcessor::Bindable_Normal);

	BindAction(context, "focus-previous", Keys::Instance()->Key_shift_tab(), false);
	BindAction(context, "focus-next", Keys::Instance()->Key_tab(), false);
	BindAction(context, "focus-left", Keys::Instance()->Key_left(), false);
	BindAction(context, "focus-right", Keys::Instance()->Key_right(), false);
	BindAction(context, "focus-up", Keys::Instance()->Key_up(), false);
	BindAction(context, "focus-down", Keys::Instance()->Key_down(), false);
}

Container::~Container()
{
	Clear();
}

void Container::UpdateAreas(void)
{
	Children::iterator i;

	for (i = children.begin(); i != children.end(); i++)
		((*i).widget)->UpdateArea();
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
		((*i).widget)->Draw();
}

void Container::AddWidget(Widget *widget)
{
	Child child;

	g_return_if_fail(widget != NULL);

	widget->UpdateArea();
	//TODO also other widget signals. maybe a descendant class would like
	//to do somethings. Eg a ListBox wants to undo move events.
	child.sig_redraw = widget->signal_redraw.connect(sigc::mem_fun(this, &Container::OnChildRedraw));
	child.widget = widget;
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
		if (child->widget == widget)
			break;
	}

	if (!child) return; //TODO a warning also?

	MoveFocus(FocusNext);

	child->sig_redraw.disconnect();
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
		widget = (*i).widget;
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
		widget = (*i).widget;
		container = dynamic_cast<Container*>(widget);

		//TODO implement widget visibility.
		//TODO dont filter out widgets in this function (or overriding
		//functions in other classes. filter out somewhere else.
		//eg when sorting before use.
		if (widget->CanFocus() /*&& widget->Visible() */) {
			iter = focus_chain.append_child(parent, widget);
		} else if (container != NULL) {
			iter = focus_chain.append_child(parent, NULL);
		}

		if (container) {
			/* the widget is a container so add its widgets
			 * as well.
			 * */
			container->GetFocusChain(focus_chain, iter);
		}
	}
}

void Container::MoveFocus(FocusDirection direction)
{
	/* Make sure we always start at the root
	 * of the widget tree. */
	if (parent != this) {
		Container *container;

		if ((container = dynamic_cast<Container*>(parent))) {
			container->MoveFocus(direction);
			return;
		} else {
			//TODO warning about custom container class??
			//what to do now? perhaps move this function
			//to the widget class?
		}
	}

	FocusChain focus_chain;
	FocusChain::pre_order_iterator iter;
	FocusChain::pre_order_iterator focus_root = focus_chain.begin();
	//bool (*cmp)(Widget*, Widget*) = NULL;
	Widget *focus_widget;
	const Container *container;

	focus_chain.set_head(NULL);

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

	iter = std::find(focus_chain.begin(), focus_chain.end(), focus_widget);

	if (iter == focus_chain.end()) {
		/* We have a focussed widget but we couldn't find it. */
		return;
	}

	container = dynamic_cast<Container*>(focus_widget->Parent());

	FocusChain::iterator cycle_root, cycle_back, cycle_begin, cycle_iter;

	/* Find the correct widget to focus. */
	switch (direction) {
		case FocusPrevious:
		case FocusUp:
		case FocusLeft:
			/* Setup variables for handling different scopes of
			 * focus cycling.
			 * */
			cycle_begin = focus_chain.begin();
			cycle_back = focus_chain.back();
			cycle_iter = iter;

			if (container) {
				FocusChain::sibling_iterator parent_iter, child_iter;
				child_iter = iter; /* Convert pre_order_iterator to sibling_iterator */
				parent_iter = focus_chain.parent(child_iter);

				switch (container->FocusCycle()) {
					case FocusCycleLocal:
						/* Local focus cycling is allowed (cycling
						 * within focused widgets parent container).
						 * */
						cycle_begin = parent_iter.begin();
						cycle_back = parent_iter.back();
						cycle_iter = child_iter;

						break;
					case FocusCycleNone:
						/* If no focus cycling is allowed, stop if the widget
						 * with focus is a first/last child.
						 * */
						if (child_iter == parent_iter.begin())
							return;

						/* If not a first/last child, then handle as
						 * the default case.
						 * */
					default:
						/* Global focus cycling is allowed (cycling
						 * amongst all widgets in a window).
						 * */
						break;
				}
			}

			/* Finally, find the next widget which will get focus. */
			do {
				if (cycle_iter == cycle_begin) {
					cycle_iter = cycle_back;
				} else {
					cycle_iter--;
				}
			} while ((*cycle_iter) == NULL);

			break;
		case FocusNext:
		case FocusDown:
		case FocusRight:
		default:
			cycle_begin = focus_chain.begin();
			cycle_back = focus_chain.back();
			cycle_iter = iter;

			if (container) {
				FocusChain::sibling_iterator parent_iter, child_iter;
				child_iter = iter;
				parent_iter = focus_chain.parent(child_iter);

				switch (container->FocusCycle()) {
					case FocusCycleLocal:
						cycle_begin = parent_iter.begin();
						cycle_back = parent_iter.back();
						cycle_iter = child_iter;

						break;
					case FocusCycleNone:
						if (child_iter == parent_iter.back())
							return;

					default:
						break;
				}
			}

			/* Finally, find the next widget which will get focus. */
			do {
				if (cycle_iter == cycle_back) {
					cycle_iter = cycle_begin;
				} else {
					cycle_iter++;
				}
			} while ((*cycle_iter) == NULL);

			break;
	}

	/* Make sure the widget is valid and the let it grab focus. */
	if ((*cycle_iter) != NULL) {
		(*cycle_iter)->GrabFocus();
	}
}

void Container::SetActive(int i)
{
	Children::iterator j;

	if (i < (int)children.size()) {
		children[i].widget->GrabFocus();
	}
}

int Container::GetActive(void)
{
	Children::iterator j;

	for (unsigned int i = 0; i < children.size(); i++) {
		if (children[i].widget->HasFocus()) {
			return i;
		}
	}

	return 0;
}
