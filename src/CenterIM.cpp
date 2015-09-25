// Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
// Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
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
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <locale.h>
#include <getopt.h>
#include <glib/gprintf.h>
#include <time.h>
#include <typeinfo>
#include <unistd.h>
#include "gettext.h"

#define CIM_CONFIG_PATH ".centerim5"

#define GLIB_IO_READ_COND (G_IO_IN | G_IO_HUP | G_IO_ERR)
#define GLIB_IO_WRITE_COND (G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL)

// Program's entry point.
int main(int argc, char *argv[])
{
  return CenterIM::run(argc, argv);
}

const char *CenterIM::color_names_[] = {
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

const char *CenterIM::scheme_names_[] = {
  nullptr,
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

CenterIM *CenterIM::my_instance_ = nullptr;

// Based on glibmm code.
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
  : slot(nslot), source(nullptr)
{
  slot.set_parent(this, &SourceConnectionNode::notify);
}

void *SourceConnectionNode::notify(void *data)
{
  SourceConnectionNode *self = reinterpret_cast<SourceConnectionNode *>(data);

  // If there is no object, this call was triggered from
  // destroy_notify_handler(), because we set self->source to 0 there.
  if (self->source) {
    GSource *s = self->source;
    self->source = nullptr;
    g_source_destroy(s);

    // Destroying the object triggers execution of destroy_notify_handler(),
    // either immediately or later, so we leave that to do the deletion.
  }

  return nullptr;
}

void SourceConnectionNode::destroy_notify_callback(void *data)
{
  SourceConnectionNode *self = reinterpret_cast<SourceConnectionNode *>(data);

  if (self) {
    // The GLib side is disconnected now, thus the GSource* is no longer valid.
    self->source = nullptr;

    delete self;
  }
}

gboolean SourceConnectionNode::source_callback(void *data)
{
  SourceConnectionNode *conn_data =
    reinterpret_cast<SourceConnectionNode *>(data);

  // Recreate the specific slot from the generic slot node.
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
  return my_instance_;
}

bool CenterIM::processInput(const TermKeyKey &key)
{
  if (idle_reporting_on_keyboard_)
    purple_idle_touch();
  return InputProcessor::processInput(key);
}

void CenterIM::quit()
{
  g_main_loop_quit(mainloop_);
}

CppConsUI::Rect CenterIM::getScreenArea(ScreenArea area)
{
  return areas_[area];
}

CppConsUI::Rect CenterIM::getScreenAreaCentered(ScreenArea area)
{
  CppConsUI::Rect s = areas_[WHOLE_AREA];
  CppConsUI::Rect r = areas_[area];
  int x = (s.width - r.width) / 2;
  int y = (s.height - r.height) / 2;
  return CppConsUI::Rect(x, y, r.width, r.height);
}

bool CenterIM::loadColorSchemeConfig()
{
  xmlnode *root =
    purple_util_read_xml_from_file("colorschemes.xml", _("color schemes"));

  if (root == nullptr) {
    // Read error, first time run?
    loadDefaultColorSchemeConfig();
    if (saveColorSchemeConfig())
      return true;
    return false;
  }

  COLORSCHEME->clear();
  bool res = false;

  for (xmlnode *scheme_node = xmlnode_get_child(root, "scheme");
       scheme_node != nullptr;
       scheme_node = xmlnode_get_next_twin(scheme_node)) {
    const char *scheme_name = xmlnode_get_attrib(scheme_node, "name");
    if (scheme_name == nullptr) {
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
      if (widget_string == nullptr) {
        LOG->error(_("Missing 'widget' attribute in the color definition."));
        goto out;
      }

      const char *property_string = xmlnode_get_attrib(color, "property");
      if (property_string == nullptr) {
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

      if (fg_string != nullptr && !stringToColor(fg_string, &fg)) {
        LOG->error(_("Unrecognized color '%s'."), fg_string);
        goto out;
      }

      if (bg_string != nullptr && !stringToColor(bg_string, &bg)) {
        LOG->error(_("Unrecognized color '%s'."), bg_string);
        goto out;
      }

      if (attrs_string != nullptr &&
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

  if (root == nullptr) {
    // Read error, first time run?
    loadDefaultKeyConfig();
    if (saveKeyConfig())
      return true;
    return false;
  }

  KEYCONFIG->clear();
  bool res = false;

  for (xmlnode *bind_node = xmlnode_get_child(root, "bind");
       bind_node != nullptr; bind_node = xmlnode_get_next_twin(bind_node)) {
    const char *context = xmlnode_get_attrib(bind_node, "context");
    if (context == nullptr) {
      LOG->error(_("Missing 'context' attribute in the bind definition."));
      goto out;
    }
    const char *action = xmlnode_get_attrib(bind_node, "action");
    if (action == nullptr) {
      LOG->error(_("Missing 'action' attribute in the bind definition."));
      goto out;
    }
    const char *key = xmlnode_get_attrib(bind_node, "key");
    if (key == nullptr) {
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
  auto conn_node = new SourceConnectionNode(slot);
  sigc::connection connection(*conn_node->get_slot());

  GSource *source = g_timeout_source_new(interval);

  if (priority != G_PRIORITY_DEFAULT)
    g_source_set_priority(source, priority);

  g_source_set_callback(source, &SourceConnectionNode::source_callback,
    conn_node, &SourceConnectionNode::destroy_notify_callback);

  g_source_attach(source, nullptr);
  g_source_unref(source); // GMainContext holds a reference.

  conn_node->install(source);
  return connection;
}

sigc::connection CenterIM::timeoutOnceConnect(
  const sigc::slot<void> &slot, unsigned interval, int priority)
{
  return timeoutConnect(sigc::bind_return(slot, FALSE), interval, priority);
}

CenterIM::CenterIM()
  : mainloop_(nullptr), mainloop_error_exit_(false), mngr_(nullptr),
    convs_expanded_(false), idle_reporting_on_keyboard_(false),
    stdin_timeout_id_(0), resize_pending_(false),
    sigwinch_write_error_(nullptr), sigwinch_write_error_size_(0)
{
  resize_pipe_[0] = -1;
  resize_pipe_[1] = -1;

  memset(&centerim_core_ui_ops_, 0, sizeof(centerim_core_ui_ops_));
  memset(&logbuf_debug_ui_ops_, 0, sizeof(logbuf_debug_ui_ops_));
  memset(&centerim_glib_eventloops_, 0, sizeof(centerim_glib_eventloops_));
}

int CenterIM::run(int argc, char *argv[])
{
  // Init CenterIM.
  g_assert(my_instance_ == nullptr);
  my_instance_ = new CenterIM;

  // Run CenterIM.
  int res = my_instance_->runAll(argc, argv);

  // Finalize CenterIM.
  g_assert(my_instance_ != nullptr);

  delete my_instance_;
  my_instance_ = nullptr;

  return res;
}

int CenterIM::runAll(int argc, char *argv[])
{
  int res = 1;
  bool cppconsui_input_initialized = false;
  bool cppconsui_output_initialized = false;
  bool screen_resizing_initialized = false;
  bool purple_initialized = false;
  CppConsUI::Error error;
  guint stdin_watch_handle;
  guint resize_watch_handle;
  sigc::connection resize_conn;
  sigc::connection top_window_change_conn;

  // Set program name for glib.
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

  // Parse command-line arguments.
  bool ascii = false;
  bool offline = false;
  const char *config_path = CIM_CONFIG_PATH;
  int opt;
  // clang-format off
  struct option long_options[] = {
    {"ascii",   no_argument,       nullptr, 'a'},
    {"help",    no_argument,       nullptr, 'h'},
    {"version", no_argument,       nullptr, 'v'},
    {"basedir", required_argument, nullptr, 'b'},
    {"offline", no_argument,       nullptr, 'o'},
    {nullptr,   0,                 nullptr,  0 }
  };
  // clang-format on

  while (
    (opt = getopt_long(argc, argv, "ahvb:o", long_options, nullptr)) != -1) {
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

  // Initialize the internal logger. It will buffer all messages produced by
  // GLib, libpurple, or CppConsUI until it is possible to output them on the
  // screen (in the log window). If any part of the initialization fails the
  // buffered messages will be printed on stderr.
  Log::init();

  // Create the main loop.
  mainloop_ = g_main_loop_new(nullptr, FALSE);

  // Initialize CppConsUI.
  CppConsUI::AppInterface interface = {
    sigc::mem_fun(this, &CenterIM::redraw_cppconsui),
    sigc::mem_fun(this, &CenterIM::log_debug_cppconsui)};
  CppConsUI::initializeConsUI(interface);

  // Get the CoreManager instance.
  mngr_ = CppConsUI::getCoreManagerInstance();
  g_assert(mngr_ != nullptr);

  // Initialize CppConsUI input and output.
  if (mngr_->initializeInput(error) != 0) {
    LOG->error("%s\n", error.getString());
    goto out;
  }
  cppconsui_input_initialized = true;
  if (mngr_->initializeOutput(error) != 0) {
    LOG->error("%s\n", error.getString());
    goto out;
  }
  cppconsui_output_initialized = true;

  // ASCII mode.
  if (ascii)
    CppConsUI::Curses::setAsciiMode(ascii);

  // Register for some signals.
  resize_conn = mngr_->signal_resize.connect(
    sigc::mem_fun(this, &CenterIM::onScreenResized));
  top_window_change_conn = mngr_->signal_top_window_change.connect(
    sigc::mem_fun(this, &CenterIM::onTopWindowChanged));

  // Declare CenterIM bindables.
  declareBindables();

  // Set up screen resizing.
  if (initializeScreenResizing() != 0) {
    LOG->error(_("Screen resizing initialization failed."));
    goto out;
  }
  screen_resizing_initialized = true;

  // Initialize libpurple.
  if (initializePurple(config_path) != 0) {
    LOG->error(_("Libpurple initialization failed."));
    goto out;
  }
  purple_initialized = true;

  // Initialize global preferences.
  initializePreferences();

  // Initialize the log window.
  LOG->initPhase2();

  // Init colorschemes and keybinds after the Log is initialized so the user can
  // see if there is any error in the configs.
  loadColorSchemeConfig();
  loadKeyConfig();

  Footer::init();

  Accounts::init();
  Connections::init();
  Notify::init();
  Request::init();

  // Initialize UI.
  Conversations::init();
  Header::init();
  // Init BuddyList last so it takes the focus.
  BuddyList::init();

  LOG->info(_("Welcome to CenterIM 5. Press %s to display main menu."),
    KEYCONFIG->getKeyBind("centerim", "generalmenu"));

  // Restore last know status on all accounts.
  ACCOUNTS->restoreStatuses(offline);

  mngr_->setTopInputProcessor(*this);
  mngr_->onScreenResized();

  // Initialize input processing.
  {
    GIOChannel *stdin_channel = g_io_channel_unix_new(STDIN_FILENO);
    stdin_watch_handle = g_io_add_watch_full(stdin_channel, G_PRIORITY_DEFAULT,
      static_cast<GIOCondition>(GLIB_IO_READ_COND), stdin_bytes_available_,
      this, nullptr);
    g_io_channel_unref(stdin_channel);
  }

  // Add a watch of the self-pipe for screen resizing.
  {
    GIOChannel *resize_channel = g_io_channel_unix_new(resize_pipe_[0]);
    resize_watch_handle = g_io_add_watch_full(resize_channel,
      G_PRIORITY_DEFAULT, static_cast<GIOCondition>(GLIB_IO_READ_COND),
      resize_bytes_available_, this, nullptr);
    g_io_channel_unref(resize_channel);
  }

  // Start the main loop.
  g_main_loop_run(mainloop_);

  // Finalize input processing.
  g_source_remove(stdin_watch_handle);
  // Also remove any stdin timeout source.
  if (stdin_timeout_id_ != 0) {
    g_source_remove(stdin_timeout_id_);
    stdin_timeout_id_ = 0;
  }

  // Remove the self-pipe watch.
  g_source_remove(resize_watch_handle);

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

  if (!mainloop_error_exit_) {
    // Everything went ok.
    res = 0;
  }

out:
  // Finalize libpurple.
  if (purple_initialized)
    finalizePurple();

  // Finalize screen resizing.
  if (screen_resizing_initialized)
    finalizeScreenResizing();

  // Finalize CppConsUI input and output.
  if (cppconsui_output_initialized && mngr_->finalizeOutput(error) != 0)
    LOG->error("%s\n", error.getString());
  if (cppconsui_input_initialized && mngr_->finalizeInput(error) != 0)
    LOG->error("%s\n", error.getString());

  // Finalize CppConsUI.
  CppConsUI::finalizeConsUI();

  // Destroy the main loop.
  if (mainloop_ != nullptr)
    g_main_loop_unref(mainloop_);

  // Finalize the log component. It will output all buffered messages (if there
  // are any) on stderr.
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
  std::fprintf(out, "CenterIM %s\n", version_);
}

int CenterIM::initializePurple(const char *config_path)
{
  g_assert(config_path != nullptr);

  // Build config path.
  if (g_path_is_absolute(config_path)) {
    // Absolute path.
    purple_util_set_user_dir(config_path);
  }
  else {
    char *path = g_build_filename(purple_home_dir(), config_path, NULL);
    g_assert(g_path_is_absolute(path));
    purple_util_set_user_dir(path);
    g_free(path);
  }

  // This does not disable debugging, but rather it disables printing to stdout.
  // Do not change this to TRUE or things will get messy.
  purple_debug_set_enabled(FALSE);

  // Catch libpurple messages.
  logbuf_debug_ui_ops_.print = purple_print;
  logbuf_debug_ui_ops_.is_enabled = purple_is_enabled;
  purple_debug_set_ui_ops(&logbuf_debug_ui_ops_);

  // Set core uiops.
  centerim_core_ui_ops_.get_ui_info = get_ui_info;
  purple_core_set_ui_ops(&centerim_core_ui_ops_);

  // Set the uiops for the eventloop.
  centerim_glib_eventloops_.timeout_add = g_timeout_add;
  centerim_glib_eventloops_.timeout_remove = g_source_remove;
  centerim_glib_eventloops_.input_add = input_add_purple;
  centerim_glib_eventloops_.input_remove = g_source_remove;
  purple_eventloop_set_ui_ops(&centerim_glib_eventloops_);

  // Add a search path for user-specific plugins.
  char *path = g_build_filename(purple_user_dir(), "plugins", NULL);
  purple_plugins_add_search_path(path);
  g_free(path);

  // Add a search path for centerim-specific plugins.
  purple_plugins_add_search_path(PKGLIBDIR);

  if (!purple_core_init(PACKAGE_NAME))
    return 1;

  purple_prefs_add_none(CONF_PREFIX);
  purple_prefs_add_none(CONF_PLUGINS_PREF);

  // Load the desired plugins.
  if (purple_prefs_exists(CONF_PLUGINS_SAVE_PREF))
    purple_plugins_load_saved(CONF_PLUGINS_SAVE_PREF);

  return 0;
}

void CenterIM::finalizePurple()
{
  purple_plugins_save_loaded(CONF_PLUGINS_SAVE_PREF);

  purple_core_set_ui_ops(nullptr);
  // purple_eventloop_set_ui_ops(NULL);
  purple_core_quit();
}

void CenterIM::initializePreferences()
{
  // Remove someday...
  if (purple_prefs_exists("/centerim"))
    purple_prefs_rename("/centerim", CONF_PREFIX);

  // Initialize preferences.
  purple_prefs_add_none(CONF_PREFIX "/dimensions");
  purple_prefs_add_int(CONF_PREFIX "/dimensions/buddylist_width", 20);
  purple_prefs_add_int(CONF_PREFIX "/dimensions/log_height", 25);
  purple_prefs_add_bool(CONF_PREFIX "/dimensions/show_header", true);
  purple_prefs_add_bool(CONF_PREFIX "/dimensions/show_footer", true);
  purple_prefs_connect_callback(
    this, CONF_PREFIX "/dimensions", dimensions_change_, this);

  purple_prefs_connect_callback(
    this, "/purple/away/idle_reporting", idle_reporting_change_, this);
  // Trigger the callback. Note: This potentially triggers other callbacks
  // inside libpurple.
  purple_prefs_trigger_callback("/purple/away/idle_reporting");
}

int CenterIM::initializeScreenResizing()
{
  int res;

  // Get error message reported from the SIGWINCH handler when a write to the
  // self-pipe fails.
  sigwinch_write_error_ =
    _("Write to the self-pipe for screen resizing failed.");
  sigwinch_write_error_size_ = std::strlen(sigwinch_write_error_) + 1;

  // Create a self-pipe.
  g_assert(resize_pipe_[0] == -1);
  g_assert(resize_pipe_[1] == -1);

  res = pipe(resize_pipe_);
  if (res != 0) {
    LOG->error(_("Creating a self-pipe for screen resizing failed."));
    return 1;
  }

  // Set close-on-exec on both descriptors.
  res = fcntl(resize_pipe_[0], F_GETFD);
  g_assert(res != -1);
  res = fcntl(resize_pipe_[0], F_SETFD, res | FD_CLOEXEC);
  g_assert(res == 0);
  res = fcntl(resize_pipe_[1], F_GETFD);
  g_assert(res != -1);
  res = fcntl(resize_pipe_[1], F_SETFD, res | FD_CLOEXEC);
  g_assert(res == 0);

  // Register a SIGWINCH handler.
  struct sigaction sig;
  sig.sa_handler = sigwinch_handler_;
  sig.sa_flags = SA_RESTART;
  res = sigemptyset(&sig.sa_mask);
  g_assert(res == 0);
  res = sigaction(SIGWINCH, &sig, nullptr);
  g_assert(res == 0);

  return 0;
}

void CenterIM::finalizeScreenResizing()
{
  int res;

  // Unregister the SIGWINCH handler.
  struct sigaction sig;
  sig.sa_handler = SIG_DFL;
  sig.sa_flags = 0;
  res = sigemptyset(&sig.sa_mask);
  g_assert(res == 0);
  res = sigaction(SIGWINCH, &sig, nullptr);
  g_assert(res == 0);

  // Destroy the self-pipe.
  g_assert(resize_pipe_[0] != -1);
  g_assert(resize_pipe_[1] != -1);

  res = close(resize_pipe_[0]);
  if (res != 0)
    LOG->error(_("Closing the self-pipe for screen resizing failed."));
  resize_pipe_[0] = -1;

  res = close(resize_pipe_[1]);
  if (res != 0)
    LOG->error(_("Closing the self-pipe for screen resizing failed."));
  resize_pipe_[1] = -1;
}

void CenterIM::onScreenResized()
{
  CppConsUI::Rect size;

  int screen_width = CppConsUI::Curses::getWidth();
  int screen_height = CppConsUI::Curses::getHeight();

  int buddylist_width;
  int log_height;
  if (convs_expanded_) {
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
  areas_[BUDDY_LIST_AREA] = size;

  size.x = areas_[BUDDY_LIST_AREA].width;
  size.width = screen_width - size.x;
  size.height = screen_height / 100.0 * log_height;
  size.y = screen_height - size.height - footer_height;
  areas_[LOG_AREA] = size;

  size.x = areas_[BUDDY_LIST_AREA].width;
  size.y = header_height;
  size.width = screen_width - size.x;
  size.height =
    screen_height - size.y - areas_[LOG_AREA].height - footer_height;
  if (convs_expanded_) {
    size.x -= 2;
    size.width += 4;
  }
  areas_[CHAT_AREA] = size;

  size.x = 0;
  size.y = 0;
  size.width = screen_width;
  size.height = header_height;
  areas_[HEADER_AREA] = size;

  size.x = 0;
  size.y = screen_height - 1;
  size.width = screen_width;
  size.height = footer_height;
  areas_[FOOTER_AREA] = size;

  size.x = 0;
  size.y = 0;
  size.width = screen_width;
  size.height = screen_height;
  areas_[WHOLE_AREA] = size;
}

void CenterIM::onTopWindowChanged()
{
  if (!convs_expanded_)
    return;

  CppConsUI::Window *top = mngr_->getTopWindow();
  if (top != nullptr && typeid(Conversation) != typeid(*top)) {
    convs_expanded_ = false;
    CONVERSATIONS->setExpandedConversations(convs_expanded_);
    mngr_->onScreenResized();
  }
}

guint CenterIM::input_add_purple(int fd, PurpleInputCondition condition,
  PurpleInputFunction function, gpointer data)
{
  auto closure = new IOClosurePurple;
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

gboolean CenterIM::stdin_bytes_available()
{
  // Disconnect any timeout handler.
  if (stdin_timeout_id_ != 0) {
    g_source_remove(stdin_timeout_id_);
    stdin_timeout_id_ = 0;
  }

  int wait;
  CppConsUI::Error error;
  if (mngr_->processStandardInput(&wait, error) != 0)
    LOG->error("%s", error.getString());

  if (wait >= 0) {
    // Connect timeout handler.
    stdin_timeout_id_ = g_timeout_add_full(
      G_PRIORITY_DEFAULT, wait, stdin_timeout_, this, nullptr);
  }

  return TRUE;
}

gboolean CenterIM::stdin_timeout()
{
  stdin_timeout_id_ = 0;

  CppConsUI::Error error;
  if (mngr_->processStandardInputTimeout(error) != 0)
    LOG->error("%s", error.getString());

  return FALSE;
}

gboolean CenterIM::resize_bytes_available()
{
  // An obvious thing here would be to read a single character because
  // sigwinch_handler() never writes more than one character into the pipe.
  // (Additional writes are protected by setting the resize_pending_ flag.)
  // However, it is possible that SIGWINCH was received after a fork() call but
  // before the child exited or exec'ed. In this case, both the parent and the
  // child will receive the signal and write in the pipe. The code should
  // attempt to read out all characters from the pipe so the resizing is not
  // unnecessarily done multiple times.
  char buf[1024];
  int res = read(resize_pipe_[0], buf, sizeof(buf));
  g_assert(res > 0);

  // The following assertion should generally hold. However, in a very unlikely
  // case when the pipe contains more than one character and the read above gets
  // interrupted and does not receive all data it will fail when
  // resize_bytes_available() is called again because of the remaining
  // characters.
  // g_assert(resize_pending_);

  resize_pending_ = false;

  CppConsUI::Error error;
  if (mngr_->resize(error) != 0) {
    LOG->error("%s", error.getString());

    // Exit the program.
    mainloop_error_exit_ = true;
    g_main_loop_quit(mainloop_);
  }
  return TRUE;
}

gboolean CenterIM::draw()
{
  CppConsUI::Error error;
  if (mngr_->draw(error) != 0) {
    LOG->error("%s", error.getString());

    // Exit the program.
    mainloop_error_exit_ = true;
    g_main_loop_quit(mainloop_);
  }
  return FALSE;
}

void CenterIM::sigwinch_handler(int signum)
{
  g_assert(signum == SIGWINCH);

  if (resize_pending_)
    return;

  int saved_errno = errno;
  int res = write(resize_pipe_[1], "@", 1);
  errno = saved_errno;
  if (res == 1) {
    resize_pending_ = true;
    return;
  }

  // Cannot reasonably recover from this error. This should be absolutely rare.
  write(STDERR_FILENO, sigwinch_write_error_, sigwinch_write_error_size_);
  _exit(13);
}

void CenterIM::redraw_cppconsui()
{
  g_timeout_add_full(G_PRIORITY_DEFAULT, 0, draw_, this, nullptr);
}

void CenterIM::log_debug_cppconsui(const char *message)
{
  LOG->debug("%s", message);
}

GHashTable *CenterIM::get_ui_info()
{
  static GHashTable *ui_info = nullptr;

  if (ui_info == nullptr) {
    ui_info = g_hash_table_new(g_str_hash, g_str_equal);

    // Note: the C-style casts are used below because otherwise we would need to
    // use const_cast and reinterpret_cast together (which is too much typing).
    g_hash_table_insert(ui_info, (void *)"name", (void *)PACKAGE_NAME);
    g_hash_table_insert(ui_info, (void *)"version", (void *)version_);
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
  mngr_->onScreenResized();
}

void CenterIM::idle_reporting_change(
  const char * /*name*/, PurplePrefType type, gconstpointer val)
{
  g_return_if_fail(type == PURPLE_PREF_STRING);

  const char *value = static_cast<const char *>(val);
  if (std::strcmp(value, "system") == 0)
    idle_reporting_on_keyboard_ = true;
  else
    idle_reporting_on_keyboard_ = false;
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
      assert(widget_string != nullptr);
      xmlnode_set_attrib(color_node, "widget", widget_string);

      char *str;

      const char *property_string =
        COLORSCHEME->propertyToPropertyName(pair.first);
      assert(property_string != nullptr);
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
      if (str != nullptr) {
        xmlnode_set_attrib(color_node, "attributes", str);
        g_free(str);
      }
    }
  }

  char *data = xmlnode_to_formatted_str(root, nullptr);
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
  static_assert(G_N_ELEMENTS(scheme_names_) == SCHEME_END,
    "Incorrect number of elements in array scheme_names_");
  assert(scheme >= 0 && scheme < SCHEME_END);
  return scheme_names_[scheme];
}

int CenterIM::stringToScheme(const char *str)
{
  for (int i = SCHEME_BEGIN; i < SCHEME_END; ++i)
    if (std::strcmp(str, scheme_names_[i]) == 0)
      return i;
  return 0;
}

char *CenterIM::colorToString(int color)
{
  if (color >= -1 && color < static_cast<int>(G_N_ELEMENTS(color_names_) - 1))
    return g_strdup(color_names_[color + 1]);
  return g_strdup_printf("%d", color);
}

bool CenterIM::stringToColor(const char *str, int *color)
{
  g_assert(str != nullptr);
  g_assert(color != nullptr);

  *color = 0;

  if (g_ascii_isdigit(str[0]) || str[0] == '-') {
    // Numeric colors.
    char *endptr;
    long i = std::strtol(str, &endptr, 10);
    if (*endptr != '\0' || errno == ERANGE || i < -1 || i > INT_MAX)
      return false;
    *color = i;
    return true;
  }

  // Symbolic colors.
  for (int i = -1; i < static_cast<int>(G_N_ELEMENTS(color_names_) - 1); ++i)
    if (!strcmp(str, color_names_[i + 1])) {
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
    return nullptr;

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
  g_assert(str != nullptr);
  g_assert(attrs != nullptr);

  gchar **tokens = g_strsplit(str, "|", 0);
  *attrs = 0;

  bool valid = true;
  for (size_t i = 0; tokens[i] != nullptr; ++i) {
    if (std::strcmp("normal", tokens[i]) == 0) {
      *attrs |= CppConsUI::Curses::Attr::NORMAL;
      continue;
    }
    if (std::strcmp("standout", tokens[i]) == 0) {
      *attrs |= CppConsUI::Curses::Attr::STANDOUT;
      continue;
    }
    if (std::strcmp("reverse", tokens[i]) == 0) {
      *attrs |= CppConsUI::Curses::Attr::REVERSE;
      continue;
    }
    if (std::strcmp("blink", tokens[i]) == 0) {
      *attrs |= CppConsUI::Curses::Attr::BLINK;
      continue;
    }
    if (std::strcmp("dim", tokens[i]) == 0) {
      *attrs |= CppConsUI::Curses::Attr::DIM;
      continue;
    }
    if (std::strcmp("bold", tokens[i]) == 0) {
      *attrs |= CppConsUI::Curses::Attr::BOLD;
      continue;
    }
    // Unrecognized attribute.
    valid = false;
    break;
  }

  g_strfreev(tokens);

  return valid;
}

void CenterIM::loadDefaultKeyConfig()
{
  // Clear current bindings and load default ones.
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
       bi != binds->end(); ++bi) {
    // Invert the map because the output should be sorted by context+action, not
    // by context+key.
    typedef std::multimap<std::string, TermKeyKey> InvertedMap;
    InvertedMap inverted;
    for (CppConsUI::KeyConfig::KeyBindContext::const_iterator ci =
           bi->second.begin();
         ci != bi->second.end(); ++ci)
      inverted.insert(std::make_pair(ci->second, ci->first));

    for (InvertedMap::iterator ci = inverted.begin(); ci != inverted.end();
         ++ci) {
      xmlnode *bind_node = xmlnode_new("bind");
      xmlnode_set_attrib(bind_node, "context", bi->first.c_str());
      xmlnode_set_attrib(bind_node, "action", ci->first.c_str());
      char *key = KEYCONFIG->termKeyToString(ci->second);
      if (key != nullptr) {
        xmlnode_set_attrib(bind_node, "key", key);
        delete[] key;
      }

      xmlnode_insert_child(root, bind_node);
    }
  }

  char *data = xmlnode_to_formatted_str(root, nullptr);
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
  // Do not allow to open the account status menu if there is any 'top' window
  // (except general menu, we can close that).
  CppConsUI::Window *top = mngr_->getTopWindow();
  if (top != nullptr) {
    if (dynamic_cast<GeneralMenu *>(top))
      top->close();
    else if (top->getType() == CppConsUI::Window::TYPE_TOP)
      return;
  }

  auto menu = new AccountStatusMenu;
  menu->show();
}

void CenterIM::actionOpenGeneralMenu()
{
  // Do not allow to open the general menu if there is any 'top' window (except
  // account status menu, we can close that).
  CppConsUI::Window *top = mngr_->getTopWindow();
  if (top != nullptr) {
    if (dynamic_cast<AccountStatusMenu *>(top))
      top->close();
    else if (top->getType() == CppConsUI::Window::TYPE_TOP)
      return;
  }

  auto menu = new GeneralMenu;
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
  CppConsUI::Window *top = mngr_->getTopWindow();
  if (top != nullptr && top->getType() == CppConsUI::Window::TYPE_TOP)
    return;

  if (!convs_expanded_) {
    CONVERSATIONS->focusActiveConversation();
    top = mngr_->getTopWindow();
    if (top == nullptr || typeid(Conversation) != typeid(*top))
      return;
  }

  convs_expanded_ = !convs_expanded_;
  CONVERSATIONS->setExpandedConversations(convs_expanded_);
  mngr_->onScreenResized();
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
  for (int i = 1; i <= 20; ++i) {
    g_sprintf(action + sizeof(action) - 3, "%d", i);
    declareBindable("centerim", action,
      sigc::bind(sigc::mem_fun(this, &CenterIM::actionFocusConversation), i),
      InputProcessor::BINDABLE_OVERRIDE);
  }
  declareBindable("centerim", "conversation-expand",
    sigc::mem_fun(this, &CenterIM::actionExpandConversation),
    InputProcessor::BINDABLE_OVERRIDE);
}

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
