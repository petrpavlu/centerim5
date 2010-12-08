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
    const gchar *desc, const TermKeyKey &key)
{
  bindables.push_back(Bindable(context, action, desc, key));
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

bool KeyConfig::Reconfig()
{
  /**
   * @todo Read the config and assign it to keys.
   */
  signal_reconfig.emit();
  return true;
}

bool KeyConfig::Register()
{
  /* Call all registered init functions, that will fill up keys by calling
   * RegisterKeyDef() themselves. */
  signal_register.emit();
  return true;
}
