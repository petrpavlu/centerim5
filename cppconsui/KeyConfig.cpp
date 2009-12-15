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
/** @file KeyConfig.cpp KeyConfig implementation 
 */

#include "KeyConfig.h"
#include <stdexcept>

KeyConfig& KeyConfig::Instance(){
	static KeyConfig instance;
	return instance;
}

bool KeyConfig::RegisterKeyDef(const KeyDef& keydef){
	KeyContext &kc = keys.insert(KeyGlobals::value_type(keydef.context, KeyGlobals::mapped_type())).first->second;
	return kc.insert(KeyContext::value_type(keydef.action, KeyContext::mapped_type(keydef, keydef.defvalue))).second;
}

const KeyConfig::KeyValue& KeyConfig::GetKeyValue(const ustring& context, const ustring& action) const {
	const KeyContext& key_context = GetContext(context);
	KeyContext::const_iterator j = key_context.find(action);
	if (j == key_context.end()) throw std::logic_error("Action " + context + "." + action + " not found!");
	return j->second;
}

const KeyConfig::KeyContext& KeyConfig::GetContext(const ustring& context) const {
	KeyGlobals::const_iterator i = keys.find(context);
	if (i == keys.end()) throw std::logic_error("Context " + context + " not found!");
	return i->second;
}

bool KeyConfig::Reconfig(){
	/** @todo read the config and assign it to keys
	 */
	signal_reconfig.emit();
	return true;
}

bool KeyConfig::Register(){
	/* it calls all registered init functions, that will
	 fill up keys by calling RegisterKeyDef themselves */
	signal_register.emit();
	return true;
}
