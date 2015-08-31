// Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
// Copyright (C) 2009-2015 Petr Pavlu <setup@dagobah.cz>
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

/// @file
/// CoreManager class
///
/// @ingroup cppconsui

#ifndef COREMANAGER_H
#define COREMANAGER_H

#include "CppConsUI.h"
#include "Window.h"

#include "libtermkey/termkey.h"
#include <iconv.h>
#include <deque>

namespace CppConsUI {

/// CppConsUI manager.
class CoreManager : public InputProcessor {
public:
  int initializeInput(Error &error);
  int finalizeInput(Error &error);

  int initializeOutput(Error &error);
  int finalizeOutput(Error &error);

  int initializeScreenResizing(Error &error);
  int finalizeScreenResizing(Error &error);

  void registerWindow(Window &window);
  void removeWindow(Window &window);
  void topWindow(Window &window);
  Window *getTopWindow();

  void enableResizing();
  void disableResizing();
  void onScreenResized();

  void setTopInputProcessor(InputProcessor &top)
  {
    top_input_processor_ = &top;
  }
  InputProcessor *getTopInputProcessor() { return top_input_processor_; }

  void logError(const char *message);
  void redraw();

  void onWindowMoveResize(
    Window &activator, const Rect &oldsize, const Rect &newsize);
  void onWindowWishSizeChange(
    Window &activator, const Size &oldsize, const Size &newsize);

  TermKey *getTermKeyHandle() { return tk_; };

  sigc::signal<void> signal_resize;
  sigc::signal<void> signal_top_window_change;

private:
  typedef std::deque<Window *> Windows;

  Windows windows_;
  AppInterface interface_;
  InputProcessor *top_input_processor_;

  unsigned stdin_input_timeout_handle_;
  unsigned stdin_input_handle_;
  unsigned resize_input_handle_;
  int pipefd_[2];

  TermKey *tk_;
  iconv_t iconv_desc_;

  bool redraw_pending_;
  bool resize_pending_;

  CoreManager(AppInterface &set_interface);
  ~CoreManager() {}
  CONSUI_DISABLE_COPY(CoreManager);

  friend void initializeConsUI(AppInterface &interface);
  friend void finalizeConsUI();

  // InputProcessor
  virtual bool processInput(const TermKeyKey &key);

  /// Reads data from the standard input. The data are first converted from user
  /// locales to the internal representation (UTF-8) and then processed by
  /// InputProcessor.
  static void stdin_input_(int fd, InputCondition cond, void *data)
  {
    static_cast<CoreManager *>(data)->stdin_input(fd, cond);
  }
  void stdin_input(int fd, InputCondition cond);
  static bool stdin_input_timeout_(void *data);
  void stdin_input_timeout();

  static void resize_input_(int fd, InputCondition cond, void *data)
  {
    static_cast<CoreManager *>(data)->resize_input(fd, cond);
  }
  void resize_input(int fd, InputCondition cond);

  static void signalHandler(int signum);
  void resize();
  void updateArea();
  void updateWindowArea(Window &window);

  static bool draw_(void *data);
  int draw(Error &error);
  int drawWindow(Window &window, Error &error);

  Windows::iterator findWindow(Window &window);
  void focusWindow();

  void redrawScreen();

  void declareBindables();
};

} // namespace CppConsUI

#endif // COREMANGER_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
