/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2011 by CenterIM developers
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
#include "Connections.h"
#include "Conversations.h"
#include "Footer.h"
#include "Header.h"
#include "Log.h"
#include "Notify.h"
#include "Request.h"
#include "Transfers.h"

#include "AccountStatusMenu.h"
#include "GeneralMenu.h"

#include <cppconsui/ColorScheme.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Keys.h>
#include <string.h>
#include "gettext.h"

#define CONF_PLUGIN_SAVE_PREF CONF_PREFIX "/plugins/loaded"

std::vector<CenterIM::LogBufferItem> *CenterIM::logbuf = NULL;

CenterIM *CenterIM::Instance()
{
  static CenterIM instance;
  return &instance;
}

bool CenterIM::ProcessInput(const TermKeyKey& key, bool more)
{
  purple_idle_touch();
  return InputProcessor::ProcessInput(key, more);
}

int CenterIM::Run(const char *config_path)
{
  char *path;
  if (config_path[0] == '/') {
    // absolute path
    path = g_strdup(config_path);
  }
  else
    path = g_build_filename(purple_home_dir(), config_path, NULL);

  if (PurpleInit(path))
    return 1;

  InitDefaultKeys();
  char *binds = g_build_filename(path, "binds.xml", NULL);
  KEYCONFIG->SetConfigFile(binds);
  g_free(binds);

  g_free(path);

  PrefsInit();
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

  /* Init key binds after the Log is initialized so the user can see if there
   * is an error in the config. */
  KEYCONFIG->Reconfig();

  Footer::Init();

  Accounts::Init();
  Connections::Init();
  Notify::Init();
  Request::Init();

  // initialize UI
  Conversations::Init();
  Header::Init();
  // init BuddyList last so it takes the focus
  BuddyList::Init();

  LOG->Info(_("Welcome to CenterIM 5. Press F4 to display main menu."));

  mngr->SetTopInputProcessor(*this);
  mngr->EnableResizing();
  mngr->StartMainLoop();

  purple_prefs_disconnect_by_handle(this);

  resize.disconnect();

  Conversations::Finalize();
  Header::Finalize();
  BuddyList::Finalize();

  Accounts::Finalize();
  Connections::Finalize();
  Notify::Finalize();
  Request::Finalize();

  Footer::Finalize();

  Log::Finalize();

  PurpleFinalize();

  return 0;
}

void CenterIM::Quit()
{
  mngr->QuitMainLoop();
}

CppConsUI::Rect CenterIM::GetScreenAreaSize(ScreenArea area)
{
  return areaSizes[area];
}

CenterIM::CenterIM()
{
  mngr = CppConsUI::CoreManager::Instance();
  resize = mngr->signal_resize.connect(sigc::mem_fun(this,
        &CenterIM::ScreenResized));

  memset(&centerim_core_ui_ops, 0, sizeof(centerim_core_ui_ops));
  memset(&logbuf_debug_ui_ops, 0, sizeof(logbuf_debug_ui_ops));
  memset(&centerim_glib_eventloops, 0, sizeof(centerim_glib_eventloops));

  DeclareBindables();
}

int CenterIM::PurpleInit(const char *config_path)
{
  g_assert(config_path);
  g_assert(config_path[0] == '/');

  purple_util_set_user_dir(config_path);

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
  char *path = g_build_filename(purple_user_dir(), "plugins", NULL);
  purple_plugins_add_search_path(path);
  g_free(path);

  if (!purple_core_init(PACKAGE_NAME)) {
    // can't do much without libpurple
    fprintf(stderr, _("Libpurple initialization failed."));
    return 1;
  }

  purple_prefs_add_none(CONF_PREFIX);

  // load the desired plugins
  if (purple_prefs_exists(CONF_PLUGIN_SAVE_PREF))
      purple_plugins_load_saved(CONF_PLUGIN_SAVE_PREF);

  return 0;
}

void CenterIM::PurpleFinalize()
{
  purple_plugins_save_loaded(CONF_PLUGIN_SAVE_PREF);

  purple_core_set_ui_ops(NULL);
  //purple_eventloop_set_ui_ops(NULL);
  purple_core_quit();
}

void CenterIM::PrefsInit()
{
  // remove someday...
  if (purple_prefs_exists("/centerim"))
    purple_prefs_rename("/centerim", CONF_PREFIX);

  // init prefs
  purple_prefs_add_none(CONF_PREFIX "/dimensions");
  purple_prefs_add_int(CONF_PREFIX "/dimensions/buddylist_width", 20);
  purple_prefs_add_int(CONF_PREFIX "/dimensions/log_height", 25);
  purple_prefs_connect_callback(this, CONF_PREFIX "/dimensions",
      dimensions_change_, this);
}

void CenterIM::ColorSchemeInit()
{
  // default colors init
  /// @todo move this to a default cfg
  COLORSCHEME->SetColorPair("accountstatusmenu", "panel", "line",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("accountstatusmenu", "horizontalline", "line",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("accountstatusmenu", "button", "normal",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::BLACK);

  COLORSCHEME->SetColorPair("buddylist", "treeview", "line",
      CppConsUI::Curses::Color::GREEN, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("buddylist", "panel", "line",
      CppConsUI::Curses::Color::BLUE, CppConsUI::Curses::Color::BLACK,
      CppConsUI::Curses::Attr::BOLD);
  COLORSCHEME->SetColorPair("buddylist", "button", "normal",
      CppConsUI::Curses::Color::GREEN, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("buddylistgroup", "button", "normal",
      CppConsUI::Curses::Color::YELLOW, CppConsUI::Curses::Color::BLACK,
      CppConsUI::Curses::Attr::BOLD);

  COLORSCHEME->SetColorPair("conversation", "textview", "text",
      CppConsUI::Curses::Color::MAGENTA, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("conversation", "textview", "color1",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("conversation", "textview", "color2",
      CppConsUI::Curses::Color::YELLOW, CppConsUI::Curses::Color::BLACK,
      CppConsUI::Curses::Attr::BOLD);
  COLORSCHEME->SetColorPair("conversation", "panel", "line",
      CppConsUI::Curses::Color::BLUE, CppConsUI::Curses::Color::BLACK,
      CppConsUI::Curses::Attr::BOLD);
  COLORSCHEME->SetColorPair("conversation", "horizontalline", "line",
      CppConsUI::Curses::Color::BLUE, CppConsUI::Curses::Color::BLACK,
      CppConsUI::Curses::Attr::BOLD);
  COLORSCHEME->SetColorPair("conversation", "textedit", "text",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::BLACK);

  COLORSCHEME->SetColorPair("conversation", "label", "text",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("conversation-active", "label", "text",
      CppConsUI::Curses::Color::YELLOW, CppConsUI::Curses::Color::BLACK,
      CppConsUI::Curses::Attr::BOLD);
  COLORSCHEME->SetColorPair("conversation-new", "label", "text",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::BLACK,
      CppConsUI::Curses::Attr::BOLD);

  COLORSCHEME->SetColorPair("footer", "label", "text",
      CppConsUI::Curses::Color::BLACK, CppConsUI::Curses::Color::WHITE);
  COLORSCHEME->SetColorPair("footer", "container", "background",
      CppConsUI::Curses::Color::BLACK, CppConsUI::Curses::Color::WHITE);

  COLORSCHEME->SetColorPair("generalmenu", "panel", "line",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("generalmenu", "horizontalline", "line",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("generalmenu", "button", "normal",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::BLACK);

  COLORSCHEME->SetColorPair("generalwindow", "panel", "line",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("generalwindow", "horizontalline", "line",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("generalwindow", "verticalline", "line",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::BLACK);

  COLORSCHEME->SetColorPair("log", "panel", "line",
      CppConsUI::Curses::Color::BLUE, CppConsUI::Curses::Color::BLACK,
      CppConsUI::Curses::Attr::BOLD);
  COLORSCHEME->SetColorPair("log", "textview", "text",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::BLACK);

  COLORSCHEME->SetColorPair("header", "label", "text",
      CppConsUI::Curses::Color::BLACK, CppConsUI::Curses::Color::WHITE);
  COLORSCHEME->SetColorPair("header", "container", "background",
      CppConsUI::Curses::Color::BLACK, CppConsUI::Curses::Color::WHITE);
}

void CenterIM::ScreenResized()
{
  CppConsUI::Rect size;

  // minimal supported screen size is 80x24
  int screen_width = CppConsUI::Curses::getmaxx();
  if (screen_width < 80)
    screen_width = 80;
  int screen_height = CppConsUI::Curses::getmaxy();
  if (screen_height < 24)
    screen_height = 24;

  int buddylist_width = purple_prefs_get_int(CONF_PREFIX
      "/dimensions/buddylist_width");
  buddylist_width = CLAMP(buddylist_width, 0, 50);
  int log_height = purple_prefs_get_int(CONF_PREFIX "/dimensions/log_height");
  log_height = CLAMP(log_height, 0, 50);

  size.x = 0;
  size.y = 1;
  size.width = screen_width / 100.0 * buddylist_width;
  size.height = screen_height - 2;
  areaSizes[BUDDY_LIST_AREA] = size;

  size.x = areaSizes[BUDDY_LIST_AREA].width;
  size.width = screen_width - size.x;
  size.height = screen_height / 100.0 * log_height;
  size.y = screen_height - size.height - 1;
  areaSizes[LOG_AREA] = size;

  size.x = areaSizes[BUDDY_LIST_AREA].width;
  size.y = 1;
  size.width = screen_width - size.x;
  size.height = screen_height - size.y - areaSizes[LOG_AREA].height - 1;
  areaSizes[CHAT_AREA] = size;

  size.x = 0;
  size.y = 0;
  size.width = screen_width;
  size.height = 1;
  areaSizes[HEADER_AREA] = size;

  size.x = 0;
  size.y = screen_height - 1;
  size.width = screen_width;
  size.height = 1;
  areaSizes[FOOTER_AREA] = size;

  size.x = 0;
  size.y = 0;
  size.width = screen_width;
  size.height = screen_height;
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

void CenterIM::dimensions_change(const char *name, PurplePrefType type,
    gconstpointer val)
{
  mngr->ScreenResized();
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
  /* Don't allow to open the account status menu if there is any 'top' window
   * (except general menu, we can close that). */
  CppConsUI::FreeWindow *top = mngr->GetTopWindow();
  if (top) {
    if (dynamic_cast<GeneralMenu*>(top))
      top->Close();
    else if (top->GetType() == CppConsUI::FreeWindow::TYPE_TOP)
      return;
  }

  AccountStatusMenu *menu = new AccountStatusMenu;
  menu->Show();
}

void CenterIM::ActionOpenGeneralMenu()
{
  /* Don't allow to open the general menu if there is any 'top' window (except
   * account status menu, we can close that). */
  CppConsUI::FreeWindow *top = mngr->GetTopWindow();
  if (top) {
    if (dynamic_cast<AccountStatusMenu*>(top))
      top->Close();
    else if (top->GetType() == CppConsUI::FreeWindow::TYPE_TOP)
      return;
  }

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

void CenterIM::DeclareBindables()
{
  DeclareBindable("centerim", "quit",
      sigc::mem_fun(this, &CenterIM::Quit),
      InputProcessor::BINDABLE_OVERRIDE);
  DeclareBindable("centerim", "buddylist",
      sigc::mem_fun(this, &CenterIM::ActionFocusBuddyList),
      InputProcessor::BINDABLE_OVERRIDE);
  DeclareBindable("centerim", "conversation-active",
      sigc::mem_fun(this, &CenterIM::ActionFocusActiveConversation),
      InputProcessor::BINDABLE_OVERRIDE);
  DeclareBindable("centerim", "accountstatusmenu",
      sigc::mem_fun(this, &CenterIM::ActionOpenAccountStatusMenu),
      InputProcessor::BINDABLE_OVERRIDE);
  DeclareBindable("centerim", "generalmenu",
      sigc::mem_fun(this, &CenterIM::ActionOpenGeneralMenu),
      InputProcessor::BINDABLE_OVERRIDE);
  DeclareBindable("centerim", "conversation-prev",
      sigc::mem_fun(this, &CenterIM::ActionFocusPrevConversation),
      InputProcessor::BINDABLE_OVERRIDE);
  DeclareBindable("centerim", "conversation-next",
      sigc::mem_fun(this, &CenterIM::ActionFocusNextConversation),
      InputProcessor::BINDABLE_OVERRIDE);
}

void CenterIM::InitDefaultKeys()
{
  KEYCONFIG->AddDefaultKeyBind("centerim", "quit", "Ctrl-q");
  KEYCONFIG->AddDefaultKeyBind("centerim", "buddylist", "F1");
  KEYCONFIG->AddDefaultKeyBind("centerim", "conversation-active", "F2");
  KEYCONFIG->AddDefaultKeyBind("centerim", "accountstatusmenu", "F3");
  KEYCONFIG->AddDefaultKeyBind("centerim", "generalmenu", "F4");

  KEYCONFIG->AddDefaultKeyBind("centerim", "buddylist", "Alt-1");
  KEYCONFIG->AddDefaultKeyBind("centerim", "conversation-active", "Alt-2");
  KEYCONFIG->AddDefaultKeyBind("centerim", "accountstatusmenu", "Alt-3");
  KEYCONFIG->AddDefaultKeyBind("centerim", "generalmenu", "Alt-4");
  KEYCONFIG->AddDefaultKeyBind("centerim", "generalmenu", "Ctrl-g");

  KEYCONFIG->AddDefaultKeyBind("centerim", "conversation-prev", "Ctrl-p");
  KEYCONFIG->AddDefaultKeyBind("centerim", "conversation-next", "Ctrl-n");

  KEYCONFIG->AddDefaultKeyBind("buddylist", "contextmenu", "Ctrl-d");

  KEYCONFIG->AddDefaultKeyBind("conversation", "send", "Ctrl-x");
}
