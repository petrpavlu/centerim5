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
// along with CenterIM.  If not, see <http://www.gnu.org/licenses/>.

/// @file
/// KeyConfig singleton class.
///
/// @ingroup cppconsui

#ifndef KEYCONFIG_H
#define KEYCONFIG_H

#include "CppConsUI.h"
#include "Keys.h"

#include <map>
#include <string>
#include <termkey.h>

namespace CppConsUI {

/// This singleton class is used to keep the key definitions. It holds the
/// context, action, description, the default and the configured key values.
///
/// The use case for a class X derived from @ref InputProcessor class is the
/// following:
/// \code
/// class X : public Y {
/// private:
///   void declareBindables();
/// };
///
/// void X::declareBindable()
/// {
///   // Register a bindable.
///   declareBindable("context", "action", sigc::mem_fun(this, X::onActionDo));
///
///   // Register a bindable conditionally.
///   if (some condition)
///     declareBindable("context", "action2", sigc::mem_fun(this,
///       X::onAction2Do));
/// }
/// \endcode
class KeyConfig {
public:
  /// Maps keys to actions for one context, {key: action}.
  typedef std::map<TermKeyKey, std::string, Keys::TermKeyCmp> KeyBindContext;

  /// Maps context to key binds in that context, {context: KeyContext}.
  typedef std::map<std::string, KeyBindContext> KeyBinds;

  /// Binds a key to an action (in a given context).
  bool bindKey(const char *context, const char *action, const char *key);

  /// Returns all key binds.
  const KeyBinds *getKeyBinds() const { return &binds_; }

  /// Returns all key binds for a given context.
  const KeyBindContext *getKeyBinds(const char *context) const;

  /// Returns a key bind for a given context and action. Note that this method
  /// returns a pointer to a static buffer.
  const char *getKeyBind(const char *context, const char *action) const;

  /// Converts a TermKeyKey to its string representation.
  char *termKeyToString(const TermKeyKey &key) const;

  /// Parses a string into a TermKeyKey.
  bool stringToTermKey(const char *key, TermKeyKey *termkey) const;

  /// Removes all key binds.
  void clear();

  /// Loads default key configuration.
  void loadDefaultKeyConfig();

private:
  /// Current key binds.
  KeyBinds binds_;

  KeyConfig() {}
  ~KeyConfig() {}
  CONSUI_DISABLE_COPY(KeyConfig);

  friend void initializeConsUI(AppInterface &interface);
  friend void finalizeConsUI();
};

} // namespace CppConsUI

#endif // KEYCONFIG_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
