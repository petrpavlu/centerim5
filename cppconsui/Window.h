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

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "Container.h"
#include "Border.h"
#include "CppConsUI.h"

#if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
#include <ncurses.h>
#else
#include <curses.h>
#endif

/* ncurses has border as a macro thats not nice */
#undef border

#include <panel.h>

class Window
: public Container
{
	public:
		Window(int x, int y, int w, int h, Border *border);
		~Window();

		virtual void Move(WINDOW* parentarea, int newx, int newy)
			{ Move(newx, newy); }
		virtual void Move(int newx, int newy);
		virtual void Resize(WINDOW* window, int neww, int newh)
			{ Resize(neww, newh); }
		virtual void Resize(int neww, int newh);
		virtual void MoveResize(WINDOW *parentarea, int newx, int newy, int neww, int newh)
			{ MoveResize(newx, newy, neww, newh); }
		virtual void MoveResize(const Rect &rect)
			{ MoveResize(rect.x, rect.y, rect.width, rect.height); }
		virtual void MoveResize(int newx, int newy, int neww, int newh);
		virtual void SetBorder(Border *border);
		virtual Border* GetBorder(void);

		virtual void Draw(void);

		virtual int Left() { return win_x; }
		virtual int Top() { return win_y; }
		virtual int Width() { return win_w; }
		virtual int Height() { return win_h; }

		PANEL *GetPanel(void) { return panel; };

		virtual void Show();
		virtual void Hide();

	protected:
		/* the window on-screen dimensions */
		int win_x, win_y, win_w, win_h;
		/* dimensions to use when copying from pad to window */
		int copy_x, copy_y, copy_w, copy_h;

		/* the pad and the `real' window for this window */
		WINDOW *window, *realwindow;
		PANEL *panel;
		Border *border; 

	private:
		Window();

		void MakeRealWindow(void);
};

#endif /* __WINDOW_H__ */
