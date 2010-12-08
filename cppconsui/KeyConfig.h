/*
 * Copyright (C) 2009-2010 by CenterIM developers
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

#include <sigc++/sigc++.h>
#include <sigc++/signal.h>

#include <libtermkey/termkey.h>

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
 *   static sigc::connection sig_register;
 *   static bool RegisterKeyDefs();
 *   void DeclareBindables();
 * };
 * sigc::connection X::sig_register = InputProcessor::AddRegisterCallback(
 *   sigc::ptr_fun(&X::RegisterKeyDefs));
 *
 * bool X::RegisterKeyDefs()
 * {
 *   KEYCONFIG->RegisterKeyDef("context", "action", _("description"),
 *     Keys::SymbolTermKey(TERMKEY_SYM_END, TERMKEY_KEYMOD_CTRL));
 *   KEYCONFIG->RegisterKeyDef("context", "action2", _("description2"),
 *     Keys::SymbolTermKey(TERMKEY_SYM_HOME, TERMKEY_KEYMOD_CTRL));
 * }
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
 *
 *   // Note: You cannot register a bindable default key it the bindable
 *   // was not registered previously by this class or other classes.
 * }
 * \endcode
 *
 * @todo add exception handling
 */
class KeyConfig
{
public:
  // Helper classes

  /**
   * External part of bindable (see InputProcessor::Bindable for an internal
   * part). Holds description and a default key for every bindable.
   */
  class Bindable
  {
  public:
    Bindable(const char *context_,
        const char *action_,
        const gchar *description_,
        const TermKeyKey &defkey_)
      /* Passed values should be always statically allocated so just save
       * pointers to them. */
      : context(context_)
      , action(action_)
      , description(description_)
      , defkey(defkey_) {}

    /**
     * The context of the key definition.
     */
    const char *context;
    /**
     * The name of the action.
     */
    const char *action;
    /**
     * A description of the action.
     */
    const gchar *description;
    /**
     * The default value, i.e. the key(s) that trigger the action.
     */
    TermKeyKey defkey;
  };

  /**
   * Maps keys to actions for one context, {key: action}.
   */
  typedef std::map<TermKeyKey, std::string, Keys::TermKeyCmp> KeyBindContext;
  /**
   * Maps context to key binds in such a context, {context: KeyContext}.
   */
  typedef std::map<std::string, KeyBindContext> KeyBinds;

  /**
   * Holds all bindables declared in a program.
   */
  typedef std::vector<Bindable> Bindables;

  /**
   * Returns the singleton class instance.
   */
  static KeyConfig *Instance();

  /**
   * Adds an action declaration and registers a default key to trigger this
   * action.
   */
  void RegisterKeyDef(const char *context, const char *action,
      const gchar *desc, const TermKeyKey &key);

  /**
   * Returns all key binds.
   */
  const KeyBinds *GetKeyBinds() const { return &binds; }
  /**
   * Returns all key binds for a given context.
   */
  const KeyBindContext *GetKeyBinds(const char *context) const;
  /**
   * Returns all bindables.
   */
  const Bindables *GetBindables() const { return &bindables; }

  /**
   * Adds a new callback function that is used when the Key values are
   * changed.
   */
  sigc::connection AddReconfigCallback(const sigc::slot<bool>& function)
    { return signal_reconfig.connect(function); }
  /**
   * It is called when needed to read the config and reread the defined
   * keys. It will also emit the signal_reconfig.
   */
  bool Reconfig();

  /**
   * Adds a new callback function that registers the caller's key defs.
   */
  sigc::connection AddRegisterCallback(const sigc::slot<bool> &function)
    { return signal_register.connect(function); }
  /**
   * Calls out the register members of the InputProcessor classes by emitting
   * the signal_register.
   */
  bool Register();

protected:

private:
  /**
   * Current key binds.
   */
  KeyBinds binds;
  /**
   * Bindables defined in all InputProcessor subclasses.
   */
  Bindables bindables;

  /**
   * Signal used to call the register function of all the InputProcessor
   * classes.
   */
  sigc::signal<bool> signal_register;
  /**
   * Signal used to call the reconfig function of all the InputProcessor
   * instances.
   */
  sigc::signal<bool> signal_reconfig;

  KeyConfig() {}
  KeyConfig(const KeyConfig&);
  KeyConfig& operator=(const KeyConfig&);
  ~KeyConfig() {}
};

#endif // __KEYCONFIG_H__
