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

#ifndef __LOG_H__
#define __LOG_H__

#include <cppconsui/TextWindow.h>
#include <libpurple/debug.h>
#include <libpurple/prefs.h>
#include <vector>

class Conf;

class Log
: public TextWindow
{
	public:
		// levels are 1:1 mapped to glib levels
		enum Level {
			Level_none,
			Level_debug, // = misc in libpurple
			Level_info,
			Level_message, // no such level in libpurple
			Level_warning,
			Level_critical, // = error in libpurple
			Level_error // = fatal in libpurle
		};

		static Log &Instance(void);

		void Write(Level level, const gchar *fmt, ...);

		// to catch libpurple's debug messages
		static void purple_print_(PurpleDebugLevel level, const char *category, const char *arg_s)
			{ Instance().purple_print(level, category, arg_s); }
		static gboolean is_enabled_(PurpleDebugLevel level, const char *category)
			{ return Instance().is_enabled(level, category); }

		void purple_print(PurpleDebugLevel level, const char *category, const char *arg_s);
		gboolean is_enabled(PurpleDebugLevel level, const char *category);

		// to catch glib's messages
		static void glib_log_handler_(const gchar *domain, GLogLevelFlags flags,
			const gchar *msg, gpointer user_data)
			{ Instance().glib_log_handler(domain, flags, msg, user_data); }
		void glib_log_handler(const gchar *domain, GLogLevelFlags flags, const gchar *msg, gpointer user_data);

		// called when log/debug pref changed
		static void debug_change_(const char *name, PurplePrefType type,
				gconstpointer val, gpointer data)
			{ ((Log *) data)->debug_change(name, type, val); }
		void debug_change(const char *name, PurplePrefType type, gconstpointer val);

		virtual void ScreenResized(void);

	protected:

	private:
		enum Type {Type_cim, Type_glib, Type_purple};

		PurpleDebugUiOps centerim_debug_ui_ops;

		GIOChannel *logfile;
		void *prefs_handle;

		int max_lines;
		Conf *conf;

		Log(void);
		Log(const Log &);
		Log &operator=(const Log &);
		~Log(void);

		void Write(Type type, Level level, const gchar *fmt, ...);
		void WriteToWindow(Level level, const gchar *fmt, ...);
		void WriteToFile(const gchar *text);
		void AddTextToWindow(gchar *text);
		Level ConvertPurpleDebugLevel(PurpleDebugLevel purplelevel);
};

#endif /* __LOG_H__ */
