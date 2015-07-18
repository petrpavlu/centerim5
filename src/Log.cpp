/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2015 by CenterIM developers
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
#include <cstdio>
#include "gettext.h"

Log *Log::my_instance = NULL;

Log *Log::instance()
{
  return my_instance;
}

#define WRITE_METHOD(name, level)                                              \
  void Log::name(const char *fmt, ...)                                         \
  {                                                                            \
    va_list args;                                                              \
    char *text;                                                                \
                                                                               \
    if (log_level_cim < level)                                                 \
      return; /* we don't want to see this log message */                      \
                                                                               \
    va_start(args, fmt);                                                       \
    text = g_strdup_vprintf(fmt, args);                                        \
    va_end(args);                                                              \
                                                                               \
    write(TYPE_CIM, level, text);                                              \
    g_free(text);                                                              \
  }

WRITE_METHOD(error, LEVEL_ERROR)
WRITE_METHOD(critical, LEVEL_CRITICAL)
WRITE_METHOD(warning, LEVEL_WARNING)
WRITE_METHOD(message, LEVEL_MESSAGE)
WRITE_METHOD(info, LEVEL_INFO)
WRITE_METHOD(debug, LEVEL_DEBUG)

#undef WRITE_METHOD

Log::LogWindow::LogWindow() : Window(0, 0, 80, 24, NULL, TYPE_NON_FOCUSABLE)
{
  setColorScheme("log");

  CppConsUI::HorizontalListBox *lbox =
    new CppConsUI::HorizontalListBox(AUTOSIZE, AUTOSIZE);
  addWidget(*lbox, 1, 1);

  lbox->appendWidget(*(new CppConsUI::Spacer(1, AUTOSIZE)));
  textview = new CppConsUI::TextView(AUTOSIZE, AUTOSIZE, true);
  lbox->appendWidget(*textview);
  lbox->appendWidget(*(new CppConsUI::Spacer(1, AUTOSIZE)));

  onScreenResized();
}

void Log::LogWindow::onScreenResized()
{
  moveResizeRect(CENTERIM->getScreenArea(CenterIM::LOG_AREA));
}

void Log::LogWindow::append(const char *text)
{
  textview->append(text);

  // shorten the window text
  size_t lines_num = textview->getLinesNumber();

  if (lines_num > 200) {
    // remove 40 extra lines
    textview->erase(0, lines_num - 200 + 40);
  }
}

Log::LogBufferItem::LogBufferItem(Type type_, Level level_, const char *text_)
  : type(type_), level(level_)
{
  text = g_strdup(text_);
}

Log::LogBufferItem::~LogBufferItem()
{
  g_free(text);
}

Log::Log()
  : log_window(NULL), phase2_active(false), logfile(NULL),
    log_level_cim(LEVEL_DEBUG), log_level_glib(LEVEL_DEBUG),
    log_level_purple(LEVEL_DEBUG)
{
#define REGISTER_G_LOG_HANDLER(name, handler)                                  \
  g_log_set_handler((name), (GLogLevelFlags)G_LOG_LEVEL_MASK, (handler), this)

  // register the glib log handlers
  default_handler = REGISTER_G_LOG_HANDLER(NULL, default_log_handler_);
  glib_handler = REGISTER_G_LOG_HANDLER("GLib", glib_log_handler_);
  gmodule_handler = REGISTER_G_LOG_HANDLER("GModule", glib_log_handler_);
  glib_gobject_handler =
    REGISTER_G_LOG_HANDLER("GLib-GObject", glib_log_handler_);
  gthread_handler = REGISTER_G_LOG_HANDLER("GThread", glib_log_handler_);
}

Log::~Log()
{
  g_log_remove_handler(NULL, default_handler);
  g_log_remove_handler("GLib", glib_handler);
  g_log_remove_handler("GModule", gmodule_handler);
  g_log_remove_handler("GLib-GObject", glib_gobject_handler);
  g_log_remove_handler("GThread", gthread_handler);

  outputBufferMessages();
}

void Log::init()
{
  g_assert(!my_instance);

  my_instance = new Log;
}

void Log::finalize()
{
  g_assert(my_instance);

  delete my_instance;
  my_instance = NULL;
}

void Log::initPhase2()
{
  /* Phase 2 is called after CppConsUI and libpurple has been initialized.
   * Logging to file is part of the phase 2. */
  g_assert(!phase2_active);

  // init prefs
  purple_prefs_add_none(CONF_PREFIX "/log");
  purple_prefs_add_bool(CONF_PREFIX "/log/debug", false);
  purple_prefs_add_string(CONF_PREFIX "/log/filename", "debug.log");
  purple_prefs_add_string(CONF_PREFIX "/log/log_level_cim", "info");
  purple_prefs_add_string(CONF_PREFIX "/log/log_level_purple", "critical");
  purple_prefs_add_string(CONF_PREFIX "/log/log_level_glib", "warning");

  updateCachedPreference(CONF_PREFIX "/log/debug");
  updateCachedPreference(CONF_PREFIX "/log/log_level_cim");
  updateCachedPreference(CONF_PREFIX "/log/log_level_purple");
  updateCachedPreference(CONF_PREFIX "/log/log_level_glib");

  // connect callbacks
  purple_prefs_connect_callback(
    this, CONF_PREFIX "/log", log_pref_change_, this);

  // create the log window
  g_assert(!log_window);
  log_window = new LogWindow;
  log_window->show();

  // phase 2 is now active
  phase2_active = true;

  // output buffered messages
  for (LogBufferItems::iterator i = log_items.begin(); i != log_items.end();
       i++) {
    LogBufferItem *item = *i;

    // determine if this message should be displayed
    Level loglevel = getLogLevel(item->getType());

    if (loglevel >= item->getLevel())
      write(item->getType(), item->getLevel(), item->getText());

    delete item;
  }
  log_items.clear();
}

void Log::finalizePhase2()
{
  /* Phase 2 finalization is done before CppConsUI and libpurple is finalized.
   * Note that log levels are unchanged after phase 2 finishes. */
  g_assert(phase2_active);

  // delete the log window
  g_assert(log_window);
  delete log_window;
  log_window = NULL;

  purple_prefs_disconnect_by_handle(this);

  // close the log file (if it is opened)
  if (logfile)
    g_io_channel_unref(logfile);

  // done with phase 2
  phase2_active = false;
}

void Log::purple_print(
  PurpleDebugLevel purplelevel, const char *category, const char *arg_s)
{
  Level level = convertPurpleDebugLevel(purplelevel);
  if (log_level_purple < level)
    return; // we don't want to see this log message

  if (!category) {
    category = "misc";
    warning(_("centerim/log: purple_print() parameter category was "
              "not defined."));
  }

  char *text = g_strdup_printf("libpurple/%s: %s", category, arg_s);
  write(TYPE_PURPLE, level, text);
  g_free(text);
}

gboolean Log::purple_is_enabled(
  PurpleDebugLevel purplelevel, const char * /*category*/)
{
  Level level = convertPurpleDebugLevel(purplelevel);

  if (log_level_purple < level)
    return FALSE;

  return TRUE;
}

void Log::default_log_handler(
  const char *domain, GLogLevelFlags flags, const char *msg)
{
  if (!msg)
    return;

  Level level = convertGLibDebugLevel(flags);
  if (log_level_glib < level)
    return; // we don't want to see this log message

  char *text = g_strdup_printf("%s: %s", domain ? domain : "g_log", msg);
  write(TYPE_GLIB, level, text);
  g_free(text);
}

void Log::glib_log_handler(
  const char *domain, GLogLevelFlags flags, const char *msg)
{
  if (!msg)
    return;

  Level level = convertGLibDebugLevel(flags);
  if (log_level_glib < level)
    return; // we don't want to see this log message

  char *text = g_strdup_printf("%s: %s", domain ? domain : "g_log", msg);
  write(TYPE_GLIB, level, text);
  g_free(text);
}

void Log::log_pref_change(
  const char *name, PurplePrefType /*type*/, gconstpointer /*val*/)
{
  // log/* preference changed
  updateCachedPreference(name);
}

void Log::updateCachedPreference(const char *name)
{
  if (!strcmp(name, CONF_PREFIX "/log/debug")) {
    bool logfile_enabled = purple_prefs_get_bool(name);

    if (logfile_enabled && !logfile) {
      char *filename = g_build_filename(purple_user_dir(),
        purple_prefs_get_string(CONF_PREFIX "/log/filename"), NULL);
      GError *err = NULL;

      if (!(logfile = g_io_channel_new_file(filename, "a", &err))) {
        error(_("centerim/log: Error opening logfile '%s' (%s)."), filename,
          err->message);
        g_clear_error(&err);
      }
      g_free(filename);
    }
    else if (!logfile_enabled && logfile) {
      // debug was disabled so close logfile if it's opened
      g_io_channel_unref(logfile);
      logfile = NULL;
    }
  }
  else if (!strcmp(name, CONF_PREFIX "/log/log_level_cim"))
    log_level_cim = stringToLevel(purple_prefs_get_string(name));
  else if (!strcmp(name, CONF_PREFIX "/log/log_level_purple"))
    log_level_purple = stringToLevel(purple_prefs_get_string(name));
  else if (!strcmp(name, CONF_PREFIX "/log/log_level_glib"))
    log_level_glib = stringToLevel(purple_prefs_get_string(name));
}

void Log::write(Type type, Level level, const char *text)
{
  // if phase 2 is not active buffer the message
  if (!phase2_active) {
    LogBufferItem *item = new LogBufferItem(type, level, text);
    log_items.push_back(item);
    return;
  }

  g_assert(log_window);
  log_window->append(text);
  writeToFile(text);
}

void Log::writeErrorToWindow(const char *fmt, ...)
{
  // can be called only if phase 2 is active
  g_assert(phase2_active);
  g_assert(log_window);

  va_list args;

  if (log_level_cim < LEVEL_ERROR)
    return; // we don't want to see this log message

  va_start(args, fmt);
  char *text = g_strdup_vprintf(fmt, args);
  va_end(args);

  log_window->append(text);
  g_free(text);
}

void Log::writeToFile(const char *text)
{
  // writing to file is enabled only in phase 2
  if (!phase2_active)
    return;

  if (!text)
    return;

  GError *err = NULL;

  // write text into logfile
  if (logfile) {
    if (g_io_channel_write_chars(logfile, text, -1, NULL, &err) !=
      G_IO_STATUS_NORMAL) {
      writeErrorToWindow(
        _("centerim/log: Error writing to logfile (%s)."), err->message);
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
      writeErrorToWindow(
        _("centerim/log: Error flushing logfile (%s)."), err->message);
      g_clear_error(&err);
    }
  }
}

void Log::outputBufferMessages()
{
  // output all buffered messages to stderr
  for (LogBufferItems::iterator i = log_items.begin(); i != log_items.end();
       i++) {
    LogBufferItem *item = *i;

    // determine if this message should be displayed
    Level loglevel = getLogLevel(item->getType());

    if (loglevel >= item->getLevel()) {
      const char *text = item->getText();
      g_assert(text);

      std::fprintf(stderr, text);

      // if necessary write missing EOL character
      if (text[strlen(text) - 1] != '\n')
        std::fprintf(stderr, "\n");
    }

    delete item;
  }
  log_items.clear();
  std::fflush(stderr);
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

  warning(
    _("centerim/log: Unknown libpurple logging level '%d'."), purplelevel);
  return LEVEL_DEBUG;
}

Log::Level Log::convertGLibDebugLevel(GLogLevelFlags gliblevel)
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

  warning(_("centerim/log: Unknown GLib logging level '%d'."), gliblevel);
  return LEVEL_DEBUG;
}

Log::Level Log::stringToLevel(const char *slevel)
{
  if (!g_ascii_strcasecmp(slevel, "none"))
    return LEVEL_NONE;
  else if (!g_ascii_strcasecmp(slevel, "debug"))
    return LEVEL_DEBUG;
  else if (!g_ascii_strcasecmp(slevel, "info"))
    return LEVEL_INFO;
  else if (!g_ascii_strcasecmp(slevel, "message"))
    return LEVEL_MESSAGE;
  else if (!g_ascii_strcasecmp(slevel, "warning"))
    return LEVEL_WARNING;
  else if (!g_ascii_strcasecmp(slevel, "critical"))
    return LEVEL_CRITICAL;
  else if (!g_ascii_strcasecmp(slevel, "error"))
    return LEVEL_ERROR;
  return LEVEL_NONE;
}

Log::Level Log::getLogLevel(Type type)
{
  switch (type) {
  case TYPE_CIM:
    return log_level_cim;
  case TYPE_GLIB:
    return log_level_glib;
  case TYPE_PURPLE:
    return log_level_purple;
  default:
    g_assert_not_reached();
  }
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
