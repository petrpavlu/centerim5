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
// along with CenterIM.  If not, see <http://www.gnu.org/licenses/>.

// Logging facility collects all messages from the program itself and libraries
// (mainly GLib and libpurple). It filters them by severity and then outputs
// them to the Log window, a file or on the standard error output (stderr).
//
// Processing consists of three phases: initialization -> normal phase ->
// finalization, and is tightly tied to the CenterIM class. The program can exit
// in any of these phases.
//
// 1) Initialization: All messages are being buffered. If an error occurs
//    during this phase, the recorded messages are printed on stderr before the
//    program exits.
// 2) Normal phase: When processing reaches this phase (implying no critical
//    error occurred during the initialization phase), messages recorded by
//    the initialization phase are output to the Log window and optionally to a
//    file. New messages coming from the normal processing of the program are
//    then output in the same way. A limited number of the most recent messages
//    is also being buffered by this phase. If the program exists from the main
//    loop normally, the recorded messages are cleared.
// 3) Finalization: All messages are being buffered and when the program is
//    about to exit, they will get printed on stderr together with any buffered
//    messages from the previous stage.
//
// This mechanism ensures that if a non-recoverable error occurs during
// execution of the program and it must exit abnormally, the error will always
// get printed on stderr.

#include "Log.h"

#include "gettext.h"
#include <cppconsui/HorizontalListBox.h>
#include <cppconsui/Spacer.h>
#include <cstdio>
#include <cstring>

// Maximum number of lines in the Log window.
#define LOG_WINDOW_MAX_LINES 200
// Number of deleted lines when the line limit in the Log window is reached.
#define LOG_WINDOW_DELETE_COUNT 40

// Maximum number of buffered messages in the normal phase.
#define LOG_MAX_BUFFERED_MESSAGES 200

Log *Log::my_instance_ = nullptr;

Log *Log::instance()
{
  return my_instance_;
}

#define WRITE_METHOD(name, level)                                              \
  void Log::name(const char *fmt, ...)                                         \
  {                                                                            \
    va_list args;                                                              \
                                                                               \
    if (log_level_cim_ < level)                                                \
      return; /* Do not show this message. */                                  \
                                                                               \
    va_start(args, fmt);                                                       \
    logv(level, fmt, args);                                                    \
    va_end(args);                                                              \
  }

WRITE_METHOD(error, LEVEL_ERROR)
WRITE_METHOD(critical, LEVEL_CRITICAL)
WRITE_METHOD(warning, LEVEL_WARNING)
WRITE_METHOD(message, LEVEL_MESSAGE)
WRITE_METHOD(info, LEVEL_INFO)
WRITE_METHOD(debug, LEVEL_DEBUG)

#undef WRITE_METHOD

static const char *now_formatted( void )
{
    time_t now;
    struct tm nowtm;
    const char *formatted_time;

    now = time( nullptr );
    if( now != 0 && localtime_r(&now, &nowtm) != nullptr )
      formatted_time = purple_date_format_long(&nowtm);
    else
      formatted_time = _("Unknown");

    return formatted_time;
}

void Log::logv( enum Level level, const char *fmt, va_list args )
{
    char *text, *all;

    text = g_strdup_vprintf(fmt, args);
    all = g_strconcat( now_formatted(), " ", text, NULL );

    write(TYPE_CIM, level, all);
    g_free(text);
    g_free(all);
}

void Log::clearAllBufferedMessages()
{
  // Delete all buffered messages.
  clearBufferedMessages(init_log_items_);
  clearBufferedMessages(log_items_);
}

Log::LogWindow::LogWindow() : Window(0, 0, 80, 24, nullptr, TYPE_NON_FOCUSABLE)
{
  setColorScheme(CenterIM::SCHEME_LOG);

  auto lbox = new CppConsUI::HorizontalListBox(AUTOSIZE, AUTOSIZE);
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
  std::size_t lines_num = textview_->getLinesNumber();

  if (lines_num > LOG_WINDOW_MAX_LINES) {
    // Remove some lines.
    textview_->erase(
      0, lines_num - LOG_WINDOW_MAX_LINES + LOG_WINDOW_DELETE_COUNT);
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
  : phase_(PHASE_INITIALIZATION), log_window_(nullptr), logfile_(nullptr),
    log_level_cim_(LEVEL_DEBUG), log_level_glib_(LEVEL_DEBUG),
    log_level_purple_(LEVEL_DEBUG)
{
#define REGISTER_G_LOG_HANDLER(name, handler)                                  \
  g_log_set_handler((name), (GLogLevelFlags)G_LOG_LEVEL_MASK, (handler), this)

  // Register the glib log handlers.
  default_handler_ = REGISTER_G_LOG_HANDLER(nullptr, default_log_handler_);
  glib_handler_ = REGISTER_G_LOG_HANDLER("GLib", glib_log_handler_);
  gmodule_handler_ = REGISTER_G_LOG_HANDLER("GModule", glib_log_handler_);
  glib_gobject_handler_ =
    REGISTER_G_LOG_HANDLER("GLib-GObject", glib_log_handler_);
  gthread_handler_ = REGISTER_G_LOG_HANDLER("GThread", glib_log_handler_);
}

Log::~Log()
{
  g_log_remove_handler(nullptr, default_handler_);
  g_log_remove_handler("GLib", glib_handler_);
  g_log_remove_handler("GModule", gmodule_handler_);
  g_log_remove_handler("GLib-GObject", glib_gobject_handler_);
  g_log_remove_handler("GThread", gthread_handler_);

  outputAllBufferedMessages();
  clearAllBufferedMessages();
}

void Log::init()
{
  g_assert(my_instance_ == nullptr);

  my_instance_ = new Log;
}

void Log::finalize()
{
  g_assert(my_instance_ != nullptr);

  delete my_instance_;
  my_instance_ = nullptr;
}

void Log::initNormalPhase()
{
  // Normal phase is called after CppConsUI and libpurple has been initialized.
  // Logging to file is part of the normal phase.
  g_assert(phase_ == PHASE_INITIALIZATION);

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
  g_assert(log_window_ == nullptr);
  log_window_ = new LogWindow;
  log_window_->show();

  // Normal phase is now active.
  phase_ = PHASE_NORMAL;

  // Output buffered messages from the initialization phase.
  for (LogBufferItem *item : init_log_items_) {
    // Determine if this message should be displayed.
    Level loglevel = getLogLevel(item->getType());

    if (loglevel >= item->getLevel())
      write(item->getType(), item->getLevel(), item->getText(), false);

    delete item;
  }
  init_log_items_.clear();
}

void Log::finalizeNormalPhase()
{
  // Normal phase finalization is done before CppConsUI and libpurple is
  // finalized. Note that log levels are unchanged after the normal phase
  // finishes.
  g_assert(phase_ == PHASE_NORMAL);

  // Delete the log window.
  g_assert(log_window_ != nullptr);
  delete log_window_;
  log_window_ = nullptr;

  purple_prefs_disconnect_by_handle(this);

  // Close the log file (if it is opened).
  if (logfile_ != nullptr)
    g_io_channel_unref(logfile_);

  // Done with the normal phase.
  phase_ = PHASE_FINALIZATION;
}

void Log::purple_print(
  PurpleDebugLevel purplelevel, const char *category, const char *arg_s)
{
  Level level = convertPurpleDebugLevel(purplelevel);
  if (log_level_purple_ < level)
    return; // Do not show this message.

  if (category == nullptr) {
    category = "misc";
    warning(_("centerim/log: purple_print() parameter category was "
              "not defined."));
  }

  char *text = g_strdup_printf("%s libpurple/%s: %s",
      now_formatted(), category, arg_s);
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
  if (msg == nullptr)
    return;

  Level level = convertGLibDebugLevel(flags);
  if (log_level_glib_ < level)
    return; // Do not show this message.

  char *text = g_strdup_printf("%s %s: %s",
      now_formatted(), domain ? domain : "g_log", msg);
  write(TYPE_GLIB, level, text);
  g_free(text);
}

void Log::glib_log_handler(
  const char *domain, GLogLevelFlags flags, const char *msg)
{
  if (msg == nullptr)
    return;

  Level level = convertGLibDebugLevel(flags);
  if (log_level_glib_ < level)
    return; // Do not show this message.

  char *text = g_strdup_printf("%s %s: %s",
      now_formatted(), domain ? domain : "g_log", msg);
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

    if (logfile_enabled && logfile_ == nullptr) {
      char *filename = g_build_filename(purple_user_dir(),
        purple_prefs_get_string(CONF_PREFIX "/log/filename"), nullptr);
      GError *err = nullptr;

      logfile_ = g_io_channel_new_file(filename, "a", &err);
      if (logfile_ == nullptr) {
        error(_("centerim/log: Error opening logfile '%s' (%s)."), filename,
          err->message);
        g_clear_error(&err);
      }
      g_free(filename);
    }
    else if (!logfile_enabled && logfile_ != nullptr) {
      // Debug was disabled so close logfile if it is opened.
      g_io_channel_unref(logfile_);
      logfile_ = nullptr;
    }
  }
  else if (std::strcmp(name, CONF_PREFIX "/log/log_level_cim") == 0)
    log_level_cim_ = stringToLevel(purple_prefs_get_string(name));
  else if (std::strcmp(name, CONF_PREFIX "/log/log_level_purple") == 0)
    log_level_purple_ = stringToLevel(purple_prefs_get_string(name));
  else if (std::strcmp(name, CONF_PREFIX "/log/log_level_glib") == 0)
    log_level_glib_ = stringToLevel(purple_prefs_get_string(name));
}

void Log::write(Type type, Level level, const char *text, bool buffer)
{
  if (buffer)
    bufferMessage(type, level, text);

  // If the normal phase is not active then only buffer the message.
  if (phase_ != PHASE_NORMAL)
    return;

  g_assert(log_window_ != nullptr);
  log_window_->append(text);
  writeToFile(text);
}

void Log::writeErrorToWindow(const char *fmt, ...)
{
  // Can be called only if the normal phase is active.
  g_assert(phase_ == PHASE_NORMAL);
  g_assert(log_window_ != nullptr);

  va_list args;

  if (log_level_cim_ < LEVEL_ERROR)
    return; // Do not show this message.

  va_start(args, fmt);
  char *text = g_strdup_vprintf(fmt, args);
  va_end(args);

  log_window_->append(text);
  g_free(text);
}

void Log::writeToFile(const char *text)
{
  // Writing to a file is possible only in the normal phase.
  g_assert(phase_ == PHASE_NORMAL);

  if (text == nullptr || logfile_ == nullptr)
    return;

  // Write text into logfile.
  GError *err = nullptr;

  if (g_io_channel_write_chars(logfile_, text, -1, nullptr, &err) !=
    G_IO_STATUS_NORMAL) {
    writeErrorToWindow(
      _("centerim/log: Error writing to logfile (%s)."), err->message);
    g_clear_error(&err);
  }
  else {
    // If necessary write missing EOL character.
    std::size_t len = std::strlen(text);
    if (len > 0 && text[len - 1] != '\n') {
      // Ignore all errors.
      g_io_channel_write_chars(logfile_, "\n", -1, nullptr, nullptr);
    }
  }

  if (g_io_channel_flush(logfile_, &err) != G_IO_STATUS_NORMAL) {
    writeErrorToWindow(
      _("centerim/log: Error flushing logfile (%s)."), err->message);
    g_clear_error(&err);
  }
}

void Log::bufferMessage(Type type, Level level, const char *text)
{
  auto item = new LogBufferItem(type, level, text);
  if (phase_ == PHASE_INITIALIZATION)
    init_log_items_.push_back(item);
  else
    log_items_.push_back(item);

  if (phase_ != PHASE_NORMAL)
    return;

  // Reduce a number of buffered messages if the normal phase is active.
  std::size_t size = log_items_.size();
  if (size <= LOG_MAX_BUFFERED_MESSAGES)
    return;

  // There can never be more than one message to remove.
  g_assert(size == LOG_MAX_BUFFERED_MESSAGES + 1);
  delete log_items_.front();
  log_items_.pop_front();
}

void Log::clearBufferedMessages(LogBufferItems &items)
{
  for (LogBufferItem *item : items)
    delete item;
  items.clear();
}

void Log::outputBufferedMessages(LogBufferItems &items)
{
  for (LogBufferItem *item : items) {
    // Determine if this message should be displayed.
    Level loglevel = getLogLevel(item->getType());

    if (loglevel >= item->getLevel()) {
      const char *text = item->getText();
      g_assert(text != nullptr);

      std::fprintf(stderr, "%s", text);

      // If necessary write missing EOL character.
      std::size_t len = std::strlen(text);
      if (len > 0 && text[len - 1] != '\n')
        std::fprintf(stderr, "\n");
    }
  }
}

void Log::outputAllBufferedMessages()
{
  // Output all buffered messages on stderr.
  outputBufferedMessages(init_log_items_);
  outputBufferedMessages(log_items_);

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
