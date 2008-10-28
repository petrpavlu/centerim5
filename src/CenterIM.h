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

#include <cppconsui/WindowManager.h>
#include <cppconsui/Keys.h>
#include <cppconsui/InputProcessor.h>

#include <glibmm/main.h>
#include <vector>


class CenterIM
: public InputProcessor
{
	public:
		static CenterIM* Instance();
		static void Delete(void);
		void Run(void);
		void Quit(void);
		void ScreenResized(void);

		void SetLocale(const char *locale)
			{ this->locale = locale; }
		const char *GetLocale(void)
			{ return locale; }

		/* for purple_core_set_ui_ops() */
		//static void ui_prefs_init_(void) { CenterIM::Instance()->ui_prefs_init(); }
		//static void debug_ui_init_(void) { CenterIM::Instance()->debug_ui_init(); }
		//static void ui_init_(void) { CenterIM::Instance()->ui_init(); }
		static void ui_uninit_(void) { CenterIM::Instance()->ui_uninit(); }
		static gboolean io_input_error_(GIOChannel *source, GIOCondition cond, gpointer data)
			{ return ((CenterIM*)data)->io_input_error(source, cond); }
		static gboolean io_input_(GIOChannel *source, GIOCondition cond, gpointer data)
			{ return ((CenterIM*)data)->io_input(source, cond); }

		void ui_prefs_init(void);
		void debug_ui_init(void);
		void ui_init(void);
		void ui_uninit(void);
		void io_init(void);
		void io_uninit(void);
		gboolean io_input_error(GIOChannel *source, GIOCondition cond);
		gboolean io_input(GIOChannel *source, GIOCondition cond);

		/* for purple_eventloop_set_ui_ops() */
		static guint timeout_add(guint interval, GSourceFunc function, gpointer data);
		static gboolean timeout_remove(guint handle);
		static guint input_add(int fd, PurpleInputCondition condition,
			PurpleInputFunction function, gpointer data);
		static gboolean input_remove(guint handle);

		/* helper functions for CenterIM::input_add_() */
		//TODO better names
		static gboolean purple_glib_io_input(GIOChannel *source, GIOCondition condition, gpointer data);

		/* helper function to catch debug messages during libpurple initialisation */
		//TODO: nice names
		static void tmp_purple_print_(PurpleDebugLevel level, const char *category, const char *arg_s);
		static gboolean tmp_isenabled_(PurpleDebugLevel level, const char *category)
			{ return true; }

		void OpenAccountStatusMenu(void);
		void OpenGeneralMenu(void);

		static void purple_glib_io_destroy(gpointer data);

		static char const * const version;

	protected:

	private:
		CenterIM();
		~CenterIM();

		void register_resize_handler(void);
		void unregister_resize_handler(void);

		static CenterIM* instance;

		const char *locale;
		const char *charset;

		GIOChannel *channel;
		guint channel_id;
		WindowManager *windowmanager;
		Accounts *accounts;
		Connections *connections;
		BuddyList *buddylist;
		Conversations *conversations;
		Transfers *transfers;
		Log *log;
		Conf *conf;
		Keys *keys;

    Glib::RefPtr<Glib::MainLoop> gmainloop;

		//TODO: nice names
		typedef struct logbuf_item {
			PurpleDebugLevel level;
			char* category;
			char* arg_s;
		} logbuf_item;
		static std::vector<logbuf_item>* logbuf;
};

#endif /* __CENTERIM_H__ */
