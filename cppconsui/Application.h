/*
 * Copyright (C) 2009 by CenterIM developers
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

/** @file Application.h Application class
 * @ingroup cppconsui
 */

#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "InputProcessor.h"
#include "WindowManager.h"

#include <glib.h>
#include <libtermkey/termkey.h>

/**
 * This class implements a simple application class.
 *
 * Application implements common tasks that almost every program needs. It
 * handles standard input and connects the first screen resizing callback.
 * Input data are converted from user locale to an internal representation and
 * then processed by InputProcessor.
 *
 * User should create a main program class that inherits Application and
 * ovewrite @ref Run(), @ref Quit() and implement @ref ScreenResized()
 * similarly to this example.
 *
 * \code
 * void MyApp::Run(void)
 * {
 *	// ui initialization
 *	// bindable initialization
 *	// ...
 *
 *	Application::Run();
 * }
 *
 * void MyApp::Quit(void) {
 *	Application::Quit();
 *
 *	// clean up
 * }
 *
 * void MyApp::ScreenResized(void) {
 *	// recalculate area sizes to fit into current screen size
 * }
 * \endcode
 */
class Application
: public InputProcessor
{
	public:
		/** Sets itself as a standard input watcher and runs glib main loop.
		 * */
		virtual void Run();
		/** Quits glib main loop and stops watching the standard input. */
		virtual void Quit();

		// glib IO callbacks
		/** Handle standard input IO errors (logs an error) and quits the
		 * application using @ref Quit() call. This function is a glib main
		 * loop callback for the standard input watcher. */
		static gboolean io_input_error_(GIOChannel *source, GIOCondition cond, gpointer data)
			{ return ((Application *) data)->io_input_error(source, cond); }
		/** Reads data from the standard input. The data are at first
		 * converted from user locales to an internal representation (UTF-8)
		 * and then processed by InputProcessor. */
		static gboolean io_input_(GIOChannel *source, GIOCondition cond, gpointer data)
			{ return ((Application *) data)->io_input(source, cond); }

		/** Callback for screen resizing. This is always the first callback
		 * processed when the screen is resized (then window callbacks are
		 * processed). Application should recalculate area size of all windows
		 * in this method. */
		virtual void ScreenResized() = 0;

	protected:
		/** @ref WindowManager instance. */
		WindowManager *windowmanager;

		/** Constructs @ref Application class. Connects Application as the
		 * first resize handler to @ref windowmanager. Enables windowmanager's
		 * screen resizing. */
		Application();
		Application(const Application &);
		Application &operator=(const Application &);
		virtual ~Application();

	private:
		GIOChannel *channel;
		guint channel_id;

		TermKey *tk;
		bool utf8;

		GMainLoop *gmainloop;

		sigc::connection resize;

		gboolean io_input_error(GIOChannel *source, GIOCondition cond);
		gboolean io_input(GIOChannel *source, GIOCondition cond);

		void StdinInputInit();
		void StdinInputUnInit();
};

#endif /* __APPLICATION_H__ */
