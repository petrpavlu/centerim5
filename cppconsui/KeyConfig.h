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
 * KeyConfig singleton class.
 *
 * @ingroup cppconsui
 */

#ifndef __KEYCONFIG_H__
#define __KEYCONFIG_H__

#include "Keys.h"

#include "libtermkey/termkey.h"
#include <glib.h>
#include <map>
#include <string>
#include <vector>
#include <sigc++/sigc++.h>

#define KEYCONFIG (CppConsUI::KeyConfig::Instance())

namespace CppConsUI
{

/**
 * This singleton class is used to keep the key definitions. It holds the
 * context, action, description, the default and the configured key values.
 *
 * The use case for a class X derived from @ref InputProcessor class is the
 * following:
 * \code
 * class X : public Y {
 * private:
 *   void DeclareBindables();
 * };
 *
 * void X::DeclareBindable()
 * {
 *   // register a bindable
 *   DeclareBindable("context", "action", sigc::mem_fun(this, X::OnActionDo));
 *
 *   // register a bindable conditionally
 *   if (some condition)
 *     DeclareBindable("context", "action2", sigc::mem_fun(this,
 *       X::OnAction2Do));
 * }
 * \endcode
 */
class KeyConfig
{
public:
  /**
   * Maps keys to actions for one context, {key: action}.
   */
  typedef std::map<TermKeyKey, std::string, Keys::TermKeyCmp> KeyBindContext;
  /**
   * Maps context to key binds in that context, {context: KeyContext}.
   */
  typedef std::map<std::string, KeyBindContext> KeyBinds;

  /**
   * Returns the singleton class instance.
   */
  static KeyConfig *Instance();

  /**
   * Binds a key to an action (in a given context).
   */
  bool BindKey(const char *context, const char *action, const char *key);

  /**
   * Returns all key binds.
   */
  const KeyBinds *GetKeyBinds() const { return &binds; }
  /**
   * Returns all key binds for a given context.
   */
  const KeyBindContext *GetKeyBinds(const char *context) const;

  /**
   */
  const char *GetKeyBind(const char *context, const char *action) const;

  void SetConfigFile(const char *filename);
  const char *GetConfigFile() const { return config; }

  /**
   * Removes all key binds.
   */
  void Clear();

  /**
   * It is called when needed to read the config and reread the defined
   * keys.
   */
  bool Reconfig();

  void AddDefaultKeyBind(const char *context, const char *action,
      const char *key);

  /**
   * Registers default key bindings.
   */
  void RegisterDefaultKeyBinds();

protected:

private:
  struct DefaultKeyBind
  {
    std::string context;
    std::string action;
    std::string key;

    DefaultKeyBind(const char *context_, const char *action_,
        const char *key_) : context(context_), action(action_), key(key_) {}
  };
  typedef std::vector<DefaultKeyBind> DefaultKeyBinds;

  DefaultKeyBinds default_key_binds;

  /**
   * Current key binds.
   */
  KeyBinds binds;

  /*
   * Key bindings config filename.
   */
  char *config;

  bool ReconfigInternal();

  static void start_element_(GMarkupParseContext *context,
      const char *element_name, const char **attribute_names,
      const char **attribute_values, gpointer user_data,
      GError **error)
    { reinterpret_cast<KeyConfig*>(user_data)->start_element(context,
        element_name, attribute_names, attribute_values, error); }
  void start_element(GMarkupParseContext *context,
      const char *element_name, const char **attribute_names,
      const char **attribute_values, GError **error);

  KeyConfig();
  KeyConfig(const KeyConfig&);
  KeyConfig& operator=(const KeyConfig&);
  ~KeyConfig();
};

} // namespace CppConsUI

#endif // __KEYCONFIG_H__

/* vim: set tabstop=2 shiftwidth=2 expandtab */
