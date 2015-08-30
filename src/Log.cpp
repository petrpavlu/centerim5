// Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
// Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
//
// This file is part of CenterIM.
//
// CenterIM is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// CenterIM is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "Log.h"

#include <cppconsui/HorizontalListBox.h>
#include <cppconsui/Spacer.h>
#include <cstdio>
#include <cstring>
#include "gettext.h"

Log *Log::my_instance_ = NULL;

Log *Log::instance()
{
  return my_instance_;
}

#define WRITE_METHOD(name, level)                                              \
  void Log::name(const char *fmt, ...)                                         \
  {                                                                            \
    va_list args;                                                              \
    char *text;                                                                \
                                                                               \
    if (log_level_cim_ < level)                                                \
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
  setColorScheme(CenterIM::SCHEME_LOG);

  CppConsUI::HorizontalListBox *lbox =
    new CppConsUI::HorizontalListBox(AUTOSIZE, AUTOSIZE);
  addWidget(*lbox, 1, 1);

  lbox->appendWidget(*(new CppConsUI::Spacer(1, AUTOSIZE)));
  textview_ = new CppConsUI::TextView(AUTOSIZE, AUTOSIZE, true);
  lbox->appendWidget(*textview_);
  lbox->appendWidget(*(new CppConsUI::Spacer(1, AUTOSIZE)));

  onScreenResized();
}

void Log::LogWindow::onScreenResized()
{
  moveResizeRect(CENTERIM->getScreenArea(CenterIM::LOG_AREA));
}

void Log::LogWindow::append(const char *text)
{
  textview_->append(text);

  // Shorten the window text.
  size_t lines_num = textview_->getLinesNumber();

  if (lines_num > 200) {
    // Remove 40 extra lines.
    textview_->erase(0, lines_num - 200 + 40);
  }
}

Log::LogBufferItem::LogBufferItem(Type type, Level level, const char *text)
  : type_(type), level_(level)
{
  text_ = g_strdup(text);
}

Log::LogBufferItem::~LogBufferItem()
{
  g_free(text_);
}

Log::Log()
  : log_window_(NULL), phase2_active_(false), logfile_(NULL),
    log_level_cim_(LEVEL_DEBUG), log_level_glib_(LEVEL_DEBUG),
    log_level_purple_(LEVEL_DEBUG)
{
#define REGISTER_G_LOG_HANDLER(name, handler)                                  \
  g_log_set_handler((name), (GLogLevelFlags)G_LOG_LEVEL_MASK, (handler), this)

  // Register the glib log handlers.
  default_handler_ = REGISTER_G_LOG_HANDLER(NULL, default_log_handler_);
  glib_handler_ = REGISTER_G_LOG_HANDLER("GLib", glib_log_handler_);
  gmodule_handler_ = REGISTER_G_LOG_HANDLER("GModule", glib_log_handler_);
  glib_gobject_handler_ =
    REGISTER_G_LOG_HANDLER("GLib-GObject", glib_log_handler_);
  gthread_handler_ = REGISTER_G_LOG_HANDLER("GThread", glib_log_handler_);
}

Log::~Log()
{
  g_log_remove_handler(NULL, default_handler_);
  g_log_remove_handler("GLib", glib_handler_);
  g_log_remove_handler("GModule", gmodule_handler_);
  g_log_remove_handler("GLib-GObject", glib_gobject_handler_);
  g_log_remove_handler("GThread", gthread_handler_);

  outputBufferMessages();
}

void Log::init()
{
  g_assert(my_instance_ == NULL);

  my_instance_ = new Log;
}

void Log::finalize()
{
  g_assert(my_instance_ != NULL);

  delete my_instance_;
  my_instance_ = NULL;
}

void Log::initPhase2()
{
  // Phase 2 is called after CppConsUI and libpurple has been initialized.
  // Logging to file is part of the phase 2.
  g_assert(!phase2_active_);

  // Init preferences.
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

  // Connect callbacks.
  purple_prefs_connect_callback(
    this, CONF_PREFIX "/log", log_pref_change_, this);

  // Create the log window.
  g_assert(log_window_ == NULL);
  log_window_ = new LogWindow;
  log_window_->show();

  // Phase 2 is now active.
  phase2_active_ = true;

  // Output buffered messages.
  for (LogBufferItems::iterator i = log_items_.begin(); i != log_items_.end();
       ++i) {
    LogBufferItem *item = *i;

    // Determine if this message should be displayed.
    Level loglevel = getLogLevel(item->getType());

    if (loglevel >= item->getLevel())
      write(item->getType(), item->getLevel(), item->getText());

    delete item;
  }
  log_items_.clear();
}

void Log::finalizePhase2()
{
  // Phase 2 finalization is done before CppConsUI and libpurple is finalized.
  // Note that log levels are unchanged after phase 2 finishes.
  g_assert(phase2_active_);

  // Delete the log window.
  g_assert(log_window_ != NULL);
  delete log_window_;
  log_window_ = NULL;

  purple_prefs_disconnect_by_handle(this);

  // Close the log file (if it is opened).
  if (logfile_ != NULL)
    g_io_channel_unref(logfile_);

  // Done with phase 2.
  phase2_active_ = false;
}

void Log::purple_print(
  PurpleDebugLevel purplelevel, const char *category, const char *arg_s)
{
  Level level = convertPurpleDebugLevel(purplelevel);
  if (log_level_purple_ < level)
    return; // We do not want to see this log message.

  if (category == NULL) {
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

  if (log_level_purple_ < level)
    return FALSE;

  return TRUE;
}

void Log::default_log_handler(
  const char *domain, GLogLevelFlags flags, const char *msg)
{
  if (msg == NULL)
    return;

  Level level = convertGLibDebugLevel(flags);
  if (log_level_glib_ < level)
    return; // We do not want to see this log message.

  char *text = g_strdup_printf("%s: %s", domain ? domain : "g_log", msg);
  write(TYPE_GLIB, level, text);
  g_free(text);
}

void Log::glib_log_handler(
  const char *domain, GLogLevelFlags flags, const char *msg)
{
  if (msg == NULL)
    return;

  Level level = convertGLibDebugLevel(flags);
  if (log_level_glib_ < level)
    return; // We do not want to see this log message.

  char *text = g_strdup_printf("%s: %s", domain ? domain : "g_log", msg);
  write(TYPE_GLIB, level, text);
  g_free(text);
}

void Log::log_pref_change(
  const char *name, PurplePrefType /*type*/, gconstpointer /*val*/)
{
  // log/* preference changed.
  updateCachedPreference(name);
}

void Log::updateCachedPreference(const char *name)
{
  if (std::strcmp(name, CONF_PREFIX "/log/debug") == 0) {
    bool logfile_enabled = purple_prefs_get_bool(name);

    if (logfile_enabled && logfile_ == NULL) {
      char *filename = g_build_filename(purple_user_dir(),
        purple_prefs_get_string(CONF_PREFIX "/log/filename"), NULL);
      GError *err = NULL;

      logfile_ = g_io_channel_new_file(filename, "a", &err);
      if (logfile_ == NULL) {
        error(_("centerim/log: Error opening logfile '%s' (%s)."), filename,
          err->message);
        g_clear_error(&err);
      }
      g_free(filename);
    }
    else if (!logfile_enabled && logfile_ != NULL) {
      // Debug was disabled so close logfile if it is opened.
      g_io_channel_unref(logfile_);
      logfile_ = NULL;
    }
  }
  else if (std::strcmp(name, CONF_PREFIX "/log/log_level_cim") == 0)
    log_level_cim_ = stringToLevel(purple_prefs_get_string(name));
  else if (std::strcmp(name, CONF_PREFIX "/log/log_level_purple") == 0)
    log_level_purple_ = stringToLevel(purple_prefs_get_string(name));
  else if (std::strcmp(name, CONF_PREFIX "/log/log_level_glib") == 0)
    log_level_glib_ = stringToLevel(purple_prefs_get_string(name));
}

void Log::write(Type type, Level level, const char *text)
{
  // If phase 2 is not active buffer the message.
  if (!phase2_active_) {
    LogBufferItem *item = new LogBufferItem(type, level, text);
    log_items_.push_back(item);
    return;
  }

  g_assert(log_window_ != NULL);
  log_window_->append(text);
  writeToFile(text);
}

void Log::writeErrorToWindow(const char *fmt, ...)
{
  // Can be called only if phase 2 is active.
  g_assert(phase2_active_);
  g_assert(log_window_ != NULL);

  va_list args;

  if (log_level_cim_ < LEVEL_ERROR)
    return; // We do not want to see this log message.

  va_start(args, fmt);
  char *text = g_strdup_vprintf(fmt, args);
  va_end(args);

  log_window_->append(text);
  g_free(text);
}

void Log::writeToFile(const char *text)
{
  // Writing to file is enabled only in phase 2.
  if (!phase2_active_)
    return;

  if (text == NULL)
    return;

  GError *err = NULL;

  // Write text into logfile.
  if (logfile_ != NULL) {
    if (g_io_channel_write_chars(logfile_, text, -1, NULL, &err) !=
      G_IO_STATUS_NORMAL) {
      writeErrorToWindow(
        _("centerim/log: Error writing to logfile (%s)."), err->message);
      g_clear_error(&err);
    }
    else {
      // If necessary write missing EOL character.
      size_t len = std::strlen(text);
      if (len > 0 && text[len - 1] != '\n') {
        // Ignore all errors.
        g_io_channel_write_chars(logfile_, "\n", -1, NULL, NULL);
      }
    }

    if (g_io_channel_flush(logfile_, &err) != G_IO_STATUS_NORMAL) {
      writeErrorToWindow(
        _("centerim/log: Error flushing logfile (%s)."), err->message);
      g_clear_error(&err);
    }
  }
}

void Log::outputBufferMessages()
{
  // Output all buffered messages to stderr.
  for (LogBufferItems::iterator i = log_items_.begin(); i != log_items_.end();
       ++i) {
    LogBufferItem *item = *i;

    // Determine if this message should be displayed.
    Level loglevel = getLogLevel(item->getType());

    if (loglevel >= item->getLevel()) {
      const char *text = item->getText();
      g_assert(text != NULL);

      std::fprintf(stderr, text);

      // If necessary write missing EOL character.
      size_t len = std::strlen(text);
      if (len > 0 && text[len - 1] != '\n')
        std::fprintf(stderr, "\n");
    }

    delete item;
  }
  log_items_.clear();
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
    return LEVEL_ERROR; // Use error level so this message is always printed.
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
  if (std::strcmp(slevel, "none") == 0)
    return LEVEL_NONE;
  else if (std::strcmp(slevel, "debug") == 0)
    return LEVEL_DEBUG;
  else if (std::strcmp(slevel, "info") == 0)
    return LEVEL_INFO;
  else if (std::strcmp(slevel, "message") == 0)
    return LEVEL_MESSAGE;
  else if (std::strcmp(slevel, "warning") == 0)
    return LEVEL_WARNING;
  else if (std::strcmp(slevel, "critical") == 0)
    return LEVEL_CRITICAL;
  else if (std::strcmp(slevel, "error") == 0)
    return LEVEL_ERROR;
  return LEVEL_NONE;
}

Log::Level Log::getLogLevel(Type type)
{
  switch (type) {
  case TYPE_CIM:
    return log_level_cim_;
  case TYPE_GLIB:
    return log_level_glib_;
  case TYPE_PURPLE:
    return log_level_purple_;
  default:
    g_assert_not_reached();
  }
}

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
