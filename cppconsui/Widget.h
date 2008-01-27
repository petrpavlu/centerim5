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

#ifndef __WIDGET_H__
#define __WIDGET_H__

#include "CppConsUI.h"
#include "InputProcessor.h"

#include <sigc++/sigc++.h>
#include <sigc++/signal.h>

struct curses_imp_t;

class Widget
: public sigc::trackable
, public InputProcessor
{
	public:
		Widget(Widget& parent, const int x, const int y, const int w, const int h);
		virtual ~Widget();

		virtual void Move(const int newx, const int newy);
		virtual void Resize(const int neww, const int newh);
		virtual void MoveResize(const int newx, const int newy, const int neww, const int newh);
		virtual void UpdateArea();

		/* The difference between the Draw() and Redraw() functions should be
		 * clarified.
		 *
		 * The Draw() function does the actual drawing on some (virtual) area
		 * of the screen. The WindowMananger object calls Draw() on all on-screen
		 * Windows. This causes all Draw() implementations needed to draw the
		 * screen to be called.
		 *
		 * The Redraw() function can be called by a Widget to tell its 
		 * Container object that the Widget has been updated and that it should
		 * be redrawn. This implies that the Container of a Widget should
		 * connect to the Redraw signal.
		 * */
		virtual void Draw(void);
		void Redraw(void);

		virtual void GiveFocus(void);
		virtual void TakeFocus(void);
		bool CanFocus(void) { return canfocus; }

		int Left() { return x; }
		int Top() { return y; }
		int X() { return x; }
		int Y() { return y; }
		int Width() { return w; }
		int Height() { return h; }

		void GetSubPad(curses_imp_t& a, const int x, const int y, const int w, const int h);

		//TODO encapsulate with a function, make sure derived classed call Move()/Resize()/Redraw() to emit signal
		//also check if this is possible at all
		sigc::signal<void, Point&, Point&> signal_move;
		sigc::signal<void, Rect&, Rect&> signal_resize;
		sigc::signal<void> signal_redraw;

	protected:
		int x, y, w, h; // screen area relative to parent area

		curses_imp_t* area;

		bool focus, canfocus;

		Widget *parent;

	private:
		Widget();
		Widget(const Widget &);

		Widget& operator=(const Widget&);
};

#endif /* __WIDGET_H__ */
