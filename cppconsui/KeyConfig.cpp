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

KeyConfig *KeyConfig::Instance()
{
  static KeyConfig instance;
  return &instance;
}

void KeyConfig::RegisterKeyDef(const char *context, const char *action,
    const TermKeyKey &key)
{
  bindables.push_back(Bindable(context, action, key));
  binds[context][key] = action;
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
  RegisterKeyDef("button", "activate",
      Keys::SymbolTermKey(TERMKEY_SYM_ENTER));

  RegisterKeyDef("checkbox", "toggle",
      Keys::SymbolTermKey(TERMKEY_SYM_ENTER));

  RegisterKeyDef("container", "focus-previous",
      Keys::SymbolTermKey(TERMKEY_SYM_TAB, TERMKEY_KEYMOD_SHIFT));
  RegisterKeyDef("container", "focus-next",
      Keys::SymbolTermKey(TERMKEY_SYM_TAB));
  RegisterKeyDef("container", "focus-left",
      Keys::SymbolTermKey(TERMKEY_SYM_LEFT));
  RegisterKeyDef("container", "focus-right",
      Keys::SymbolTermKey(TERMKEY_SYM_RIGHT));
  RegisterKeyDef("container", "focus-up",
      Keys::SymbolTermKey(TERMKEY_SYM_UP));
  RegisterKeyDef("container", "focus-down",
      Keys::SymbolTermKey(TERMKEY_SYM_DOWN));

  RegisterKeyDef("coremanager", "redraw-screen",
      Keys::UnicodeTermKey("l", TERMKEY_KEYMOD_CTRL));

  RegisterKeyDef("window", "close-window",
      Keys::SymbolTermKey(TERMKEY_SYM_ESCAPE));

  RegisterKeyDef("textentry", "cursor-right",
      Keys::SymbolTermKey(TERMKEY_SYM_RIGHT));
  RegisterKeyDef("textentry", "cursor-left",
      Keys::SymbolTermKey(TERMKEY_SYM_LEFT));
  RegisterKeyDef("textentry", "cursor-down",
      Keys::SymbolTermKey(TERMKEY_SYM_DOWN));
  RegisterKeyDef("textentry", "cursor-up",
      Keys::SymbolTermKey(TERMKEY_SYM_UP));
  RegisterKeyDef("textentry", "cursor-right-word",
      Keys::SymbolTermKey(TERMKEY_SYM_RIGHT, TERMKEY_KEYMOD_CTRL));
  RegisterKeyDef("textentry", "cursor-left-word",
      Keys::SymbolTermKey(TERMKEY_SYM_LEFT, TERMKEY_KEYMOD_CTRL));
  RegisterKeyDef("textentry", "cursor-end",
      Keys::SymbolTermKey(TERMKEY_SYM_END));
  RegisterKeyDef("textentry", "cursor-begin",
      Keys::SymbolTermKey(TERMKEY_SYM_HOME));
  RegisterKeyDef("textentry", "delete-char",
      Keys::SymbolTermKey(TERMKEY_SYM_DELETE));
  RegisterKeyDef("textentry", "backspace",
      Keys::SymbolTermKey(TERMKEY_SYM_BACKSPACE));

  // XXX move to default key bindings config
  RegisterKeyDef("textentry", "backspace",
      Keys::SymbolTermKey(TERMKEY_SYM_DEL));

  /// @todo enable
  /*
  RegisterKeyDef("textentry", "delete-word-end",
      Keys::SymbolTermKey(TERMKEY_SYM_DELETE, TERMKEY_KEYMOD_CTRL));
  RegisterKeyDef("textentry", "delete-word-begin",
      Keys::SymbolTermKey(TERMKEY_SYM_BACKSPACE, TERMKEY_KEYMOD_CTRL));

  // XXX move to default key bindings config
  RegisterKeyDef("textentry", "delete-word-begin",
      Keys::SymbolTermKey(TERMKEY_SYM_DEL, TERMKEY_KEYMOD_CTRL));

  RegisterKeyDef("textentry", "toggle-overwrite",
      Keys::SymbolTermKey(TERMKEY_SYM_INSERT));
  */

  RegisterKeyDef("textentry", "cursor-right",
      Keys::SymbolTermKey(TERMKEY_SYM_RIGHT));
  RegisterKeyDef("textentry", "cursor-left",
      Keys::SymbolTermKey(TERMKEY_SYM_LEFT));
  RegisterKeyDef("textentry", "cursor-right-word",
      Keys::SymbolTermKey(TERMKEY_SYM_RIGHT, TERMKEY_KEYMOD_CTRL));
  RegisterKeyDef("textentry", "cursor-left-word",
      Keys::SymbolTermKey(TERMKEY_SYM_LEFT, TERMKEY_KEYMOD_CTRL));
  RegisterKeyDef("textentry", "cursor-end",
      Keys::SymbolTermKey(TERMKEY_SYM_END));
  RegisterKeyDef("textentry", "cursor-begin",
      Keys::SymbolTermKey(TERMKEY_SYM_HOME));
  RegisterKeyDef("textentry", "delete-char",
      Keys::SymbolTermKey(TERMKEY_SYM_DELETE));
  RegisterKeyDef("textentry", "backspace",
      Keys::SymbolTermKey(TERMKEY_SYM_BACKSPACE));

  // XXX move to default key bindings config
  RegisterKeyDef("textentry", "backspace",
      Keys::SymbolTermKey(TERMKEY_SYM_DEL));
  RegisterKeyDef("textentry", "tab",
      Keys::SymbolTermKey(TERMKEY_SYM_TAB));

  /// @todo enable
  /*
  RegisterKeyDef("textentry", "delete-word-end",
      Keys::SymbolTermKey(TERMKEY_SYM_DELETE, TERMKEY_KEYMOD_CTRL));
  RegisterKeyDef("textentry", "delete-word-begin",
      Keys::SymbolTermKey(TERMKEY_SYM_BACKSPACE, TERMKEY_KEYMOD_CTRL));

  // XXX move to default key bindings config
  RegisterKeyDef("textentry", "delete-word-begin",
      Keys::SymbolTermKey(TERMKEY_SYM_DEL, TERMKEY_KEYMOD_CTRL));

  RegisterKeyDef("textentry", "toggle-overwrite",
      Keys::SymbolTermKey(TERMKEY_SYM_INSERT));
  */

  RegisterKeyDef("textentry", "activate",
      Keys::SymbolTermKey(TERMKEY_SYM_ENTER));

  RegisterKeyDef("treeview", "fold-subtree", Keys::UnicodeTermKey("-"));
  RegisterKeyDef("treeview", "unfold-subtree", Keys::UnicodeTermKey("+"));
}
