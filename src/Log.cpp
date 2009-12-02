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

#include "Log.h"

#include "Conf.h"

#include "CIMWindowManager.h"

#include <cppconsui/CppConsUI.h>
#include <cppconsui/TextWindow.h>
#include <cppconsui/WindowManager.h>
#include <libpurple/debug.h>
#include <libpurple/util.h>
#include <glib.h>

Log* Log::Instance(void)
{
	static Log instance;
	return &instance;
}

//TODO sensible defaults
Log::Log(void)
: TextWindow(0, 0, 80, 24, NULL)
{
	conf = Conf::Instance();

#define REGISTER_G_LOG_HANDLER(name) \
	g_log_set_handler((name), (GLogLevelFlags)(G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL \
				| G_LOG_FLAG_RECURSION), glib_log_handler_, NULL)

	SetBorder(new Border());
	// TODO max_lines not used anywhere..
	max_lines = conf->GetLogMaxLines();
	MoveResize(conf->GetLogDimensions());

	// register the glib log handlers
	REGISTER_G_LOG_HANDLER(NULL);
	REGISTER_G_LOG_HANDLER("GLib");
	REGISTER_G_LOG_HANDLER("GModule");
	REGISTER_G_LOG_HANDLER("GLib-GObject");
	REGISTER_G_LOG_HANDLER("GThread");

	// redirect the debug messages to log
	g_set_print_handler(glib_print_);
	g_set_printerr_handler(glib_printerr_);

	// set the purple debug callbacks
	centerim_debug_ui_ops.print = purple_print_;
	centerim_debug_ui_ops.is_enabled = is_enabled_;
	purple_debug_set_ui_ops(&centerim_debug_ui_ops);

	// connect callbacks
	prefs_handle = purple_prefs_get_handle();
	purple_prefs_connect_callback(prefs_handle, CONF_PREFIX "log/debug", debug_change_, this);
}

Log::~Log(void)
{
	purple_prefs_disconnect_by_handle(prefs_handle);

	if (logfile)
		g_io_channel_unref(logfile);

	delete GetBorder();
}

void Log::Write(Level level, const char *fmt, ...)
{
	va_list args;
	gchar *text;

	if (conf->GetLogLevelCIM() < level)
		return; // we don't want to see this log message

	va_start(args, fmt);
	text = g_strdup_vprintf(fmt, args);
	va_end(args);

	WriteToFile(text);
	AddLine(text);

	g_free(text);
}

void Log::purple_print(PurpleDebugLevel purplelevel, const char *category, const char *arg_s)
{
	Level level = ConvertPurpleDebugLevel(purplelevel);

	if (!category) {
		category = "misc";
		Write(Level_warning, "centerim/log: purple_print() parameter category was not defined.\n");
	}

	Write(Type_purple, level, "libpurple/%s: %s", category, arg_s);
}

gboolean Log::is_enabled(PurpleDebugLevel purplelevel, const char *category)
{
	Level level = ConvertPurpleDebugLevel(purplelevel);

	if (conf->GetLogLevelPurple() < level)
		return FALSE;

	return TRUE;
}

void Log::glib_log_handler(const gchar *domain, GLogLevelFlags flags,
	const gchar *msg, gpointer user_data)
{
	Level level;
	char *new_msg = NULL;
	char *new_domain = NULL;

	if (flags & G_LOG_LEVEL_DEBUG)
		level = Level_debug;
	else if (flags & G_LOG_LEVEL_INFO)
		level = Level_info;
	else if (flags & G_LOG_LEVEL_MESSAGE)
		level = Level_message;
	else if (flags & G_LOG_LEVEL_WARNING)
		level = Level_warning;
	else if (flags & G_LOG_LEVEL_CRITICAL)
		level = Level_critical;
	else if (flags & G_LOG_LEVEL_ERROR)
		level = Level_error;
	else {
		Write(Type_cim, Level_warning, "centerim/log: Unknown glib logging level in %d.\n", flags);
		/* This will never happen. Actually should not, because some day, it
		 * will happen :) So lets initialize level, so that we don't have
		 * uninitialized values :) */
		level = Level_debug;
	}

	if (msg != NULL)
		new_msg = purple_utf8_try_convert(msg);

	if (domain != NULL)
		new_domain = purple_utf8_try_convert(domain);

	if (new_msg != NULL) {
		Write(Type_glib, level, "glib/%s: %s", (new_domain != NULL ? new_domain : "g_log"), new_msg);
		g_free(new_msg);
	}

	g_free(new_domain);
}

/* TODO I'm not quite sure if these two glib callbacks are really needed
 * because we don't use g_print() and g_printerr() anywhere...
 * see http://library.gnome.org/devel/glib/stable/glib-Warnings-and-Assertions.html#g-print
 */
void Log::glib_print(const char *msg)
{
	// TODO other level more approriate?
	Write(Type_glib, Level_debug, "glib/misc: %s", msg);
}

void Log::glib_printerr(const char *msg)
{
	// TODO other level more approriate?
	Write(Type_glib, Level_debug, "glib/miscerr: %s", msg);
}

void Log::debug_change(const char *name, PurplePrefType type, gconstpointer val)
{
	// debug was disabled so close logfile if it's opened
	if (!conf->GetBool(CONF_PREFIX "log/debug", false) && logfile) {
		g_io_channel_unref(logfile);
		logfile = NULL;
	}
}

void Log::ScreenResized()
{
	MoveResize((CIMWindowManager::Instance())->ScreenAreaSize(CIMWindowManager::Log));
}

void Log::Write(Type type, Level level, const char *fmt, ...)
{
	va_list args;
	gchar *text;

	if ((type == Type_glib && conf->GetLogLevelGlib() < level)
		|| (type == Type_purple && conf->GetLogLevelPurple() < level)
		|| (type == Type_cim && conf->GetLogLevelCIM() < level))
		return; // we don't want to see this log message

	va_start(args, fmt);
	text = g_strdup_vprintf(fmt, args);
	va_end(args);

	WriteToFile(text);
	AddLine(text);

	g_free(text);
}

void Log::WriteToFile(const gchar *text)
{
	GError *err = NULL;

	if (conf->GetDebugEnabled()) {
		// open logfile if it isn't already opened
		if (!logfile) {
			gchar *filename = g_build_filename(purple_home_dir(), CIM_CONFIG_PATH,
					conf->GetString(CONF_PREFIX "log/filename", "debug"), NULL);
			logfile = g_io_channel_new_file(filename, "a", &err);
			if (err) {
				Write(Type_cim, Level_error, _("Error opening logfile `%s' (%s).\n"), filename, err->message);
				g_error_free(err);
				err = NULL;
			}
			g_free(filename);
		}

		// write text into logfile
		if (logfile) {
			g_io_channel_write_chars(logfile, text, -1, NULL, &err);
			if (err) {
				Write(Type_cim, Level_error, _("Error writing to logfile (%s).\n"), err->message);
				g_error_free(err);
				err = NULL;
			}
			g_io_channel_flush(logfile, &err);
			if (err) {
				Write(Type_cim, Level_error, _("Error flushing logfile (%s).\n"), err->message);
				g_error_free(err);
				err = NULL;
			}
		}
	}
}

Log::Level Log::ConvertPurpleDebugLevel(PurpleDebugLevel purplelevel)
{
	if (purplelevel == PURPLE_DEBUG_MISC)
		return Level_debug;
	if (purplelevel == PURPLE_DEBUG_INFO)
		return Level_info;
	if (purplelevel == PURPLE_DEBUG_WARNING)
		return Level_warning;
	if (purplelevel == PURPLE_DEBUG_ERROR)
		return Level_critical;
	if (purplelevel == PURPLE_DEBUG_FATAL)
		return Level_error;
	if (purplelevel == PURPLE_DEBUG_ALL)
		return Level_error; // use error level so this message is always printed

	Write(Level_warning, "centerim/log: Unknown libpurple logging level: %d.\n", purplelevel);
	/* This will never happen. Actually should not, because some day, it
	 * will happen :) So lets initialize level, so that we don't have
	 * uninitialized values :) */
	return Level_debug;
}
