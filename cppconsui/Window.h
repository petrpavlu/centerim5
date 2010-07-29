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

/** @file Window.h Window class
 *  @ingroup cppconsui
 */

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "Container.h"
#include "Panel.h"
#include "CppConsUI.h"

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
: public Container
{
	public:
		Window(int x, int y, int w, int h, LineStyle::Type ltype = LineStyle::DEFAULT);
		virtual ~Window();

		// Widget
		virtual void MoveResize(int newx, int newy, int neww, int newh);
		virtual void MoveResizeRect(const Rect &rect)
			{ MoveResize(rect.x, rect.y, rect.width, rect.height); }
		virtual void Draw();

		// Container
		virtual bool SetFocusChild(Widget& child);
		virtual bool IsWidgetVisible(const Widget& widget) const;
		virtual Curses::Window *GetSubPad(const Widget &child, int begin_x,
				int begin_y, int ncols, int nlines);

		virtual int Left() const { return win_x; }
		virtual int Top() const { return win_y; }
		virtual int Width() const { return win_w; }
		virtual int Height() const { return win_h; }

		virtual void Show();
		virtual void Hide();
		virtual void Close();

		/** this function is called when the screen is resized */
		virtual void ScreenResized();

		void SetBorderStyle(LineStyle::Type ltype);
		LineStyle::Type GetBorderStyle() const;

		sigc::signal<void, Window&> signal_close;
		//sigc::signal<void, Window*> signal_show;
		//sigc::signal<void, Window*> signal_hide;

	protected:
		/* the window on-screen dimensions */
		int win_x, win_y, win_w, win_h;
		/* dimensions to use when copying from pad to window */
		int copy_x, copy_y, copy_w, copy_h;

		/** the `real' window for this window */
		Curses::Window *realwindow;

		Panel *panel;

		virtual void UpdateArea();
		virtual void MakeRealWindow();

	private:
		Window(const Window&);
		Window& operator=(const Window&);

		virtual void ActionClose();

		/** it handles the automatic registration of defined keys */
		DECLARE_SIG_REGISTERKEYS();
		static bool RegisterKeys();
		void DeclareBindables();
};

#endif /* __WINDOW_H__ */
