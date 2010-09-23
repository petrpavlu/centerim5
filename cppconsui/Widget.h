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
 * Widget class.
 *
 * @ingroup cppconsui
 */

#ifndef __WIDGET_H__
#define __WIDGET_H__

#define AUTOSIZE INT_MIN

#include "CppConsUI.h"
#include "InputProcessor.h"
#include "ConsuiCurses.h"

#include <sigc++/sigc++.h>

class Container;
class Window;

class Widget
: public sigc::trackable
, public InputProcessor
{
	public:
		Widget(int w, int h);
		virtual ~Widget();

		/**
		 * Moves and resizes this widget. Emits signal_moveresize.
		 */
		virtual void MoveResize(int newx, int newy, int neww, int newh);
		/**
		 * The @ref area is recreated, redraw signal is emitted. It is called
		 * whenever the coordinates of the widget change.
		 */
		virtual void UpdateArea();
		/**
		 * The Draw() method does the actual drawing on a (virtual) area of
		 * the screen. The @ref CoreMananager singleton calls Draw() on all
		 * on-screen Windows. This causes all Draw() implementations needed to
		 * draw the screen to be called.
		 */
		virtual void Draw() = 0;
		/**
		 * Finds the widget that could be the focus widget from the focus
		 * chain starting with this widget:
		 */
		virtual Widget *GetFocusWidget();
		/**
		 * Deletes the focus (and input) chain starting from this widget.
		 */
		virtual void CleanFocus();
		/**
		 * @todo
		 */
		virtual void RestoreFocus();
		/**
		 * Takes focus from the widget. Used when this window is no longer
		 * a top window.
		 */
		virtual void UngrabFocus();
		/**
		 * Makes this widget the widget that has the focus. This operation is
		 * successful if the widget is visible and all predecessors are
		 * visible too.
		 */
		virtual bool GrabFocus();
		
		void SetFocusAbility(bool val) { can_focus = val; }
		bool CanFocus() const { return can_focus; }

		bool HasFocus() const { return has_focus; }

		void SetVisibility(bool visible);
		bool IsVisible() const { return visible; };
		bool IsVisibleRecursive() const;

		virtual void SetParent(Container& parent);
		Container *GetParent() const { return parent; }

		virtual int Left() const { return xpos; }
		virtual int Top() const { return ypos; }
		virtual int Width() const { return width; }
		virtual int Height() const { return height; }

		void SetColorScheme(const char *scheme);
		const char *GetColorScheme();

		sigc::signal<void, Widget&, Rect&, Rect&> signal_moveresize;
		/**
		 * Redraw signal is used by a widget to tell its @ref Container object
		 * that the widget has been updated and that it should be redrawn.
		 * Usually only the @ref Container of a widget is connected to this
		 * signal.
		 */
		sigc::signal<void, Widget&> signal_redraw;
		/**
		 * Signal emitted whenever a widget grabs or looses the focus.
		 */
		sigc::signal<void, Widget&, bool> signal_focus;
		/**
		 * Signal emmited whenever a visible property is changed.
		 */
		sigc::signal<void, Widget&, bool> signal_visible;

	protected:
		/**
		 * Screen area relative to parent area. Note that this is a requested
		 * area if parent window is big enough, real width/height can differ.
		 */
		int xpos, ypos, width, height;
		/**
		 * Flag if the widget can grab the focus.
		 */
		bool can_focus;
		/**
		 * Flag if the widget has the focus. Only one widget can have the
		 * focus in the application.
		 */
		bool has_focus;
		/**
		 * Visibility flag.
		 */
		bool visible;
		/**
		 * This is the implementation dependent area of the widget.
		 */
		Curses::Window *area;
		/**
		 * Parent widget.
		 */
		Container *parent;
		/**
		 * Color scheme.
		 */
		char *color_scheme;

		/**
		 * @todo
		 */
		virtual Container *GetTopContainer();

	private:
		Widget(const Widget&);
		Widget& operator=(const Widget&);
};

#endif /* __WIDGET_H__ */
