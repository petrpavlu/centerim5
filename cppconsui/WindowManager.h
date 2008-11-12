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

#ifndef __WINDOWMANAGER_H__
#define __WINDOWMANAGER_H__

#include "Window.h"
#include "InputProcessor.h"

#if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include <vector>

class WindowManager
: public InputProcessor
{
	public:
		~WindowManager(void);

		static WindowManager* Instance(void);
		void Delete(void);

		void CloseWindow(Window *window);

		virtual void Add(Window *window);
		void Remove(Window *window);

		bool Draw(void);

		void ScreenResized(void);
		bool Resize(void);

	protected:
		typedef struct {
			Window* window;
			sigc::connection redraw;
			sigc::connection resize;
		} WindowInfo;
		typedef std::vector<WindowInfo> Windows;

		void Redraw(void);
		void WindowRedraw(Widget* widget);

		void FocusWindow(void);
		void Swap(Window* fst, Window* snd);
		Windows::iterator FindWindow(Window *window);
		bool HasWindow(Window *window);

		Windows windows;
		WINDOW *defaultwindow;
		sigc::signal<void> signal_resize;
		int screenW, screenH;

		bool redrawpending;
		bool resizepending;

		WindowManager(void);
		WindowManager(const WindowManager&);

		WindowManager& operator=(const WindowManager&);

	private:
		bool CloseWindowCallback(Window *window);

		static WindowManager *instance;
};

#endif /* __WINDOWMANAGER_H__ */
