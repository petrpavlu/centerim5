/*
 * Copyright (C) 2009-2011 by CenterIM developers
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

/**
 * @file
 * KeyConfig class implementation.
 *
 * @ingroup cppconsui
 */

#include "KeyConfig.h"
#include "CoreManager.h"

#include "gettext.h"

namespace CppConsUI
{

KeyConfig *KeyConfig::Instance()
{
  static KeyConfig instance;
  return &instance;
}

bool KeyConfig::BindKey(const char *context, const char *action,
    const char *key)
{
  TermKeyKey tkey;
  const char *res = termkey_strpkey(COREMANAGER->GetTermKeyHandle(), key,
      &tkey, TERMKEY_FORMAT_LONGMOD);
  if (!res || res[0])
    return false;

  binds[context][tkey] = action;
  return true;
}

const KeyConfig::KeyBindContext *KeyConfig::GetKeyBinds(
    const char *context) const
{
  KeyBinds::const_iterator i = binds.find(context);
  if (i == binds.end())
    return NULL;
  return &i->second;
}

const char *KeyConfig::GetKeyBind(const char *context,
    const char *action) const
{
  KeyBinds::const_iterator i = binds.find(context);
  if (i == binds.end())
    return NULL;

  for (KeyBindContext::const_iterator j = i->second.begin();
      j != i->second.end(); j++)
    if (!j->second.compare(action)) {
      TermKeyKey key = j->first;
      static char out[256];
      termkey_strfkey(COREMANAGER->GetTermKeyHandle(), out, sizeof(out), &key,
          TERMKEY_FORMAT_CARETCTRL);
      return out;
    }

  return _("<unbound>");
}

void KeyConfig::SetConfigFile(const char *filename)
{
  if (config)
    g_free(config);

  config = g_strdup(filename);
}

void KeyConfig::Clear()
{
  binds.clear();
}

bool KeyConfig::Reconfig()
{
  g_assert(config);

  Clear();

  if (!ReconfigInternal()) {
    // fallback to default key binds
    Clear();
    RegisterDefaultKeyBinds();
    return false;
  }

  return true;
}

void KeyConfig::AddDefaultKeyBind(const char *context, const char *action,
      const char *key)
{
  default_key_binds.push_back(DefaultKeyBind(context, action, key));
}

void KeyConfig::RegisterDefaultKeyBinds()
{
  Clear();

  for (DefaultKeyBinds::iterator i = default_key_binds.begin();
      i != default_key_binds.end(); i++)
    if (!BindKey(i->context.c_str(), i->action.c_str(), i->key.c_str()))
      g_warning(_("Unrecognized key '%s' in default keybinds."),
          i->key.c_str());
}

bool KeyConfig::ReconfigInternal()
{
  g_assert(config);

  // read the file contents
  char *contents;
  gsize length;
  GError *err = NULL;
  if (!g_file_get_contents(config, &contents, &length, &err)) {
    // generate default keybinding file
    err = NULL;
    GIOChannel *chan;
    if (!(chan = g_io_channel_new_file(config, "w", &err))) {
      if (err) {
        g_warning(_("Error opening keybinding file '%s' (%s)."),
            config, err->message);
        g_error_free(err);
        err = NULL;
      }
      else
        g_warning(_("Error opening keybinding file '%s'."), config);
      return false;
    }

#define ERROR()                                                   \
do {                                                              \
if (err) {                                                        \
  g_warning(_("Error writing to keybinding file '%s' (%s)."),     \
      config, err->message);                                      \
  g_error_free(err);                                              \
  err = NULL;                                                     \
}                                                                 \
else                                                              \
  g_warning(_("Error writing to keybinding file '%s'."), config); \
g_io_channel_unref(chan);                                         \
return false;                                                     \
} while (0)

    const char *buf = "<?xml version='1.0' encoding='UTF-8' ?>\n\n"
                      "<keyconfig version='1.0'>\n";
    if (g_io_channel_write_chars(chan, buf, -1, NULL, &err)
        != G_IO_STATUS_NORMAL)
      ERROR();

    for (DefaultKeyBinds::iterator i = default_key_binds.begin();
        i != default_key_binds.end(); i++) {
      char *buf2 = g_strdup_printf(
          "\t<bind context='%s' action='%s' key='%s'/>\n",
          i->context.c_str(), i->action.c_str(), i->key.c_str());
      GIOStatus s = g_io_channel_write_chars(chan, buf2, -1, NULL, &err);
      g_free(buf2);

      if (s != G_IO_STATUS_NORMAL)
        ERROR();
    }

    buf = "</keyconfig>\n";
    if (g_io_channel_write_chars(chan, buf, -1, NULL, &err)
        != G_IO_STATUS_NORMAL)
      ERROR();

#undef ERROR

    g_io_channel_unref(chan);

    if (!g_file_get_contents(config, &contents, &length, &err)) {
      if (err) {
        g_warning(_("Error reading keybinding file '%s' (%s)."), config,
            err->message);
        g_error_free(err);
        err = NULL;
      }
      else
        g_warning(_("Error reading keybinding file '%s'."), config);
      return false;
    }
  }

  // parse the file
  bool res = true;
  GMarkupParser parser = {};
  parser.start_element = start_element_;
  GMarkupParseContext *context = g_markup_parse_context_new(&parser,
      G_MARKUP_PREFIX_ERROR_POSITION, this, NULL);
  if (!g_markup_parse_context_parse(context, contents, length, &err)
      || !g_markup_parse_context_end_parse(context, &err)) {
    if (err) {
      g_warning(_("Error parsing keybinding file '%s' (%s)."), config,
          err->message);
      g_error_free(err);
      err = NULL;
    }
    else
      g_warning(_("Error parsing keybinding file '%s'."), config);
    res = false;
  }
  g_markup_parse_context_free(context);
  g_free(contents);

  return res;
}

void KeyConfig::start_element(GMarkupParseContext *context,
    const char *element_name, const char **attribute_names,
    const char **attribute_values, GError **error)
{
  const GSList *stack = g_markup_parse_context_get_element_stack(context);
  guint size = g_slist_length(const_cast<GSList*>(stack));
  if (size == 1) {
    if (strcmp(element_name, "keyconfig")) {
      *error = g_error_new(g_markup_error_quark(), G_MARKUP_ERROR_PARSE,
          _("Expected 'keyconfig' element, found '%s'"), element_name);
    }
  }
  else if (size == 2) {
    if (strcmp(element_name, "bind")) {
      *error = g_error_new(g_markup_error_quark(), G_MARKUP_ERROR_PARSE,
          _("Expected 'bind' element, found '%s'"), element_name);
      return;
    }

    const char *context;
    const char *action;
    const char *key;
    if (!g_markup_collect_attributes (element_name, attribute_names,
          attribute_values, error,
          G_MARKUP_COLLECT_STRING, "context", &context,
          G_MARKUP_COLLECT_STRING, "action", &action,
          G_MARKUP_COLLECT_STRING, "key", &key,
          G_MARKUP_COLLECT_INVALID))
      return;

    if (!BindKey(context, action, key)) {
      *error = g_error_new(g_markup_error_quark(), G_MARKUP_ERROR_INVALID_CONTENT,
          _("Unrecognized key '%s'"), key);
      return;
    }
  }
  else
    *error = g_error_new(g_markup_error_quark(), G_MARKUP_ERROR_PARSE,
        _("Unexpected element '%s'"), element_name);
}

KeyConfig::KeyConfig()
: config(NULL)
{
  AddDefaultKeyBind("button", "activate", "Enter");

  AddDefaultKeyBind("checkbox", "toggle", "Enter");

  AddDefaultKeyBind("container", "focus-previous", "Shift-Tab");
  AddDefaultKeyBind("container", "focus-next", "Tab");
  AddDefaultKeyBind("container", "focus-up", "Up");
  AddDefaultKeyBind("container", "focus-down", "Down");
  AddDefaultKeyBind("container", "focus-left", "Left");
  AddDefaultKeyBind("container", "focus-right", "Right");
  AddDefaultKeyBind("container", "focus-page-up", "PageUp");
  AddDefaultKeyBind("container", "focus-page-down", "PageDown");
  AddDefaultKeyBind("container", "focus-begin", "Home");
  AddDefaultKeyBind("container", "focus-end", "End");

  AddDefaultKeyBind("coremanager", "redraw-screen", "Ctrl-l");

  AddDefaultKeyBind("textentry", "cursor-right", "Right");
  AddDefaultKeyBind("textentry", "cursor-left", "Left");
  AddDefaultKeyBind("textentry", "cursor-down", "Down");
  AddDefaultKeyBind("textentry", "cursor-up", "Up");
  AddDefaultKeyBind("textentry", "cursor-right-word", "Ctrl-Right");
  AddDefaultKeyBind("textentry", "cursor-left-word", "Ctrl-Left");
  AddDefaultKeyBind("textentry", "cursor-end", "End");
  AddDefaultKeyBind("textentry", "cursor-begin", "Home");
  AddDefaultKeyBind("textentry", "delete-char", "Delete");
  AddDefaultKeyBind("textentry", "backspace", "Backspace");

  AddDefaultKeyBind("textentry", "delete-word-end", "Ctrl-Delete");
  // XXX Is Ctrl-Backspace a valid combination?
  AddDefaultKeyBind("textentry", "delete-word-begin", "Ctrl-Backspace");
  /// @todo enable
  /*
  AddDefaultKeyBind("textentry", "toggle-overwrite", "Insert");
  */

  AddDefaultKeyBind("textentry", "activate", "Enter");

  AddDefaultKeyBind("textview", "scroll-up", "PageUp");
  AddDefaultKeyBind("textview", "scroll-down", "PageDown");

  AddDefaultKeyBind("treeview", "fold-subtree", "-");
  AddDefaultKeyBind("treeview", "unfold-subtree", "+");

  AddDefaultKeyBind("window", "close-window", "Escape");
}

KeyConfig::~KeyConfig()
{
  if (config)
    g_free(config);
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 expandtab */
