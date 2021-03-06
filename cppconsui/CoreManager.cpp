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

#include "CoreManager.h"

#include "ColorScheme.h"
#include "KeyConfig.h"

#include "gettext.h"
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <langinfo.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define ICONV_NONE reinterpret_cast<iconv_t>(-1)

namespace CppConsUI {

namespace {

// Workaround for implementations that declare the inbuf parameter of the
// iconv() function as "const char **".
template <class T> class sloppy {
};

// Convert between "T **" and "const T **".
template <class T> class sloppy<T **> {
public:
  sloppy(T **t) : t_(t) {}
  sloppy(const T **t) : t_(const_cast<T **>(t)) {}

  operator T **() const { return t_; }
  operator const T **() const { return const_cast<const T **>(t_); }

private:
  T **t_;
};

} // anonymous namespace

int CoreManager::initializeInput(Error &error)
{
  assert(tk_ == nullptr);
  assert(iconv_desc_ == ICONV_NONE);

  // Get the current character encoding.
  const char *codeset = nl_langinfo(CODESET);

  // Initialize libtermkey.
  tk_ = termkey_new(STDIN_FILENO, TERMKEY_FLAG_NOTERMIOS);
  if (tk_ == nullptr) {
    error = Error(
      ERROR_LIBTERMKEY_INITIALIZATION, _("Libtermkey initialization failed."));
    goto error_cleanup;
  }
  termkey_set_canonflags(tk_, TERMKEY_CANON_DELBS);

  // If the codeset differs from UTF-8, setup iconv for conversion.
  if (std::strcmp(codeset, "UTF-8") != 0) {
    iconv_desc_ = iconv_open("UTF-8", codeset);
    if (iconv_desc_ == ICONV_NONE) {
      error = Error(ERROR_ICONV_INITIALIZATION);
      error.setFormattedString(
        _("Iconv initialization failed. Cannot create a conversion descriptor "
          "from %s to UTF-8."),
        codeset);
      goto error_cleanup;
    }
  }

  return 0;

error_cleanup:
  if (iconv_desc_ != ICONV_NONE) {
    int res = iconv_close(iconv_desc_);
    assert(res == 0);
    iconv_desc_ = ICONV_NONE;
  }

  if (tk_ != nullptr) {
    termkey_destroy(tk_);
    tk_ = nullptr;
  }

  return error.getCode();
}

int CoreManager::finalizeInput(Error & /*error*/)
{
  assert(tk_ != nullptr);

  if (iconv_desc_ != ICONV_NONE) {
    int res = iconv_close(iconv_desc_);
    // Closing iconv can fail only if the conversion descriptor is invalid but
    // that should never happen in CppConsUI.
    assert(res == 0);
    iconv_desc_ = ICONV_NONE;
  }

  termkey_destroy(tk_);
  tk_ = nullptr;

  return 0;
}

int CoreManager::initializeOutput(Error &error)
{
  return Curses::initScreen(error);
}

int CoreManager::finalizeOutput(Error &error)
{
  // Delete all windows. This must be done very carefully because deleting one
  // window can lead to deletion of another one. It is however required for each
  // window to deregister itself from CoreManager when it is deleted (and not to
  // register any new windows).
  while (!windows_.empty())
    delete windows_.front();

  return Curses::finalizeScreen(error);
}

int CoreManager::processStandardInput(int *wait, Error &error)
{
  assert(wait != nullptr);
  *wait = -1;

  termkey_advisereadable(tk_);

  TermKeyKey key;
  TermKeyResult ret;
  while ((ret = termkey_getkey(tk_, &key)) == TERMKEY_RES_KEY) {
    if (key.type == TERMKEY_TYPE_UNICODE && iconv_desc_ != ICONV_NONE) {
      std::size_t inbytesleft, outbytesleft;
      char *inbuf, *outbuf;
      std::size_t res;
      char utf8[sizeof(key.utf8) - 1];

      // Convert data from the user charset to UTF-8.
      inbuf = key.utf8;
      inbytesleft = strlen(key.utf8);
      outbuf = utf8;
      outbytesleft = sizeof(utf8);
      res = iconv(iconv_desc_, sloppy<char **>(&inbuf), &inbytesleft, &outbuf,
        &outbytesleft);
      if (res != static_cast<std::size_t>(-1) && inbytesleft != 0) {
        // No error occured but not all bytes have been converted.
        errno = EINVAL;
        res = static_cast<std::size_t>(-1);
      }
      if (res == static_cast<std::size_t>(-1)) {
        error = Error(ERROR_INPUT_CONVERSION);
        error.setFormattedString(
          _("Error converting input to UTF-8 (%s)."), std::strerror(errno));
        return error.getCode();
      }

      std::size_t outbytes = sizeof(utf8) - outbytesleft;
      std::memcpy(key.utf8, utf8, outbytes);
      key.utf8[outbytes] = '\0';

      key.code.codepoint = UTF8::getUniChar(key.utf8);
    }

    processInput(key);
  }
  if (ret == TERMKEY_RES_AGAIN) {
    *wait = termkey_get_waittime(tk_);
    assert(*wait >= 0);
  }

  return 0;
}

int CoreManager::processStandardInputTimeout(Error & /*error*/)
{
  TermKeyKey key;
  if (termkey_getkey_force(tk_, &key) == TERMKEY_RES_KEY) {
    // This should happen only for Esc key, so no need to do the locale -> UTF-8
    // conversion.
    processInput(key);
  }
  return 0;
}

int CoreManager::resize(Error &error)
{
  struct winsize size;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) >= 0) {
    if (Curses::resizeTerm(size.ws_col, size.ws_row, error) != 0)
      return error.getCode();
  }

  // Update area.
  updateArea();

  // Make sure everything is redrawn from the scratch.
  redraw(true);

  // Trigger the resize event.
  onScreenResized();

  return 0;
}

int CoreManager::draw(Error &error)
{
  if (pending_redraw_ == REDRAW_NONE)
    return 0;

#if defined(DEBUG) && 0
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
#endif // DEBUG

  if (pending_redraw_ == REDRAW_FROM_SCRATCH)
    DRAW(Curses::clear(error));
  else
    DRAW(Curses::erase(error));

  // Non-focusable -> normal -> top.
  for (Window *window : windows_)
    if (window->isVisible() && window->getType() == Window::TYPE_NON_FOCUSABLE)
      DRAW(drawWindow(*window, error));

  for (Window *window : windows_)
    if (window->isVisible() && window->getType() == Window::TYPE_NORMAL)
      DRAW(drawWindow(*window, error));

  for (Window *window : windows_)
    if (window->isVisible() && window->getType() == Window::TYPE_TOP)
      DRAW(drawWindow(*window, error));

  // Copy virtual ncurses screen to the physical screen.
  DRAW(Curses::refresh(error));

#if defined(DEBUG) && 0
  struct timespec ts2;
  clock_gettime(CLOCK_MONOTONIC, &ts2);
  unsigned long tdiff =
    (ts2.tv_sec - ts.tv_sec) * 1000000 + ts2.tv_nsec / 1000 - ts.tv_nsec / 1000;

  char message[sizeof("redraw: time=us") + PRINTF_WIDTH(unsigned long)];
  sprintf(message, "redraw: time=%luus", tdiff);
  logDebug(message);
#endif // DEBUG

  pending_redraw_ = REDRAW_NONE;

  return 0;
}

void CoreManager::registerWindow(Window &window)
{
  assert(!window.isVisible());

  Windows::iterator i = findWindow(window);
  assert(i == windows_.end());

  windows_.push_front(&window);
  updateWindowArea(window);
}

void CoreManager::removeWindow(Window &window)
{
  Windows::iterator i = findWindow(window);
  assert(i != windows_.end());

  windows_.erase(i);

  focusWindow();
  redraw();
}

void CoreManager::hideWindow(Window &window)
{
  Windows::iterator i = findWindow(window);
  assert(i != windows_.end());

  focusWindow();
  redraw();
}

void CoreManager::topWindow(Window &window)
{
  Windows::iterator i = findWindow(window);
  assert(i != windows_.end());

  windows_.erase(i);
  windows_.push_back(&window);

  focusWindow();
  redraw();
}

Window *CoreManager::getTopWindow()
{
  return dynamic_cast<Window *>(input_child_);
}

void CoreManager::logDebug(const char *message)
{
  interface_.logDebug(message);
}

void CoreManager::redraw(bool from_scratch)
{
  if (pending_redraw_ == REDRAW_NONE)
    interface_.redraw();

  if (pending_redraw_ == REDRAW_FROM_SCRATCH)
    return;
  pending_redraw_ = from_scratch ? REDRAW_FROM_SCRATCH : REDRAW_NORMAL;
}

bool CoreManager::isRedrawPending() const
{
  return pending_redraw_ != REDRAW_NONE;
}

void CoreManager::onScreenResized()
{
  // Signal the resize event.
  signal_resize();

  // Propagate the resize event to all windows.
  for (Window *window : windows_)
    window->onScreenResized();
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
  : top_input_processor_(nullptr), tk_(nullptr), iconv_desc_(ICONV_NONE),
    pending_redraw_(REDRAW_NONE)
{
  // Validate the passed interface.
  assert(!set_interface.redraw.empty());
  assert(!set_interface.logDebug.empty());

  interface_ = set_interface;

  declareBindables();
}

bool CoreManager::processInput(const TermKeyKey &key)
{
  if (top_input_processor_ && top_input_processor_->processInput(key))
    return true;

  return InputProcessor::processInput(key);
}

void CoreManager::updateArea()
{
  for (Window *window : windows_)
    updateWindowArea(*window);
}

void CoreManager::updateWindowArea(Window &window)
{
  int screen_width = Curses::getWidth();
  int screen_height = Curses::getHeight();

  int window_x = window.getLeft();
  int window_y = window.getTop();

  // Calculate the real width.
  int window_width = window.getWidth();
  if (window_width == Widget::AUTOSIZE) {
    window_width = window.getWishWidth();
    if (window_width == Widget::AUTOSIZE)
      window_width = screen_width - window_x;
  }
  if (window_width < 0)
    window_width = 0;

  // Calculate the real height.
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

  // Calculate a viewport for the window.
  if (window_x < 0) {
    window_view_x = -window_x;
    window_x += window_view_x;
    if (window_view_x > window_width)
      window_view_x = window_width;
    window_view_width -= window_view_x;
  }
  if (window_y < 0) {
    window_view_y = -window_y;
    window_y += window_view_y;
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
  return std::find(windows_.begin(), windows_.end(), &window);
}

void CoreManager::focusWindow()
{
  // Check if there are any windows left.
  Window *win = nullptr;
  Windows::reverse_iterator i;

  // Try to find a top window first.
  for (i = windows_.rbegin(); i != windows_.rend(); ++i)
    if ((*i)->isVisible() && (*i)->getType() == Window::TYPE_TOP) {
      win = *i;
      break;
    }

  // Normal windows.
  if (win == nullptr)
    for (i = windows_.rbegin(); i != windows_.rend(); ++i)
      if ((*i)->isVisible() && (*i)->getType() == Window::TYPE_NORMAL) {
        win = *i;
        break;
      }

  Window *focus = dynamic_cast<Window *>(getInputChild());
  if (win == nullptr || win != focus) {
    // Take the focus from the old window with the focus.
    if (focus != nullptr) {
      focus->ungrabFocus();
      clearInputChild();
    }

    // Give the focus to the window.
    if (win != nullptr) {
      setInputChild(*win);
      win->restoreFocus();
    }
    signal_top_window_change();
  }
}

void CoreManager::redrawScreen()
{
  // Make sure everything is redrawn from the scratch.
  redraw(true);
}

void CoreManager::declareBindables()
{
  declareBindable("coremanager", "redraw-screen",
    sigc::mem_fun(this, &CoreManager::redrawScreen),
    InputProcessor::BINDABLE_OVERRIDE);
}

} // namespace CppConsUI

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
