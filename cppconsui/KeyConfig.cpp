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

KeyConfig *KeyConfig::Instance()
{
  static KeyConfig instance;
  return &instance;
}

void KeyConfig::RegisterKeyDef(const char *context, const char *action,
    const char *key)
{
  TermKeyKey tkey;
  const char *res = termkey_strpkey(CoreManager::GetTermKeyHandle(), key,
      &tkey, TERMKEY_FORMAT_LONGMOD);
  g_assert(res && !res[0]);

  bindables.push_back(Bindable(context, action, tkey));
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

void KeyConfig::Reconfig()
{
  /**
   * @todo Read the config and assign it to keys.
   */
}

void KeyConfig::Register()
{
  RegisterKeyDef("button", "activate", "Enter");

  RegisterKeyDef("checkbox", "toggle", "Enter");

  RegisterKeyDef("container", "focus-previous", "Shift-Tab");
  RegisterKeyDef("container", "focus-next", "Tab");
  RegisterKeyDef("container", "focus-left", "Left");
  RegisterKeyDef("container", "focus-right", "Right");
  RegisterKeyDef("container", "focus-up", "Up");
  RegisterKeyDef("container", "focus-down", "Down");

  RegisterKeyDef("coremanager", "redraw-screen", "Ctrl-l");

  RegisterKeyDef("window", "close-window", "Escape");

  RegisterKeyDef("textentry", "cursor-right", "Right");
  RegisterKeyDef("textentry", "cursor-left", "Left");
  RegisterKeyDef("textentry", "cursor-down", "Down");
  RegisterKeyDef("textentry", "cursor-up", "Right");
  RegisterKeyDef("textentry", "cursor-right-word", "Ctrl-Right");
  RegisterKeyDef("textentry", "cursor-left-word", "Ctrl-Left");
  RegisterKeyDef("textentry", "cursor-end", "End");
  RegisterKeyDef("textentry", "cursor-begin", "Home");
  RegisterKeyDef("textentry", "delete-char", "Delete");
  RegisterKeyDef("textentry", "backspace", "Backspace");

  // XXX move to default key bindings config
  RegisterKeyDef("textentry", "backspace", "DEL");

  /// @todo enable
  /*
  RegisterKeyDef("textentry", "delete-word-end", "Ctrl-Delete");
  RegisterKeyDef("textentry", "delete-word-begin", "Ctrl-Backspace");

  // XXX move to default key bindings config
  RegisterKeyDef("textentry", "delete-word-begin", "Ctrl-Delete");

  RegisterKeyDef("textentry", "toggle-overwrite", "Insert");
  */

  RegisterKeyDef("textentry", "cursor-right", "Right");
  RegisterKeyDef("textentry", "cursor-left", "Left");
  RegisterKeyDef("textentry", "cursor-right-word", "Ctrl-Right");
  RegisterKeyDef("textentry", "cursor-left-word", "Ctrl-Left");
  RegisterKeyDef("textentry", "cursor-end", "End");
  RegisterKeyDef("textentry", "cursor-begin", "Home");
  RegisterKeyDef("textentry", "delete-char", "Delete");
  RegisterKeyDef("textentry", "backspace", "Backspace");

  // XXX move to default key bindings config
  RegisterKeyDef("textentry", "backspace", "DEL");
  RegisterKeyDef("textentry", "tab", "Tab");

  /// @todo enable
  /*
  RegisterKeyDef("textentry", "delete-word-end", "Ctrl-Delete");
  RegisterKeyDef("textentry", "delete-word-begin", "Ctrl-Backspace");

  // XXX move to default key bindings config
  RegisterKeyDef("textentry", "delete-word-begin", "Ctrl-Delete");

  RegisterKeyDef("textentry", "toggle-overwrite", "Insert");
  */

  RegisterKeyDef("textentry", "activate", "Enter");

  RegisterKeyDef("textview", "scroll-up", "PageUp");
  RegisterKeyDef("textview", "scroll-down", "PageDown");

  RegisterKeyDef("treeview", "fold-subtree", "-");
  RegisterKeyDef("treeview", "unfold-subtree", "+");
}
