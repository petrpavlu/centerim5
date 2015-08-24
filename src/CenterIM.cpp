/*
 * Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
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
#include <cstdio>
#include <locale.h>
#include <errno.h>
#include <getopt.h>
#include <glib/gprintf.h>
#include <time.h>
#include <typeinfo>
#include "gettext.h"

#define CIM_CONFIG_PATH ".centerim5"

// program's entry point
int main(int argc, char *argv[])
{
  return CenterIM::run(argc, argv);
}

const char *CenterIM::color_names[] = {
  "default", // -1
  "black",   //  0
  "red",     //  1
  "green",   //  2
  "yellow",  //  3
  "blue",    //  4
  "magenta", //  5
  "cyan",    //  6
  "white",   //  7
};

const char *CenterIM::scheme_names[] = {
  NULL,
  "accountstatusmenu",        // SCHEME_ACCOUNTSTATUSMENU
  "buddylist",                // SCHEME_BUDDYLIST
  "buddylistbuddy",           // SCHEME_BUDDYLISTBUDDY
  "buddylistbuddy_away",      // SCHEME_BUDDYLISTBUDDY_AWAY
  "buddylistbuddy_na",        // SCHEME_BUDDYLISTBUDDY_NA
  "buddylistbuddy_offline",   // SCHEME_BUDDYLISTBUDDY_OFFLINE
  "buddylistbuddy_online",    // SCHEME_BUDDYLISTBUDDY_ONLINE
  "buddylistchat",            // SCHEME_BUDDYLISTCHAT
  "buddylistcontact",         // SCHEME_BUDDYLISTCONTACT
  "buddylistcontact_away",    // SCHEME_BUDDYLISTCONTACT_AWAY
  "buddylistcontact_na",      // SCHEME_BUDDYLISTCONTACT_NA
  "buddylistcontact_offline", // SCHEME_BUDDYLISTCONTACT_OFFLINE
  "buddylistcontact_online",  // SCHEME_BUDDYLISTCONTACT_ONLINE
  "buddylistgroup",           // SCHEME_BUDDYLISTGROUP
  "conversation",             // SCHEME_CONVERSATION
  "conversation_active",      // SCHEME_CONVERSATION_ACTIVE
  "conversation_new",         // SCHEME_CONVERSATION_NEW
  "footer",                   // SCHEME_FOOTER
  "generalmenu",              // SCHEME_GENERALMENU
  "generalwindow",            // SCHEME_GENERALWINDOW
  "header",                   // SCHEME_HEADER
  "header_request",           // SCHEME_HEADER_REQUEST
  "log",                      // SCHEME_LOG
};

CenterIM *CenterIM::my_instance = NULL;

// based on glibmm code
class SourceConnectionNode {
public:
  explicit inline SourceConnectionNode(const sigc::slot_base &nslot);

  static void *notify(void *data);
  static void destroy_notify_callback(void *data);
  static gboolean source_callback(void *data);

  inline void install(GSource *nsource);
  inline sigc::slot_base *get_slot();

private:
  sigc::slot_base slot;
  GSource *source;
};

inline SourceConnectionNode::SourceConnectionNode(const sigc::slot_base &nslot)
  : slot(nslot), source(0)
{
  slot.set_parent(this, &SourceConnectionNode::notify);
}

void *SourceConnectionNode::notify(void *data)
{
  SourceConnectionNode *self = reinterpret_cast<SourceConnectionNode *>(data);

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
  SourceConnectionNode *self = reinterpret_cast<SourceConnectionNode *>(data);

  if (self) {
    /* The GLib side is disconnected now, thus the GSource* is no longer
     * valid. */
    self->source = 0;

    delete self;
  }
}

gboolean SourceConnectionNode::source_callback(void *data)
{
  SourceConnectionNode *conn_data =
    reinterpret_cast<SourceConnectionNode *>(data);

  // recreate the specific slot from the generic slot node
  return (*static_cast<sigc::slot<bool> *>(conn_data->get_slot()))();
}

inline void SourceConnectionNode::install(GSource *nsource)
{
  source = nsource;
}

inline sigc::slot_base *SourceConnectionNode::get_slot()
{
  return &slot;
}

CenterIM *CenterIM::instance()
{
  return my_instance;
}

bool CenterIM::processInput(const TermKeyKey &key)
{
  if (idle_reporting_on_keyboard)
    purple_idle_touch();
  return InputProcessor::processInput(key);
}

void CenterIM::quit()
{
  g_main_loop_quit(mainloop);
}

CppConsUI::Rect CenterIM::getScreenArea(ScreenArea area)
{
  return areas[area];
}

CppConsUI::Rect CenterIM::getScreenAreaCentered(ScreenArea area)
{
  CppConsUI::Rect s = areas[WHOLE_AREA];
  CppConsUI::Rect r = areas[area];
  int x = (s.width - r.width) / 2;
  int y = (s.height - r.height) / 2;
  return CppConsUI::Rect(x, y, r.width, r.height);
}

bool CenterIM::loadColorSchemeConfig()
{
  xmlnode *root =
    purple_util_read_xml_from_file("colorschemes.xml", _("color schemes"));

  if (root == NULL) {
    // Read error, first time run?
    loadDefaultColorSchemeConfig();
    if (saveColorSchemeConfig())
      return true;
    return false;
  }

  COLORSCHEME->clear();
  bool res = false;

  for (xmlnode *scheme_node = xmlnode_get_child(root, "scheme");
       scheme_node != NULL; scheme_node = xmlnode_get_next_twin(scheme_node)) {
    const char *scheme_name = xmlnode_get_attrib(scheme_node, "name");
    if (scheme_name == NULL) {
      LOG->error(_("Missing 'name' attribute in the scheme definition."));
      goto out;
    }

    int scheme = stringToScheme(scheme_name);
    if (scheme == 0) {
      LOG->error(_("Unrecognized scheme '%s'."), scheme_name);
      goto out;
    }

    for (xmlnode *color = xmlnode_get_child(scheme_node, "color"); color;
         color = xmlnode_get_next_twin(color)) {
      const char *widget_string = xmlnode_get_attrib(color, "widget");
      if (widget_string == NULL) {
        LOG->error(_("Missing 'widget' attribute in the color definition."));
        goto out;
      }

      const char *property_string = xmlnode_get_attrib(color, "property");
      if (property_string == NULL) {
        LOG->error(_("Missing 'property' attribute in the color definition."));
        goto out;
      }

      CppConsUI::ColorScheme::PropertyConversionResult conv_res;
      int property;
      int subproperty;
      conv_res = COLORSCHEME->stringPairToPropertyPair(
        widget_string, property_string, &property, &subproperty);
      switch (conv_res) {
      case CppConsUI::ColorScheme::CONVERSION_SUCCESS:
        break;
      case CppConsUI::ColorScheme::CONVERSION_ERROR_WIDGET:
        LOG->error(_("Unrecognized widget '%s'."), widget_string);
        goto out;
      case CppConsUI::ColorScheme::CONVERSION_ERROR_PROPERTY:
        LOG->error(_("Unrecognized property '%s'."), property_string);
        goto out;
      default:
        g_assert_not_reached();
      }

      const char *fg_string = xmlnode_get_attrib(color, "foreground");
      const char *bg_string = xmlnode_get_attrib(color, "background");
      const char *attrs_string = xmlnode_get_attrib(color, "attributes");

      int fg = CppConsUI::Curses::Color::DEFAULT;
      int bg = CppConsUI::Curses::Color::DEFAULT;
      int attrs = 0;

      if (fg_string != NULL && !stringToColor(fg_string, &fg)) {
        LOG->error(_("Unrecognized color '%s'."), fg_string);
        goto out;
      }

      if (bg_string != NULL && !stringToColor(bg_string, &bg)) {
        LOG->error(_("Unrecognized color '%s'."), bg_string);
        goto out;
      }

      if (attrs_string != NULL &&
        !stringToColorAttributes(attrs_string, &attrs)) {
        LOG->error(_("Unrecognized attributes '%s'."), attrs_string);
        goto out;
      }

      COLORSCHEME->setAttributes(scheme, property, subproperty, fg, bg, attrs);
    }
  }

  res = true;

out:
  if (!res) {
    LOG->error(_("Error parsing 'colorschemes.xml', "
                 "loading default color scheme."));
    loadDefaultColorSchemeConfig();
  }

  xmlnode_free(root);

  return res;
}

bool CenterIM::loadKeyConfig()
{
  xmlnode *root =
    purple_util_read_xml_from_file("binds.xml", _("key bindings"));

  if (!root) {
    // read error, first time run?
    loadDefaultKeyConfig();
    if (saveKeyConfig())
      return true;
    return false;
  }

  KEYCONFIG->clear();
  bool res = false;

  for (xmlnode *bind = xmlnode_get_child(root, "bind"); bind;
       bind = xmlnode_get_next_twin(bind)) {
    const char *context = xmlnode_get_attrib(bind, "context");
    if (!context) {
      LOG->error(_("Missing 'context' attribute in the bind definition."));
      goto out;
    }
    const char *action = xmlnode_get_attrib(bind, "action");
    if (!action) {
      LOG->error(_("Missing 'action' attribute in the bind definition."));
      goto out;
    }
    const char *key = xmlnode_get_attrib(bind, "key");
    if (!key) {
      LOG->error(_("Missing 'key' attribute in the bind definition."));
      goto out;
    }

    if (!KEYCONFIG->bindKey(context, action, key)) {
      LOG->error(_("Unrecognized key '%s'."), key);
      goto out;
    }
  }

  res = true;

out:
  if (!res) {
    LOG->error(_("Error parsing 'binds.xml', loading default keys."));
    loadDefaultKeyConfig();
  }

  xmlnode_free(root);

  return res;
}

sigc::connection CenterIM::timeoutConnect(
  const sigc::slot<bool> &slot, unsigned interval, int priority)
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

sigc::connection CenterIM::timeoutOnceConnect(
  const sigc::slot<void> &slot, unsigned interval, int priority)
{
  return timeoutConnect(sigc::bind_return(slot, FALSE), interval, priority);
}

CenterIM::CenterIM()
  : mainloop(NULL), mngr(NULL), convs_expanded(false),
    idle_reporting_on_keyboard(false)
{
  memset(&centerim_core_ui_ops, 0, sizeof(centerim_core_ui_ops));
  memset(&logbuf_debug_ui_ops, 0, sizeof(logbuf_debug_ui_ops));
  memset(&centerim_glib_eventloops, 0, sizeof(centerim_glib_eventloops));
}

int CenterIM::run(int argc, char *argv[])
{
  // init CenterIM
  g_assert(!my_instance);
  my_instance = new CenterIM;

  // run CenterIM
  int res = my_instance->runAll(argc, argv);

  // finalize CenterIM
  g_assert(my_instance);

  delete my_instance;
  my_instance = NULL;

  return res;
}

int CenterIM::runAll(int argc, char *argv[])
{
  int res = 1;
  bool purple_initialized = false;
  bool cppconsui_input_initialized = false;
  bool cppconsui_output_initialized = false;
  bool cppconsui_screen_resizing_initialized = false;
  CppConsUI::Error error;

  // set GLib program name
  g_set_prgname(PACKAGE_NAME);

  setlocale(LC_ALL, "");
  tzset();

#if ENABLE_NLS
  bindtextdomain(PACKAGE_NAME, LOCALEDIR);
  bind_textdomain_codeset(PACKAGE_NAME, "UTF-8");
  bind_textdomain_codeset("pidgin", "UTF-8");
  textdomain(PACKAGE_NAME);
#endif

  signal(SIGPIPE, SIG_IGN);

  // parse command-line arguments
  bool ascii = false;
  bool offline = false;
  const char *config_path = CIM_CONFIG_PATH;
  int opt;
  // clang-format off
  struct option long_options[] = {
    {"ascii",   no_argument,       NULL, 'a'},
    {"help",    no_argument,       NULL, 'h'},
    {"version", no_argument,       NULL, 'v'},
    {"basedir", required_argument, NULL, 'b'},
    {"offline", no_argument,       NULL, 'o'},
    {NULL,      0,                 NULL,  0 }
  };
  // clang-format on

  while ((opt = getopt_long(argc, argv, "ahvb:o", long_options, NULL)) != -1) {
    switch (opt) {
    case 'a':
      ascii = true;
      break;
    case 'h':
      printUsage(stdout, argv[0]);
      return 0;
    case 'v':
      printVersion(stdout);
      return 0;
    case 'b':
      config_path = optarg;
      break;
    case 'o':
      offline = true;
      break;
    default:
      printUsage(stderr, argv[0]);
      return 1;
    }
  }

  if (optind < argc) {
    std::fprintf(stderr, _("%s: unexpected argument after options\n"), argv[0]);
    printUsage(stderr, argv[0]);
    return 1;
  }

  /* Initialize the internal logger. It will buffer all messages produced by
   * GLib, libpurple, or CppConsUI until it is possible to output them on the
   * screen (in the log window). If any part of the initialization fails the
   * buffered messages will be printed on stderr. */
  Log::init();

  // create the main loop
  mainloop = g_main_loop_new(NULL, FALSE);

  // Initialize CppConsUI.
  CppConsUI::AppInterface interface = {timeout_add_cppconsui,
    timeout_remove_cppconsui, input_add_cppconsui, input_remove_cppconsui,
    log_error_cppconsui};
  CppConsUI::initializeConsUI(interface);

  // Get the CoreManager instance.
  mngr = CppConsUI::getCoreManagerInstance();
  g_assert(mngr);

  // Initialize CoreManager's input, output and screen resizing.
  if (mngr->initializeInput(error) != 0) {
    LOG->error("%s\n", error.getString());
    goto out;
  }
  cppconsui_input_initialized = true;
  if (mngr->initializeOutput(error) != 0) {
    LOG->error("%s\n", error.getString());
    goto out;
  }
  cppconsui_output_initialized = true;
  if (mngr->initializeScreenResizing(error) != 0) {
    LOG->error("%s\n", error.getString());
    goto out;
  }
  cppconsui_screen_resizing_initialized = true;

  // ASCII mode
  if (ascii)
    CppConsUI::Curses::setAsciiMode(ascii);

  // Register for some signals.
  resize_conn = mngr->signal_resize.connect(
    sigc::mem_fun(this, &CenterIM::onScreenResized));
  top_window_change_conn = mngr->signal_top_window_change.connect(
    sigc::mem_fun(this, &CenterIM::onTopWindowChanged));

  // declare CenterIM bindables
  declareBindables();

  // initialize libpurple
  if (purpleInit(config_path)) {
    LOG->error(_("Libpurple initialization failed."));
    goto out;
  }
  purple_initialized = true;

  // initialize global preferences
  prefsInit();

  // initialize the log window
  LOG->initPhase2();

  /* Init colorschemes and keybinds after the Log is initialized so the user
   * can see if there is any error in the configs. */
  loadColorSchemeConfig();
  loadKeyConfig();

  Footer::init();

  Accounts::init();
  Connections::init();
  Notify::init();
  Request::init();

  // initialize UI
  Conversations::init();
  Header::init();
  // init BuddyList last so it takes the focus
  BuddyList::init();

  LOG->info(_("Welcome to CenterIM 5. Press %s to display main menu."),
    KEYCONFIG->getKeyBind("centerim", "generalmenu"));

  // restore last know status on all accounts
  ACCOUNTS->restoreStatuses(offline);

  mngr->setTopInputProcessor(*this);
  mngr->enableResizing();

  // start the main loop
  g_main_loop_run(mainloop);

  purple_prefs_disconnect_by_handle(this);

  resize_conn.disconnect();
  top_window_change_conn.disconnect();

  Conversations::finalize();
  Header::finalize();
  BuddyList::finalize();

  Accounts::finalize();
  Connections::finalize();
  Notify::finalize();
  Request::finalize();

  Footer::finalize();

  LOG->finalizePhase2();

  // everything went ok
  res = 0;

out:
  // finalize libpurple
  if (purple_initialized)
    purpleFinalize();

  // Finalize CoreManager's input, output and screen resizing.
  if (cppconsui_screen_resizing_initialized &&
    mngr->finalizeScreenResizing(error) != 0)
    LOG->error("%s\n", error.getString());
  if (cppconsui_output_initialized && mngr->finalizeOutput(error) != 0)
    LOG->error("%s\n", error.getString());
  if (cppconsui_input_initialized && mngr->finalizeInput(error) != 0)
    LOG->error("%s\n", error.getString());

  // Finalize CppConsUI.
  CppConsUI::finalizeConsUI();

  // destroy the main loop
  if (mainloop)
    g_main_loop_unref(mainloop);

  /* Finalize the log component. It will output all buffered messages (if
   * there are any) on stderr. */
  Log::finalize();

  return res;
}

void CenterIM::printUsage(FILE *out, const char *prg_name)
{
  // clang-format off
  std::fprintf(out, _(
"Usage: %s [option]...\n\n"
"Options:\n"
"  -a, --ascii                use ASCII characters to draw lines and boxes\n"
"  -h, --help                 display command line usage\n"
"  -v, --version              show the program version info\n"
"  -b, --basedir <directory>  specify another base directory\n"
"  -o, --offline              start with all accounts set offline\n"),
    prg_name);
  // clang-format on
}

void CenterIM::printVersion(FILE *out)
{
  std::fprintf(out, "CenterIM %s\n", version);
}

int CenterIM::purpleInit(const char *config_path)
{
  g_assert(config_path);

  // build config path
  if (g_path_is_absolute(config_path)) {
    // absolute path
    purple_util_set_user_dir(config_path);
  }
  else {
    char *path = g_build_filename(purple_home_dir(), config_path, NULL);
    g_assert(g_path_is_absolute(path));
    purple_util_set_user_dir(path);
    g_free(path);
  }

  /* This does not disable debugging, but rather it disables printing to
   * stdout. Don't change this to TRUE or things will get messy. */
  purple_debug_set_enabled(FALSE);

  // catch libpurple messages
  logbuf_debug_ui_ops.print = purple_print;
  logbuf_debug_ui_ops.is_enabled = purple_is_enabled;
  purple_debug_set_ui_ops(&logbuf_debug_ui_ops);

  // set core uiops
  centerim_core_ui_ops.get_ui_info = get_ui_info;
  purple_core_set_ui_ops(&centerim_core_ui_ops);

  // set the uiops for the eventloop
  centerim_glib_eventloops.timeout_add = g_timeout_add;
  centerim_glib_eventloops.timeout_remove = g_source_remove;
  centerim_glib_eventloops.input_add = input_add_purple;
  centerim_glib_eventloops.input_remove = g_source_remove;
  purple_eventloop_set_ui_ops(&centerim_glib_eventloops);

  // search user-specific plugins
  char *path = g_build_filename(purple_user_dir(), "plugins", NULL);
  purple_plugins_add_search_path(path);
  g_free(path);

  // search centerim-specific plugins
  purple_plugins_add_search_path(PKGLIBDIR);

  if (!purple_core_init(PACKAGE_NAME))
    return 1;

  purple_prefs_add_none(CONF_PREFIX);
  purple_prefs_add_none(CONF_PLUGINS_PREF);

  // load the desired plugins
  if (purple_prefs_exists(CONF_PLUGINS_SAVE_PREF))
    purple_plugins_load_saved(CONF_PLUGINS_SAVE_PREF);

  return 0;
}

void CenterIM::purpleFinalize()
{
  purple_plugins_save_loaded(CONF_PLUGINS_SAVE_PREF);

  purple_core_set_ui_ops(NULL);
  // purple_eventloop_set_ui_ops(NULL);
  purple_core_quit();
}

void CenterIM::prefsInit()
{
  // remove someday...
  if (purple_prefs_exists("/centerim"))
    purple_prefs_rename("/centerim", CONF_PREFIX);

  // init prefs
  purple_prefs_add_none(CONF_PREFIX "/dimensions");
  purple_prefs_add_int(CONF_PREFIX "/dimensions/buddylist_width", 20);
  purple_prefs_add_int(CONF_PREFIX "/dimensions/log_height", 25);
  purple_prefs_add_bool(CONF_PREFIX "/dimensions/show_header", true);
  purple_prefs_add_bool(CONF_PREFIX "/dimensions/show_footer", true);
  purple_prefs_connect_callback(
    this, CONF_PREFIX "/dimensions", dimensions_change_, this);

  purple_prefs_connect_callback(
    this, "/purple/away/idle_reporting", idle_reporting_change_, this);
  /* Proceed the callback. Note: This potentially triggers other callbacks
   * inside libpurple. */
  purple_prefs_trigger_callback("/purple/away/idle_reporting");
}

void CenterIM::onScreenResized()
{
  CppConsUI::Rect size;

  int screen_width = CppConsUI::Curses::getWidth();
  int screen_height = CppConsUI::Curses::getHeight();

  int buddylist_width;
  int log_height;
  if (convs_expanded) {
    buddylist_width = 0;
    log_height = 0;
  }
  else {
    buddylist_width =
      purple_prefs_get_int(CONF_PREFIX "/dimensions/buddylist_width");
    buddylist_width = CLAMP(buddylist_width, 0, 50);
    log_height = purple_prefs_get_int(CONF_PREFIX "/dimensions/log_height");
    log_height = CLAMP(log_height, 0, 50);
  }

  bool show_header =
    purple_prefs_get_bool(CONF_PREFIX "/dimensions/show_header");
  int header_height;
  if (show_header)
    header_height = 1;
  else
    header_height = 0;
  bool show_footer =
    purple_prefs_get_bool(CONF_PREFIX "/dimensions/show_footer");
  int footer_height;
  if (show_footer)
    footer_height = 1;
  else
    footer_height = 0;

  size.x = 0;
  size.y = header_height;
  size.width = screen_width / 100.0 * buddylist_width;
  size.height = screen_height - header_height - footer_height;
  areas[BUDDY_LIST_AREA] = size;

  size.x = areas[BUDDY_LIST_AREA].width;
  size.width = screen_width - size.x;
  size.height = screen_height / 100.0 * log_height;
  size.y = screen_height - size.height - footer_height;
  areas[LOG_AREA] = size;

  size.x = areas[BUDDY_LIST_AREA].width;
  size.y = header_height;
  size.width = screen_width - size.x;
  size.height = screen_height - size.y - areas[LOG_AREA].height - footer_height;
  if (convs_expanded) {
    size.x -= 2;
    size.width += 4;
  }
  areas[CHAT_AREA] = size;

  size.x = 0;
  size.y = 0;
  size.width = screen_width;
  size.height = header_height;
  areas[HEADER_AREA] = size;

  size.x = 0;
  size.y = screen_height - 1;
  size.width = screen_width;
  size.height = footer_height;
  areas[FOOTER_AREA] = size;

  size.x = 0;
  size.y = 0;
  size.width = screen_width;
  size.height = screen_height;
  areas[WHOLE_AREA] = size;
}

void CenterIM::onTopWindowChanged()
{
  if (!convs_expanded)
    return;

  CppConsUI::Window *top = mngr->getTopWindow();
  if (top && typeid(Conversation) != typeid(*top)) {
    convs_expanded = false;
    CONVERSATIONS->setExpandedConversations(convs_expanded);
    mngr->onScreenResized();
  }
}

#define GLIB_IO_READ_COND (G_IO_IN | G_IO_HUP | G_IO_ERR)
#define GLIB_IO_WRITE_COND (G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL)

guint CenterIM::input_add_purple(int fd, PurpleInputCondition condition,
  PurpleInputFunction function, gpointer data)
{
  IOClosurePurple *closure = new IOClosurePurple;
  GIOChannel *channel;
  int cond = 0;

  closure->function = function;
  closure->data = data;

  if (condition & PURPLE_INPUT_READ)
    cond |= GLIB_IO_READ_COND;
  if (condition & PURPLE_INPUT_WRITE)
    cond |= GLIB_IO_WRITE_COND;

  channel = g_io_channel_unix_new(fd);
  closure->result = g_io_add_watch_full(channel, G_PRIORITY_DEFAULT,
    static_cast<GIOCondition>(cond), io_input_purple, closure,
    io_destroy_purple);

  g_io_channel_unref(channel);
  return closure->result;
}

gboolean CenterIM::io_input_purple(
  GIOChannel *source, GIOCondition condition, gpointer data)
{
  IOClosurePurple *closure = static_cast<IOClosurePurple *>(data);
  int purple_cond = 0;

  if (condition & G_IO_IN)
    purple_cond |= PURPLE_INPUT_READ;
  if (condition & G_IO_OUT)
    purple_cond |= PURPLE_INPUT_WRITE;

  closure->function(closure->data, g_io_channel_unix_get_fd(source),
    static_cast<PurpleInputCondition>(purple_cond));

  return TRUE;
}

void CenterIM::io_destroy_purple(gpointer data)
{
  delete static_cast<IOClosurePurple *>(data);
}

unsigned CenterIM::input_add_cppconsui(int fd,
  CppConsUI::InputCondition condition, CppConsUI::InputFunction function,
  void *data)
{
  IOClosureCppConsUI *closure = new IOClosureCppConsUI;
  GIOChannel *channel;
  int cond = 0;

  closure->function = function;
  closure->data = data;

  if (condition & CppConsUI::INPUT_CONDITION_READ)
    cond |= GLIB_IO_READ_COND;
  if (condition & CppConsUI::INPUT_CONDITION_WRITE)
    cond |= GLIB_IO_WRITE_COND;

  channel = g_io_channel_unix_new(fd);
  closure->result = g_io_add_watch_full(channel, G_PRIORITY_DEFAULT,
    static_cast<GIOCondition>(cond), io_input_cppconsui, closure,
    io_destroy_cppconsui);

  g_io_channel_unref(channel);
  return closure->result;
}

gboolean CenterIM::io_input_cppconsui(
  GIOChannel *source, GIOCondition condition, gpointer data)
{
  IOClosureCppConsUI *closure = static_cast<IOClosureCppConsUI *>(data);
  int cppconsui_cond = 0;

  if (condition & G_IO_IN)
    cppconsui_cond |= CppConsUI::INPUT_CONDITION_READ;
  if (condition & G_IO_OUT)
    cppconsui_cond |= CppConsUI::INPUT_CONDITION_WRITE;

  closure->function(g_io_channel_unix_get_fd(source),
    static_cast<CppConsUI::InputCondition>(cppconsui_cond), closure->data);

  return TRUE;
}

void CenterIM::io_destroy_cppconsui(gpointer data)
{
  delete static_cast<IOClosureCppConsUI *>(data);
}

unsigned CenterIM::timeout_add_cppconsui(
  unsigned interval, CppConsUI::SourceFunction function, void *data)
{
  SourceClosureCppConsUI *closure = new SourceClosureCppConsUI;
  closure->function = function;
  closure->data = data;

  return g_timeout_add_full(G_PRIORITY_DEFAULT, interval,
    timeout_function_cppconsui, closure, timeout_destroy_cppconsui);
}

gboolean CenterIM::timeout_function_cppconsui(gpointer data)
{
  SourceClosureCppConsUI *closure = static_cast<SourceClosureCppConsUI *>(data);
  return closure->function(closure->data);
}

void CenterIM::timeout_destroy_cppconsui(gpointer data)
{
  delete static_cast<SourceClosureCppConsUI *>(data);
}

bool CenterIM::timeout_remove_cppconsui(unsigned handle)
{
  return g_source_remove(handle);
}

bool CenterIM::input_remove_cppconsui(unsigned handle)
{
  return g_source_remove(handle);
}

void CenterIM::log_error_cppconsui(const char *message)
{
  LOG->warning("%s", message);
}

GHashTable *CenterIM::get_ui_info()
{
  static GHashTable *ui_info = NULL;

  if (!ui_info) {
    ui_info = g_hash_table_new(g_str_hash, g_str_equal);

    /* Note: the C-style casts are used below because otherwise we would need
     * to use the const_cast and reinterpret_cast together (which is too much
     * typing). */
    g_hash_table_insert(ui_info, (void *)"name", (void *)PACKAGE_NAME);
    g_hash_table_insert(ui_info, (void *)"version", (void *)version);
    g_hash_table_insert(ui_info, (void *)"website", (void *)PACKAGE_URL);

    g_hash_table_insert(
      ui_info, (void *)"dev_website", (void *)PACKAGE_BUGREPORT);
    g_hash_table_insert(ui_info, (void *)"client_type", (void *)"pc");
  }

  return ui_info;
}

void CenterIM::purple_print(
  PurpleDebugLevel level, const char *category, const char *arg_s)
{
  LOG->purple_print(level, category, arg_s);
}

gboolean CenterIM::purple_is_enabled(
  PurpleDebugLevel level, const char *category)
{
  return LOG->purple_is_enabled(level, category);
}

void CenterIM::dimensions_change(
  const char * /*name*/, PurplePrefType /*type*/, gconstpointer /*val*/)
{
  mngr->onScreenResized();
}

void CenterIM::idle_reporting_change(
  const char * /*name*/, PurplePrefType type, gconstpointer val)
{
  g_return_if_fail(type == PURPLE_PREF_STRING);

  const char *value = static_cast<const char *>(val);
  if (!strcmp(value, "system"))
    idle_reporting_on_keyboard = true;
  else
    idle_reporting_on_keyboard = false;
}

void CenterIM::loadDefaultColorSchemeConfig()
{
  COLORSCHEME->clear();

  // Inititialize default color schemes.
  COLORSCHEME->setAttributes(SCHEME_ACCOUNTSTATUSMENU,
    CppConsUI::ColorScheme::PROPERTY_PANEL_LINE, CppConsUI::Curses::Color::CYAN,
    CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setAttributes(SCHEME_ACCOUNTSTATUSMENU,
    CppConsUI::ColorScheme::PROPERTY_HORIZONTALLINE_LINE,
    CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setAttributes(SCHEME_ACCOUNTSTATUSMENU,
    CppConsUI::ColorScheme::PROPERTY_BUTTON_NORMAL,
    CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT);

  COLORSCHEME->setAttributes(SCHEME_BUDDYLIST,
    CppConsUI::ColorScheme::PROPERTY_TREEVIEW_LINE,
    CppConsUI::Curses::Color::GREEN, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setAttributes(SCHEME_BUDDYLIST,
    CppConsUI::ColorScheme::PROPERTY_PANEL_LINE, CppConsUI::Curses::Color::BLUE,
    CppConsUI::Curses::Color::DEFAULT, CppConsUI::Curses::Attr::BOLD);
  COLORSCHEME->setAttributes(SCHEME_BUDDYLIST,
    CppConsUI::ColorScheme::PROPERTY_BUTTON_NORMAL,
    CppConsUI::Curses::Color::GREEN, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setAttributes(SCHEME_BUDDYLISTGROUP,
    CppConsUI::ColorScheme::PROPERTY_BUTTON_NORMAL,
    CppConsUI::Curses::Color::MAGENTA, CppConsUI::Curses::Color::DEFAULT);

  COLORSCHEME->setAttributes(SCHEME_BUDDYLISTBUDDY_AWAY,
    CppConsUI::ColorScheme::PROPERTY_BUTTON_NORMAL,
    CppConsUI::Curses::Color::BLUE, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setAttributes(SCHEME_BUDDYLISTBUDDY_NA,
    CppConsUI::ColorScheme::PROPERTY_BUTTON_NORMAL,
    CppConsUI::Curses::Color::MAGENTA, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setAttributes(SCHEME_BUDDYLISTBUDDY_OFFLINE,
    CppConsUI::ColorScheme::PROPERTY_BUTTON_NORMAL,
    CppConsUI::Curses::Color::RED, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setAttributes(SCHEME_BUDDYLISTBUDDY_ONLINE,
    CppConsUI::ColorScheme::PROPERTY_BUTTON_NORMAL,
    CppConsUI::Curses::Color::GREEN, CppConsUI::Curses::Color::DEFAULT);

  COLORSCHEME->setAttributes(SCHEME_BUDDYLISTCONTACT_AWAY,
    CppConsUI::ColorScheme::PROPERTY_BUTTON_NORMAL,
    CppConsUI::Curses::Color::BLUE, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setAttributes(SCHEME_BUDDYLISTCONTACT_NA,
    CppConsUI::ColorScheme::PROPERTY_BUTTON_NORMAL,
    CppConsUI::Curses::Color::MAGENTA, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setAttributes(SCHEME_BUDDYLISTCONTACT_OFFLINE,
    CppConsUI::ColorScheme::PROPERTY_BUTTON_NORMAL,
    CppConsUI::Curses::Color::RED, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setAttributes(SCHEME_BUDDYLISTCONTACT_ONLINE,
    CppConsUI::ColorScheme::PROPERTY_BUTTON_NORMAL,
    CppConsUI::Curses::Color::GREEN, CppConsUI::Curses::Color::DEFAULT);

  COLORSCHEME->setAttributes(SCHEME_CONVERSATION,
    CppConsUI::ColorScheme::PROPERTY_TEXTVIEW_TEXT,
    CppConsUI::Curses::Color::MAGENTA, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setAttributesExt(SCHEME_CONVERSATION,
    CppConsUI::ColorScheme::PROPERTY_TEXTVIEW_TEXT, 1,
    CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setAttributesExt(SCHEME_CONVERSATION,
    CppConsUI::ColorScheme::PROPERTY_TEXTVIEW_TEXT, 2,
    CppConsUI::Curses::Color::MAGENTA, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setAttributes(SCHEME_CONVERSATION,
    CppConsUI::ColorScheme::PROPERTY_PANEL_LINE, CppConsUI::Curses::Color::BLUE,
    CppConsUI::Curses::Color::DEFAULT, CppConsUI::Curses::Attr::BOLD);
  COLORSCHEME->setAttributes(SCHEME_CONVERSATION,
    CppConsUI::ColorScheme::PROPERTY_HORIZONTALLINE_LINE,
    CppConsUI::Curses::Color::BLUE, CppConsUI::Curses::Color::DEFAULT,
    CppConsUI::Curses::Attr::BOLD);
  COLORSCHEME->setAttributes(SCHEME_CONVERSATION,
    CppConsUI::ColorScheme::PROPERTY_VERTICALLINE_LINE,
    CppConsUI::Curses::Color::BLUE, CppConsUI::Curses::Color::DEFAULT,
    CppConsUI::Curses::Attr::BOLD);
  COLORSCHEME->setAttributes(SCHEME_CONVERSATION,
    CppConsUI::ColorScheme::PROPERTY_TEXTEDIT_TEXT,
    CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT);

  COLORSCHEME->setAttributes(SCHEME_CONVERSATION,
    CppConsUI::ColorScheme::PROPERTY_LABEL_TEXT, CppConsUI::Curses::Color::CYAN,
    CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setAttributes(SCHEME_CONVERSATION_ACTIVE,
    CppConsUI::ColorScheme::PROPERTY_LABEL_TEXT,
    CppConsUI::Curses::Color::MAGENTA, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setAttributes(SCHEME_CONVERSATION_NEW,
    CppConsUI::ColorScheme::PROPERTY_LABEL_TEXT, CppConsUI::Curses::Color::CYAN,
    CppConsUI::Curses::Color::DEFAULT, CppConsUI::Curses::Attr::BOLD);

  COLORSCHEME->setAttributes(SCHEME_FOOTER,
    CppConsUI::ColorScheme::PROPERTY_LABEL_TEXT,
    CppConsUI::Curses::Color::BLACK, CppConsUI::Curses::Color::WHITE);
  COLORSCHEME->setAttributes(SCHEME_FOOTER,
    CppConsUI::ColorScheme::PROPERTY_CONTAINER_BACKGROUND,
    CppConsUI::Curses::Color::BLACK, CppConsUI::Curses::Color::WHITE);

  COLORSCHEME->setAttributes(SCHEME_GENERALMENU,
    CppConsUI::ColorScheme::PROPERTY_PANEL_LINE, CppConsUI::Curses::Color::CYAN,
    CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setAttributes(SCHEME_GENERALMENU,
    CppConsUI::ColorScheme::PROPERTY_HORIZONTALLINE_LINE,
    CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setAttributes(SCHEME_GENERALMENU,
    CppConsUI::ColorScheme::PROPERTY_BUTTON_NORMAL,
    CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT);

  COLORSCHEME->setAttributes(SCHEME_GENERALWINDOW,
    CppConsUI::ColorScheme::PROPERTY_PANEL_LINE, CppConsUI::Curses::Color::CYAN,
    CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setAttributes(SCHEME_GENERALWINDOW,
    CppConsUI::ColorScheme::PROPERTY_HORIZONTALLINE_LINE,
    CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT);
  COLORSCHEME->setAttributes(SCHEME_GENERALWINDOW,
    CppConsUI::ColorScheme::PROPERTY_VERTICALLINE_LINE,
    CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT);

  COLORSCHEME->setAttributes(SCHEME_LOG,
    CppConsUI::ColorScheme::PROPERTY_PANEL_LINE, CppConsUI::Curses::Color::BLUE,
    CppConsUI::Curses::Color::DEFAULT, CppConsUI::Curses::Attr::BOLD);
  COLORSCHEME->setAttributes(SCHEME_LOG,
    CppConsUI::ColorScheme::PROPERTY_TEXTVIEW_TEXT,
    CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::DEFAULT);

  COLORSCHEME->setAttributes(SCHEME_HEADER,
    CppConsUI::ColorScheme::PROPERTY_LABEL_TEXT,
    CppConsUI::Curses::Color::BLACK, CppConsUI::Curses::Color::WHITE);
  COLORSCHEME->setAttributes(SCHEME_HEADER,
    CppConsUI::ColorScheme::PROPERTY_CONTAINER_BACKGROUND,
    CppConsUI::Curses::Color::BLACK, CppConsUI::Curses::Color::WHITE);
  COLORSCHEME->setAttributes(SCHEME_HEADER_REQUEST,
    CppConsUI::ColorScheme::PROPERTY_LABEL_TEXT, CppConsUI::Curses::Color::RED,
    CppConsUI::Curses::Color::WHITE);
}

bool CenterIM::saveColorSchemeConfig()
{
  xmlnode *root = xmlnode_new("colorscheme");
  xmlnode_set_attrib(root, "version", "1.0");

  for (CppConsUI::ColorScheme::Schemes::const_iterator si =
         COLORSCHEME->getSchemes().begin();
       si != COLORSCHEME->getSchemes().end(); ++si) {
    xmlnode *scheme_node = xmlnode_new("scheme");
    xmlnode_set_attrib(scheme_node, "name", schemeToString(si->first));
    xmlnode_insert_child(root, scheme_node);

    for (CppConsUI::ColorScheme::Properties::const_iterator pi =
           si->second.begin();
         pi != si->second.end(); ++pi) {
      xmlnode *color_node = xmlnode_new("color");
      xmlnode_insert_child(scheme_node, color_node);

      CppConsUI::ColorScheme::PropertyPair pair = pi->first;
      CppConsUI::ColorScheme::Color color = pi->second;

      const char *widget_string = COLORSCHEME->propertyToWidgetName(pair.first);
      assert(widget_string != NULL);
      xmlnode_set_attrib(color_node, "widget", widget_string);

      char *str;

      const char *property_string =
        COLORSCHEME->propertyToPropertyName(pair.first);
      assert(property_string != NULL);
      if (pair.second != 0) {
        str = g_strdup_printf("%s_%d", property_string, pair.second);
        xmlnode_set_attrib(color_node, "property", str);
        g_free(str);
      }
      else
        xmlnode_set_attrib(color_node, "property", property_string);

      if (color.foreground != CppConsUI::Curses::Color::DEFAULT) {
        str = colorToString(color.foreground);
        xmlnode_set_attrib(color_node, "foreground", str);
        g_free(str);
      }

      if (color.background != CppConsUI::Curses::Color::DEFAULT) {
        str = colorToString(color.background);
        xmlnode_set_attrib(color_node, "background", str);
        g_free(str);
      }

      str = colorAttributesToString(color.attrs);
      if (str != NULL) {
        xmlnode_set_attrib(color_node, "attributes", str);
        g_free(str);
      }
    }
  }

  char *data = xmlnode_to_formatted_str(root, NULL);
  bool res = true;
  if (!purple_util_write_data_to_file("colorschemes.xml", data, -1)) {
    LOG->error(_("Error saving 'colorschemes.xml'."));
    res = false;
  }
  g_free(data);
  xmlnode_free(root);
  return res;
}

const char *CenterIM::schemeToString(int scheme)
{
  static_assert(G_N_ELEMENTS(scheme_names) == SCHEME_END,
    "Incorrect number of elements in array scheme_names");
  assert(scheme >= 0 && scheme < SCHEME_END);
  return scheme_names[scheme];
}

int CenterIM::stringToScheme(const char *str)
{
  for (int i = SCHEME_BEGIN; i < SCHEME_END; ++i)
    if (strcmp(str, scheme_names[i]) == 0)
      return i;
  return 0;
}

char *CenterIM::colorToString(int color)
{
  if (color >= -1 && color < static_cast<int>(G_N_ELEMENTS(color_names) - 1))
    return g_strdup(color_names[color + 1]);
  return g_strdup_printf("%d", color);
}

bool CenterIM::stringToColor(const char *str, int *color)
{
  g_assert(str);
  g_assert(color);

  *color = 0;

  if (g_ascii_isdigit(str[0]) || str[0] == '-') {
    // numeric colors
    long i = strtol(str, NULL, 10);
    if (errno == ERANGE || i > INT_MAX || i < -1)
      return false;
    *color = i;
    return true;
  }

  // symbolic colors
  for (int i = -1; i < static_cast<int>(G_N_ELEMENTS(color_names) - 1); i++)
    if (!strcmp(str, color_names[i + 1])) {
      *color = i;
      return true;
    }

  return false;
}

char *CenterIM::colorAttributesToString(int attrs)
{
#define APPEND(str)                                                            \
  do {                                                                         \
    if (s.size())                                                              \
      s.append("|");                                                           \
    s.append(str);                                                             \
  } while (0)

  std::string s;

  if (attrs == CppConsUI::Curses::Attr::NORMAL)
    return NULL;

  if (attrs & CppConsUI::Curses::Attr::STANDOUT)
    APPEND("standout");
  if (attrs & CppConsUI::Curses::Attr::REVERSE)
    APPEND("reverse");
  if (attrs & CppConsUI::Curses::Attr::BLINK)
    APPEND("blink");
  if (attrs & CppConsUI::Curses::Attr::DIM)
    APPEND("dim");
  if (attrs & CppConsUI::Curses::Attr::BOLD)
    APPEND("bold");

  return g_strdup(s.c_str());

#undef APPEND
}

bool CenterIM::stringToColorAttributes(const char *str, int *attrs)
{
  g_assert(str);
  g_assert(attrs);

  gchar **tokens = g_strsplit(str, "|", 0);
  *attrs = 0;

  bool valid = true;
  for (size_t i = 0; tokens[i]; i++) {
    if (!strcmp("normal", tokens[i])) {
      *attrs |= CppConsUI::Curses::Attr::NORMAL;
      continue;
    }
    if (!strcmp("standout", tokens[i])) {
      *attrs |= CppConsUI::Curses::Attr::STANDOUT;
      continue;
    }
    if (!strcmp("reverse", tokens[i])) {
      *attrs |= CppConsUI::Curses::Attr::REVERSE;
      continue;
    }
    if (!strcmp("blink", tokens[i])) {
      *attrs |= CppConsUI::Curses::Attr::BLINK;
      continue;
    }
    if (!strcmp("dim", tokens[i])) {
      *attrs |= CppConsUI::Curses::Attr::DIM;
      continue;
    }
    if (!strcmp("bold", tokens[i])) {
      *attrs |= CppConsUI::Curses::Attr::BOLD;
      continue;
    }
    // unrecognized attribute
    valid = false;
    break;
  }

  g_strfreev(tokens);

  return valid;
}

void CenterIM::loadDefaultKeyConfig()
{
  // clear current bindings and load default ones
  KEYCONFIG->loadDefaultKeyConfig();

  KEYCONFIG->bindKey("centerim", "quit", "Ctrl-q");
  KEYCONFIG->bindKey("centerim", "buddylist", "F1");
  KEYCONFIG->bindKey("centerim", "conversation-active", "F2");
  KEYCONFIG->bindKey("centerim", "accountstatusmenu", "F3");
  KEYCONFIG->bindKey("centerim", "generalmenu", "F4");
  KEYCONFIG->bindKey("centerim", "generalmenu", "Ctrl-g");
  KEYCONFIG->bindKey("centerim", "buddylist-toggle-offline", "F5");
  KEYCONFIG->bindKey("centerim", "conversation-expand", "F6");

  KEYCONFIG->bindKey("centerim", "conversation-prev", "Ctrl-p");
  KEYCONFIG->bindKey("centerim", "conversation-next", "Ctrl-n");
  KEYCONFIG->bindKey("centerim", "conversation-number1", "Alt-1");
  KEYCONFIG->bindKey("centerim", "conversation-number2", "Alt-2");
  KEYCONFIG->bindKey("centerim", "conversation-number3", "Alt-3");
  KEYCONFIG->bindKey("centerim", "conversation-number4", "Alt-4");
  KEYCONFIG->bindKey("centerim", "conversation-number5", "Alt-5");
  KEYCONFIG->bindKey("centerim", "conversation-number6", "Alt-6");
  KEYCONFIG->bindKey("centerim", "conversation-number7", "Alt-7");
  KEYCONFIG->bindKey("centerim", "conversation-number8", "Alt-8");
  KEYCONFIG->bindKey("centerim", "conversation-number9", "Alt-9");
  KEYCONFIG->bindKey("centerim", "conversation-number10", "Alt-0");
  KEYCONFIG->bindKey("centerim", "conversation-number11", "Alt-q");
  KEYCONFIG->bindKey("centerim", "conversation-number12", "Alt-w");
  KEYCONFIG->bindKey("centerim", "conversation-number13", "Alt-e");
  KEYCONFIG->bindKey("centerim", "conversation-number14", "Alt-r");
  KEYCONFIG->bindKey("centerim", "conversation-number15", "Alt-t");
  KEYCONFIG->bindKey("centerim", "conversation-number16", "Alt-y");
  KEYCONFIG->bindKey("centerim", "conversation-number17", "Alt-u");
  KEYCONFIG->bindKey("centerim", "conversation-number18", "Alt-i");
  KEYCONFIG->bindKey("centerim", "conversation-number19", "Alt-o");
  KEYCONFIG->bindKey("centerim", "conversation-number20", "Alt-p");

  KEYCONFIG->bindKey("buddylist", "contextmenu", "Ctrl-d");
  KEYCONFIG->bindKey("buddylist", "filter", "/");

  KEYCONFIG->bindKey("conversation", "send", "Ctrl-x");
}

bool CenterIM::saveKeyConfig()
{
  xmlnode *root = xmlnode_new("keyconfig");
  xmlnode_set_attrib(root, "version", "1.0");

  const CppConsUI::KeyConfig::KeyBinds *binds = KEYCONFIG->getKeyBinds();
  for (CppConsUI::KeyConfig::KeyBinds::const_iterator bi = binds->begin();
       bi != binds->end(); bi++) {
    /* Invert the map because the output should be sorted by context+action,
     * not by context+key. */
    typedef std::multimap<std::string, TermKeyKey> InvertedMap;
    InvertedMap inverted;
    for (CppConsUI::KeyConfig::KeyBindContext::const_iterator ci =
           bi->second.begin();
         ci != bi->second.end(); ci++)
      inverted.insert(std::make_pair(ci->second, ci->first));

    for (InvertedMap::iterator ci = inverted.begin(); ci != inverted.end();
         ci++) {
      xmlnode *bind = xmlnode_new("bind");
      xmlnode_set_attrib(bind, "context", bi->first.c_str());
      xmlnode_set_attrib(bind, "action", ci->first.c_str());
      char *key;
      if ((key = KEYCONFIG->termKeyToString(ci->second))) {
        xmlnode_set_attrib(bind, "key", key);
        delete[] key;
      }

      xmlnode_insert_child(root, bind);
    }
  }

  char *data = xmlnode_to_formatted_str(root, NULL);
  bool res = true;
  if (!purple_util_write_data_to_file("binds.xml", data, -1)) {
    LOG->error(_("Error saving 'binds.xml'."));
    res = false;
  }
  g_free(data);
  xmlnode_free(root);
  return res;
}

void CenterIM::actionFocusBuddyList()
{
  BUDDYLIST->show();
}

void CenterIM::actionFocusActiveConversation()
{
  CONVERSATIONS->focusActiveConversation();
}

void CenterIM::actionOpenAccountStatusMenu()
{
  /* Don't allow to open the account status menu if there is any 'top' window
   * (except general menu, we can close that). */
  CppConsUI::Window *top = mngr->getTopWindow();
  if (top) {
    if (dynamic_cast<GeneralMenu *>(top))
      top->close();
    else if (top->getType() == CppConsUI::Window::TYPE_TOP)
      return;
  }

  AccountStatusMenu *menu = new AccountStatusMenu;
  menu->show();
}

void CenterIM::actionOpenGeneralMenu()
{
  /* Don't allow to open the general menu if there is any 'top' window (except
   * account status menu, we can close that). */
  CppConsUI::Window *top = mngr->getTopWindow();
  if (top) {
    if (dynamic_cast<AccountStatusMenu *>(top))
      top->close();
    else if (top->getType() == CppConsUI::Window::TYPE_TOP)
      return;
  }

  GeneralMenu *menu = new GeneralMenu;
  menu->show();
}

void CenterIM::actionBuddyListToggleOffline()
{
  gboolean cur =
    purple_prefs_get_bool(CONF_PREFIX "/blist/show_offline_buddies");
  purple_prefs_set_bool(CONF_PREFIX "/blist/show_offline_buddies", !cur);
}

void CenterIM::actionFocusPrevConversation()
{
  CONVERSATIONS->focusPrevConversation();
}

void CenterIM::actionFocusNextConversation()
{
  CONVERSATIONS->focusNextConversation();
}

void CenterIM::actionFocusConversation(int i)
{
  CONVERSATIONS->focusConversation(i);
}

void CenterIM::actionExpandConversation()
{
  CppConsUI::Window *top = mngr->getTopWindow();
  if (top && top->getType() == CppConsUI::Window::TYPE_TOP)
    return;

  if (!convs_expanded) {
    CONVERSATIONS->focusActiveConversation();
    top = mngr->getTopWindow();
    if (!top || typeid(Conversation) != typeid(*top))
      return;
  }

  convs_expanded = !convs_expanded;
  CONVERSATIONS->setExpandedConversations(convs_expanded);
  mngr->onScreenResized();
}

void CenterIM::declareBindables()
{
  declareBindable("centerim", "quit", sigc::mem_fun(this, &CenterIM::quit),
    InputProcessor::BINDABLE_OVERRIDE);
  declareBindable("centerim", "buddylist",
    sigc::mem_fun(this, &CenterIM::actionFocusBuddyList),
    InputProcessor::BINDABLE_OVERRIDE);
  declareBindable("centerim", "conversation-active",
    sigc::mem_fun(this, &CenterIM::actionFocusActiveConversation),
    InputProcessor::BINDABLE_OVERRIDE);
  declareBindable("centerim", "accountstatusmenu",
    sigc::mem_fun(this, &CenterIM::actionOpenAccountStatusMenu),
    InputProcessor::BINDABLE_OVERRIDE);
  declareBindable("centerim", "generalmenu",
    sigc::mem_fun(this, &CenterIM::actionOpenGeneralMenu),
    InputProcessor::BINDABLE_OVERRIDE);
  declareBindable("centerim", "buddylist-toggle-offline",
    sigc::mem_fun(this, &CenterIM::actionBuddyListToggleOffline),
    InputProcessor::BINDABLE_OVERRIDE);
  declareBindable("centerim", "conversation-prev",
    sigc::mem_fun(this, &CenterIM::actionFocusPrevConversation),
    InputProcessor::BINDABLE_OVERRIDE);
  declareBindable("centerim", "conversation-next",
    sigc::mem_fun(this, &CenterIM::actionFocusNextConversation),
    InputProcessor::BINDABLE_OVERRIDE);
  char action[] = "conversation-numberXX";
  for (int i = 1; i <= 20; i++) {
    g_sprintf(action + sizeof(action) - 3, "%d", i);
    declareBindable("centerim", action,
      sigc::bind(sigc::mem_fun(this, &CenterIM::actionFocusConversation), i),
      InputProcessor::BINDABLE_OVERRIDE);
  }
  declareBindable("centerim", "conversation-expand",
    sigc::mem_fun(this, &CenterIM::actionExpandConversation),
    InputProcessor::BINDABLE_OVERRIDE);
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
