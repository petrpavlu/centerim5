/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2009-2011 by CenterIM developers
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

/**
 * @file
 * CoreManager class
 *
 * @ingroup cppconsui
 */

#ifndef __COREMANAGER_H__
#define __COREMANAGER_H__

#include "InputProcessor.h"
#include "FreeWindow.h"

#include <glib.h>
#include <libtermkey/termkey.h>
#include <vector>

/**
 * This class implements a core part of CppConsUI.
 */

#define COREMANAGER (CoreManager::Instance())

class CoreManager
: public InputProcessor
{
public:
  static CoreManager *Instance();

  /**
   * Sets itself as a standard input watcher and runs glib main loop.
   */
  void StartMainLoop();
  /**
   * Quits glib main loop and stops watching the standard input.
   */
  void QuitMainLoop();

  void AddWindow(FreeWindow& window);
  void RemoveWindow(FreeWindow& window);
  bool HasWindow(const FreeWindow& window) const;
  FreeWindow *GetTopWindow();

  int GetScreenWidth() const { return screen_width; }
  int GetScreenHeight() const { return screen_height; }

  void EnableResizing();
  void DisableResizing();
  void ScreenResized();

  void SetTopInputProcessor(InputProcessor& top)
    { top_input_processor = &top; }
  InputProcessor *GetTopInputProcessor()
    { return top_input_processor; }

  void Redraw();

  sigc::connection TimeoutConnect(const sigc::slot<bool>& slot,
      unsigned interval, int priority = G_PRIORITY_DEFAULT);
  sigc::connection TimeoutOnceConnect(const sigc::slot<void>& slot,
      unsigned interval, int priority = G_PRIORITY_DEFAULT);

  static TermKey *GetTermKeyHandle() { return tk; };

  sigc::signal<void> signal_resize;

protected:

private:
  typedef std::vector<FreeWindow *> Windows;

  Windows windows;

  InputProcessor *top_input_processor;

  GIOChannel *io_input_channel;
  guint io_input_channel_id;
  sigc::connection io_input_timeout_conn;
  GIOChannel *resize_channel;
  guint resize_channel_id;
  int pipefd[2];
  bool pipe_valid;

  static TermKey *tk;
  bool utf8;

  GMainLoop *gmainloop;

  int screen_width, screen_height;

  bool redraw_pending;
  bool resize_pending;

  CoreManager();
  CoreManager(const CoreManager&);
  CoreManager& operator=(const CoreManager&);
  ~CoreManager();

  // InputProcessor
  virtual bool ProcessInput(const TermKeyKey& key);

  // glib IO callbacks
  /**
   * Handles standard input IO errors (logs an error) and quits the
   * application. This function is a glib main loop callback for the standard
   * input watcher.
   */
  static gboolean io_input_error_(GIOChannel *source, GIOCondition cond,
      gpointer data)
    { return reinterpret_cast<CoreManager *>(data)->io_input_error(source,
        cond); }
  gboolean io_input_error(GIOChannel *source, GIOCondition cond);
  /**
   * Reads data from the standard input. The data are at first converted from
   * user locales to an internal representation (UTF-8) and then processed by
   * InputProcessor.
   */
  static gboolean io_input_(GIOChannel *source, GIOCondition cond,
      gpointer data)
    { return reinterpret_cast<CoreManager *>(data)->io_input(source, cond); }
  gboolean io_input(GIOChannel *source, GIOCondition cond);
  void io_input_timeout();

  static gboolean resize_input_(GIOChannel *source, GIOCondition cond,
      gpointer data)
    { return reinterpret_cast<CoreManager *>(data)->resize_input(source,
        cond); }
  gboolean resize_input(GIOChannel *source, GIOCondition cond);

  void InputInit();
  void InputUnInit();

  static void SignalHandler(int signum);
  void Resize();

  void Draw();

  Windows::iterator FindWindow(FreeWindow& window);
  void FocusWindow();

  void DeclareBindables();
};

#endif // __APPLICATION_H__
