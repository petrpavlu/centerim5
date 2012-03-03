/*
 * Copyright (C) 2009-2012 by CenterIM developers
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

char *KeyConfig::TermKeyToString(TermKeyKey key) const
{
  char out[256];
  termkey_strfkey(COREMANAGER->GetTermKeyHandle(), out, sizeof(out), &key,
      TERMKEY_FORMAT_LONGMOD);

  return g_strdup(out);
}

bool KeyConfig::StringToTermKey(const char *key, TermKeyKey *termkey) const
{
  const char *res = termkey_strpkey(COREMANAGER->GetTermKeyHandle(), key,
      termkey, TERMKEY_FORMAT_LONGMOD);
  return res && !res[0];
}

void KeyConfig::Clear()
{
  binds.clear();
}

void KeyConfig::LoadDefaultKeyConfig()
{
  BindKey("button", "activate", "Enter");

  BindKey("checkbox", "toggle", "Enter");

  BindKey("container", "focus-previous", "Shift-Tab");
  BindKey("container", "focus-next", "Tab");
  BindKey("container", "focus-up", "Up");
  BindKey("container", "focus-down", "Down");
  BindKey("container", "focus-left", "Left");
  BindKey("container", "focus-right", "Right");
  BindKey("container", "focus-page-up", "PageUp");
  BindKey("container", "focus-page-down", "PageDown");
  BindKey("container", "focus-begin", "Home");
  BindKey("container", "focus-end", "End");

  BindKey("coremanager", "redraw-screen", "Ctrl-l");

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

  BindKey("textentry", "delete-word-end", "Ctrl-Delete");
  BindKey("textentry", "delete-word-begin", "Ctrl-Backspace");
  /// @todo enable
  /*
  BindKey("textentry", "toggle-overwrite", "Insert");
  */

  BindKey("textentry", "activate", "Enter");

  BindKey("textview", "scroll-up", "PageUp");
  BindKey("textview", "scroll-down", "PageDown");

  BindKey("treeview", "fold-subtree", "-");
  BindKey("treeview", "unfold-subtree", "+");

  BindKey("window", "close-window", "Escape");
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 tw=78 expandtab : */
