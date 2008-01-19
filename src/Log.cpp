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

#include <cppconsui/CppConsUI.h>
#include <cppconsui/TextWindow.h>
#include <cppconsui/WindowManager.h>
#include <libpurple/debug.h>
#include <libpurple/util.h>
#include <stdio.h>
#include <glibmm/ustring.h>
#include <glib.h>

//TODO move inside log object
static PurpleDebugUiOps centerim_debug_ui_ops =
{
        Log::purple_print_,
        Log::isenabled_,
        NULL,
        NULL,
        NULL,
        NULL
};

Log* Log::instance = NULL;

Log* Log::Instance(void)
{
        if (!instance) instance = new Log();
        return instance;
}

void Log::Delete(void)
{
        if (instance) {
                delete instance;
                instance = NULL;
        }
}

//TODO sensible defaults
Log::Log(void)
: TextWindow(0, 0, 80, 24, NULL)
{
	conf = Conf::Instance();
/* Xerox */
#define REGISTER_G_LOG_HANDLER(name) \
        g_log_set_handler((name), (GLogLevelFlags)(G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL \
		| G_LOG_FLAG_RECURSION), \
		glib_log_handler_, NULL)

	SetBorder(new Border());
	max_lines = conf->GetLogMaxLines();
	MoveResize(conf->GetLogDimensions());

        /* Register the glib log handlers. */
        REGISTER_G_LOG_HANDLER(NULL);
        REGISTER_G_LOG_HANDLER("GLib");
        REGISTER_G_LOG_HANDLER("GModule");
        REGISTER_G_LOG_HANDLER("GLib-GObject");
        REGISTER_G_LOG_HANDLER("GThread");

	/* Redirect the debug messages to stderr */
	g_set_print_handler(glib_print_);   
        g_set_printerr_handler(glib_printerr_);

	/* Set the purple debug callbacks */
	purple_debug_set_ui_ops(&centerim_debug_ui_ops);
}

Log::~Log(void)
{
	delete GetBorder(); //TODO what if NULL?
}

void Log::WriteInfo(const std::string text)
{
	Write(PURPLE_DEBUG_INFO, text);	
}

void Log::Write(PurpleDebugLevel level, const char *fmt, ...)
{
	va_list args;
	char buf[1024]; //TODO lets hope this is enough!

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	Write(level, std::string(buf));
}

void Log::Write(PurpleDebugLevel level, const std::string text)
{
	//TODO get from config
	bool debug = true;
	//TODO get from config
	PurpleDebugLevel conflevel = PURPLE_DEBUG_ALL;

	//TODO if debugging is enabled, write to file and
	//show all debug messages in window
	if (debug) {
		level = PURPLE_DEBUG_ALL;
	} else {
		//TODO get from config
		PurpleDebugLevel conflevel = PURPLE_DEBUG_ALL;
	}
	//TODO if not enabled only show PURPLE_DEBUG_INFO level messages
	//perhaps also give user more control on what to show
	if (level != PURPLE_DEBUG_ALL
		&& level != PURPLE_DEBUG_MISC
		&& level != PURPLE_DEBUG_FATAL 
		&& level > conflevel)
		return;

	if (debug)
		WriteToFile(text);

	AddLine(text);
}

void Log::WriteToFile(const std::string test)
{
	//TODO if debugging is enabled, write to file
	//TODO try to use the log.h api of libpurple
}

void Log::purple_print(PurpleDebugLevel level, const char *category, const char *arg_s)
{
	std::string text;
	text.append("libpurple/");
	text.append(category);
	text.append(": ");
	text.append(arg_s);
	Write(level, text);
}

gboolean Log::isenabled(PurpleDebugLevel level, const char *category)
{
	//MISC and FATAL should always be shown!
	if (level == PURPLE_DEBUG_MISC || level == PURPLE_DEBUG_FATAL)
		return TRUE;

	//TODO lookup in config if purple debugging is enabled
	//and return false/true appropiately
	return TRUE;
}

/* Xerox (almost) */
void Log::glib_log_handler(const gchar *domain, GLogLevelFlags flags,
	const gchar *msg, gpointer user_data)
{
        PurpleDebugLevel level;
        char *new_msg = NULL;
        char *new_domain = NULL;

        if ((flags & G_LOG_LEVEL_ERROR) == G_LOG_LEVEL_ERROR)
                level = PURPLE_DEBUG_ERROR;
        else if ((flags & G_LOG_LEVEL_CRITICAL) == G_LOG_LEVEL_CRITICAL)
                level = PURPLE_DEBUG_FATAL;
        else if ((flags & G_LOG_LEVEL_WARNING) == G_LOG_LEVEL_WARNING)
                level = PURPLE_DEBUG_WARNING;
        else if ((flags & G_LOG_LEVEL_MESSAGE) == G_LOG_LEVEL_MESSAGE)
                level = PURPLE_DEBUG_INFO;
        else if ((flags & G_LOG_LEVEL_INFO) == G_LOG_LEVEL_INFO)
                level = PURPLE_DEBUG_INFO;
        else if ((flags & G_LOG_LEVEL_DEBUG) == G_LOG_LEVEL_DEBUG)
                level = PURPLE_DEBUG_MISC;
        else
        {
		Write(PURPLE_DEBUG_MISC, "centerim/log: Unknown glib logging level in %d\n", flags);
                /* This will never happen. Actually should not, because some day, it will happen :)
		 * So lets initialize lever, so that we don't have uninitialized values :) */
		level = PURPLE_DEBUG_ALL;
        }

        if (msg != NULL)
                new_msg = purple_utf8_try_convert(msg);

        if (domain != NULL)
                new_domain = purple_utf8_try_convert(domain);

        if (new_msg != NULL)
        {
		Write(level, "glib/%s: %s", (new_domain != NULL ? new_domain : "g_log"), new_msg);
                g_free(new_msg);
        }

        g_free(new_domain);
}

void Log::glib_print(const char *msg)
{
	std::string text;
	text.append("glib/misc: ");
	text.append(msg);
	//TODO other level more approriate?
	Write(PURPLE_DEBUG_MISC, text);
}

void Log::glib_printerr(const char *msg)
{
	std::string text;
	text.append("glib/miscerr: ");
	text.append(msg);
	//TODO other level more approriate?
	Write(PURPLE_DEBUG_MISC, text);
}
