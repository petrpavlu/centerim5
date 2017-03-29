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
// along with CenterIM.  If not, see <http://www.gnu.org/licenses/>.

/// @file
/// CoreManager class
///
/// @ingroup cppconsui

#ifndef COREMANAGER_H
#define COREMANAGER_H

#include "CppConsUI.h"
#include "Window.h"

#include "libtermkey/termkey.h"
#include <deque>
#include <iconv.h>

namespace CppConsUI {

/// CppConsUI manager.
class CoreManager : public InputProcessor {
public:
  int initializeInput(Error &error);
  int finalizeInput(Error &error);

  int initializeOutput(Error &error);
  int finalizeOutput(Error &error);

  /// Reads data from the standard input. The data are first converted from the
  /// user locale to the internal representation (UTF-8) and then processed by
  /// InputProcessor.
  int processStandardInput(int *wait, Error &error);
  int processStandardInputTimeout(Error &error);

  int resize(Error &error);
  int draw(Error &error);

  void registerWindow(Window &window);
  void removeWindow(Window &window);
  void hideWindow(Window &window);
  void topWindow(Window &window);
  Window *getTopWindow();

  void setTopInputProcessor(InputProcessor &top)
  {
    top_input_processor_ = &top;
  }
  InputProcessor *getTopInputProcessor() { return top_input_processor_; }

  void logDebug(const char *message);
  void redraw(bool from_scratch = false);

  bool isRedrawPending() const;

  void onScreenResized();

  void onWindowMoveResize(
    Window &activator, const Rect &oldsize, const Rect &newsize);
  void onWindowWishSizeChange(
    Window &activator, const Size &oldsize, const Size &newsize);

  TermKey *getTermKeyHandle() { return tk_; };

  sigc::signal<void> signal_resize;
  sigc::signal<void> signal_top_window_change;

private:
  typedef std::deque<Window *> Windows;

  enum PendingRedraw {
    REDRAW_NONE,
    REDRAW_NORMAL,
    REDRAW_FROM_SCRATCH,
  };

  Windows windows_;
  AppInterface interface_;
  InputProcessor *top_input_processor_;

  TermKey *tk_;
  iconv_t iconv_desc_;

  PendingRedraw pending_redraw_;

  CoreManager(AppInterface &set_interface);
  ~CoreManager() {}
  CONSUI_DISABLE_COPY(CoreManager);

  friend void initializeConsUI(AppInterface &interface);
  friend void finalizeConsUI();

  // InputProcessor
  virtual bool processInput(const TermKeyKey &key) override;

  void updateArea();
  void updateWindowArea(Window &window);

  int drawWindow(Window &window, Error &error);

  Windows::iterator findWindow(Window &window);
  void focusWindow();

  void redrawScreen();

  void declareBindables();
};

} // namespace CppConsUI

#endif // COREMANGER_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
