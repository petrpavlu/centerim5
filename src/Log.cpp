/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2013 by CenterIM developers
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
 */

#include "Log.h"

#include <cppconsui/HorizontalListBox.h>
#include <cppconsui/Spacer.h>
#include "gettext.h"

Log *Log::my_instance = NULL;

Log *Log::instance()
{
  return my_instance;
}

void Log::onScreenResized()
{
  moveResizeRect(CENTERIM->getScreenArea(CenterIM::LOG_AREA));
}

#define WRITE_METHOD(name, level)                       \
void Log::name(const char *fmt, ...)                    \
{                                                       \
  va_list args;                                         \
  char *text;                                           \
                                                        \
  if (getLogLevel("cim") < level)                       \
    return; /* we don't want to see this log message */ \
                                                        \
  va_start(args, fmt);                                  \
  text = g_strdup_vprintf(fmt, args);                   \
  va_end(args);                                         \
                                                        \
  write(text);                                          \
  g_free(text);                                         \
}

WRITE_METHOD(error, LEVEL_ERROR)
WRITE_METHOD(critical, LEVEL_CRITICAL)
WRITE_METHOD(warning, LEVEL_WARNING)
WRITE_METHOD(message, LEVEL_MESSAGE)
WRITE_METHOD(info, LEVEL_INFO)
WRITE_METHOD(debug, LEVEL_DEBUG)

#undef WRITE_METHOD

Log::Log()
: Window(0, 0, 80, 24, NULL, TYPE_NON_FOCUSABLE)
, logfile(NULL)
{
  setColorScheme("log");

  memset(&centerim_debug_ui_ops, 0, sizeof(centerim_debug_ui_ops));

  CppConsUI::HorizontalListBox *lbox = new CppConsUI::HorizontalListBox(
      AUTOSIZE, AUTOSIZE);
  addWidget(*lbox, 0, 0);

  lbox->appendWidget(*(new CppConsUI::Spacer(1, AUTOSIZE)));
  textview = new CppConsUI::TextView(AUTOSIZE, AUTOSIZE, true);
  lbox->appendWidget(*textview);
  lbox->appendWidget(*(new CppConsUI::Spacer(1, AUTOSIZE)));

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
  purple_prefs_add_bool(CONF_PREFIX "/log/debug", false);
  purple_prefs_add_string(CONF_PREFIX "/log/filename", "debug.log");
  purple_prefs_add_string(CONF_PREFIX "/log/log_level_cim", "info");
  purple_prefs_add_string(CONF_PREFIX "/log/log_level_cppconsui", "warning");
  purple_prefs_add_string(CONF_PREFIX "/log/log_level_purple", "critical");
  purple_prefs_add_string(CONF_PREFIX "/log/log_level_glib", "warning");

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

void Log::init()
{
  g_assert(!my_instance);

  my_instance = new Log;
  my_instance->show();
}

void Log::finalize()
{
  g_assert(my_instance);

  delete my_instance;
  my_instance = NULL;
}

void Log::purple_print(PurpleDebugLevel purplelevel, const char *category,
    const char *arg_s)
{
  if (getLogLevel("purple") < convertPurpleDebugLevel(purplelevel))
    return; // we don't want to see this log message

  if (!category) {
    category = "misc";
    warning(_("centerim/log: purple_print() parameter category was "
         "not defined."));
  }

  char *text = g_strdup_printf("libpurple/%s: %s", category, arg_s);
  write(text);
  g_free(text);
}

gboolean Log::is_enabled(PurpleDebugLevel purplelevel,
    const char * /*category*/)
{
  Level level = convertPurpleDebugLevel(purplelevel);

  if (getLogLevel("purple") < level)
    return FALSE;

  return TRUE;
}

void Log::default_log_handler(const char *domain, GLogLevelFlags flags,
  const char *msg)
{
  if (getLogLevel("glib") < convertGlibDebugLevel(flags))
    return; // we don't want to see this log message

  if (!msg)
    return;

  char *text = g_strdup_printf("%s: %s", domain ? domain : "g_log", msg);
  write(text);
  g_free(text);
}

void Log::glib_log_handler(const char *domain, GLogLevelFlags flags,
  const char *msg)
{
  if (getLogLevel("glib") < convertGlibDebugLevel(flags))
    return; // we don't want to see this log message

  if (!msg)
    return;

  char *text = g_strdup_printf("%s: %s", domain ? domain : "g_log", msg);
  write(text);
  g_free(text);
}

void Log::cppconsui_log_handler(const char *domain, GLogLevelFlags flags,
  const char *msg)
{
  if (getLogLevel("cppconsui") < convertGlibDebugLevel(flags))
    return; // we don't want to see this log message

  if (!msg)
    return;

  char *text = g_strdup_printf("%s: %s", domain ? domain : "g_log", msg);
  write(text);
  g_free(text);
}

void Log::debug_change(const char * /*name*/, PurplePrefType type,
    gconstpointer val)
{
  g_assert(type == PURPLE_PREF_BOOLEAN);

  // debug was disabled so close logfile if it's opened
  if (!*static_cast<const gboolean*>(val) && logfile) {
    g_io_channel_unref(logfile);
    logfile = NULL;
  }
}

void Log::shortenWindowText()
{
  size_t lines_num = textview->getLinesNumber();

  if (lines_num > 200) {
    // remove 40 extra lines
    textview->erase(0, lines_num - 200 + 40);
  }
}

void Log::write(const char *text)
{
  writeToFile(text);
  textview->append(text);
  shortenWindowText();
}

void Log::writeErrorToWindow(const char *fmt, ...)
{
  va_list args;
  char *text;

  if (getLogLevel("cim") < LEVEL_ERROR)
    return; // we don't want to see this log message

  va_start(args, fmt);
  text = g_strdup_vprintf(fmt, args);
  va_end(args);

  textview->append(text);
  shortenWindowText();

  g_free(text);
}

void Log::writeToFile(const char *text)
{
  g_return_if_fail(text);

  GError *err = NULL;

  if (purple_prefs_get_bool(CONF_PREFIX "/log/debug")) {
    // open logfile if it isn't already opened
    if (!logfile) {
      char *filename = g_build_filename(purple_user_dir(),
          purple_prefs_get_string(CONF_PREFIX "/log/filename"), NULL);
      if (!(logfile = g_io_channel_new_file(filename, "a", &err))) {
        writeErrorToWindow(
            _("centerim/log: Error opening logfile '%s' (%s)."), filename,
            err->message);
        g_clear_error(&err);
      }
      g_free(filename);
    }

    // write text into logfile
    if (logfile) {
      if (g_io_channel_write_chars(logfile, text, -1, NULL, &err)
          != G_IO_STATUS_NORMAL) {
        writeErrorToWindow(_("centerim/log: Error writing to logfile (%s)."),
            err->message);
        g_clear_error(&err);
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
        writeErrorToWindow(_("centerim/log: Error flushing logfile (%s)."),
            err->message);
        g_clear_error(&err);
      }
    }
  }
}

Log::Level Log::convertPurpleDebugLevel(PurpleDebugLevel purplelevel)
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

  warning(_("centerim/log: Unknown libpurple logging level: %d."),
      purplelevel);
  return LEVEL_DEBUG;
}

Log::Level Log::convertGlibDebugLevel(GLogLevelFlags gliblevel)
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

  warning(_("centerim/log: Unknown GLib logging level: %d."), gliblevel);
  /* This will never happen. Actually should not, because some day, it will
   * happen. :) So lets initialize level, so that we don't have uninitialized
   * values. :) */
  return LEVEL_DEBUG;
}

Log::Level Log::getLogLevel(const char *type)
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

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
