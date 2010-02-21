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

KeyConfig *KeyConfig::Instance()
{
	static KeyConfig instance;
	return &instance;
}

void KeyConfig::Bind(const char *context, const char *action, const TermKeyKey &key)
{
	keys[context][key] = action;
}

const KeyConfig::KeyContext *KeyConfig::GetContext(const char *context) const
{
	KeyGlobals::const_iterator i = keys.find(context);
	if (i == keys.end())
		return NULL;
	return &i->second;
}

void KeyConfig::Clear()
{
	signal_register.clear();
	signal_reconfig.clear();
	keys.clear();
}

bool KeyConfig::Reconfig()
{
	/** @todo read the config and assign it to keys
	 */
	signal_reconfig.emit();
	return true;
}

bool KeyConfig::Register()
{
	/* it calls all registered init functions, that will
	 fill up keys by calling RegisterKeyDef themselves */
	signal_register.emit();
	return true;
}
