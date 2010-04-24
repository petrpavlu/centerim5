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

#ifndef __CENTERIM_H__
#define __CENTERIM_H__

#include "Accounts.h"
#include "Connections.h"
#include "BuddyList.h"
#include "Conversations.h"
#include "Transfers.h"
#include "Log.h"
#include "Conf.h"

#include <libpurple/core.h>
#include <libpurple/debug.h>
#include <libpurple/eventloop.h>

#include <cppconsui/Application.h>
#include <cppconsui/Keys.h>
#include <cppconsui/InputProcessor.h>
#include <cppconsui/WindowManager.h>

#include <vector>

class CenterIM
: public Application
{
	public:
		enum ScreenArea {BuddyListArea, LogArea, ChatArea, WholeArea, AreaMax};

		static CenterIM &Instance(void);

		virtual void Run(void);
		virtual void Quit(void);

		// PurpleCoreUiOps callbacks
		/** Returns information about CenterIM such as name, website etc. */
		static GHashTable *get_ui_info(void);

		// PurpleEventLoopUiOps callbacks
		/** Adds timeout to glib main loop context. */
		static guint timeout_add(guint interval, GSourceFunc function, gpointer data);
		/** Removes timeout from glib main loop context. */
		static gboolean timeout_remove(guint handle);
		/** Adds IO watch to glib main loop context. */
		static guint input_add(int fd, PurpleInputCondition condition,
			PurpleInputFunction function, gpointer data);
		/** Removes input from glib main loop context. */
		static gboolean input_remove(guint handle);

		// helper function for input_add
		/** Process IO input to purple callback. */
		static gboolean purple_glib_io_input(GIOChannel *source, GIOCondition condition, gpointer data);
		/** Destroyes libpurple io input callback internal data. */
		static void purple_glib_io_destroy(gpointer data);

		// PurpleDebugUiOps callbacks
		// helper function to catch debug messages during libpurple initialization
		/** Catches and buffers libpurple debug messages until the Log object
		 * can be instantiated. */
		static void tmp_purple_print(PurpleDebugLevel level, const char *category, const char *arg_s);
		static gboolean tmp_is_enabled(PurpleDebugLevel level, const char *category)
			{ return TRUE; }

		/** Returns size of selected area. */
		Rect ScreenAreaSize(ScreenArea area);

		/** Recalculates area sizes to fit into current screen size. */
		virtual void ScreenResized(void);

		void OpenAccountStatusMenu(void);
		void OpenGeneralMenu(void);

		static const char * const version;

	protected:

	private:
		static const int originalW = 139;
		static const int originalH = 56;

		PurpleCoreUiOps centerim_core_ui_ops;
		PurpleDebugUiOps logbuf_debug_ui_ops;
		PurpleEventLoopUiOps centerim_glib_eventloops;

		struct IOClosure {
			PurpleInputFunction function;
			guint result;
			gpointer data;

			IOClosure(void) : function(NULL), result(0), data(NULL) {}
		};

		struct LogBufferItem {
			PurpleDebugLevel level;
			char *category;
			char *arg_s;
		};
		static std::vector<LogBufferItem> *logbuf;

		Accounts *accounts;
		Connections *connections;
		BuddyList *buddylist;
		Conversations *conversations;
		Transfers *transfers;
		Conf *conf;

		Rect areaSizes[AreaMax];

		CenterIM(void);
		CenterIM(const CenterIM &);
		CenterIM &operator=(const CenterIM &);
		~CenterIM(void) {};

		void PurpleInit(void);
		void DebugUIInit(void);
		void UIInit(void);
		void UIUnInit(void);

		/** it handles the automatic registration of defined keys */
		DECLARE_SIG_REGISTERKEYS();
		static bool RegisterKeys();
		void DeclareBindables();
};

#endif /* __CENTERIM_H__ */
