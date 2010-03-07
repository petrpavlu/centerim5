/* This file is part of CenterIM.
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
 * @file KeyConfig.h KeyConfig singleton class
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

/**
 * This singleton class is used to keep the key definitions.
 *
 * It holds the context, action, description, the default and the configured key values.
 * It is using a function static variable as the Instance() because it needs to be
 * instantiated when used during the static class members instantiations.
 *
 * The use case for a class X derived from @ref InputProcessor class is the following:
 * class X: public Y {
 *	private:
 *     static sigc::connection sig_register;
 *     static bool RegisterKeyDefs();
 *     void DeclareBindables();
 * };
 * sigc::connection X::sig_register = Instance().AddRegisterCallback(sigc::ptr_fun( &X::RegisterKeyDefs ) );
 * bool X::RegisterKeyDefs(){
 *		KeyConfig::Instance().RegisterKeyDef(KeyConfig::KeyDef("context", "action", _("description"), KEYS->Key_ctrl_end()));
 *		KeyConfig::Instance().RegisterKeyDef(KeyConfig::KeyDef("context", "action2", _("description2"), KEYS->Key_ctrl_home()));
 * }
 * void X::DeclareBindable(){
 *		DeclareBindable("context", "action", sigc::mem_fun(this, X::OnActionDo)); // registers the bindable
 *		if (some condition)
 *			DeclareBindable("context", "action2", sigc::mem_fun(this, X::OnAction2Do)); // it can conditionally register the Bindable
 *		DeclareBindable("context", "action3", sigc::mem_fun(this, X::OnAction3Do), true); // it reconfigures the action3 defined by Y (for example)
 *		UndeclareBindable("context", "action4"); // it unregisters the action4 defined by Y (for example)
 *		// we cannot register a bindable for a keydef that wasn't registered previously by this class or other classes
 * }
 * @todo add exception handling
 */

#define KEYCONFIG (KeyConfig::Instance())

class KeyConfig
{
	public:
		// Helper classes

		/**
		 * External part of bindable (see InputProcessor::Bindable for
		 * an internal part). Holds description and a default key for every
		 * bindable.
		 */
		class Bindable
		{
			public:
				Bindable(const char *context_,
						const char *action_,
						const gchar *description_,
						const TermKeyKey &defkey_)
					/* Given values should be always statically allocated so
					 * just save pointers to them. */
					: context(context_)
					  , action(action_)
					  , description(description_)
					  , defkey(defkey_) {}

				const char *context; ///< the context of the key definition, like (...)
				const char *action; ///< the name of the action, like (...)
				const gchar *description; ///< a description of the action
				TermKeyKey defkey; ///< the default value, i.e. the key(s) that trigger the action
		};

		/** Maps keys to actions for one context, { key: action }. */
		typedef std::map<TermKeyKey, std::string, Keys::TermKeyCmp> KeyBindContext;
		/** Maps context to key binds in such a context, { context : KeyContext }. */
		typedef std::map<std::string, KeyBindContext> KeyBinds;

		/** Holds all bindables declared in a program. */
		typedef std::vector<Bindable> Bindables;

		/** Returns the singleton class instance. */
		static KeyConfig *Instance();

		/**
		 * Adds an action declaration and registers a default key to trigger
		 * this action.
		 */
		void RegisterKeyDef(const char *context, const char *action,
				const gchar *desc, const TermKeyKey &key);

		/** Returns all key binds. */
		const KeyBinds *GetKeyBinds() const { return &binds; }
		/** Returns all key binds for a given context. */
		const KeyBindContext *GetKeyBinds(const char *context) const;
		/** Returns all bindables. */
		const Bindables *GetBindables() const { return &bindables; }

		/** Adds a new callback function that is used when
		 * the Key values are changed.
		 */
		sigc::connection AddReconfigCallback(const sigc::slot<bool>& function){
			return signal_reconfig.connect(function);
		}
		/**
		 * It is called when needed to read the config and
		 * reread the defined keys.
		 * It will also emit the signal_reconfig.
		 */
		bool Reconfig();

		/** Adds a new callback function that registers the caller's
		 * key defs.
		 */
		sigc::connection AddRegisterCallback(const sigc::slot<bool> &function){
			return signal_register.connect(function);
		}
		/**
		 * Calls out the register members of the InputProcessor classes
		 * by emitting the signal_register.
		 */
		bool Register();

	private:
		KeyConfig() { ; }
		KeyConfig(const KeyConfig &);
		KeyConfig &operator=(const KeyConfig &);
		virtual ~KeyConfig() { ; }

		/**
		 * Current key binds.
		 */
		KeyBinds binds;
		/**
		 * Bindables defined in all InputProcessor subclasses.
		 */
		Bindables bindables;
		sigc::signal<bool> signal_register; ///< Signal used to call the register function of all the InputProcessor classes
		sigc::signal<bool> signal_reconfig; ///< Signal used to call the reconfig function of all the InputProcessor instances
};

#endif /* __KEYCONFIG_H__ */
