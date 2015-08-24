/*
 * Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2009-2015 Petr Pavlu <setup@dagobah.cz>
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

#include "CoreManager.h"

#include "ColorScheme.h"
#include "KeyConfig.h"

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <langinfo.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "gettext.h"

#define ICONV_NONE reinterpret_cast<iconv_t>(-1)

namespace CppConsUI {

int CoreManager::initializeInput(Error &error)
{
  assert(tk == NULL);
  assert(iconv_desc == ICONV_NONE);
  assert(stdin_input_handle == 0);

  // Get the codeset.
  const char *codeset = nl_langinfo(CODESET);

  // Initialize libtermkey.
  tk = termkey_new(STDIN_FILENO, TERMKEY_FLAG_NOTERMIOS);
  if (tk == NULL) {
    error = Error(
      ERROR_LIBTERMKEY_INITIALIZATION, _("Libtermkey initialization failed."));
    goto error_cleanup;
  }
  termkey_set_canonflags(tk, TERMKEY_CANON_DELBS);

  // If the codeset differs from UTF-8, setup iconv for conversion.
  if (std::strcmp(codeset, "UTF-8") != 0) {
    iconv_desc = iconv_open("UTF-8", codeset);
    if (iconv_desc == ICONV_NONE) {
      error = Error(ERROR_ICONV_INITIALIZATION);
      error.setFormattedString(
        _("Iconv initialization failed. Cannot create conversion from %s to "
          "UTF-8."),
        codeset);
      goto error_cleanup;
    }
  }

  stdin_input_handle = interface.inputAdd(
    STDIN_FILENO, INPUT_CONDITION_READ, CoreManager::stdin_input_, this);
  assert(stdin_input_handle != 0);

  return 0;

error_cleanup:
  if (iconv_desc != ICONV_NONE) {
    int res = iconv_close(iconv_desc);
    assert(res == 0);
    iconv_desc = ICONV_NONE;
  }

  if (tk != NULL) {
    termkey_destroy(tk);
    tk = NULL;
  }

  return error.getCode();
}

int CoreManager::finalizeInput(Error & /*error*/)
{
  assert(tk != NULL);
  assert(stdin_input_handle != 0);

  interface.inputRemove(stdin_input_handle);
  stdin_input_handle = 0;

  if (iconv_desc != ICONV_NONE) {
    int res = iconv_close(iconv_desc);
    // Closing iconv can fail only if the conversion descriptor is invalid but
    // that should never happen in CppConsUI.
    assert(res == 0);
    iconv_desc = ICONV_NONE;
  }

  termkey_destroy(tk);
  tk = NULL;

  return 0;
}

int CoreManager::initializeOutput(Error &error)
{
  return Curses::initScreen(error);
}

int CoreManager::finalizeOutput(Error &error)
{
  // Close all windows, work with a copy of the windows vector because the
  // original vector can be changed by calling the close() methods.
  Windows windows_copy = windows;
  for (Windows::iterator i = windows_copy.begin(); i != windows_copy.end(); i++)
    (*i)->close();

  // Delete all remaining windows. This prevents memory leaks for windows that
  // have the close() method overridden and calling it does not remove the
  // object from memory.
  windows_copy = windows;
  for (Windows::iterator i = windows_copy.begin(); i != windows_copy.end(); i++)
    delete *i;

  return Curses::finalizeScreen(error);
}

int CoreManager::initializeScreenResizing(Error &error)
{
  assert(pipefd[0] == -1);
  assert(pipefd[1] == -1);
  assert(resize_input_handle == 0);

  // Set up screen resizing.
  if (pipe(pipefd) != 0) {
    error = Error(ERROR_SCREEN_RESIZING_INITIALIZATION);
    error.setFormattedString(
      _("Screen resizing initialization failed. Cannot create self-pipe: %s."),
      strerror(errno));
    return error.getCode();
  }

  resize_input_handle = interface.inputAdd(
    pipefd[0], INPUT_CONDITION_READ, CoreManager::resize_input_, this);
  assert(resize_input_handle != 0);

  return 0;
}

int CoreManager::finalizeScreenResizing(Error &error)
{
  assert(pipefd[0] != -1);
  assert(pipefd[1] != -1);
  assert(resize_input_handle != 0);

  interface.inputRemove(resize_input_handle);
  resize_input_handle = 0;

  int close0_res = close(pipefd[0]);
  pipefd[0] = -1;

  int close1_res = close(pipefd[1]);
  pipefd[1] = -1;

  if (close0_res != 0 || close1_res != 0) {
    error = Error(ERROR_SCREEN_RESIZING_FINALIZATION);
    // Set the string using the last error.
    error.setFormattedString(
      _("Screen resizing finalization failed. Cannot close self-pipe: %s."),
      strerror(errno));
    return error.getCode();
  }

  return 0;
}

void CoreManager::registerWindow(Window &window)
{
  assert(!window.isVisible());

  Windows::iterator i = findWindow(window);
  assert(i == windows.end());

  windows.push_front(&window);
  updateWindowArea(window);
}

void CoreManager::removeWindow(Window &window)
{
  Windows::iterator i = findWindow(window);
  assert(i != windows.end());

  windows.erase(i);

  focusWindow();
  redraw();
}

void CoreManager::topWindow(Window &window)
{
  Windows::iterator i = findWindow(window);
  assert(i != windows.end());

  windows.erase(i);
  windows.push_back(&window);

  focusWindow();
  redraw();
}

Window *CoreManager::getTopWindow()
{
  return dynamic_cast<Window *>(input_child);
}

void CoreManager::enableResizing()
{
  onScreenResized();

  // register resize handler
  struct sigaction sig;
  sig.sa_handler = signalHandler;
  sigemptyset(&sig.sa_mask);
  sig.sa_flags = SA_RESTART;
  sigaction(SIGWINCH, &sig, NULL);
}

void CoreManager::disableResizing()
{
  // unregister resize handler
  struct sigaction sig;
  sig.sa_handler = SIG_DFL;
  sigemptyset(&sig.sa_mask);
  sig.sa_flags = 0;
  sigaction(SIGWINCH, &sig, NULL);
}

void CoreManager::onScreenResized()
{
  if (resize_pending)
    return;

  write(pipefd[1], "@", 1);
  resize_pending = true;
}

void CoreManager::logError(const char *message)
{
  interface.logError(message);
}

void CoreManager::redraw()
{
  if (redraw_pending)
    return;

  redraw_pending = true;
  interface.timeoutAdd(0, CoreManager::draw_, this);
}

void CoreManager::onWindowMoveResize(
  Window &activator, const Rect & /*oldsize*/, const Rect & /*newsize*/)
{
  updateWindowArea(activator);
}

void CoreManager::onWindowWishSizeChange(
  Window &activator, const Size &oldsize, const Size &newsize)
{
  if ((activator.getWidth() != Widget::AUTOSIZE ||
        oldsize.getWidth() == newsize.getWidth()) &&
    (activator.getHeight() != Widget::AUTOSIZE ||
        oldsize.getHeight() == newsize.getHeight()))
    return;

  updateWindowArea(activator);
}

CoreManager::CoreManager(AppInterface &set_interface)
  : top_input_processor(NULL), stdin_input_timeout_handle(0),
    stdin_input_handle(0), resize_input_handle(0), tk(NULL),
    iconv_desc(ICONV_NONE), redraw_pending(false), resize_pending(false)
{
  pipefd[0] = pipefd[1] = -1;

  // validate the passed interface
  assert(set_interface.timeoutAdd != NULL);
  assert(set_interface.timeoutRemove != NULL);
  assert(set_interface.inputAdd != NULL);
  assert(set_interface.inputRemove != NULL);
  assert(set_interface.logError != NULL);

  interface = set_interface;

  declareBindables();
}

bool CoreManager::processInput(const TermKeyKey &key)
{
  if (top_input_processor && top_input_processor->processInput(key))
    return true;

  return InputProcessor::processInput(key);
}

void CoreManager::stdin_input(int /*fd*/, InputCondition /*cond*/)
{
  termkey_advisereadable(tk);

  TermKeyKey key;
  TermKeyResult ret;
  while ((ret = termkey_getkey(tk, &key)) == TERMKEY_RES_KEY) {
    if (key.type == TERMKEY_TYPE_UNICODE && iconv_desc != ICONV_NONE) {
      size_t inbytesleft, outbytesleft;
      char *inbuf, *outbuf;
      size_t res;
      char utf8[sizeof(key.utf8) - 1];

      // convert data from user charset to UTF-8
      inbuf = key.utf8;
      inbytesleft = strlen(key.utf8);
      outbuf = utf8;
      outbytesleft = sizeof(utf8);
      res = iconv(iconv_desc, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
      if (res != static_cast<size_t>(-1) && inbytesleft != 0) {
        // no error occured but not all bytes has been converted
        errno = EINVAL;
        res = static_cast<size_t>(-1);
      }
      if (res == static_cast<size_t>(-1)) {
        char text[256];
        snprintf(text, sizeof(text), _("Error converting input to UTF-8 (%s)."),
          std::strerror(errno));
        logError(text);
        continue;
      }

      size_t outbytes = sizeof(utf8) - outbytesleft;
      std::memcpy(key.utf8, utf8, outbytes);
      key.utf8[outbytes] = '\0';

      key.code.codepoint = UTF8::getUniChar(key.utf8);
    }

    processInput(key);
  }
  if (ret == TERMKEY_RES_AGAIN) {
    int wait = termkey_get_waittime(tk);
    stdin_input_timeout_handle =
      interface.timeoutAdd(wait, CoreManager::stdin_input_timeout_, this);
  }
}

bool CoreManager::stdin_input_timeout_(void *data)
{
  CoreManager *instance = static_cast<CoreManager *>(data);
  instance->stdin_input_timeout();
  return false;
}

void CoreManager::stdin_input_timeout()
{
  assert(stdin_input_timeout_handle != 0);
  stdin_input_timeout_handle = 0;

  TermKeyKey key;
  if (termkey_getkey_force(tk, &key) == TERMKEY_RES_KEY) {
    /* This should happen only for Esc key, so no need to do locale->utf8
     * conversion. */
    processInput(key);
  }
}

void CoreManager::resize_input(int fd, InputCondition /*cond*/)
{
  // stay sane
  assert(fd == pipefd[0]);

  char buf[1024];
  read(fd, buf, sizeof(buf));

  if (resize_pending)
    resize();
}

void CoreManager::signalHandler(int signum)
{
  assert(signum == SIGWINCH);

  COREMANAGER->onScreenResized();
}

void CoreManager::resize()
{
  /// @todo Implement correct error reporting when resize() fails.

  resize_pending = false;

  struct winsize size;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) >= 0) {
    Error error;
    Curses::resizeTerm(size.ws_col, size.ws_row, error);

    // Make sure everything is redrawn from the scratch.
    Curses::clear(error);
  }

  // Signal the resize event.
  signal_resize();
  for (Windows::iterator i = windows.begin(); i != windows.end(); i++)
    (*i)->onScreenResized();

  // Update area.
  updateArea();

  redraw();
}

void CoreManager::updateArea()
{
  for (Windows::iterator i = windows.begin(); i != windows.end(); i++)
    updateWindowArea(**i);
}

void CoreManager::updateWindowArea(Window &window)
{
  int screen_width = Curses::getWidth();
  int screen_height = Curses::getHeight();

  int window_x = window.getLeft();
  int window_y = window.getTop();

  // calculate the real width
  int window_width = window.getWidth();
  if (window_width == Widget::AUTOSIZE) {
    window_width = window.getWishWidth();
    if (window_width == Widget::AUTOSIZE)
      window_width = screen_width - window_x;
  }
  if (window_width < 0)
    window_width = 0;

  // calculate the real height
  int window_height = window.getHeight();
  if (window_height == Widget::AUTOSIZE) {
    window_height = window.getWishHeight();
    if (window_height == Widget::AUTOSIZE)
      window_height = screen_height - window_y;
  }
  if (window_height < 0)
    window_height = 0;

  window.setRealPosition(window_x, window_y);
  window.setRealSize(window_width, window_height);
}

bool CoreManager::draw_(void *data)
{
  CoreManager *instance = static_cast<CoreManager *>(data);
  /// @todo Implement correct reporting when draw() fails.
  Error error;
  instance->draw(error);
  return false;
}

int CoreManager::draw(Error &error)
{
  if (!redraw_pending)
    return 0;

#if defined(DEBUG) && 0
  struct timespec ts = {0, 0};
  clock_gettime(CLOCK_MONOTONIC, &ts);

  Curses::resetStats();
#endif // DEBUG

  DRAW(Curses::erase(error));

  // Non-focusable -> normal -> top.
  for (Windows::iterator i = windows.begin(); i != windows.end(); i++)
    if ((*i)->isVisible() && (*i)->getType() == Window::TYPE_NON_FOCUSABLE)
      DRAW(drawWindow(**i, error));

  for (Windows::iterator i = windows.begin(); i != windows.end(); i++)
    if ((*i)->isVisible() && (*i)->getType() == Window::TYPE_NORMAL)
      DRAW(drawWindow(**i, error));

  for (Windows::iterator i = windows.begin(); i != windows.end(); i++)
    if ((*i)->isVisible() && (*i)->getType() == Window::TYPE_TOP)
      DRAW(drawWindow(**i, error));

  // Copy virtual ncurses screen to the physical screen.
  DRAW(Curses::refresh(error));

#if defined(DEBUG) && 0
  const Curses::Stats *stats = Curses::getStats();

  struct timespec ts2 = {0, 0};
  clock_gettime(CLOCK_MONOTONIC, &ts2);
  unsigned long tdiff = (ts2.tv_sec * 1000000 + ts2.tv_nsec / 1000) -
    (ts.tv_sec * 1000000 + ts.tv_nsec / 1000);

  char message[sizeof("redraw: time=us, newpad/newwin/subpad calls=//") +
    PRINTF_WIDTH(unsigned long)+3 * PRINTF_WIDTH(int)];
  sprintf(message, "redraw: time=%luus, newpad/newwin/subpad calls=%d/%d/%d",
    tdiff, stats->newpad_calls, stats->newwin_calls, stats->subpad_calls);

  logError(message);
#endif // DEBUG

  redraw_pending = false;

  return 0;
}

int CoreManager::drawWindow(Window &window, Error &error)
{
  int screen_width = Curses::getWidth();
  int screen_height = Curses::getHeight();

  int window_x = window.getRealLeft();
  int window_y = window.getRealTop();
  int window_width = window.getRealWidth();
  int window_height = window.getRealHeight();
  int window_x2 = window_x + window_width;
  int window_y2 = window_y + window_height;

  int window_view_x = 0;
  int window_view_y = 0;
  int window_view_width = window_width;
  int window_view_height = window_height;

  // calculate a viewport for the window
  if (window_x < 0) {
    window_view_x = -window_x;
    if (window_view_x > window_width)
      window_view_x = window_width;
    window_view_width -= window_view_x;
  }
  if (window_y < 0) {
    window_view_y = -window_y;
    if (window_view_y > window_height)
      window_view_y = window_height;
    window_view_height -= window_view_y;
  }

  if (window_x2 > screen_width) {
    window_view_width -= window_x2 - screen_width;
    if (window_view_width < 0)
      window_view_width = 0;
  }
  if (window_y2 > screen_height) {
    window_view_height -= window_y2 - screen_height;
    if (window_view_height < 0)
      window_view_height = 0;
  }
  Curses::ViewPort window_area(window_x, window_y, window_view_x, window_view_y,
    window_view_width, window_view_height);
  return window.draw(window_area, error);
}

CoreManager::Windows::iterator CoreManager::findWindow(Window &window)
{
  return std::find(windows.begin(), windows.end(), &window);
}

void CoreManager::focusWindow()
{
  // check if there are any windows left
  Window *win = NULL;
  Windows::reverse_iterator i;

  // try to find a top window first
  for (i = windows.rbegin(); i != windows.rend(); i++)
    if ((*i)->isVisible() && (*i)->getType() == Window::TYPE_TOP) {
      win = *i;
      break;
    }

  // normal windows
  if (!win)
    for (i = windows.rbegin(); i != windows.rend(); i++)
      if ((*i)->isVisible() && (*i)->getType() == Window::TYPE_NORMAL) {
        win = *i;
        break;
      }

  Window *focus = dynamic_cast<Window *>(getInputChild());
  if (!win || win != focus) {
    // take the focus from the old window with the focus
    if (focus) {
      focus->ungrabFocus();
      clearInputChild();
    }

    // give the focus to the window
    if (win) {
      setInputChild(*win);
      win->restoreFocus();
    }
    signal_top_window_change();
  }
}

void CoreManager::redrawScreen()
{
  /// @todo Implement correct error handling when clear() fails.

  // Make sure everything is redrawn from the scratch.
  Error error;
  Curses::clear(error);

  redraw();
}

void CoreManager::declareBindables()
{
  declareBindable("coremanager", "redraw-screen",
    sigc::mem_fun(this, &CoreManager::redrawScreen),
    InputProcessor::BINDABLE_OVERRIDE);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
