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

#ifndef LOG_H
#define LOG_H

#include "CenterIM.h"

#include <cppconsui/TextView.h>
#include <cppconsui/Window.h>
#include <deque>
#include <libpurple/purple.h>
#include <string>

#define LOG (Log::instance())

class Log {
public:
  // Levels are 1:1 mapped to glib levels.
  enum Level {
    LEVEL_NONE,     //
    LEVEL_ERROR,    // = Fatal in libpurple.
    LEVEL_CRITICAL, // = Error in libpurple.
    LEVEL_WARNING,  //
    LEVEL_MESSAGE,  // No such level in libpurple.
    LEVEL_INFO,     //
    LEVEL_DEBUG,    // = Misc in libpurple.
  };

  static Log *instance();

  void error(const char *fmt, ...)
    CPPCONSUI_GNUC_ATTRIBUTE((format(printf, 2, 3)));
  void critical(const char *fmt, ...)
    CPPCONSUI_GNUC_ATTRIBUTE((format(printf, 2, 3)));
  void warning(const char *fmt, ...)
    CPPCONSUI_GNUC_ATTRIBUTE((format(printf, 2, 3)));
  void message(const char *fmt, ...)
    CPPCONSUI_GNUC_ATTRIBUTE((format(printf, 2, 3)));
  void info(const char *fmt, ...)
    CPPCONSUI_GNUC_ATTRIBUTE((format(printf, 2, 3)));
  void debug(const char *fmt, ...)
    CPPCONSUI_GNUC_ATTRIBUTE((format(printf, 2, 3)));

  void logv( enum Level level, const char *fmt, va_list args );

  void clearAllBufferedMessages();

private:
  enum Type {
    TYPE_CIM,
    TYPE_GLIB,
    TYPE_PURPLE,
  };

  enum Phase {
    PHASE_INITIALIZATION,
    PHASE_NORMAL,
    PHASE_FINALIZATION,
  };

  class LogWindow : public CppConsUI::Window {
  public:
    LogWindow();
    virtual ~LogWindow() override {}

    // FreeWindow
    virtual void onScreenResized() override;

    void append(const char *text);

  protected:
    CppConsUI::TextView *textview_;

  private:
    CONSUI_DISABLE_COPY(LogWindow);
  };

  class LogBufferItem {
  public:
    LogBufferItem(Type type, Level level, const char *text);
    ~LogBufferItem();

    Type getType() const { return type_; }
    Level getLevel() const { return level_; }
    const char *getText() const { return text_; }

  protected:
    Type type_;
    Level level_;
    char *text_;

  private:
    CONSUI_DISABLE_COPY(LogBufferItem);
  };

  typedef std::deque<LogBufferItem *> LogBufferItems;
  LogBufferItems init_log_items_;
  LogBufferItems log_items_;

  guint default_handler_;
  guint glib_handler_;
  guint gmodule_handler_;
  guint glib_gobject_handler_;
  guint gthread_handler_;
  guint cppconsui_handler_;

  static Log *my_instance_;

  Phase phase_;
  LogWindow *log_window_;
  GIOChannel *logfile_;

  Level log_level_cim_;
  Level log_level_glib_;
  Level log_level_purple_;

  Log();
  ~Log();
  CONSUI_DISABLE_COPY(Log);

  static void init();
  static void finalize();
  void initNormalPhase();
  void finalizeNormalPhase();
  friend class CenterIM;

  // To catch libpurple's debug messages.
  void purple_print(
    PurpleDebugLevel level, const char *category, const char *arg_s);
  gboolean purple_is_enabled(PurpleDebugLevel level, const char *category);

  // To catch default messages.
  static void default_log_handler_(const char *domain, GLogLevelFlags flags,
    const char *msg, gpointer user_data)
  {
    reinterpret_cast<Log *>(user_data)->default_log_handler(domain, flags, msg);
  }
  void default_log_handler(
    const char *domain, GLogLevelFlags flags, const char *msg);

  // To catch glib's messages.
  static void glib_log_handler_(const char *domain, GLogLevelFlags flags,
    const char *msg, gpointer user_data)
  {
    reinterpret_cast<Log *>(user_data)->glib_log_handler(domain, flags, msg);
  }
  void glib_log_handler(
    const char *domain, GLogLevelFlags flags, const char *msg);

  // Called when log preferences change.
  static void log_pref_change_(
    const char *name, PurplePrefType type, gconstpointer val, gpointer data)
  {
    reinterpret_cast<Log *>(data)->log_pref_change(name, type, val);
  }
  void log_pref_change(
    const char *name, PurplePrefType type, gconstpointer val);

  void updateCachedPreference(const char *name);
  void write(Type type, Level level, const char *text, bool buffer = true);
  void writeErrorToWindow(const char *fmt, ...);
  void writeToFile(const char *text);
  void bufferMessage(Type type, Level level, const char *text);
  void clearBufferedMessages(LogBufferItems &items);
  void outputBufferedMessages(LogBufferItems &items);
  void outputAllBufferedMessages();
  Level convertPurpleDebugLevel(PurpleDebugLevel purplelevel);
  Level convertGLibDebugLevel(GLogLevelFlags gliblevel);
  Level stringToLevel(const char *slevel);
  Level getLogLevel(Type type);
};

#endif // LOG_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
