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

#include <sigc++/sigc++.h>
#include <sigc++/signal.h>

#include <glibmm/ustring.h>

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

#define KEYCONFIG (&KeyConfig::Instance())

class KeyConfig {
public:
	/* Helper classes */
	
	/** Holder of a key definition */
	class KeyDef {
	public:
		KeyDef(const gchar *context_,
			   const gchar *action_,
			   const gchar *description_,
			   const gchar *defvalue_
			   )
		: context(context_)
		, action(action_)
		, description(description_)
		, defvalue(defvalue_)
		{ ; }
		
		const gchar *context; ///< the context of the key definition, like (...)
		const gchar *action; ///< the name of the action, like (...)
		const gchar *description; ///< a description of the action
		const gchar *defvalue; ///< the default value, i.e. the key(s) that trigger the action - it is "" if no default key is assigned
	};
	
	/** Holder of a key definition along with the user-defined value for the action */
	class KeyValue {
	public:
		KeyValue(const KeyDef& key_, 
				 const gchar* value_)
		: key(key_)
		, value(value_)
		{ ; }
		KeyDef key;
		const gchar* value; ///< configured value of the key ("" if not assigned yet)
	};
	
	typedef Glib::ustring ustring; 	/**< std::string like type used as key */
	typedef std::map<ustring, KeyValue> KeyContext; /**< Holds all KeyValue instances for a context { action: KeyValue } */
	typedef std::map<ustring, KeyContext> KeyGlobals; /** Holds all Key contexts { context : KeyContext } */
	
	/** returns the singleton class instance */
	static KeyConfig& Instance();
	
	/** 
	 * Adds a KeyDef instance to the globals.
	 * It does not allow to add an existing keycombo
	 * @todo throw an exception when it does.
	 */
	bool RegisterKeyDef(const KeyDef&);
	/** 
	 * Returns the KeyValue reference for the given context,action
	 * @todo throw an exception if not found
	 */
	const KeyValue& GetKeyValue(const ustring& context, const ustring& action) const;
	
	const KeyGlobals& GetGlobals() const { return keys; } ///< returns all keys
	const KeyContext& GetContext(const ustring& context) const; ///< returns all keys for a context 
	
	void Clear() { 
		signal_register.clear(); 
		signal_reconfig.clear(); 
		keys.clear(); 
	}
	
	
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
	sigc::connection AddRegisterCallback(const sigc::slot<bool>& function){
		return signal_register.connect(function);
	}
	/**
	 * Calls out the register members of the InputProcessor classes
	 * by emitting the signal_register.
	 */
	bool Register();

private:
	KeyConfig() { ; }
	KeyConfig(const KeyConfig&);
	~KeyConfig() { ; }

	KeyGlobals keys; /// The key actions defined in all InputProcessor subclasses. {context : {action : KeyValue } }
	sigc::signal<bool> signal_register; ///< Signal used to call the register function of all the InputProcessor classes
	sigc::signal<bool> signal_reconfig; ///< Signal used to call the reconfig function of all the InputProcessor instances
};

#endif /* __KEYCONFIG_H__ */
