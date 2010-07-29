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
 * Window class.
 *
 * @ingroup cppconsui
 */

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "FreeWindow.h"
#include "Panel.h"

/** Window class is the class implementing the root node of the Widget chain defined by
 *  Widget::parent.
 *
 * It handles the drawing of it's pad and subpads of it's children to a
 * @ref realwindow "real window".
 *
 * @todo I have the following idea about how to change the implementation of Window.
 *  First, I think it's the WindowManager's job to create the "realwindow" for this
 *  window. Also, give instructions where on this "realwindow" to copy the pad of
 *  the Window. This way, we keep the Widget as a pad and not as a window.
 *  Also, the border should be handled also by the WindowManager, directly on the
 *  "realwindow". However, it's Window's job to specify if it wants a border and what
 *  kind of border.
 * @todo Make the difference between the pad dimensions and the window dimensions clearer.
 *  E.g. the Move, Resize, etc are referring to the window's dimensions and not pad's
 *  dimensions. Are those the same ? (don't mind the border) Can a window be larger than
 *  its physical window ?
 */
class Window
: public FreeWindow
{
	public:
		Window(int x, int y, int w, int h, LineStyle::Type ltype = LineStyle::DEFAULT);
		virtual ~Window() {}

		// Widget
		virtual void MoveResize(int newx, int newy, int neww, int newh);

		// Container
		virtual Curses::Window *GetSubPad(const Widget &child, int begin_x,
				int begin_y, int ncols, int nlines);

		void SetBorderStyle(LineStyle::Type ltype);
		LineStyle::Type GetBorderStyle() const;

	protected:
		Panel *panel;

	private:
		Window(const Window&);
		Window& operator=(const Window&);
};

#endif /* __WINDOW_H__ */
