/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2009-2013 by CenterIM developers
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

/**
 * @file
 * CoreManager class
 *
 * @ingroup cppconsui
 */

#ifndef __COREMANAGER_H__
#define __COREMANAGER_H__

#include "FreeWindow.h"

#include "libtermkey/termkey.h"
#include <glib.h>
#include <vector>

namespace CppConsUI
{

int initializeConsUI();
int finalizeConsUI();

#define COREMANAGER (CppConsUI::CoreManager::instance())

/**
 * This class implements a core part of CppConsUI.
 */
class CoreManager
: public InputProcessor
{
public:
  static CoreManager *instance();

  /**
   * Sets itself as a standard input watcher and runs glib main loop.
   */
  void startMainLoop();
  /**
   * Quits glib main loop and stops watching the standard input.
   */
  void quitMainLoop();

  void addWindow(FreeWindow& window);
  void removeWindow(FreeWindow& window);
  bool hasWindow(const FreeWindow& window) const;
  FreeWindow *getTopWindow();

  void enableResizing();
  void disableResizing();
  void onScreenResized();

  void setTopInputProcessor(InputProcessor& top)
    { top_input_processor = &top; }
  InputProcessor *getTopInputProcessor()
    { return top_input_processor; }

  void redraw();

  sigc::connection timeoutConnect(const sigc::slot<bool>& slot,
      unsigned interval, int priority = G_PRIORITY_DEFAULT);
  sigc::connection timeoutOnceConnect(const sigc::slot<void>& slot,
      unsigned interval, int priority = G_PRIORITY_DEFAULT);

  TermKey *getTermKeyHandle() { return tk; };

  sigc::signal<void> signal_resize;
  sigc::signal<void> signal_top_window_change;

protected:

private:
  typedef std::vector<FreeWindow*> Windows;

  Windows windows;

  InputProcessor *top_input_processor;

  GIOChannel *io_input_channel;
  guint io_input_channel_id;
  sigc::connection io_input_timeout_conn;
  GIOChannel *resize_channel;
  guint resize_channel_id;
  int pipefd[2];
  bool pipe_valid;

  TermKey *tk;
  bool utf8;

  GMainLoop *gmainloop;

  bool redraw_pending;
  bool resize_pending;

  static CoreManager *my_instance;

  CoreManager();
  CoreManager(const CoreManager&);
  CoreManager& operator=(const CoreManager&);
  ~CoreManager();

  static int init();
  static int finalize();
  friend int initializeConsUI();
  friend int finalizeConsUI();

  // InputProcessor
  virtual bool processInput(const TermKeyKey& key);

  // glib IO callbacks
  /**
   * Handles standard input IO errors (logs an error) and quits the
   * application. This function is a glib main loop callback for the standard
   * input watcher.
   */
  static gboolean io_input_error_(GIOChannel *source, GIOCondition cond,
      gpointer data)
    { return reinterpret_cast<CoreManager*>(data)->io_input_error(source,
        cond); }
  gboolean io_input_error(GIOChannel *source, GIOCondition cond);
  /**
   * Reads data from the standard input. The data are at first converted from
   * user locales to an internal representation (UTF-8) and then processed by
   * InputProcessor.
   */
  static gboolean io_input_(GIOChannel *source, GIOCondition cond,
      gpointer data)
    { return reinterpret_cast<CoreManager*>(data)->io_input(source, cond); }
  gboolean io_input(GIOChannel *source, GIOCondition cond);
  void io_input_timeout();

  static gboolean resize_input_(GIOChannel *source, GIOCondition cond,
      gpointer data)
    { return reinterpret_cast<CoreManager*>(data)->resize_input(source,
        cond); }
  gboolean resize_input(GIOChannel *source, GIOCondition cond);

  void initInput();
  void finalizeInput();

  static void signalHandler(int signum);
  void resize();

  void draw();

  Windows::iterator findWindow(FreeWindow& window);
  void focusWindow();

  void redrawScreen();

  void declareBindables();
};

} // namespace CppConsUI

#endif // __COREMANGER_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
