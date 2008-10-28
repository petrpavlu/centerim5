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

class Window
: public Container
{
	public:
		Window(int x, int y, int w, int h, Border *border);
		virtual ~Window();

		virtual void Close(void);
		virtual void Move(int newx, int newy);
		virtual void Resize(int neww, int newh);
		virtual void MoveResize(const Rect &rect)
			{ MoveResize(rect.x, rect.y, rect.width, rect.height); }
		virtual void MoveResize(int newx, int newy, int neww, int newh);
		void UpdateArea();
		virtual void SetBorder(Border *border);
		virtual Border* GetBorder(void);

		virtual void Draw(void);

		virtual int Left() { return win_x; }
		virtual int Top() { return win_y; }
		virtual int Width() { return win_w; }
		virtual int Height() { return win_h; }

		//TODO this is not real nice. find a better way to let the windowmanager 
		//get this info.
		WINDOW* GetWindow(void) { return realwindow; };

		virtual void Show();
		virtual void Hide();

		virtual void ScreenResized();
		
		sigc::signal<void, Window*> signal_close;
		//sigc::signal<void, Window*> signal_show;
		//sigc::signal<void, Window*> signal_hide;

	protected:
		/* the window on-screen dimensions */
		int win_x, win_y, win_w, win_h;
		/* dimensions to use when copying from pad to window */
		int copy_x, copy_y, copy_w, copy_h;

		/* the `real' window for this window */
		WINDOW *realwindow;
		Border *border;

		FocusChain focus_chain;

	private:
		Window(void);
		Window(const Window&);

		Window& operator=(const Window&);

		void MakeRealWindow(void);
};

#endif /* __WINDOW_H__ */
