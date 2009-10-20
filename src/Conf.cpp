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


#include "Conf.h"

#include <cppconsui/CppConsUI.h>

#include <libpurple/prefs.h>
#include <libpurple/plugin.h>
#include <libpurple/pounce.h>

//TODO sensible values
#define CONF_PERCENTAGE_MIN		0
#define CONF_PERCENTAGE_MAX		100

#define CONF_LOG_MAX_LINES_MIN		10
#define CONF_LOG_MAX_LINES_MAX		100000
#define CONF_LOG_MAX_LINES_DEFAULT	1000

#define CONF_LOG_DIMENSIONS_X		40
#define CONF_LOG_DIMENSIONS_Y		40
#define CONF_LOG_DIMENSIONS_WIDTH	100
#define CONF_LOG_DIMENSIONS_HEIGHT	16

#define CONF_BUDDYLIST_DIMENSIONS_X		0
#define CONF_BUDDYLIST_DIMENSIONS_Y		0
#define CONF_BUDDYLIST_DIMENSIONS_WIDTH		40
#define CONF_BUDDYLIST_DIMENSIONS_HEIGHT	50

#define CONF_CHAT_DIMENSIONS_Y		00
#define CONF_CHAT_DIMENSIONS_WIDTH	100
#define CONF_CHAT_DIMENSIONS_HEIGHT	40
#define CONF_CHAT_PARTITIONING_DEFAULT 80 /* 20% for the input window */

#define CONF_ACCOUNTS_DIMENSIONS_X		10
#define CONF_ACCOUNTS_DIMENSIONS_Y		50
#define CONF_ACCOUNTS_DIMENSIONS_WIDTH	80
#define CONF_ACCOUNTS_DIMENSIONS_HEIGHT	40

#define CONF_CHAT_DIMENSIONS_X		40
#define CONF_LOG_IMS_DEFAULT		TRUE
#define CONF_LOG_CHATS_DEFAULT		TRUE
#define CONF_LOG_SYSTEM_DEFAULT		FALSE

Conf* Conf::instance = NULL;

Conf* Conf::Instance(void)
{
	if (!instance) instance = new Conf();
	return instance;
}

void Conf::Delete(void)
{
	if (instance) {
		delete instance;
		instance = NULL;
	}
}

void Conf::Reload(void)
{
	purple_prefs_load();
}

void Conf::Save(void)
{
	/* Save the list of loaded plugins */
        purple_plugins_save_loaded(CONF_PLUGIN_SAVE_PREF);

	/* Preferences are saved automatically by libpurple, so no
	 * call to a `purple_prefs_save' */
}

int Conf::GetInt(const gchar *pref, const int defaultvalue)
{
	int i;

	if (purple_prefs_exists(pref)) {
		i = purple_prefs_get_int(pref);
	} else {
		AddPath(pref);
		purple_prefs_add_int(pref, defaultvalue);
		i = defaultvalue;
		Save();
	}

	return i;
}

int Conf::GetInt(const gchar *pref, const int defaultvalue, const int min, const int max)
{
	int i;

	if (purple_prefs_exists(pref)) {
		i = purple_prefs_get_int(pref);
		if (i < min || i > max) {
			SetInt(pref, defaultvalue);
			i = defaultvalue;
		}
	} else {
		AddPath(pref);
		purple_prefs_add_int(pref, defaultvalue);
		i = defaultvalue;
		Save();
	}

	return i;
}

void Conf::SetInt(const gchar *pref, const int value)
{
	purple_prefs_set_int(pref, value);
	Save();
}

bool Conf::GetBool(const gchar *pref, const bool defaultvalue)
{
	bool b;

	if (purple_prefs_exists(pref)) {
		b = purple_prefs_get_bool(pref);
	} else {
		AddPath(pref);
		purple_prefs_add_bool(pref, defaultvalue);
		b = defaultvalue;
		Save();
	}

	return b;
}

void Conf::SetBool(const gchar *pref, const bool value)
{
	purple_prefs_set_bool(pref, value);
	Save();
}

const gchar* Conf::GetString(const gchar *pref, const gchar *defaultvalue)
{
	const gchar* s;

	if (purple_prefs_exists(pref)) {
		s = purple_prefs_get_string(pref);
	} else {
		AddPath(pref);
		purple_prefs_add_string(pref, defaultvalue);
		s = defaultvalue;
		Save();
	}

	return s;
}

void Conf::SetString(const gchar *pref, const gchar *value)
{
	purple_prefs_set_string(pref, value);
	Save();
}

Rect Conf::GetLogDimensions(void)
{
	return GetDimensions("log/",
		CONF_LOG_DIMENSIONS_X, CONF_LOG_DIMENSIONS_Y,
		CONF_LOG_DIMENSIONS_WIDTH, CONF_LOG_DIMENSIONS_HEIGHT);
}

Rect Conf::GetBuddyListDimensions(void)
{
	return GetDimensions("buddylist/",
		CONF_BUDDYLIST_DIMENSIONS_X, CONF_BUDDYLIST_DIMENSIONS_Y,
		CONF_BUDDYLIST_DIMENSIONS_WIDTH, CONF_BUDDYLIST_DIMENSIONS_HEIGHT);
}

Rect Conf::GetChatDimensions(void)
{
	return GetDimensions("chat/",
		CONF_CHAT_DIMENSIONS_X, CONF_CHAT_DIMENSIONS_Y,
		CONF_CHAT_DIMENSIONS_WIDTH, CONF_CHAT_DIMENSIONS_HEIGHT);
}

Rect Conf::GetAccountWindowDimensions(void)
{
	return GetDimensions("accounts/",
		CONF_ACCOUNTS_DIMENSIONS_X, CONF_ACCOUNTS_DIMENSIONS_Y,
		CONF_ACCOUNTS_DIMENSIONS_WIDTH, CONF_ACCOUNTS_DIMENSIONS_HEIGHT);
}

Rect Conf::GetDimensions(const gchar *window, const int defx, const int defy, const int defw, const int defh)
{
	gchar *prefx = g_strconcat(CONF_PREFIX, window, "dimensions/x", NULL);
	gchar *prefy = g_strconcat(CONF_PREFIX, window, "dimensions/y", NULL);
	gchar *prefw = g_strconcat(CONF_PREFIX, window, "dimensions/width", NULL);
	gchar *prefh = g_strconcat(CONF_PREFIX, window, "dimensions/height", NULL);
	int x = GetInt(prefx, defx);
	int y = GetInt(prefy, defy);
	int w = GetInt(prefw, defw);
	int h = GetInt(prefh, defh);
	g_free(prefx);
	g_free(prefy);
	g_free(prefw);
	g_free(prefh);

	return (Rect(x, y, w, h));
}

void Conf::SetDimensions(const gchar *window, const int x, const int y, const int width, const int height)
{
	gchar *prefx = g_strconcat(CONF_PREFIX, window, "dimensions/x", NULL);
	gchar *prefy = g_strconcat(CONF_PREFIX, window, "dimensions/y", NULL);
	gchar *prefw = g_strconcat(CONF_PREFIX, window, "dimensions/width", NULL);
	gchar *prefh = g_strconcat(CONF_PREFIX, window, "dimensions/height", NULL);
	SetInt(prefx, x);
	SetInt(prefy, y);
	SetInt(prefw, width);
	SetInt(prefh, height);
	g_free(prefx);
	g_free(prefy);
	g_free(prefw);
	g_free(prefh);
}

void Conf::SetDimensions(const gchar *window, const Rect &rect)
{
	SetDimensions(window, rect.x, rect.y, rect.width, rect.height);
}

bool Conf::GetDebugEnabled(void)
{
	gchar *pref = g_strconcat(CONF_PREFIX, "log/debug", NULL);
	bool b;

	b = GetBool(pref, false);

	g_free(pref);

	return b;
}

Log::Level Conf::GetLogLevel(const gchar *type)
{
	gchar *pref = g_strconcat(CONF_PREFIX, "log/log_level_", type, NULL);
	const gchar *slevel;
	Log::Level level = Log::Level_debug;
	
	if (!g_ascii_strncasecmp(type, "cim", 3))
		slevel = GetString(pref, "info");
	else
		slevel = GetString(pref, "none");

	if (!g_ascii_strncasecmp(slevel,"none", 4)) level = Log::Level_none;
	else if (!g_ascii_strncasecmp(slevel, "info", 4)) level = Log::Level_info;
	else if (!g_ascii_strncasecmp(slevel, "warning", 7)) level = Log::Level_warning;
	else if (!g_ascii_strncasecmp(slevel, "critical", 8)) level = Log::Level_critical;
	else if (!g_ascii_strncasecmp(slevel, "error", 5)) level = Log::Level_error;
	else if (!g_ascii_strncasecmp(slevel, "debug", 5)) level = Log::Level_debug;
	else {
		if (!g_ascii_strncasecmp(type, "cim", 3))
			SetLogLevel(type, Log::Level_info);
		else
			SetLogLevel(type, Log::Level_none);
	}
	g_free(pref);

	return level;
}

void Conf::SetLogLevel(const gchar *type, const Log::Level level)
{
	gchar *pref = g_strconcat(CONF_PREFIX, "log/log_level_", type, NULL);
	const gchar *slevel;

	if (level == Log::Level_none) slevel = "none";
	else if (level == Log::Level_info) slevel = "info";
	else if (level == Log::Level_warning) slevel = "warning";
	else if (level == Log::Level_critical) slevel = "critical";
	else if (level == Log::Level_error) slevel = "error";
	else if (level == Log::Level_debug) slevel = "debug";
	else {
		//TODO error!
	}
	//TODO finish

	g_free(pref);
}

unsigned int Conf::GetLogMaxLines()
{
	gchar *pref = g_strconcat(CONF_PREFIX, "log/LogMaxLines", NULL);

	int i = GetInt(pref, CONF_LOG_MAX_LINES_DEFAULT,
			CONF_LOG_MAX_LINES_MIN, CONF_LOG_MAX_LINES_MAX);

	g_free(pref);

	return i;
}

unsigned int Conf::GetChatPartitioning(void)
{
	gchar *pref = g_strconcat(CONF_PREFIX, "chat/partitioning", NULL);

	int i = GetInt(pref, CONF_CHAT_PARTITIONING_DEFAULT,
			CONF_PERCENTAGE_MIN, CONF_PERCENTAGE_MAX);

	g_free(pref);

	return i;
}

bool Conf::GetLogIms(void)
{
	return GetBool("/purple/logging/log_ims", CONF_LOG_IMS_DEFAULT);
}

bool Conf::GetLogChats(void)
{
	return GetBool("/purple/logging/log_chats", CONF_LOG_CHATS_DEFAULT);
}

bool Conf::GetLogSystem(void)
{
	return GetBool("/purple/logging/log_system", CONF_LOG_SYSTEM_DEFAULT);
}

void Conf::AddPath(const std::string &s)
{
	std::string::size_type i = 1;
	std::string ss;

	while ((i = s.find_first_of('/', i+1)) && i != std::string::npos) {
		ss = s.substr(0, i);
		purple_prefs_add_none(ss.c_str());
	}
}

Conf::Conf()
{
	/* let libpurple load the configuration file */
	purple_prefs_load();
	/* convert settings from older libpurple versions */
	purple_prefs_update_old();

        /* Load the desired plugins. The client should save the list of loaded plugins in
         * the preferences using purple_plugins_save_loaded(PLUGIN_SAVE_PREF) */
        purple_plugins_load_saved(CONF_PLUGIN_SAVE_PREF);

	/* Load the pounces. */
        purple_pounces_load();
}

Conf::~Conf()
{
	Save();
}
