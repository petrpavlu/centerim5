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

#include "CoreManager.h"

#include "ColorScheme.h"
#include "KeyConfig.h"

#include <stdio.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include "gettext.h"

namespace CppConsUI
{

// based on glibmm code
class SourceConnectionNode
{
  public:
    explicit inline SourceConnectionNode(const sigc::slot_base& nslot);

    static void *notify(void *data);
    static void destroy_notify_callback(void *data);
    static gboolean source_callback(void *data);

    inline void install(GSource *nsource);
    inline sigc::slot_base *get_slot();

  protected:

  private:
    sigc::slot_base slot;
    GSource *source;
};

inline SourceConnectionNode::SourceConnectionNode(
    const sigc::slot_base& nslot)
: slot(nslot)
, source(0)
{
  slot.set_parent(this, &SourceConnectionNode::notify);
}

void *SourceConnectionNode::notify(void *data)
{
  SourceConnectionNode *self
    = reinterpret_cast<SourceConnectionNode*>(data);

  /* If there is no object, this call was triggered from
   * destroy_notify_handler(), because we set self->source to 0 there. */
  if (self->source) {
    GSource *s = self->source;
    self->source = 0;
    g_source_destroy(s);

    /* Destroying the object triggers execution of destroy_notify_handler(),
     * eiter immediately or later, so we leave that to do the deletion. */
  }

  return 0;
}

void SourceConnectionNode::destroy_notify_callback(void *data)
{
  SourceConnectionNode *self = reinterpret_cast<SourceConnectionNode*>(data);

  if (self) {
    /* The GLib side is disconnected now, thus the GSource* is no longer
     * valid. */
    self->source = 0;

    delete self;
  }
}

gboolean SourceConnectionNode::source_callback(void *data)
{
  SourceConnectionNode *conn_data
    = reinterpret_cast<SourceConnectionNode*>(data);

  // recreate the specific slot from the generic slot node
  return (*static_cast<sigc::slot<bool>*>(conn_data->get_slot()))();
}

inline void SourceConnectionNode::install(GSource *nsource)
{
  source = nsource;
}

inline sigc::slot_base *SourceConnectionNode::get_slot()
{
  return &slot;
}

int initializeConsUI()
{
  int res;

  res = ColorScheme::init();
  if (res)
    return res;

  res = KeyConfig::init();
  if (res) {
    // not good, destroy already initialized ColorScheme
    ColorScheme::finalize();
    return res;
  }

  // CoreManager depends on KeyConfig so it has to be initialized after it
  res = CoreManager::init();
  if (res) {
    // not good, destroy already initialized KeyConfig and ColorScheme
    KeyConfig::finalize();
    ColorScheme::finalize();
    return res;
  }

  return 0;
}

int finalizeConsUI()
{
  int max = 0;
  int res;

  res = CoreManager::finalize();
  max = MAX(max, res);

  res = KeyConfig::finalize();
  max = MAX(max, res);

  res = ColorScheme::finalize();
  max = MAX(max, res);

  return max;
}

CoreManager *CoreManager::my_instance = NULL;

CoreManager *CoreManager::instance()
{
  return my_instance;
}

void CoreManager::startMainLoop()
{
  g_main_loop_run(gmainloop);
}

void CoreManager::quitMainLoop()
{
  g_main_loop_quit(gmainloop);
}

void CoreManager::addWindow(FreeWindow& window)
{
  Windows::iterator i = findWindow(window);

  if (i != windows.end()) {
    /* Window is already added, addWindow() was called to bring the window to
     * the top. */
    windows.erase(i);
    windows.push_back(&window);
  }
  else {
    windows.push_back(&window);
    window.onScreenResized();
  }

  focusWindow();
  redraw();
}

void CoreManager::removeWindow(FreeWindow& window)
{
  Windows::iterator i;

  for (i = windows.begin(); i != windows.end(); i++)
    if (*i == &window)
      break;

  g_assert(i != windows.end());

  windows.erase(i);

  focusWindow();
  redraw();
}

bool CoreManager::hasWindow(const FreeWindow& window) const
{
  for (Windows::const_iterator i = windows.begin(); i != windows.end(); i++)
    if (*i == &window)
      return true;

  return false;
}

FreeWindow *CoreManager::getTopWindow()
{
  return dynamic_cast<FreeWindow*>(input_child);
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
  if (pipe_valid && !resize_pending) {
    write(pipefd[1], "@", 1);
    resize_pending = true;
  }
}

void CoreManager::redraw()
{
  if (!redraw_pending) {
    redraw_pending = true;
    timeoutOnceConnect(sigc::mem_fun(this, &CoreManager::draw), 0);
  }
}

sigc::connection CoreManager::timeoutConnect(const sigc::slot<bool>& slot,
    unsigned interval, int priority)
{
  SourceConnectionNode *conn_node = new SourceConnectionNode(slot);
  sigc::connection connection(*conn_node->get_slot());

  GSource *source = g_timeout_source_new(interval);

  if (priority != G_PRIORITY_DEFAULT)
    g_source_set_priority(source, priority);

  g_source_set_callback(source, &SourceConnectionNode::source_callback,
      conn_node, &SourceConnectionNode::destroy_notify_callback);

  g_source_attach(source, NULL);
  g_source_unref(source); // GMainContext holds a reference

  conn_node->install(source);
  return connection;
}

sigc::connection CoreManager::timeoutOnceConnect(const sigc::slot<void>& slot,
    unsigned interval, int priority)
{
  return timeoutConnect(sigc::bind_return(slot, FALSE), interval, priority);
}

CoreManager::CoreManager()
: top_input_processor(NULL), io_input_channel(NULL), io_input_channel_id(0)
, resize_channel(NULL), resize_channel_id(0), pipe_valid(false), tk(NULL)
, utf8(false), gmainloop(NULL), redraw_pending(false), resize_pending(false)
{
  initInput();

  /**
   * @todo Check the return value. Throw an exception if we can't init curses.
   */
  Curses::init_screen();

  // create the main loop
  gmainloop = g_main_loop_new(NULL, FALSE);

  declareBindables();
}

CoreManager::~CoreManager()
{
  // destroy the main loop
  g_main_loop_unref(gmainloop);

  finalizeInput();

  /* Close all windows, work with a copy of the windows vector because the
   * original vector can be changed by calling the close() methods. */
  Windows windows_copy = windows;
  for (Windows::iterator i = windows_copy.begin(); i != windows_copy.end();
      i++)
    (*i)->close();

  /* Delete all remaining windows. This prevents memory leaks for windows that
   * have the close() method overridden and calling it does not remove the
   * object from memory. */
  windows_copy = windows;
  for (Windows::iterator i = windows_copy.begin(); i != windows_copy.end();
      i++)
    delete *i;

  Curses::clear();
  Curses::noutrefresh();
  Curses::doupdate();
  Curses::finalize_screen();
}

int CoreManager::init()
{
  g_assert(!my_instance);

  my_instance = new CoreManager;
  return 0;
}

int CoreManager::finalize()
{
  g_assert(my_instance);

  delete my_instance;
  my_instance = NULL;
  return 0;
}

bool CoreManager::processInput(const TermKeyKey& key)
{
  if (top_input_processor && top_input_processor->processInput(key))
    return true;

  return InputProcessor::processInput(key);
}

gboolean CoreManager::io_input_error(GIOChannel * /*source*/,
    GIOCondition /*cond*/)
{
  // log a critical warning and bail out if we lost stdin
  g_critical("Stdin lost!");
  exit(1);

  return TRUE;
}

gboolean CoreManager::io_input(GIOChannel * /*source*/, GIOCondition /*cond*/)
{
  if (io_input_timeout_conn.connected())
    io_input_timeout_conn.disconnect();

  termkey_advisereadable(tk);

  TermKeyKey key;
  TermKeyResult ret;
  while ((ret = termkey_getkey(tk, &key)) == TERMKEY_RES_KEY) {
    if (key.type == TERMKEY_TYPE_UNICODE && !utf8) {
      gsize bwritten;
      GError *err = NULL;
      char *utf8;

      // convert data from user charset to UTF-8
      if (!(utf8 = g_locale_to_utf8(key.utf8, -1, NULL, &bwritten, &err))) {
        g_warning(_("Error converting input to UTF-8 (%s)."), err->message);
        g_clear_error(&err);
        continue;
      }

      memcpy(key.utf8, utf8, bwritten + 1);
      g_free(utf8);

      key.code.codepoint = g_utf8_get_char(key.utf8);
    }

    processInput(key);
  }
  if (ret == TERMKEY_RES_AGAIN) {
    int wait = termkey_get_waittime(tk);
    io_input_timeout_conn = timeoutOnceConnect(sigc::mem_fun(this,
          &CoreManager::io_input_timeout), wait);
  }

  return TRUE;
}

void CoreManager::io_input_timeout()
{
  TermKeyKey key;
  if (termkey_getkey_force(tk, &key) == TERMKEY_RES_KEY) {
    /* This should happen only for Esc key, so no need to do locale->utf8
     * conversion. */
    processInput(key);
  }
}

gboolean CoreManager::resize_input(GIOChannel *source, GIOCondition /*cond*/)
{
  char buf[1024];
  gsize bytes_read;
  GError *err = NULL;
  g_io_channel_read_chars(source, buf, sizeof(buf), &bytes_read, &err);
  if (err)
    g_clear_error(&err);

  if (resize_pending)
    resize();

  return TRUE;
}

void CoreManager::initInput()
{
  // init libtermkey
  TERMKEY_CHECK_VERSION;
  if (!(tk = termkey_new(STDIN_FILENO, TERMKEY_FLAG_NOTERMIOS))) {
    g_critical(_("Libtermkey initialization failed."));
    exit(1);
  }
  termkey_set_canonflags(tk, TERMKEY_CANON_DELBS);
  utf8 = g_get_charset(NULL);

  io_input_channel = g_io_channel_unix_new(STDIN_FILENO);
  // set channel encoding to NULL so it can be unbuffered
  g_io_channel_set_encoding(io_input_channel, NULL, NULL);
  g_io_channel_set_buffered(io_input_channel, FALSE);
  g_io_channel_set_close_on_unref(io_input_channel, TRUE);

  io_input_channel_id = g_io_add_watch_full(io_input_channel, G_PRIORITY_HIGH,
      static_cast<GIOCondition>(G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_PRI),
      io_input_, this, NULL);
  g_io_add_watch_full(io_input_channel, G_PRIORITY_HIGH, G_IO_NVAL,
      io_input_error_, this, NULL);
  g_io_channel_unref(io_input_channel);

  // screen resizing
  if (!pipe(pipefd)) {
    pipe_valid = true;
    resize_channel = g_io_channel_unix_new(pipefd[0]);
    g_io_channel_set_encoding(resize_channel, NULL, NULL);
    g_io_channel_set_buffered(resize_channel, FALSE);
    g_io_channel_set_close_on_unref(resize_channel, TRUE);

    resize_channel_id = g_io_add_watch_full(resize_channel, G_PRIORITY_HIGH,
        G_IO_IN, resize_input_, this, NULL);
  }
}

void CoreManager::finalizeInput()
{
  termkey_destroy(tk);
  tk = NULL;

  g_source_remove(io_input_channel_id);
  io_input_channel_id = 0;
  g_io_channel_unref(io_input_channel);
  io_input_channel = NULL;

  if (pipe_valid) {
    g_source_remove(resize_channel_id);
    resize_channel_id = 0;
    g_io_channel_unref(resize_channel);
    resize_channel = NULL;
    close(pipefd[0]);
    close(pipefd[1]);
  }
}

void CoreManager::signalHandler(int signum)
{
  if (signum == SIGWINCH)
    COREMANAGER->onScreenResized();
}

void CoreManager::resize()
{
  struct winsize size;

  resize_pending = false;

  if (ioctl(fileno(stdout), TIOCGWINSZ, &size) >= 0) {
    Curses::resizeterm(size.ws_row, size.ws_col);

    // make sure everything is redrawn from the scratch
    Curses::clear();
  }

  signal_resize();
  redraw();
}

void CoreManager::draw()
{
  if (!redraw_pending)
    return;

#if defined(DEBUG) && GLIB_MAJOR_VERSION >= 2 && GLIB_MINOR_VERSION >= 28
  gint64 t1 = g_get_monotonic_time();
  Curses::reset_stats();
#endif // defined(DEBUG) && GLIB_VERSION >= 2.28

  Curses::erase();
  Curses::noutrefresh();

  // non-focusable -> normal -> top
  for (Windows::iterator i = windows.begin(); i != windows.end(); i++)
    if ((*i)->getType() == FreeWindow::TYPE_NON_FOCUSABLE)
      (*i)->draw();

  for (Windows::iterator i = windows.begin(); i != windows.end(); i++)
    if ((*i)->getType() == FreeWindow::TYPE_NORMAL)
      (*i)->draw();

  for (Windows::iterator i = windows.begin(); i != windows.end(); i++)
    if ((*i)->getType() == FreeWindow::TYPE_TOP)
      (*i)->draw();

  // copy virtual ncurses screen to the physical screen
  Curses::doupdate();

#if defined(DEBUG) && GLIB_MAJOR_VERSION >= 2 && GLIB_MINOR_VERSION >= 28
  const Curses::Stats *stats = Curses::get_stats();
  gint64 tdiff = g_get_monotonic_time() - t1;
  g_debug("redraw: time=%"G_GINT64_FORMAT"us, newpad/newwin/subpad "
      "calls=%u/%u/%u", tdiff, stats->newpad_calls, stats->newwin_calls,
      stats->subpad_calls);
#endif // defined(DEBUG) && GLIB_VERSION >= 2.28

  redraw_pending = false;
}

CoreManager::Windows::iterator CoreManager::findWindow(FreeWindow& window)
{
  return std::find(windows.begin(), windows.end(), &window);
}

void CoreManager::focusWindow()
{
  // check if there are any windows left
  FreeWindow *win = NULL;
  Windows::reverse_iterator i;

  // try to find a top window first
  for (i = windows.rbegin(); i != windows.rend(); i++)
    if ((*i)->getType() == FreeWindow::TYPE_TOP) {
      win = *i;
      break;
    }

  // normal windows
  if (!win)
    for (i = windows.rbegin(); i != windows.rend(); i++)
      if ((*i)->getType() == FreeWindow::TYPE_NORMAL) {
        win = *i;
        break;
      }

  FreeWindow *focus = dynamic_cast<FreeWindow*>(getInputChild());
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
  // make sure everything is redrawn from the scratch
  Curses::clear();

  redraw();
}

void CoreManager::declareBindables()
{
  declareBindable("coremanager", "redraw-screen",
      sigc::mem_fun(this, &CoreManager::redrawScreen),
      InputProcessor::BINDABLE_OVERRIDE);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
