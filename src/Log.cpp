/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2011 by CenterIM developers
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

#include "CenterIM.h"

#include <string.h>
#include "gettext.h"

Log *Log::instance = NULL;

Log *Log::Instance()
{
  return instance;
}

void Log::MoveResize(int newx, int newy, int neww, int newh)
{
  Window::MoveResize(newx, newy, neww, newh);

  textview->MoveResize(1, 0, width - 2, height);
}

void Log::ScreenResized()
{
  MoveResizeRect(CENTERIM->GetScreenAreaSize(CenterIM::LOG_AREA));
}

#define WRITE_METHOD(name, level)                       \
void Log::name(const char *fmt, ...)                   \
{                                                       \
  va_list args;                                         \
  char *text;                                          \
                                                        \
  if (GetLogLevel("cim") < level)                       \
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

Log::Log()
: Window(0, 0, 80, 24, NULL, TYPE_NON_FOCUSABLE)
, logfile(NULL)
{
  SetColorScheme("log");

  memset(&centerim_debug_ui_ops, 0, sizeof(centerim_debug_ui_ops));

  textview = new CppConsUI::TextView(width - 2, height, true);
  AddWidget(*textview, 1, 0);

#define REGISTER_G_LOG_HANDLER(name, handler) \
  g_log_set_handler((name), (GLogLevelFlags)G_LOG_LEVEL_MASK, (handler), this)

  // register the glib log handlers
  default_handler = REGISTER_G_LOG_HANDLER(NULL, default_log_handler_);
  glib_handler = REGISTER_G_LOG_HANDLER("GLib", glib_log_handler_);
  gmodule_handler = REGISTER_G_LOG_HANDLER("GModule", glib_log_handler_);
  glib_gobject_handler = REGISTER_G_LOG_HANDLER("GLib-GObject",
      glib_log_handler_);
  gthread_handler = REGISTER_G_LOG_HANDLER("GThread", glib_log_handler_);
  cppconsui_handler = REGISTER_G_LOG_HANDLER("cppconsui",
      cppconsui_log_handler_);

  // init prefs
  purple_prefs_add_none(CONF_PREFIX "/log");
  purple_prefs_add_int(CONF_PREFIX "/log/log_max_lines", 500);
  purple_prefs_add_bool(CONF_PREFIX "/log/debug", false);
  purple_prefs_add_string(CONF_PREFIX "/log/filename", "debug.log");
  purple_prefs_add_string(CONF_PREFIX "/log/log_level_cim", "info");
  purple_prefs_add_string(CONF_PREFIX "/log/log_level_cppconsui", "error");
  purple_prefs_add_string(CONF_PREFIX "/log/log_level_purple", "error");
  purple_prefs_add_string(CONF_PREFIX "/log/log_level_glib", "error");

  // connect callbacks
  purple_prefs_connect_callback(this, CONF_PREFIX "/log/debug", debug_change_,
      this);

  // set the purple debug callbacks
  centerim_debug_ui_ops.print = purple_print_;
  centerim_debug_ui_ops.is_enabled = is_enabled_;
  purple_debug_set_ui_ops(&centerim_debug_ui_ops);
}

Log::~Log()
{
  purple_debug_set_ui_ops(NULL);

  g_log_remove_handler(NULL, default_handler);
  g_log_remove_handler("GLib", glib_handler);
  g_log_remove_handler("GModule", gmodule_handler);
  g_log_remove_handler("GLib-GObject", glib_gobject_handler);
  g_log_remove_handler("GThread", gthread_handler);
  g_log_remove_handler("cppconsui", cppconsui_handler);

  purple_prefs_disconnect_by_handle(this);

  if (logfile)
    g_io_channel_unref(logfile);
}

void Log::Init()
{
  g_assert(!instance);

  instance = new Log;
  instance->Show();
}

void Log::Finalize()
{
  g_assert(instance);

  delete instance;
  instance = NULL;
}

void Log::purple_print(PurpleDebugLevel purplelevel, const char *category,
    const char *arg_s)
{
  if (GetLogLevel("purple") < ConvertPurpleDebugLevel(purplelevel))
    return; // we don't want to see this log message

  if (!category) {
    category = "misc";
    Warning(_("centerim/log: purple_print() parameter category was not defined."));
  }

  char *text = g_strdup_printf("libpurple/%s: %s", category, arg_s);
  Write(text);
  g_free(text);
}

gboolean Log::is_enabled(PurpleDebugLevel purplelevel, const char *category)
{
  Level level = ConvertPurpleDebugLevel(purplelevel);

  if (GetLogLevel("purple") < level)
    return FALSE;

  return TRUE;
}

void Log::default_log_handler(const char *domain, GLogLevelFlags flags,
  const char *msg)
{
  if (GetLogLevel("glib") < ConvertGlibDebugLevel(flags))
    return; // we don't want to see this log message

  if (!msg)
    return;

  char *text = g_strdup_printf("%s: %s", domain ? domain : "g_log", msg);
  Write(text);
  g_free(text);
}

void Log::glib_log_handler(const char *domain, GLogLevelFlags flags,
  const char *msg)
{
  if (GetLogLevel("glib") < ConvertGlibDebugLevel(flags))
    return; // we don't want to see this log message

  if (!msg)
    return;

  char *text = g_strdup_printf("%s: %s", domain ? domain : "g_log", msg);
  Write(text);
  g_free(text);
}

void Log::cppconsui_log_handler(const char *domain, GLogLevelFlags flags,
  const char *msg)
{
  if (GetLogLevel("cppconsui") < ConvertGlibDebugLevel(flags))
    return; // we don't want to see this log message

  if (!msg)
    return;

  char *text = g_strdup_printf("%s: %s", domain ? domain : "g_log", msg);
  Write(text);
  g_free(text);
}

void Log::debug_change(const char *name, PurplePrefType type,
    gconstpointer val)
{
  g_assert(type == PURPLE_PREF_BOOLEAN);

  // debug was disabled so close logfile if it's opened
  if (!*static_cast<const gboolean*>(val) && logfile) {
    g_io_channel_unref(logfile);
    logfile = NULL;
  }
}

void Log::ShortenWindowText()
{
  int max_lines = purple_prefs_get_int(CONF_PREFIX "/log/log_max_lines");
  max_lines = CLAMP(max_lines, 50, 1000);
  int lines_num = textview->GetLinesNumber();

  if (lines_num > max_lines) {
    // remove 20 extra lines
    textview->Erase(0, lines_num - max_lines + 20);
  }
}

void Log::Write(const char *text)
{
  WriteToFile(text);
  textview->Append(text);
  ShortenWindowText();
}

void Log::WriteErrorToWindow(const char *fmt, ...)
{
  va_list args;
  char *text;

  if (GetLogLevel("cim") < LEVEL_ERROR)
    return; // we don't want to see this log message

  va_start(args, fmt);
  text = g_strdup_vprintf(fmt, args);
  va_end(args);

  textview->Append(text);
  ShortenWindowText();

  g_free(text);
}

void Log::WriteToFile(const char *text)
{
  g_return_if_fail(text);

  GError *err = NULL;

  if (purple_prefs_get_bool(CONF_PREFIX "/log/debug")) {
    // open logfile if it isn't already opened
    if (!logfile) {
      char *filename = g_build_filename(purple_user_dir(),
          purple_prefs_get_string(CONF_PREFIX "/log/filename"), NULL);
      if ((logfile = g_io_channel_new_file(filename, "a", &err))
          == NULL) {
        if (err) {
          WriteErrorToWindow(
              _("centerim/log: Error opening logfile `%s' (%s)."), filename,
              err->message);
          g_error_free(err);
          err = NULL;
        }
        else
          WriteErrorToWindow(_("centerim/log: Error opening logfile `%s'."),
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
              _("centerim/log: Error writing to logfile (%s)."),
              err->message);
          g_error_free(err);
          err = NULL;
        }
        else
          WriteErrorToWindow(_("centerim/log: Error writing to logfile."));
      }
      else {
        // if necessary write missing EOL character
        size_t len = strlen(text);
        if (len && text[len - 1] != '\n') {
          // ignore all errors
          g_io_channel_write_chars(logfile, "\n", -1, NULL, NULL);
        }
      }

      if (g_io_channel_flush(logfile, &err) != G_IO_STATUS_NORMAL) {
        if (err) {
          WriteErrorToWindow(
              _("centerim/log: Error flushing logfile (%s)."),
              err->message);
          g_error_free(err);
          err = NULL;
        }
        else
          WriteErrorToWindow(_("centerim/log: Error flushing logfile."));
      }
    }
  }
}

Log::Level Log::ConvertPurpleDebugLevel(PurpleDebugLevel purplelevel)
{
  switch (purplelevel) {
    case PURPLE_DEBUG_MISC:
      return LEVEL_DEBUG;
    case PURPLE_DEBUG_INFO:
      return LEVEL_INFO;
    case PURPLE_DEBUG_WARNING:
      return LEVEL_WARNING;
    case PURPLE_DEBUG_ERROR:
      return LEVEL_CRITICAL;
    case PURPLE_DEBUG_FATAL:
      return LEVEL_ERROR;
    case PURPLE_DEBUG_ALL:
      return LEVEL_ERROR; // use error level so this message is always printed
  }

  Warning(_("centerim/log: Unknown libpurple logging level: %d."),
      purplelevel);
  return LEVEL_DEBUG;
}

Log::Level Log::ConvertGlibDebugLevel(GLogLevelFlags gliblevel)
{
  if (gliblevel & G_LOG_LEVEL_DEBUG)
    return LEVEL_DEBUG;
  if (gliblevel & G_LOG_LEVEL_INFO)
    return LEVEL_INFO;
  if (gliblevel & G_LOG_LEVEL_MESSAGE)
    return LEVEL_MESSAGE;
  if (gliblevel & G_LOG_LEVEL_WARNING)
    return LEVEL_WARNING;
  if (gliblevel & G_LOG_LEVEL_CRITICAL)
    return LEVEL_CRITICAL;
  if (gliblevel & G_LOG_LEVEL_ERROR)
    return LEVEL_ERROR;

  Warning(_("centerim/log: Unknown GLib logging level: %d."), gliblevel);
  /* This will never happen. Actually should not, because some day, it will
   * happen. :) So lets initialize level, so that we don't have uninitialized
   * values. :) */
  return LEVEL_DEBUG;
}

Log::Level Log::GetLogLevel(const char *type)
{
  char *pref = g_strconcat(CONF_PREFIX "/log/log_level_", type, NULL);
  const char *slevel = "none";
  if (purple_prefs_exists(pref))
    slevel = purple_prefs_get_string(pref);
  g_free(pref);

  Level level;
  if (!g_ascii_strcasecmp(slevel, "none"))
    level = LEVEL_NONE;
  else if (!g_ascii_strcasecmp(slevel, "debug"))
    level = LEVEL_DEBUG;
  else if (!g_ascii_strcasecmp(slevel, "info"))
    level = LEVEL_INFO;
  else if (!g_ascii_strcasecmp(slevel, "message"))
    level = LEVEL_MESSAGE;
  else if (!g_ascii_strcasecmp(slevel, "warning"))
    level = LEVEL_WARNING;
  else if (!g_ascii_strcasecmp(slevel, "critical"))
    level = LEVEL_CRITICAL;
  else if (!g_ascii_strcasecmp(slevel, "error"))
    level = LEVEL_ERROR;
  else
    level = LEVEL_NONE;

  return level;
}
