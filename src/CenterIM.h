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

#ifndef __CENTERIM_H__
#define __CENTERIM_H__

#include <glib.h>

#include <libpurple/core.h>
#include <libpurple/debug.h>
#include <libpurple/eventloop.h>

#include <cppconsui/CoreManager.h>
#include <vector>

#define CONF_PREFIX "/centerim/"

#define CENTERIM (CenterIM::Instance())

class CenterIM
: public InputProcessor
{
public:
  enum ScreenArea {
    HEADER_AREA,
    BUDDY_LIST_AREA,
    LOG_AREA,
    CHAT_AREA,
    WHOLE_AREA,
    AREAS_NUM
  };

  static CenterIM *Instance();

  int Run();
  void Quit();

  // returns size of selected area
  Rect GetScreenAreaSize(ScreenArea area);

  Rect GetDimensions(const char *window, int defx, int defy, int defwidth,
      int defheight);

  void SetDimensions(const char *window, int x, int y, int width, int height);
  void SetDimensions(const char *window, const Rect& rect);

  Rect GetAccountWindowDimensions();

  static const char * const version;

protected:

private:
  CoreManager *mngr;
  sigc::connection resize;

  PurpleCoreUiOps centerim_core_ui_ops;
  PurpleDebugUiOps logbuf_debug_ui_ops;
  PurpleEventLoopUiOps centerim_glib_eventloops;

  struct IOClosure
  {
    PurpleInputFunction function;
    guint result;
    gpointer data;

    IOClosure() : function(NULL), result(0), data(NULL) {}
  };

  struct LogBufferItem
  {
    PurpleDebugLevel level;
    char *category;
    char *arg_s;
  };
  typedef std::vector<LogBufferItem> LogBufferItems;
  static LogBufferItems *logbuf;

  Rect areaSizes[AREAS_NUM];

  CenterIM();
  CenterIM(const CenterIM&);
  CenterIM& operator=(const CenterIM&);
  ~CenterIM() {}

  int PurpleInit();
  void PurpleFinalize();
  void ColorSchemeInit();

  // recalculates area sizes to fit into current screen size
  void ScreenResized();

  // PurpleCoreUiOps callbacks
  // returns information about CenterIM such as name, website etc.
  static GHashTable *get_ui_info();

  // PurpleEventLoopUiOps callbacks
  // adds timeout to glib main loop context
  static guint timeout_add(guint interval, GSourceFunc function, gpointer data);
  // removes timeout from glib main loop context
  static gboolean timeout_remove(guint handle);
  // adds IO watch to glib main loop context
  static guint input_add(int fd, PurpleInputCondition condition,
      PurpleInputFunction function, gpointer data);
  // removes input from glib main loop context
  static gboolean input_remove(guint handle);

  // helper function for input_add
  // process IO input to purple callback
  static gboolean purple_glib_io_input(GIOChannel *source,
      GIOCondition condition, gpointer data);
  // destroyes libpurple io input callback internal data
  static void purple_glib_io_destroy(gpointer data);

  // PurpleDebugUiOps callbacks
  // helper function to catch debug messages during libpurple initialization
  /* Catches and buffers libpurple debug messages until the Log object can be
   * instantiated. */
  static void tmp_purple_print(PurpleDebugLevel level, const char *category,
      const char *arg_s);
  static gboolean tmp_is_enabled(PurpleDebugLevel level, const char *category)
    { return TRUE; }

  void ActionFocusBuddyList();
  void ActionFocusActiveConversation();
  void ActionOpenAccountStatusMenu();
  void ActionOpenGeneralMenu();
  void ActionFocusPrevConversation();
  void ActionFocusNextConversation();

  /* Automatic registration of defined keys. */
  DECLARE_SIG_REGISTERKEYS();
  static bool RegisterKeys();
  void DeclareBindables();
};

#endif // __CENTERIM_H__
