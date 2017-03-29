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
// along with CenterIM.  If not, see <http://www.gnu.org/licenses/>.

#ifndef CENTERIM_H
#define CENTERIM_H

#include <cppconsui/CoreManager.h>
#include <cppconsui/CppConsUI.h>
#include <libpurple/purple.h>
#include <vector>

#define CONF_PREFIX "/centerim5"
#define CONF_PLUGINS_PREF CONF_PREFIX "/plugins"
#define CONF_PLUGINS_SAVE_PREF CONF_PLUGINS_PREF "/loaded"

#define CENTERIM (CenterIM::instance())

class CenterIM : public CppConsUI::InputProcessor {
public:
  enum ScreenArea {
    BUDDY_LIST_AREA,
    CHAT_AREA,
    FOOTER_AREA,
    HEADER_AREA,
    LOG_AREA,
    WHOLE_AREA,
    AREAS_NUM
  };

  // Color schemes. Keep synchronized with scheme_names.
  enum Scheme {
    SCHEME_BEGIN = 1,
    SCHEME_ACCOUNTSTATUSMENU = SCHEME_BEGIN,
    SCHEME_BUDDYLIST,
    SCHEME_BUDDYLISTBUDDY,
    SCHEME_BUDDYLISTBUDDY_AWAY,
    SCHEME_BUDDYLISTBUDDY_NA,
    SCHEME_BUDDYLISTBUDDY_OFFLINE,
    SCHEME_BUDDYLISTBUDDY_ONLINE,
    SCHEME_BUDDYLISTCHAT,
    SCHEME_BUDDYLISTCONTACT,
    SCHEME_BUDDYLISTCONTACT_AWAY,
    SCHEME_BUDDYLISTCONTACT_NA,
    SCHEME_BUDDYLISTCONTACT_OFFLINE,
    SCHEME_BUDDYLISTCONTACT_ONLINE,
    SCHEME_BUDDYLISTGROUP,
    SCHEME_CONVERSATION,
    SCHEME_CONVERSATION_ACTIVE,
    SCHEME_CONVERSATION_NEW,
    SCHEME_FOOTER,
    SCHEME_GENERALMENU,
    SCHEME_GENERALWINDOW,
    SCHEME_HEADER,
    SCHEME_HEADER_REQUEST,
    SCHEME_LOG,

    SCHEME_END,
  };

  static CenterIM *instance();

  // InputProcessor
  virtual bool processInput(const TermKeyKey &key) override;

  void quit();

  // Returns a position and size of a selected area.
  CppConsUI::Rect getScreenArea(ScreenArea area);
  CppConsUI::Rect getScreenAreaCentered(ScreenArea area);

  static const char *const version_;

  bool loadColorSchemeConfig();
  bool loadKeyConfig();

  bool isEnabledExpandedConversationMode() const { return convs_expanded_; }

  sigc::connection timeoutConnect(const sigc::slot<bool> &slot,
    unsigned interval, int priority = G_PRIORITY_DEFAULT);
  sigc::connection timeoutOnceConnect(const sigc::slot<void> &slot,
    unsigned interval, int priority = G_PRIORITY_DEFAULT);

private:
  struct IOClosurePurple {
    PurpleInputFunction function;
    guint result;
    gpointer data;

    IOClosurePurple() : function(nullptr), result(0), data(nullptr) {}
  };

  GMainLoop *mainloop_;
  bool mainloop_error_exit_;
  CppConsUI::CoreManager *mngr_;
  // Flag indicating if the conversation full-screen mode is activated.
  bool convs_expanded_;
  // Flag indicating if idle reporting is based on keyboard presses.
  bool idle_reporting_on_keyboard_;

  guint stdin_timeout_id_;
  int resize_pipe_[2];
  volatile bool resize_pending_;
  const char *sigwinch_write_error_;
  std::size_t sigwinch_write_error_size_;

  PurpleCoreUiOps centerim_core_ui_ops_;
  PurpleDebugUiOps logbuf_debug_ui_ops_;
  PurpleEventLoopUiOps centerim_glib_eventloops_;

  CppConsUI::Rect areas_[AREAS_NUM];

  static const char *color_names_[];
  static const char *scheme_names_[];

  static CenterIM *my_instance_;

  CenterIM();
  virtual ~CenterIM() {}
  CONSUI_DISABLE_COPY(CenterIM);

  static int run(int argc, char *argv[]);
  friend int main(int argc, char *argv[]);

  int runAll(int argc, char *argv[]);
  void printUsage(FILE *out, const char *prg_name);
  void printVersion(FILE *out);
  int initializePurple(const char *config_path);
  void finalizePurple();
  void initializePreferences();

  int initializeScreenResizing();
  void finalizeScreenResizing();

  // Recalculates area sizes to fit into current screen size.
  void onScreenResized();

  void onTopWindowChanged();

  // PurpleEventLoopUiOps callbacks.
  // Adds IO watch to glib main loop context.
  static guint input_add_purple(int fd, PurpleInputCondition condition,
    PurpleInputFunction function, gpointer data);

  // Helper functions for input_add_purple().
  // Processes IO input to purple callback.
  static gboolean io_input_purple(
    GIOChannel *source, GIOCondition condition, gpointer data);
  // Destroyes libpurple IO input callback internal data.
  static void io_destroy_purple(gpointer data);

  static gboolean stdin_bytes_available_(
    GIOChannel * /*source*/, GIOCondition /*condition*/, gpointer data)
  {
    return reinterpret_cast<CenterIM *>(data)->stdin_bytes_available();
  }
  gboolean stdin_bytes_available();
  static gboolean stdin_timeout_(gpointer data)
  {
    return reinterpret_cast<CenterIM *>(data)->stdin_timeout();
  }
  gboolean stdin_timeout();

  static gboolean resize_bytes_available_(
    GIOChannel * /*source*/, GIOCondition /*condition*/, gpointer data)
  {
    return reinterpret_cast<CenterIM *>(data)->resize_bytes_available();
  }
  gboolean resize_bytes_available();

  // Draws everything.
  static gboolean draw_(gpointer data)
  {
    return reinterpret_cast<CenterIM *>(data)->draw();
  }
  gboolean draw();

  static void sigwinch_handler_(int signum)
  {
    CENTERIM->sigwinch_handler(signum);
  }
  void sigwinch_handler(int signum);

  // CppConsUI callbacks.
  // Registers a redraw request.
  void redraw_cppconsui();
  // Logs a debug message produced by CppConsUI.
  void log_debug_cppconsui(const char *message);

  // PurpleCoreUiOps callbacks.
  // Returns information about CenterIM such as name, website etc.
  static GHashTable *get_ui_info();

  // PurpleDebugUiOps callbacks.
  static void purple_print(
    PurpleDebugLevel level, const char *category, const char *arg_s);
  static gboolean purple_is_enabled(
    PurpleDebugLevel level, const char *category);

  // Called when the CONF_PREFIX/dimensions preferences change their values.
  static void dimensions_change_(
    const char *name, PurplePrefType type, gconstpointer val, gpointer data)
  {
    reinterpret_cast<CenterIM *>(data)->dimensions_change(name, type, val);
  }
  void dimensions_change(
    const char *name, PurplePrefType type, gconstpointer val);

  // Called when the /libpurple/away/idle_reporting preference changes its
  // value.
  static void idle_reporting_change_(
    const char *name, PurplePrefType type, gconstpointer val, gpointer data)
  {
    reinterpret_cast<CenterIM *>(data)->idle_reporting_change(name, type, val);
  }
  void idle_reporting_change(
    const char *name, PurplePrefType type, gconstpointer val);

  // Config handling.
  void loadDefaultColorSchemeConfig();
  bool saveColorSchemeConfig();
  const char *schemeToString(int scheme);
  int stringToScheme(const char *str);
  char *colorToString(int color);
  bool stringToColor(const char *str, int *color);
  char *colorAttributesToString(int attrs);
  bool stringToColorAttributes(const char *str, int *attrs);
  void loadDefaultKeyConfig();
  bool saveKeyConfig();

  void actionFocusBuddyList();
  void actionFocusActiveConversation();
  void actionOpenAccountStatusMenu();
  void actionOpenGeneralMenu();
  void actionBuddyListToggleOffline();
  void actionFocusPrevConversation();
  void actionFocusNextConversation();
  void actionFocusConversation(int i);
  void actionExpandConversation();

  void declareBindables();
};

#endif // CENTERIM_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
