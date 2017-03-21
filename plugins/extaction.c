// Copyright (C) 2012-2013 Petr Pavlu <setup@dagobah.cz>
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

// Plugin to support external-action functionality in CenterIM5.
//
// When an event such as received-im-msg, or buddy-signed-on occurs this plugin
// asynchronously executes a user-defined external program.
//
// An example how to use this plugin can be found in contrib/extnotify.py.
//
// TODO Add support for more kinds of events, currently only received-im-msg and
// received-chat-msg actions are supported.

#define PURPLE_PLUGINS

#include <glib.h>
#include <libpurple/purple.h>
#define DEFAULT_TEXT_DOMAIN PACKAGE_NAME
#include "gettext.h"

#define PLUGIN_ID "core-cim_pack-ext_action"
#define PLUGIN_PREF "/plugins/core/cim_pack-extaction"
#define PLUGIN_PREF_COMMAND PLUGIN_PREF "/command"

#define UNUSED(x) (void)(x)

static PurplePlugin *ea_plugin = NULL;

static void on_new_message(
  PurpleAccount *account, const char *remote, const char *message)
{
  const char *command = purple_prefs_get_path(PLUGIN_PREF_COMMAND);

  // The command should be never NULL.
  g_return_if_fail(command != NULL);

  if (command[0] == '\0') {
    // No command is set.
    return;
  }

  const char *protocol = purple_account_get_protocol_name(account);
  char *local =
    g_strdup(purple_normalize(account, purple_account_get_username(account)));
  char *nohtml = purple_markup_strip_html(message);
  PurpleBuddy *buddy = purple_find_buddy(account, remote);
  char *icon_encoded = NULL;
  if (buddy != NULL) {
    // Get buddy alias and icon.
    remote = purple_buddy_get_alias(buddy);
    PurpleBuddyIcon *icon = purple_buddy_get_icon(buddy);
    if (icon != NULL) {
      size_t len;
      gconstpointer data = purple_buddy_icon_get_data(icon, &len);
      icon_encoded = g_base64_encode(data, len);
    }
  }

  char *argv[2];
  argv[0] = g_strdup(command);
  argv[1] = NULL;

  // Prepare child's environment variables.
  char **envp = g_get_environ();
  envp = g_environ_setenv(envp, "EVENT_TYPE", "msg", TRUE);
  envp = g_environ_setenv(envp, "EVENT_NETWORK", protocol, TRUE);
  envp = g_environ_setenv(envp, "EVENT_LOCAL_USER", local, TRUE);
  envp = g_environ_setenv(envp, "EVENT_REMOTE_USER", remote, TRUE);
  if (icon_encoded != NULL)
    envp = g_environ_setenv(envp, "EVENT_REMOTE_USER_ICON", icon_encoded, TRUE);
  envp = g_environ_setenv(envp, "EVENT_MESSAGE", nohtml, TRUE);
  envp = g_environ_setenv(envp, "EVENT_MESSAGE_HTML", message, TRUE);

  // Spawn the command.
  GError *err = NULL;
  if (!g_spawn_async(NULL, argv, envp, G_SPAWN_SEARCH_PATH |
          G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL,
        NULL, NULL, NULL, &err)) {
    purple_debug_error("extaction", "%s", err->message);
    g_clear_error(&err);
  }

  // Free all resources.
  g_free(argv[0]);
  g_strfreev(envp);

  g_free(local);
  g_free(nohtml);
  g_free(icon_encoded);
}

static void on_new_im_message(PurpleAccount *account, const char *name,
  const char *message, PurpleConversation *conv, PurpleMessageFlags flags,
  gpointer data)
{
  UNUSED(conv);
  UNUSED(flags);
  UNUSED(data);

  on_new_message(account, name, message);
}

static void on_new_chat_message(PurpleAccount *account, const char *who,
  const char *message, PurpleConversation *conv, PurpleMessageFlags flags,
  gpointer data)
{
  UNUSED(conv);
  UNUSED(flags);
  UNUSED(data);

  on_new_message(account, who, message);
}

static gboolean plugin_load(PurplePlugin *plugin)
{
  ea_plugin = plugin;

  void *conv_handle = purple_conversations_get_handle();

  // Connect callbacks.
  purple_signal_connect(conv_handle, "received-im-msg", plugin,
    PURPLE_CALLBACK(on_new_im_message), NULL);

  purple_signal_connect(conv_handle, "received-chat-msg", plugin,
    PURPLE_CALLBACK(on_new_chat_message), NULL);

  return TRUE;
}

static gboolean plugin_unload(PurplePlugin *plugin)
{
  // Disconnect callbacks.
  purple_signals_disconnect_by_handle(plugin);
  return TRUE;
}

// UI
static PurplePluginPrefFrame *plugin_get_pref_frame(PurplePlugin *plugin)
{
  UNUSED(plugin);

  PurplePluginPrefFrame *frame = purple_plugin_pref_frame_new();

  PurplePluginPref *pref = purple_plugin_pref_new_with_name_and_label(
    PLUGIN_PREF_COMMAND, _("Command"));
  purple_plugin_pref_frame_add(frame, pref);

  return frame;
}

// clang-format off
static PurplePluginUiInfo prefs_info = {
  plugin_get_pref_frame,
  0,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

static PurplePluginInfo info = {
  PURPLE_PLUGIN_MAGIC,
  PURPLE_MAJOR_VERSION,
  PURPLE_MINOR_VERSION,
  PURPLE_PLUGIN_STANDARD,
  NULL,
  0,
  NULL,
  PURPLE_PRIORITY_DEFAULT,
  PLUGIN_ID,
  N_("External actions"),
  "1.0",
  N_("Executes an external program when a specific event occurs."),
  N_("When an event such as received-im-msg, or buddy-signed-on occurs this "
      "plugin executes a user-defined external program."),
  "Petr Pavlu <setup@dagobah.cz>",
  PACKAGE_URL,
  plugin_load,
  plugin_unload,
  NULL,
  NULL,
  NULL,
  &prefs_info,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};
// clang-format on

static void init_plugin(PurplePlugin *plugin)
{
  UNUSED(plugin);

  purple_prefs_add_none(PLUGIN_PREF);
  purple_prefs_add_path(PLUGIN_PREF_COMMAND, "");
}

PURPLE_INIT_PLUGIN(extaction, init_plugin, info)
