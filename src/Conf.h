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

#ifndef __CONF_H__
#define __CONF_H__

#include "Log.h"

#include <cppconsui/CppConsUI.h>

#include <string>

#include "Defines.h"

//TODO Do those REALLY belong here?
#define CONF_DEFAULT_LOG_MAX_LINES	1000
#define CONF_DEFAULT_LOG_MIN_LINES	10

#define CONF_DEFAULT_LOG_WINDOW_HEIGHT	6
#define CONF_DEFAULT_LOG_WINDOW_WIDTH	80

class Log;

class Conf
{
	public:
		static Conf* Instance(void);
		static void Delete(void);

		void Reload(void);
		void Save(void);

		/* Configuration base get/set methods */
		int GetInt(const gchar *pref, const int defaultvalue);
		int GetInt(const gchar *pref, const int defaultvalue, const int min, const int max);
		void SetInt(const gchar *pref, const int value);
		bool GetBool(const gchar *pref, const bool defaultvalue);
		void SetBool(const gchar *pref, const bool value);
		const gchar* GetString(const gchar *pref, const gchar *defaultvalue);
		void SetString(const gchar *pref, const gchar *value);

		Rect GetDimensions(const gchar *window, const int defx, const int defy, const int defwidth, const int defheight);
		void SetDimensions(const gchar *window, const Rect &rect);
		void SetDimensions(const gchar *window, const int x, const int y, const int width, const int height);

		/* for debugging and logging */
		bool GetDebugEnabled(void);
		Log::Level GetLogLevelGlib(void)
			{ return GetLogLevel("glib"); }
		Log::Level GetLogLevelPurple(void)
			{ return GetLogLevel("purple"); }
		Log::Level GetLogLevelCIM(void)
			{ return GetLogLevel("cim"); }
		void SetLogLevelGlib(Log::Level level)
			{ SetLogLevel("glib", level); }
		void SetLogLevelPurple(Log::Level level)
			{ SetLogLevel("purple", level); }
		void SetLogLevelCIM(Log::Level level)
			{ SetLogLevel("cim", level); }

		unsigned int GetLogMaxLines(void);
		unsigned int GetChatPartitioning(void);

		Rect GetLogDimensions(void);
		Rect GetBuddyListDimensions(void);
		Rect GetChatDimensions(void);
		Rect GetAccountWindowDimensions(void);

		bool GetLogIms(void);
		bool GetLogChats(void);
		bool GetLogSystem(void);

	protected:

	private:
		static Conf* instance;

		Conf();
		~Conf();

		void AddPath(const std::string &s);

		Log::Level GetLogLevel(const gchar *type);
		void SetLogLevel(const gchar *type, Log::Level level);
};

#endif /* __CONF_H__ */
