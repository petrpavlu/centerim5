// Copyright (C) 2009-2015 Petr Pavlu <setup@dagobah.cz>
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

/// @file
/// KeyConfig class implementation.
///
/// @ingroup cppconsui

#include "KeyConfig.h"

#include "CoreManager.h"

#include <cstring>
#include "gettext.h"

namespace CppConsUI {

bool KeyConfig::bindKey(
  const char *context, const char *action, const char *key)
{
  TermKeyKey tkey;
  const char *res = termkey_strpkey(
    COREMANAGER->getTermKeyHandle(), key, &tkey, TERMKEY_FORMAT_LONGMOD);
  if (res == NULL || res[0] != '\0')
    return false;

  binds_[context][tkey] = action;
  return true;
}

const KeyConfig::KeyBindContext *KeyConfig::getKeyBinds(
  const char *context) const
{
  KeyBinds::const_iterator i = binds_.find(context);
  if (i == binds_.end())
    return NULL;
  return &i->second;
}

const char *KeyConfig::getKeyBind(const char *context, const char *action) const
{
  KeyBinds::const_iterator i = binds_.find(context);
  if (i == binds_.end())
    return NULL;

  for (KeyBindContext::const_iterator j = i->second.begin();
       j != i->second.end(); ++j)
    if (!j->second.compare(action)) {
      TermKeyKey key = j->first;
      static char out[256];
      termkey_strfkey(COREMANAGER->getTermKeyHandle(), out, sizeof(out), &key,
        TERMKEY_FORMAT_CARETCTRL);
      return out;
    }

  return _("<unbound>");
}

char *KeyConfig::termKeyToString(const TermKeyKey &key) const
{
  TermKeyKey key_copy = key;
  char out[256];
  termkey_strfkey(COREMANAGER->getTermKeyHandle(), out, sizeof(out), &key_copy,
    TERMKEY_FORMAT_LONGMOD);

  size_t size = std::strlen(out) + 1;
  char *res = new char[size];
  std::strcpy(res, out);
  return res;
}

bool KeyConfig::stringToTermKey(const char *key, TermKeyKey *termkey) const
{
  const char *res = termkey_strpkey(
    COREMANAGER->getTermKeyHandle(), key, termkey, TERMKEY_FORMAT_LONGMOD);
  return res != NULL && res[0] == '\0';
}

void KeyConfig::clear()
{
  binds_.clear();
}

void KeyConfig::loadDefaultKeyConfig()
{
  // Clear current bindings.
  clear();

  bindKey("button", "activate", "Enter");

  bindKey("checkbox", "toggle", "Enter");

  bindKey("container", "focus-previous", "Shift-Tab");
  bindKey("container", "focus-next", "Tab");
  bindKey("container", "focus-up", "Up");
  bindKey("container", "focus-down", "Down");
  bindKey("container", "focus-left", "Left");
  bindKey("container", "focus-right", "Right");
  bindKey("container", "focus-page-up", "PageUp");
  bindKey("container", "focus-page-down", "PageDown");
  bindKey("container", "focus-begin", "Home");
  bindKey("container", "focus-end", "End");

  bindKey("coremanager", "redraw-screen", "Ctrl-l");

  bindKey("textentry", "cursor-right", "Right");
  bindKey("textentry", "cursor-left", "Left");
  bindKey("textentry", "cursor-down", "Down");
  bindKey("textentry", "cursor-up", "Up");
  bindKey("textentry", "cursor-right-word", "Ctrl-Right");
  bindKey("textentry", "cursor-left-word", "Ctrl-Left");
  bindKey("textentry", "cursor-end", "End");
  bindKey("textentry", "cursor-begin", "Home");
  bindKey("textentry", "delete-char", "Delete");
  bindKey("textentry", "backspace", "Backspace");

  bindKey("textentry", "delete-word-end", "Ctrl-Delete");
  /// @todo Enable toogle-overwrite bind.
  // bindKey("textentry", "toggle-overwrite", "Insert");

  bindKey("textentry", "activate", "Enter");

  bindKey("textview", "scroll-up", "PageUp");
  bindKey("textview", "scroll-down", "PageDown");

  bindKey("treeview", "fold-subtree", "-");
  bindKey("treeview", "unfold-subtree", "+");

  bindKey("window", "close-window", "Escape");
}

} // namespace CppConsUI

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
