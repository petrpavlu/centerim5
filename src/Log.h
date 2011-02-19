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

#ifndef __LOG_H__
#define __LOG_H__

#include <cppconsui/TextView.h>
#include <cppconsui/Window.h>
#include <libpurple/purple.h>
#include <vector>

#define LOG (Log::Instance())

class Log
: public Window
{
public:
  // levels are 1:1 mapped to glib levels
  enum Level {
    LEVEL_NONE,
    LEVEL_ERROR, // = fatal in libpurle
    LEVEL_CRITICAL, // = error in libpurple
    LEVEL_WARNING,
    LEVEL_MESSAGE, // no such level in libpurple
    LEVEL_INFO,
    LEVEL_DEBUG // = misc in libpurple
  };

  static Log *Instance();

  // FreeWindow
  virtual void MoveResize(int newx, int newy, int neww, int newh);
  virtual void ScreenResized();

  void Error(const char *fmt, ...);
  void Critical(const char *fmt, ...);
  void Warning(const char *fmt, ...);
  void Message(const char *fmt, ...);
  void Info(const char *fmt, ...);
  void Debug(const char *fmt, ...);

protected:

private:
  enum Type {
    TYPE_CIM,
    TYPE_GLIB,
    TYPE_PURPLE
  };

  PurpleDebugUiOps centerim_debug_ui_ops;

  GIOChannel *logfile;

  TextView *textview;

  guint default_handler;
  guint glib_handler;
  guint gmodule_handler;
  guint glib_gobject_handler;
  guint gthread_handler;
  guint cppconsui_handler;

  static Log *instance;

  Log();
  Log(const Log&);
  Log& operator=(const Log&);
  virtual ~Log();

  static void Init();
  static void Finalize();
  friend class CenterIM;

  // to catch libpurple's debug messages
  static void purple_print_(PurpleDebugLevel level, const char *category,
      const char *arg_s)
    { LOG->purple_print(level, category, arg_s); }
  static gboolean is_enabled_(PurpleDebugLevel level, const char *category)
    { return LOG->is_enabled(level, category); }

  void purple_print(PurpleDebugLevel level, const char *category,
      const char *arg_s);
  gboolean is_enabled(PurpleDebugLevel level, const char *category);

  // to catch default messages
  static void default_log_handler_(const char *domain, GLogLevelFlags flags,
      const char *msg, gpointer user_data)
    { reinterpret_cast<Log*>(user_data)->default_log_handler(domain, flags,
        msg); }
  void default_log_handler(const char *domain, GLogLevelFlags flags,
      const char *msg);

  // to catch glib's messages
  static void glib_log_handler_(const char *domain, GLogLevelFlags flags,
      const char *msg, gpointer user_data)
    { reinterpret_cast<Log*>(user_data)->glib_log_handler(domain, flags,
        msg); }
  void glib_log_handler(const char *domain, GLogLevelFlags flags,
      const char *msg);

  // to catch cppconsui messages
  static void cppconsui_log_handler_(const char *domain,
      GLogLevelFlags flags, const char *msg, gpointer user_data)
    { reinterpret_cast<Log*>(user_data)->cppconsui_log_handler(domain, flags,
        msg); }
  void cppconsui_log_handler(const char *domain, GLogLevelFlags flags,
      const char *msg);

  // called when log/debug pref changed
  static void debug_change_(const char *name, PurplePrefType type,
      gconstpointer val, gpointer data)
    { reinterpret_cast<Log *>(data)->debug_change(name, type, val); }
  void debug_change(const char *name, PurplePrefType type,
      gconstpointer val);

  void ShortenWindowText();
  void Write(const char *text);
  void WriteErrorToWindow(const char *fmt, ...);
  void WriteToFile(const char *text);
  Level ConvertPurpleDebugLevel(PurpleDebugLevel purplelevel);
  Level ConvertGlibDebugLevel(GLogLevelFlags gliblevel);
  Level GetLogLevel(const char *type);
  bool GetDebugEnabled();
};

#endif // __LOG_H__
