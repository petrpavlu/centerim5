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

#if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include <panel.h>

#include <vector>

class WindowManager
{
	public:
		~WindowManager(void);

		static WindowManager* Instance(void);
		void Delete(void);

		void Add(Window *window);
		void Remove(Window *window);

		void Draw(void);
		PANEL *GetPanel(Window &win);

		int ProcessInput(const char *input, int bytes);

	protected:
		typedef std::pair<Window*, sigc::connection> WindowPair;
		typedef std::vector<WindowPair> Windows;

		void Redraw(void);

		void FocusPanel(void);
		void Swap(Window* fst, Window* snd);
		Windows::iterator FindWindow(Window *window);
		bool HasWindow(Window *window);

		Windows windows;
		Window *focuswindow;
		WINDOW *defaultwindow;

	private:
		WindowManager(void);

		static WindowManager *instance;


};

#endif /* __WINDOWMANAGER_H__ */
