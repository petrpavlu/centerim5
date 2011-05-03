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

KeyConfig *KeyConfig::Instance()
{
  static KeyConfig instance;
  return &instance;
}

void KeyConfig::BindKey(const char *context, const char *action,
    const char *key)
{
  TermKeyKey tkey;
  const char *res = termkey_strpkey(COREMANAGER->GetTermKeyHandle(), key,
      &tkey, TERMKEY_FORMAT_LONGMOD);
  if (!res || res[0]) {
    g_warning(_("Unrecognized key (%s)."), key);
    return;
  }

  binds[context][tkey] = action;
}

const KeyConfig::KeyBindContext *KeyConfig::GetKeyBinds(
    const char *context) const
{
  KeyBinds::const_iterator i = binds.find(context);
  if (i == binds.end())
    return NULL;
  return &i->second;
}

char *KeyConfig::GetKeyBind(const char *context, const char *action) const
{
  KeyBinds::const_iterator i = binds.find(context);
  if (i == binds.end())
    return NULL;

  for (KeyBindContext::const_iterator j = i->second.begin();
      j != i->second.end(); j++)
    if (!j->second.compare(action)) {
      TermKeyKey key = j->first;
      char out[256];
      termkey_strfkey(COREMANAGER->GetTermKeyHandle(), out, sizeof(out), &key,
          TERMKEY_FORMAT_CARETCTRL);
      return g_strdup(out);
    }

  return NULL;
}

void KeyConfig::SetConfigFile(const char *filename)
{
  if (config)
    g_free(config);

  if (filename)
    config = g_strdup(filename);
  else
    config = NULL;
}

void KeyConfig::Reconfig()
{
  if (!config)
    return;

  /**
   * @todo Read the config and assign it to keys.
   */
}

void KeyConfig::RegisterDefaultKeys()
{
  BindKey("button", "activate", "Enter");

  BindKey("checkbox", "toggle", "Enter");

  BindKey("container", "focus-previous", "Shift-Tab");
  BindKey("container", "focus-next", "Tab");
  BindKey("container", "focus-left", "Left");
  BindKey("container", "focus-right", "Right");
  BindKey("container", "focus-up", "Up");
  BindKey("container", "focus-down", "Down");

  BindKey("coremanager", "redraw-screen", "Ctrl-l");

  BindKey("window", "close-window", "Escape");

  BindKey("textentry", "cursor-right", "Right");
  BindKey("textentry", "cursor-left", "Left");
  BindKey("textentry", "cursor-down", "Down");
  BindKey("textentry", "cursor-up", "Up");
  BindKey("textentry", "cursor-right-word", "Ctrl-Right");
  BindKey("textentry", "cursor-left-word", "Ctrl-Left");
  BindKey("textentry", "cursor-end", "End");
  BindKey("textentry", "cursor-begin", "Home");
  BindKey("textentry", "delete-char", "Delete");
  BindKey("textentry", "backspace", "Backspace");

  // XXX move to default key bindings config
  BindKey("textentry", "backspace", "DEL");

  /// @todo enable
  /*
  BindKey("textentry", "delete-word-end", "Ctrl-Delete");
  BindKey("textentry", "delete-word-begin", "Ctrl-Backspace");

  // XXX move to default key bindings config
  BindKey("textentry", "delete-word-begin", "Ctrl-DEL");

  BindKey("textentry", "toggle-overwrite", "Insert");
  */

  BindKey("textentry", "cursor-right", "Right");
  BindKey("textentry", "cursor-left", "Left");
  BindKey("textentry", "cursor-right-word", "Ctrl-Right");
  BindKey("textentry", "cursor-left-word", "Ctrl-Left");
  BindKey("textentry", "cursor-end", "End");
  BindKey("textentry", "cursor-begin", "Home");
  BindKey("textentry", "delete-char", "Delete");
  BindKey("textentry", "backspace", "Backspace");

  // XXX move to default key bindings config
  BindKey("textentry", "backspace", "DEL");
  BindKey("textentry", "tab", "Tab");

  /// @todo enable
  /*
  BindKey("textentry", "delete-word-end", "Ctrl-Delete");
  BindKey("textentry", "delete-word-begin", "Ctrl-Backspace");

  // XXX move to default key bindings config
  BindKey("textentry", "delete-word-begin", "Ctrl-DEL");

  BindKey("textentry", "toggle-overwrite", "Insert");
  */

  BindKey("textentry", "activate", "Enter");

  BindKey("textview", "scroll-up", "PageUp");
  BindKey("textview", "scroll-down", "PageDown");

  BindKey("treeview", "fold-subtree", "-");
  BindKey("treeview", "unfold-subtree", "+");
}

KeyConfig::KeyConfig()
: config(NULL)
{
}

KeyConfig::~KeyConfig()
{
  if (config)
    g_free(config);
}
