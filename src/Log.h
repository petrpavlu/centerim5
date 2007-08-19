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

#include "Conf.h"

#include <cppconsui/TextWindow.h>
#include <libpurple/debug.h>
#include <string>
#include <vector>


class Log
: public TextWindow
{
	public:
                static Log* Instance(void);
                static void Delete(void);

		//TODO WriteFatal, WriteMisc etc
		void WriteInfo(const std::string text);
		void Write(PurpleDebugLevel level, const std::string text);
		void Write(PurpleDebugLevel level, const char *fmt, ...);
		void WriteToFile(const std::string test);

		/* to catch libpurple's debug messages */
		static void purple_print_(PurpleDebugLevel level, const char *category, const char *arg_s)
			{ Log::Instance()->purple_print(level, category, arg_s); }
		static gboolean isenabled_(PurpleDebugLevel level, const char *category)
			{ return Log::Instance()->isenabled(level, category); }
		static void glib_log_handler_(const gchar *domain, GLogLevelFlags flags,
			const gchar *msg, gpointer user_data)
			{ Log::Instance()->glib_log_handler(domain, flags, msg, user_data); }
		static void glib_print_(const char *msg)
			{ Log::Instance()->glib_print_(msg); }
		static void glib_printerr_(const char *msg)
			{ Log::Instance()->glib_printerr_(msg); }

		void purple_print(PurpleDebugLevel level, const char *category, const char *arg_s);
		gboolean isenabled(PurpleDebugLevel level, const char *category);
		void glib_log_handler(const gchar *domain, GLogLevelFlags flags, const gchar *msg, gpointer user_data);
		void glib_print(const char *msg);
		void glib_printerr(const char *msg);

	protected:

	private:
		Log(void);
		~Log(void);

		static Log *instance;

		std::vector<std::wstring> buffer;
		int max_lines;
		Conf *conf;
};

#endif /* __LOG_H__ */
