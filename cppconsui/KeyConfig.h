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

#include <sigc++/sigc++.h>
#include <sigc++/signal.h>

#include <string>
#include <vector>
#include <map>

#define KEYCONFIG (KeyConfig::Instance())

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
  void BindKey(const char *context, const char *action, const char *key);

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
  char *GetKeyBind(const char *context, const char *action) const;

  void SetConfigFile(const char *filename);
  const char *GetConfigFile() const { return config; }

  /**
   * It is called when needed to read the config and reread the defined
   * keys.
   */
  void Reconfig();

  /**
   * Registers default key bindings.
   */
  void RegisterDefaultKeys();

protected:

private:
  /**
   * Current key binds.
   */
  KeyBinds binds;

  /*
   * Key bindings config filename.
   */
  char *config;

  KeyConfig();
  KeyConfig(const KeyConfig&);
  KeyConfig& operator=(const KeyConfig&);
  ~KeyConfig();
};

#endif // __KEYCONFIG_H__
