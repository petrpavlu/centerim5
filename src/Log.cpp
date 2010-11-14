/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010 by CenterIM developers
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
#include "CenterIM.h"
#include "Defines.h"

#include <libpurple/debug.h>
#include <libpurple/util.h>
#include <cstring>
#include "gettext.h"

Log *Log::Instance()
{
  static Log instance;
  return &instance;
}

// TODO sensible defaults
Log::Log()
: Window(0, 0, 80, 24, NULL, TYPE_NON_FOCUSABLE)
, logfile(NULL)
, prefs_handle(NULL)
{
  SetColorScheme("log");

  memset(&centerim_debug_ui_ops, 0, sizeof(centerim_debug_ui_ops));

  textview = new TextView(width - 2, height, true);
  AddWidget(*textview, 1, 0);

#define REGISTER_G_LOG_HANDLER(name) \
  g_log_set_handler((name), (GLogLevelFlags)(G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL \
        | G_LOG_FLAG_RECURSION), glib_log_handler_, NULL)

  // register the glib log handlers
  REGISTER_G_LOG_HANDLER(NULL);
  REGISTER_G_LOG_HANDLER("GLib");
  REGISTER_G_LOG_HANDLER("GModule");
  REGISTER_G_LOG_HANDLER("GLib-GObject");
  REGISTER_G_LOG_HANDLER("GThread");
  REGISTER_G_LOG_HANDLER("cppconsui");

  // connect callbacks
  prefs_handle = purple_prefs_get_handle();
  purple_prefs_connect_callback(prefs_handle, CONF_PREFIX "log/debug", debug_change_, this);

  // set the purple debug callbacks
  centerim_debug_ui_ops.print = purple_print_;
  centerim_debug_ui_ops.is_enabled = is_enabled_;
  purple_debug_set_ui_ops(&centerim_debug_ui_ops);
}

Log::~Log()
{
  purple_prefs_disconnect_by_handle(prefs_handle);

  if (logfile)
    g_io_channel_unref(logfile);
}

void Log::MoveResize(int newx, int newy, int neww, int newh)
{
  Window::MoveResize(newx, newy, neww, newh);

  textview->MoveResize(1, 0, width - 2, height);
}

#define WRITE_METHOD(name, level)                       \
void Log::name(const gchar *fmt, ...)                   \
{                                                       \
  va_list args;                                         \
  gchar *text;                                          \
                                                        \
  if (CONF->GetLogLevelCIM() < level)                   \
    return; /* we don't want to see this log message */ \
                                                        \
  va_start(args, fmt);                                  \
  text = g_strdup_vprintf(fmt, args);                   \
  va_end(args);                                         \
                                                        \
  Write(text);                                          \
  g_free(text);                                         \
}

WRITE_METHOD(Error, LEVEL_ERROR)
WRITE_METHOD(Critical, LEVEL_CRITICAL)
WRITE_METHOD(Warning, LEVEL_WARNING)
WRITE_METHOD(Message, LEVEL_MESSAGE)
WRITE_METHOD(Info, LEVEL_INFO)
WRITE_METHOD(Debug, LEVEL_DEBUG)

#undef WRITE_METHOD

void Log::purple_print(PurpleDebugLevel purplelevel, const char *category, const char *arg_s)
{
  Level level = ConvertPurpleDebugLevel(purplelevel);

  if (!category) {
    category = "misc";
    Warning("centerim/log: purple_print() parameter category was not defined.\n");
  }

  Write(TYPE_PURPLE, level, "libpurple/%s: %s", category, arg_s);
}

gboolean Log::is_enabled(PurpleDebugLevel purplelevel, const char *category)
{
  Level level = ConvertPurpleDebugLevel(purplelevel);

  if (CONF->GetLogLevelPurple() < level)
    return FALSE;

  return TRUE;
}

void Log::glib_log_handler(const gchar *domain, GLogLevelFlags flags,
  const gchar *msg, gpointer user_data)
{
  Level level;

  if (flags & G_LOG_LEVEL_DEBUG)
    level = LEVEL_DEBUG;
  else if (flags & G_LOG_LEVEL_INFO)
    level = LEVEL_INFO;
  else if (flags & G_LOG_LEVEL_MESSAGE)
    level = LEVEL_MESSAGE;
  else if (flags & G_LOG_LEVEL_WARNING)
    level = LEVEL_WARNING;
  else if (flags & G_LOG_LEVEL_CRITICAL)
    level = LEVEL_CRITICAL;
  else if (flags & G_LOG_LEVEL_ERROR)
    level = LEVEL_ERROR;
  else {
    Warning("centerim/log: Unknown glib logging level in %d.\n", flags);
    /* This will never happen. Actually should not, because some day, it
     * will happen :) So lets initialize level, so that we don't have
     * uninitialized values :) */
    level = LEVEL_DEBUG;
  }

  if (msg)
    Write(TYPE_GLIB, level, "%s: %s", domain ? domain : "g_log", msg);
}

void Log::debug_change(const char *name, PurplePrefType type, gconstpointer val)
{
  // debug was disabled so close logfile if it's opened
  if (!CONF->GetBool(CONF_PREFIX "log/debug", false) && logfile) {
    g_io_channel_unref(logfile);
    logfile = NULL;
  }
}

void Log::ScreenResized()
{
  MoveResizeRect(CENTERIM->ScreenAreaSize(CenterIM::LOG_AREA));
}

void Log::ShortenWindowText()
{
  int max_lines = CONF->GetLogMaxLines();
  int lines_num = textview->GetLinesNumber();

  if (lines_num > max_lines) {
    // remove 20 extra lines
    textview->Erase(0, lines_num - max_lines + 20);
  }
}

void Log::Write(const gchar *text)
{
  WriteToFile(text);
  textview->Append(text);
  ShortenWindowText();
}

void Log::Write(Type type, Level level, const gchar *fmt, ...)
{
  va_list args;
  gchar *text;

  if ((type == TYPE_GLIB && CONF->GetLogLevelGlib() < level)
    || (type == TYPE_PURPLE && CONF->GetLogLevelPurple() < level))
    return; // we don't want to see this log message

  va_start(args, fmt);
  text = g_strdup_vprintf(fmt, args);
  va_end(args);

  Write(text);

  g_free(text);
}

void Log::WriteErrorToWindow(const gchar *fmt, ...)
{
  va_list args;
  gchar *text;

  if (CONF->GetLogLevelCIM() < LEVEL_ERROR)
    return; // we don't want to see this log message

  va_start(args, fmt);
  text = g_strdup_vprintf(fmt, args);
  va_end(args);

  textview->Append(text);
  ShortenWindowText();

  g_free(text);
}

void Log::WriteToFile(const gchar *text)
{
  GError *err = NULL;

  if (CONF->GetDebugEnabled()) {
    // open logfile if it isn't already opened
    if (!logfile) {
      gchar *filename = g_build_filename(purple_user_dir(),
          CONF->GetString(CONF_PREFIX "log/filename", "debug"),
          NULL);
      if ((logfile = g_io_channel_new_file(filename, "a", &err))
          == NULL) {
        if (err) {
          WriteErrorToWindow(
              _("centerim/log: Error opening logfile `%s' (%s).\n"), filename,
              err->message);
          g_error_free(err);
          err = NULL;
        }
        else
          WriteErrorToWindow(_("centerim/log: Error opening logfile `%s'.\n"),
              filename);
      }
      g_free(filename);
    }

    // write text into logfile
    if (logfile) {
      if (g_io_channel_write_chars(logfile, text, -1, NULL, &err)
          != G_IO_STATUS_NORMAL) {
        if (err) {
          WriteErrorToWindow(
              _("centerim/log: Error writing to logfile (%s).\n"),
              err->message);
          g_error_free(err);
          err = NULL;
        }
        else
          WriteErrorToWindow(_("centerim/log: Error writing to logfile.\n"));
      }
      if (g_io_channel_flush(logfile, &err) != G_IO_STATUS_NORMAL) {
        if (err) {
          WriteErrorToWindow(
              _("centerim/log: Error flushing logfile (%s).\n"),
              err->message);
          g_error_free(err);
          err = NULL;
        }
        else
          WriteErrorToWindow(_("centerim/log: Error flushing logfile.\n"));
      }
    }
  }
}

Log::Level Log::ConvertPurpleDebugLevel(PurpleDebugLevel purplelevel)
{
  if (purplelevel == PURPLE_DEBUG_MISC)
    return LEVEL_DEBUG;
  if (purplelevel == PURPLE_DEBUG_INFO)
    return LEVEL_INFO;
  if (purplelevel == PURPLE_DEBUG_WARNING)
    return LEVEL_WARNING;
  if (purplelevel == PURPLE_DEBUG_ERROR)
    return LEVEL_CRITICAL;
  if (purplelevel == PURPLE_DEBUG_FATAL)
    return LEVEL_ERROR;
  if (purplelevel == PURPLE_DEBUG_ALL)
    return LEVEL_ERROR; // use error level so this message is always printed

  Warning("centerim/log: Unknown libpurple logging level: %d.\n",
      purplelevel);
  /* This will never happen. Actually should not, because some day, it will
   * happen :) So lets initialize level, so that we don't have uninitialized
   * values :) */
  return LEVEL_DEBUG;
}
