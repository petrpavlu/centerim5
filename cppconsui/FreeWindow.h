/*
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
 * FreeWindow class.
 *
 * @ingroup cppconsui
 */

#ifndef __FREEWINDOW_H__
#define __FREEWINDOW_H__

#include "Container.h"

class FreeWindow
: public Container
{
	public:
		FreeWindow(int x, int y, int w, int h);
		virtual ~FreeWindow();

		// Widget
		virtual void MoveResize(int newx, int newy, int neww, int newh);
		virtual void MoveResizeRect(const Rect &rect)
			{ MoveResize(rect.x, rect.y, rect.width, rect.height); }
		virtual void UpdateArea();
		virtual void Draw();
		virtual int Left() const { return win_x; }
		virtual int Top() const { return win_y; }
		virtual int Width() const { return win_w; }
		virtual int Height() const { return win_h; }

		// Container
		virtual bool SetFocusChild(Widget& child);
		virtual bool IsWidgetVisible(const Widget& widget) const;

		virtual void Show();
		virtual void Hide();
		virtual void Close();

		/** this function is called when the screen is resized */
		virtual void ScreenResized();

		sigc::signal<void, FreeWindow&> signal_close;
		sigc::signal<void, FreeWindow&> signal_show;
		sigc::signal<void, FreeWindow&> signal_hide;

	protected:
		/**
		 * The window on-screen dimensions.
		 */
		int win_x, win_y, win_w, win_h;
		/**
		 * Dimensions to use when copying from pad to window.
		 */
		int copy_w, copy_h;

		/**
		 * The `real' window for this window.
		 */
		Curses::Window *realwindow;

		/**
		 * Create the `real' window (not a pad) and makes sure its dimensions
		 * do not exceed screen size
		 */
		virtual void MakeRealWindow();

	private:
		FreeWindow(const FreeWindow&);
		FreeWindow& operator=(const FreeWindow&);

		virtual void ActionClose();

		/** it handles the automatic registration of defined keys */
		DECLARE_SIG_REGISTERKEYS();
		static bool RegisterKeys();
		void DeclareBindables();
};

#endif /* __FREEWINDOW_H__ */
