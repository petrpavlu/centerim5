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

#include "CenterIM.h"

#include "Accounts.h"
#include "BuddyList.h"
#include "Conf.h"
#include "Connections.h"
#include "Conversations.h"
#include "Header.h"
#include "Log.h"
#include "Notify.h"
#include "Request.h"
#include "Transfers.h"

#include "AccountStatusMenu.h"
#include "GeneralMenu.h"

#include <cppconsui/ColorScheme.h>
#include <cppconsui/Keys.h>
#include <cstring>
#include "gettext.h"

#define CONTEXT_CENTERIM "centerim"

// TODO configurable path via command line option
#define CIM_CONFIG_PATH ".centerim5"

std::vector<CenterIM::LogBufferItem> *CenterIM::logbuf = NULL;

CenterIM *CenterIM::Instance()
{
  static CenterIM instance;
  return &instance;
}

CenterIM::CenterIM()
{
  mngr = CoreManager::Instance();
  resize = mngr->signal_resize.connect(sigc::mem_fun(this,
        &CenterIM::ScreenResized));

  memset(&centerim_core_ui_ops, 0, sizeof(centerim_core_ui_ops));
  memset(&logbuf_debug_ui_ops, 0, sizeof(logbuf_debug_ui_ops));
  memset(&centerim_glib_eventloops, 0, sizeof(centerim_glib_eventloops));

  DeclareBindables();
}

void CenterIM::DeclareBindables()
{
  DeclareBindable(CONTEXT_CENTERIM, "quit",
      sigc::mem_fun(this, &CenterIM::Quit),
      InputProcessor::BINDABLE_OVERRIDE);
  DeclareBindable(CONTEXT_CENTERIM, "buddylist",
      sigc::mem_fun(this, &CenterIM::ActionFocusBuddyList),
      InputProcessor::BINDABLE_OVERRIDE);
  DeclareBindable(CONTEXT_CENTERIM, "conversation-active",
      sigc::mem_fun(this, &CenterIM::ActionFocusActiveConversation),
      InputProcessor::BINDABLE_OVERRIDE);
  DeclareBindable(CONTEXT_CENTERIM, "accountstatusmenu",
      sigc::mem_fun(this, &CenterIM::ActionOpenAccountStatusMenu),
      InputProcessor::BINDABLE_OVERRIDE);
  DeclareBindable(CONTEXT_CENTERIM, "generalmenu",
      sigc::mem_fun(this, &CenterIM::ActionOpenGeneralMenu),
      InputProcessor::BINDABLE_OVERRIDE);
  DeclareBindable(CONTEXT_CENTERIM, "conversation-prev",
      sigc::mem_fun(this, &CenterIM::ActionFocusPrevConversation),
      InputProcessor::BINDABLE_OVERRIDE);
  DeclareBindable(CONTEXT_CENTERIM, "conversation-next",
      sigc::mem_fun(this, &CenterIM::ActionFocusNextConversation),
      InputProcessor::BINDABLE_OVERRIDE);
}

DEFINE_SIG_REGISTERKEYS(CenterIM, RegisterKeys);
bool CenterIM::RegisterKeys()
{
  RegisterKeyDef(CONTEXT_CENTERIM, "quit", _("Quit CenterIM."),
      Keys::UnicodeTermKey("q", TERMKEY_KEYMOD_CTRL));
  RegisterKeyDef(CONTEXT_CENTERIM, "buddylist",
      _("Switch the focus to the buddy list."),
      Keys::FunctionTermKey(1));
  RegisterKeyDef(CONTEXT_CENTERIM, "conversation-active",
      _("Switch the focus to the active conversation."),
      Keys::FunctionTermKey(2));
  RegisterKeyDef(CONTEXT_CENTERIM, "accountstatusmenu",
      _("Open the account status menu."),
      Keys::FunctionTermKey(3));
  RegisterKeyDef(CONTEXT_CENTERIM, "generalmenu",
      _("Open the general menu."),
      Keys::FunctionTermKey(4));

  // XXX move to default key bindings config
  RegisterKeyDef(CONTEXT_CENTERIM, "generalmenu",
      _("Open the general menu."),
      Keys::UnicodeTermKey("g", TERMKEY_KEYMOD_CTRL));
  RegisterKeyDef(CONTEXT_CENTERIM, "generalmenu",
      _("Open the general menu."),
      Keys::UnicodeTermKey("4", TERMKEY_KEYMOD_ALT));

  RegisterKeyDef(CONTEXT_CENTERIM, "conversation-prev",
      _("Switch the focus to the previous conversation."),
      Keys::UnicodeTermKey("p", TERMKEY_KEYMOD_ALT));
  RegisterKeyDef(CONTEXT_CENTERIM, "conversation-next",
      _("Switch the focus to the next conversation."),
      Keys::UnicodeTermKey("n", TERMKEY_KEYMOD_ALT));
  return true;
}

int CenterIM::Run()
{
  if (PurpleInit())
    return 1;

  // initialize Conf component so we can calculate area sizes of all windows
  Conf::Init();

  ColorSchemeInit();

  // initialize Log component
  Log::Init();
  if (logbuf) {
    for (LogBufferItems::iterator i = logbuf->begin(); i != logbuf->end(); i++) {
      purple_debug(i->level, i->category, i->arg_s);
      g_free(i->category);
      g_free(i->arg_s);
    }

    delete logbuf;
    logbuf = NULL;
  }

  Accounts::Init();
  Connections::Init();
  Notify::Init();
  Request::Init();

  // initialize UI
  Conversations::Init();
  Header::Init();
  // init BuddyList last so it takes the focus
  BuddyList::Init();

  LOG->Info(_("Welcome to CenterIM 5. Press F4 to display main menu.\n"));

  mngr->SetTopInputProcessor(*this);
  mngr->EnableResizing();
  mngr->StartMainLoop();

  resize.disconnect();

  Conversations::Finalize();
  Header::Finalize();
  BuddyList::Finalize();

  Accounts::Finalize();
  Connections::Finalize();
  Notify::Finalize();
  Request::Finalize();

  Log::Finalize();

  Conf::Finalize();

  PurpleFinalize();

  return 0;
}

void CenterIM::Quit()
{
  mngr->QuitMainLoop();
}

Rect CenterIM::GetScreenAreaSize(ScreenArea area)
{
  return areaSizes[area];
}

int CenterIM::PurpleInit()
{
  // set the configuration file location
  char *path;
  path = g_build_filename(purple_home_dir(), CIM_CONFIG_PATH, NULL);
  purple_util_set_user_dir(path);
  g_free(path);

  /* This does not disable debugging, but rather it disables printing to
   * stdout. Don't change this to TRUE or things will get messy. */
  purple_debug_set_enabled(FALSE);

  /* This catches and buffers libpurple debug messages until the Log object
   * can be instantiated. */
  logbuf_debug_ui_ops.print = tmp_purple_print;
  logbuf_debug_ui_ops.is_enabled = tmp_is_enabled;
  purple_debug_set_ui_ops(&logbuf_debug_ui_ops);

  centerim_core_ui_ops.get_ui_info = get_ui_info;
  purple_core_set_ui_ops(&centerim_core_ui_ops);

  // set the uiops for the eventloop
  centerim_glib_eventloops.timeout_add = timeout_add;
  centerim_glib_eventloops.timeout_remove = timeout_remove;
  centerim_glib_eventloops.input_add = input_add;
  centerim_glib_eventloops.input_remove = input_remove;
  purple_eventloop_set_ui_ops(&centerim_glib_eventloops);

  // in case we ever write centerim specific plugins
  path = g_build_filename(purple_user_dir(), "plugins", NULL);
  purple_plugins_add_search_path(path);
  g_free(path);

  if (!purple_core_init(PACKAGE_NAME)) {
    // can't do much without libpurple
    fprintf(stderr, _("Could not initialize libpurple core.\n"));
    return 1;
  }
  return 0;
}

void CenterIM::PurpleFinalize()
{
  purple_core_set_ui_ops(NULL);
  //purple_eventloop_set_ui_ops(NULL);
  purple_core_quit();
}

void CenterIM::ColorSchemeInit()
{
  // default colors init
  /// @todo move this to a default cfg
  COLORSCHEME->SetColorPair("accountstatusmenu",   "panel",          "line",       Curses::Color::CYAN,    Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("accountstatusmenu",   "horizontalline", "line",       Curses::Color::CYAN,    Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("accountstatusmenu",   "button",         "normal",     Curses::Color::CYAN,    Curses::Color::BLACK);

  COLORSCHEME->SetColorPair("accountstatuspopup",  "panel",          "line",       Curses::Color::CYAN,    Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("accountstatuspopup",  "horizontalline", "line",       Curses::Color::CYAN,    Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("accountstatuspopup",  "button",         "normal",     Curses::Color::CYAN,    Curses::Color::BLACK);

  COLORSCHEME->SetColorPair("accountwindow",       "panel",          "line",       Curses::Color::CYAN,    Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("accountwindow",       "horizontalline", "line",       Curses::Color::CYAN,    Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("accountwindow",       "verticalline",   "line",       Curses::Color::CYAN,    Curses::Color::BLACK);

  COLORSCHEME->SetColorPair("buddylist",           "treeview",       "line",       Curses::Color::GREEN,   Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("buddylist",           "panel",          "line",       Curses::Color::BLUE,    Curses::Color::BLACK, Curses::Attr::BOLD);
  COLORSCHEME->SetColorPair("buddylist",           "button",         "normal",     Curses::Color::GREEN,   Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("buddylistgroup",      "button",         "normal",     Curses::Color::YELLOW,  Curses::Color::BLACK, Curses::Attr::BOLD);

  COLORSCHEME->SetColorPair("generalmenu",         "panel",          "line",       Curses::Color::CYAN,    Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("generalmenu",         "horizontalline", "line",       Curses::Color::CYAN,    Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("generalmenu",         "button",         "normal",     Curses::Color::CYAN,    Curses::Color::BLACK);

  COLORSCHEME->SetColorPair("log",                 "panel",          "line",       Curses::Color::BLUE,    Curses::Color::BLACK, Curses::Attr::BOLD);
  COLORSCHEME->SetColorPair("log",                 "textview",       "text",       Curses::Color::CYAN,    Curses::Color::BLACK);

  COLORSCHEME->SetColorPair("conversation",        "textview",       "text",       Curses::Color::MAGENTA, Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("conversation",        "textview",       "color1",     Curses::Color::CYAN,    Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("conversation",        "textview",       "color2",     Curses::Color::YELLOW,  Curses::Color::BLACK, Curses::Attr::BOLD);
  COLORSCHEME->SetColorPair("conversation",        "panel",          "line",       Curses::Color::BLUE,    Curses::Color::BLACK, Curses::Attr::BOLD);
  COLORSCHEME->SetColorPair("conversation",        "horizontalline", "line",       Curses::Color::BLUE,    Curses::Color::BLACK, Curses::Attr::BOLD);

  COLORSCHEME->SetColorPair("conversation",        "label",          "text",       Curses::Color::CYAN,    Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("conversation-active", "label",          "text",       Curses::Color::YELLOW,  Curses::Color::BLACK, Curses::Attr::BOLD);
  COLORSCHEME->SetColorPair("conversation-new",    "label",          "text",       Curses::Color::CYAN,    Curses::Color::BLACK, Curses::Attr::BOLD);

  COLORSCHEME->SetColorPair("header",              "label",          "text",       Curses::Color::BLACK,   Curses::Color::WHITE);
  COLORSCHEME->SetColorPair("header",              "container",      "background", Curses::Color::BLACK,   Curses::Color::WHITE);
}

void CenterIM::ScreenResized()
{
  // TODO make configurable
  Rect size;

  size.x = 0;
  size.y = 1;
  size.width = mngr->GetScreenWidth() / 4;
  size.height = mngr->GetScreenHeight() - 1;
  areaSizes[BUDDY_LIST_AREA] = size;

  size.x = areaSizes[BUDDY_LIST_AREA].width;
  size.width = mngr->GetScreenWidth() - size.x;
  size.height = 15;
  size.y = mngr->GetScreenHeight() - size.height;
  areaSizes[LOG_AREA] = size;

  size.x = areaSizes[BUDDY_LIST_AREA].width;
  size.y = 1;
  size.width = mngr->GetScreenWidth() - size.x;
  size.height = mngr->GetScreenHeight() - (size.y +
      areaSizes[LOG_AREA].height);
  areaSizes[CHAT_AREA] = size;

  size.x = 0;
  size.y = 0;
  size.width = mngr->GetScreenWidth();
  size.height = 1;
  areaSizes[HEADER_AREA] = size;

  size.x = 0;
  size.y = 0;
  size.width = mngr->GetScreenWidth();
  size.height = mngr->GetScreenHeight();
  areaSizes[WHOLE_AREA] = size;
}

GHashTable *CenterIM::get_ui_info()
{
  static GHashTable *ui_info = NULL;

  if (!ui_info) {
    ui_info = g_hash_table_new(g_str_hash, g_str_equal);

    g_hash_table_insert(ui_info, (void *) "name", (void *) PACKAGE_NAME);
    g_hash_table_insert(ui_info, (void *) "version",
        (void *) PACKAGE_VERSION);
    g_hash_table_insert(ui_info, (void *) "website",
        (void *) "http://www.centerim.org/");

    // TODO
    g_hash_table_insert(ui_info, (void *) "dev_website",
        (void *) "http://www.centerim.org/");
    g_hash_table_insert(ui_info, (void *) "client_type", (void *) "pc");
  }

  return ui_info;
}

guint CenterIM::timeout_add(guint interval, GSourceFunc function, gpointer data)
{
  return g_timeout_add(interval, function, data);
}

gboolean CenterIM::timeout_remove(guint handle)
{
  return g_source_remove(handle);
}

#define PURPLE_GLIB_READ_COND  (G_IO_IN | G_IO_HUP | G_IO_ERR)
#define PURPLE_GLIB_WRITE_COND (G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL)

guint CenterIM::input_add(int fd, PurpleInputCondition condition,
  PurpleInputFunction function, gpointer data)
{
  IOClosure *closure = new IOClosure;
  GIOChannel *channel;
  int cond = 0;

  closure->function = function;
  closure->data = data;

  if (condition & PURPLE_INPUT_READ)
    cond |= PURPLE_GLIB_READ_COND;
  if (condition & PURPLE_INPUT_WRITE)
    cond |= PURPLE_GLIB_WRITE_COND;

  channel = g_io_channel_unix_new(fd);
  closure->result = g_io_add_watch_full(channel, G_PRIORITY_DEFAULT,
      static_cast<GIOCondition>(cond), purple_glib_io_input, closure,
      purple_glib_io_destroy);

  g_io_channel_unref(channel);
  return closure->result;
}

gboolean CenterIM::input_remove(guint handle)
{
  return g_source_remove(handle);
}

gboolean CenterIM::purple_glib_io_input(GIOChannel *source,
    GIOCondition condition, gpointer data)
{
  IOClosure *closure = (IOClosure *) data;
  int purple_cond = 0;

  if (condition & PURPLE_GLIB_READ_COND)
    purple_cond |= PURPLE_INPUT_READ;
  if (condition & PURPLE_GLIB_WRITE_COND)
    purple_cond |= PURPLE_INPUT_WRITE;

  closure->function(closure->data, g_io_channel_unix_get_fd(source),
      static_cast<PurpleInputCondition>(purple_cond));

  return TRUE;
}

void CenterIM::purple_glib_io_destroy(gpointer data)
{
  delete (IOClosure *) data;
}

void CenterIM::tmp_purple_print(PurpleDebugLevel level, const char *category,
    const char *arg_s)
{
  if (!logbuf)
    logbuf = new std::vector<LogBufferItem>;

  LogBufferItem item;
  item.level = level;
  item.category = g_strdup(category);
  item.arg_s = g_strdup(arg_s);
  logbuf->push_back(item);
}

void CenterIM::ActionFocusBuddyList()
{
  BUDDYLIST->Show();
}

void CenterIM::ActionFocusActiveConversation()
{
  CONVERSATIONS->FocusActiveConversation();
}

void CenterIM::ActionOpenAccountStatusMenu()
{
  AccountStatusMenu *menu = new AccountStatusMenu;
  menu->Show();
}

void CenterIM::ActionOpenGeneralMenu()
{
  GeneralMenu *menu = new GeneralMenu;
  menu->Show();
}

void CenterIM::ActionFocusPrevConversation()
{
  CONVERSATIONS->FocusPrevConversation();
}

void CenterIM::ActionFocusNextConversation()
{
  CONVERSATIONS->FocusNextConversation();
}
