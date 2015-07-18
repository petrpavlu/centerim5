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

#ifndef __LOG_H__
#define __LOG_H__

#include "CenterIM.h"

#include <cppconsui/TextView.h>
#include <cppconsui/Window.h>
#include <libpurple/purple.h>
#include <string>

#define LOG (Log::instance())

class Log {
public:
  // levels are 1:1 mapped to glib levels
  enum Level {
    LEVEL_NONE,
    LEVEL_ERROR,    // = fatal in libpurle
    LEVEL_CRITICAL, // = error in libpurple
    LEVEL_WARNING,
    LEVEL_MESSAGE, // no such level in libpurple
    LEVEL_INFO,
    LEVEL_DEBUG, // = misc in libpurple
  };

  static Log *instance();

  void error(const char *fmt, ...) _attribute((format(printf, 2, 3)));
  void critical(const char *fmt, ...) _attribute((format(printf, 2, 3)));
  void warning(const char *fmt, ...) _attribute((format(printf, 2, 3)));
  void message(const char *fmt, ...) _attribute((format(printf, 2, 3)));
  void info(const char *fmt, ...) _attribute((format(printf, 2, 3)));
  void debug(const char *fmt, ...) _attribute((format(printf, 2, 3)));

protected:
private:
  enum Type {
    TYPE_CIM,
    TYPE_GLIB,
    TYPE_PURPLE,
  };

  class LogWindow : public CppConsUI::Window {
  public:
    LogWindow();

    // FreeWindow
    virtual void onScreenResized();

    void append(const char *text);

  protected:
    CppConsUI::TextView *textview;

  private:
    CONSUI_DISABLE_COPY(LogWindow);
  };

  class LogBufferItem {
  public:
    LogBufferItem(Type type_, Level level_, const char *text_);
    ~LogBufferItem();

    Type getType() const { return type; }
    Level getLevel() const { return level; }
    const char *getText() const { return text; }

  protected:
    Type type;
    Level level;
    char *text;

  private:
    CONSUI_DISABLE_COPY(LogBufferItem);
  };

  typedef std::vector<LogBufferItem *> LogBufferItems;
  LogBufferItems log_items;

  guint default_handler;
  guint glib_handler;
  guint gmodule_handler;
  guint glib_gobject_handler;
  guint gthread_handler;
  guint cppconsui_handler;

  static Log *my_instance;

  LogWindow *log_window;
  bool phase2_active;
  GIOChannel *logfile;

  Level log_level_cim;
  Level log_level_glib;
  Level log_level_purple;

  Log();
  virtual ~Log();
  CONSUI_DISABLE_COPY(Log);

  static void init();
  static void finalize();
  void initPhase2();
  void finalizePhase2();
  friend class CenterIM;

  // to catch libpurple's debug messages
  void purple_print(
    PurpleDebugLevel level, const char *category, const char *arg_s);
  gboolean purple_is_enabled(PurpleDebugLevel level, const char *category);

  // to catch default messages
  static void default_log_handler_(const char *domain, GLogLevelFlags flags,
    const char *msg, gpointer user_data)
  {
    reinterpret_cast<Log *>(user_data)->default_log_handler(domain, flags, msg);
  }
  void default_log_handler(
    const char *domain, GLogLevelFlags flags, const char *msg);

  // to catch glib's messages
  static void glib_log_handler_(const char *domain, GLogLevelFlags flags,
    const char *msg, gpointer user_data)
  {
    reinterpret_cast<Log *>(user_data)->glib_log_handler(domain, flags, msg);
  }
  void glib_log_handler(
    const char *domain, GLogLevelFlags flags, const char *msg);

  // called when a log pref is changed
  static void log_pref_change_(
    const char *name, PurplePrefType type, gconstpointer val, gpointer data)
  {
    reinterpret_cast<Log *>(data)->log_pref_change(name, type, val);
  }
  void log_pref_change(
    const char *name, PurplePrefType type, gconstpointer val);

  void updateCachedPreference(const char *name);
  void write(Type type, Level level, const char *text);
  void writeErrorToWindow(const char *fmt, ...);
  void writeToFile(const char *text);
  void outputBufferMessages();
  Level convertPurpleDebugLevel(PurpleDebugLevel purplelevel);
  Level convertGLibDebugLevel(GLogLevelFlags gliblevel);
  Level stringToLevel(const char *slevel);
  Level getLogLevel(Type type);
};

#endif // __LOG_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
