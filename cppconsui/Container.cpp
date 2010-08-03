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
 * Container class implementation.
 *
 * @ingroup cppconsui
 */

#include "Container.h"

#include "ConsuiCurses.h"
#include "Keys.h"
#include "Window.h"

#include <algorithm>
#include "gettext.h"

#define CONTEXT_CONTAINER "container"

/* NOTES:
 * Widgets added to a container will be deleted by the
 * container.
 * */

Container::Container(int w, int h)
: Widget(w, h)
, focus_cycle_scope(FocusCycleGlobal)
, focus_child(NULL)
{
	DeclareBindables();
}

Container::~Container()
{
	CleanFocus();

	Clear();
}

void Container::DeclareBindables()
{
	DeclareBindable(CONTEXT_CONTAINER, "focus-previous",
			sigc::bind(sigc::mem_fun(this, &Container::MoveFocus), Container::FocusPrevious),
			InputProcessor::Bindable_Normal);
	DeclareBindable(CONTEXT_CONTAINER, "focus-next",
			sigc::bind(sigc::mem_fun(this, &Container::MoveFocus), Container::FocusNext),
			InputProcessor::Bindable_Normal);
	DeclareBindable(CONTEXT_CONTAINER, "focus-left",
			sigc::bind(sigc::mem_fun(this, &Container::MoveFocus), Container::FocusLeft),
			InputProcessor::Bindable_Normal);
	DeclareBindable(CONTEXT_CONTAINER, "focus-right",
			sigc::bind(sigc::mem_fun(this, &Container::MoveFocus), Container::FocusRight),
			InputProcessor::Bindable_Normal);
	DeclareBindable(CONTEXT_CONTAINER, "focus-up",
			sigc::bind(sigc::mem_fun(this, &Container::MoveFocus), Container::FocusUp),
			InputProcessor::Bindable_Normal);
	DeclareBindable(CONTEXT_CONTAINER, "focus-down",
			sigc::bind(sigc::mem_fun(this, &Container::MoveFocus), Container::FocusDown),
			InputProcessor::Bindable_Normal);
}

DEFINE_SIG_REGISTERKEYS(Container, RegisterKeys);
bool Container::RegisterKeys()
{
	RegisterKeyDef(CONTEXT_CONTAINER, "focus-previous",
			_("Focusses the previous widget"),
			Keys::SymbolTermKey(TERMKEY_SYM_TAB, TERMKEY_KEYMOD_SHIFT));
	RegisterKeyDef(CONTEXT_CONTAINER, "focus-next",
			_("Focusses the next widget"),
			Keys::SymbolTermKey(TERMKEY_SYM_TAB));
	RegisterKeyDef(CONTEXT_CONTAINER, "focus-left",
			_("Focus the next widget to the left."),
			Keys::SymbolTermKey(TERMKEY_SYM_LEFT));
	RegisterKeyDef(CONTEXT_CONTAINER, "focus-right",
			_("Focus the next widget to the right."),
			Keys::SymbolTermKey(TERMKEY_SYM_RIGHT));
	RegisterKeyDef(CONTEXT_CONTAINER, "focus-up",
			_("Focus the next widget above."),
			Keys::SymbolTermKey(TERMKEY_SYM_UP));
	RegisterKeyDef(CONTEXT_CONTAINER, "focus-down",
			_("Focus the next widget below."),
			Keys::SymbolTermKey(TERMKEY_SYM_DOWN));
	return true;
}

void Container::MoveResize(int newx, int newy, int neww, int newh)
{
	Widget::MoveResize(newx, newy, neww, newh);

	UpdateAreas();
}

void Container::Draw()
{
	for (Children::iterator i = children.begin(); i != children.end(); i++)
		if (i->widget->IsVisible())
			i->widget->Draw();
}

Widget *Container::GetFocusWidget()
{
	if (focus_child)
		return focus_child->GetFocusWidget();
	return NULL;
}

void Container::CleanFocus()
{
	if (!focus_child) {
		/* Apparently there is no widget with focus because the chain ends
		 * here. */
		return;
	}

	// first propagate focus stealing to the widget with focus
	focus_child->CleanFocus();
	focus_child = NULL;
	ClearInputChild();
}

void Container::RestoreFocus()
{
	if (focus_child)
		focus_child->RestoreFocus();
}

void Container::SetParent(Container& parent)
{
	Widget::SetParent(parent);

	Widget *focus = GetFocusWidget();
	if (!GetTopContainer()->GetFocusWidget() && focus) {
		/* There is no focused widget in a top container but we've got a
		 * focused widget so link up the chain. */
		focus->GrabFocus();
	}
}

void Container::AddWidget(Widget& widget, int x, int y)
{
	Child child;

	widget.MoveResize(x, y, widget.Width(), widget.Height());
	widget.SetParent(*this);
	/**
	 * @todo Also other widget signals. Maybe a descendant class would like to
	 * do somethings. Eg a ListBox wants to undo move events.
	 */
	child.sig_redraw = widget.signal_redraw.connect(sigc::mem_fun(this,
				&Container::OnChildRedraw));
	child.sig_moveresize = widget.signal_moveresize.connect(sigc::mem_fun(
				this, &Container::OnChildMoveResize));
	child.widget = &widget;
	children.push_back(child);
}

void Container::RemoveWidget(Widget& widget)
{
	Children::iterator i;

	for (i = children.begin(); i != children.end(); i++)
		if (i->widget == &widget)
			break;

	g_assert(i != children.end());

	delete i->widget;
	children.erase(i);
}

void Container::Clear()
{
	for (Children::iterator i = children.begin(); i != children.end(); i++)
		delete i->widget;
	children.clear();
}

bool Container::IsWidgetVisible(const Widget& widget) const
{
	if (!parent || !visible)
		return false;

	return parent->IsWidgetVisible(*this);
}

bool Container::SetFocusChild(Widget& child)
{
	// focus cannot be set for widget without a parent
	if (!parent || !visible)
		return false;

	bool res = parent->SetFocusChild(*this);
	focus_child = &child;
	SetInputChild(child);
	return res;
}

void Container::GetFocusChain(FocusChain& focus_chain,
		FocusChain::iterator parent)
{
	for (Children::iterator i = children.begin(); i != children.end(); i++) {
		Widget *widget = i->widget;
		Container *container = dynamic_cast<Container *>(widget);

		if (container && container->IsVisible()) {
			// the widget is a container so add its widgets as well
			FocusChain::pre_order_iterator iter
				= focus_chain.append_child(parent, NULL);
			container->GetFocusChain(focus_chain, iter);

			/* If this is not a focusable widget and it has no focusable
			 * children, remove it from the chain. */
			if (!focus_chain.number_of_children(iter))
				focus_chain.erase(iter);
		}
		else if ((widget->CanFocus() && widget->IsVisible())
				|| widget == focus_child) {
			// widget can be focused or is focused already
			focus_chain.append_child(parent, widget);
		}
	}
}

void Container::MoveFocus(FocusDirection direction)
{
	/**
	 * @todo Don't move up if focus cycle is local.
	 */
	/* Make sure we always start at the root of the widget tree, things are
	 * a bit easier then. */
	if (parent) {
		parent->MoveFocus(direction);
		return;
	}

	FocusChain focus_chain(NULL);

	GetFocusChain(focus_chain, focus_chain.begin());
	FocusChain::pre_order_iterator iter = focus_chain.begin();
	Widget *focus_widget = GetFocusWidget();

	if (focus_widget) {
		iter = std::find(focus_chain.begin(), focus_chain.end(), focus_widget);

		// we have a focused widget but we couldn't find it
		g_assert(iter != focus_chain.end());

		Widget *widget = *iter;
		Container *parent = widget->GetParent();
		if (!widget->IsVisible() || !parent->IsWidgetVisible(*widget)) {
			/* Currently focused widget is no longer visible, MoveFocus was
			 * called to fix it. */

			// try to change focus locally first
			FocusChain::pre_order_iterator parent_iter
				= focus_chain.parent(iter);
			iter = focus_chain.erase(iter);
			FocusChain::pre_order_iterator i = iter;
			while (i != parent_iter.end()) {
				if (*i)
					break;
				i++;
			}
			if (i == parent_iter.end()) {
				for (i = parent_iter.begin(); i != iter; i++) {
					if (*i)
						break;
				}
			}
			if (i != parent_iter.end() && (*i)) {
				// local focus change was sucessful
				(*i)->GrabFocus();
				return;
			}

			/* Focus widget couldn't be changed in local scope, give focus to
			 * any widget. */
			CleanFocus();
			focus_widget = NULL;
		}
	}

	if (!focus_widget) {
		/* There is no node assigned to receive focus so give focus to the
		 * first widget. */
		FocusChain::pre_order_iterator i = iter;
		while (i != focus_chain.end()) {
			if (*i)
				break;
			i++;
		}
		if (i == focus_chain.end()) {
			for (i = focus_chain.begin(); i != iter; i++) {
				if (*i)
					break;
			}
		}

		if (i != focus_chain.end() && (*i))
			(*i)->GrabFocus();

		return;
	}

	Container *container = focus_widget->GetParent();
	FocusChain::pre_order_iterator cycle_begin, cycle_end, parent_iter;

	// find the correct widget to focus
	switch (direction) {
		case FocusPrevious:
		case FocusUp:
		case FocusLeft:
			// setup variables for handling different scopes of focus cycling
			cycle_begin = focus_chain.begin();
			cycle_end = focus_chain.end();
			parent_iter = focus_chain.parent(iter);

			switch (container->GetFocusCycle()) {
				case FocusCycleLocal:
					/* Local focus cycling is allowed (cycling amongs all
					 * widgets of a parent container). */
					cycle_begin = parent_iter.begin();
					cycle_end = parent_iter.end();
					break;
				case FocusCycleNone:
					/* If no focus cycling is allowed, stop if the widget with
					 * focus is a first/last child. */
					if (iter == parent_iter.begin())
						return;

					/* If not a first/last child, then handle as the default
					 * case. */
				default:
					/* Global focus cycling is allowed (cycling amongst all
					 * widgets in a window). */
					break;
			}

			// finally, find the next widget which will get the focus
			do {
				if (iter == cycle_begin) {
					iter = cycle_end;
					iter--;
				}
				else {
					iter--;
				}
			} while (!*iter);

			break;
		case FocusNext:
		case FocusDown:
		case FocusRight:
		default:
			cycle_begin = focus_chain.begin();
			cycle_end = focus_chain.end();
			parent_iter = focus_chain.parent(iter);

			switch (container->GetFocusCycle()) {
				case FocusCycleLocal:
					cycle_begin = parent_iter.begin();
					cycle_end = parent_iter.end();
					break;
				case FocusCycleNone:
					if (iter == --parent_iter.end())
						return;

				default:
					break;
			}

			// finally, find the next widget which will get focus
			do {
				iter++;
				if (iter == cycle_end)
					iter = cycle_begin;
			} while (!*iter);

			break;
	}

	/* Make sure the widget is valid and the let it grab focus. */
	if (*iter) {
		(*iter)->GrabFocus();
	}
}

void Container::SetActive(int i)
{
	if (i < 0 || (int) children.size() <= i) {
		if (children.size())
			i = 0;
		else
			return;
	}

	children[i].widget->GrabFocus();
}

int Container::GetActive() const
{
	for (Children::const_iterator j = children.begin(); j != children.end(); j++)
		if (j->widget->HasFocus())
			return j - children.begin();

	return -1;
}

Curses::Window *Container::GetSubPad(const Widget& child, int begin_x, int begin_y, int ncols, int nlines)
{
	if (!area)
		return NULL;

	int realw = area->getmaxx();
	int realh = area->getmaxy();

	/* Extend requested subpad to whole parent area or shrink requested area
	 * if necessary. */
	if (nlines < 0 || nlines > realh - begin_y)
		nlines = realh - begin_y;

	if (ncols < 0 || ncols > realw - begin_x)
		ncols = realw - begin_x;

	return area->subpad(begin_x, begin_y, ncols, nlines);
}

void Container::UpdateAreas()
{
	for (Children::iterator i = children.begin(); i != children.end(); i++)
		i->widget->UpdateArea();
}

void Container::OnChildRedraw(Widget& widget)
{
	signal_redraw(*this);
}

void Container::OnChildMoveResize(Widget& widget, Rect& oldsize, Rect& newsize)
{
}
