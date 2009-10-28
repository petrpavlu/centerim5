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
#include <string>
#include <vector>

class Conf;

class Log
: public TextWindow
{
	public:
		enum Type {Type_cim, Type_glib, Type_purple};
		enum Level {Level_none,
			Level_info,
			Level_fatal,
			Level_error,
			Level_critical,
			Level_warning,
			Level_debug }; //TODO check order with purpledebuglevel and glib log flags
		// TODO -- why not use directly the either purpledebuglevel or glib log flags ? -- do we need this mapping ?

                static Log* Instance(void);
                static void Delete(void);

		void Close(void);

		//TODO WriteFatal, WriteMisc etc
		void WriteInfo(const std::string text)
			{ Write(Log::Type_cim, Log::Level_info, text); }

		void Write(Log::Type type, Log::Level level, const std::string text);
		void Write(Log::Level level, const std::string text)
			{ Write(Log::Type_cim, level, text);}
		void Write(Log::Type type, Log::Level level, const char *fmt, ...);
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
			{ Log::Instance()->glib_print(msg); }
		static void glib_printerr_(const char *msg)
			{ Log::Instance()->glib_printerr(msg); }

		void purple_print(PurpleDebugLevel level, const char *category, const char *arg_s);
		gboolean isenabled(PurpleDebugLevel level, const char *category);
		void glib_log_handler(const gchar *domain, GLogLevelFlags flags, const gchar *msg, gpointer user_data);
		void glib_print(const char *msg);
		void glib_printerr(const char *msg);

		virtual void ScreenResized();

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
