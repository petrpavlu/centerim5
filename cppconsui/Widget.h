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

#include <curses.h>

#include <sigc++/sigc++.h>
#include <sigc++/signal.h>

class Widget
: public sigc::trackable
, public InputProcessor
{
	public:
		Widget(WINDOW* parentarea, int x, int y, int w, int h);
		~Widget();

		virtual void Move(WINDOW *parentarea, int newx, int newy);
		virtual void Resize(WINDOW *parentarea, int neww, int newh);
		virtual void MoveResize(WINDOW *parentarea, int newx, int newy, int neww, int newh);
		void UpdateArea(WINDOW *parentarea);

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
		int Width() { return w; }
		int Height() { return h; }

		sigc::signal<void, Point&, Point&> signal_move;
		sigc::signal<void, Rect&, Rect&> signal_resize;
		sigc::signal<void> signal_redraw;

	protected:
		int x, y, w, h; // screen area relative to parent area
		WINDOW *area;
		bool focus, canfocus;

	private:
		Widget();
		Widget(const Widget &);
};

#endif /* __WIDGET_H__ */
