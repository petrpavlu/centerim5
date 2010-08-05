/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2009-2010 by CenterIM developers
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
 * CoreManager class
 *
 * @ingroup cppconsui
 */

#ifndef __COREMANAGER_H__
#define __COREMANAGER_H__

#include "InputProcessor.h"
#include "FreeWindow.h"

#include <glib.h>
#include <libtermkey/termkey.h>
#include <vector>

/**
 * This class implements a core part of CppConsUI.
 */

#define COREMANAGER (CoreManager::Instance())

class CoreManager
: public InputProcessor
{
	public:
		static CoreManager *Instance();

		/**
		 * Sets itself as a standard input watcher and runs glib main loop.
		 */
		void StartMainLoop();
		/**
		 * Quits glib main loop and stops watching the standard input.
		 */
		void QuitMainLoop();

		void AddWindow(FreeWindow& window);
		void RemoveWindow(FreeWindow& window);
		bool HasWindow(FreeWindow& window) const;
		FreeWindow *GetTopWindow();

		int GetScreenWidth() const { return screen_width; }
		int GetScreenHeight() const { return screen_height; }

		void EnableResizing();
		void DisableResizing();

		void SetTopInputProcessor(InputProcessor& top)
			{ top_input_processor = &top; }
		InputProcessor *GetTopInputProcessor()
			{ return top_input_processor; }

		sigc::connection TimeoutConnect(const sigc::slot<bool>& slot,
				unsigned interval, int priority);
		sigc::connection TimeoutOnceConnect(const sigc::slot<void>& slot,
				unsigned interval, int priority);

		sigc::signal<void> signal_resize;

	protected:

	private:
		struct WindowInfo
		{
			FreeWindow *window;
			sigc::connection redraw;
			sigc::connection resize;
		};
		typedef std::vector<WindowInfo> Windows;

		Windows windows;

		InputProcessor *top_input_processor;

		GIOChannel *channel;
		guint channel_id;

		TermKey *tk;
		bool utf8;

		GMainLoop *gmainloop;

		int screen_width, screen_height;

		bool redrawpending;
		bool resizepending;

		CoreManager();
		CoreManager(const CoreManager&);
		CoreManager& operator=(const CoreManager&);
		virtual ~CoreManager();

		// InputProcessor
		virtual bool ProcessInput(const TermKeyKey& key);

		// glib IO callbacks
		/**
		 * Handles standard input IO errors (logs an error) and quits the
		 * application using @ref Quit() call. This function is a glib main
		 * loop callback for the standard input watcher.
		 */
		static gboolean io_input_error_(GIOChannel *source, GIOCondition cond,
				gpointer data)
			{ return reinterpret_cast<CoreManager *>(data)->io_input_error(
					source, cond); }
		gboolean io_input_error(GIOChannel *source,
				GIOCondition cond);
		/**
		 * Reads data from the standard input. The data are at first converted
		 * from user locales to an internal representation (UTF-8) and then
		 * processed by InputProcessor.
		 */
		static gboolean io_input_(GIOChannel *source, GIOCondition cond,
				gpointer data)
			{ return reinterpret_cast<CoreManager *>(data)->io_input(source,
					cond); }
		gboolean io_input(GIOChannel *source, GIOCondition cond);

		void StdinInputInit();
		void StdinInputUnInit();

		static void SignalHandler(int signum);
		void ScreenResized();
		void Resize();

		void Draw();
		void Redraw();
		void WindowRedraw(Widget& widget);

		Windows::iterator FindWindow(FreeWindow& window);
		void FocusWindow();

		/** it handles the automatic registration of defined keys */
		DECLARE_SIG_REGISTERKEYS();
		static bool RegisterKeys();
		void DeclareBindables();
};

#endif /* __APPLICATION_H__ */
