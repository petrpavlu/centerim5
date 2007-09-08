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

#include <cppconsui/CppConsUI.h>

//TODO: configurable path using ./configure
#define CIM_CONFIG_PATH		".centerim"
#define CONF_PREFIX		"/centerim/"

#define CONF_PLUGIN_SAVE_PREF	"/centerim/plugins/loaded"

#define EXCEPTION_NONE			0
#define EXCEPTION_PURPLE_CORE_INIT	100

#define CONF_DEFAULT_LOG_MAX_LINES	1000
#define CONF_DEFAULT_LOG_MIN_LINES	10

#define CONF_DEFAULT_LOG_WINDOW_HEIGHT	6
#define CONF_DEFAULT_LOG_WINDOW_WIDTH	80

class Conf
{
	public:
		static Conf* Instance(void);
		static void Delete(void);

		void Reload(void);
		void Save(void);

		/* Configuration base get/set methods */
		int GetInt(const char *pref, const int defaultvalue);
		int GetInt(const char *pref, const int defaultvalue, const int min, const int max);
		void SetInt(const char *pref, const int value);
		bool GetBool(const char *pref, const bool defaultvalue);
		void SetBool(const char *pref, const bool value);

		Rect GetDimensions(const char *window, const int defx, const int defy, const int defwidth, const int defheight);
		void SetDimensions(const char *window, const Rect &rect);
		void SetDimensions(const char *window, const int x, const int y, const int width, const int height);

		unsigned int GetLogMaxLines(void);
		unsigned int GetChatPartitioning(void);

		Rect GetLogDimensions(void);
		Rect GetBuddyListDimensions(void);
		Rect GetChatDimensions(void);

		bool GetLogIms(void);
		bool GetLogChats(void);

	protected:

	private:
		Conf();
		~Conf();

		static Conf* instance;
};

#endif /* __CONF_H__ */
