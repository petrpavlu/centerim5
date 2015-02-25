/*
 * Copyright (C) 2009-2015 by CenterIM developers
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
 */

/**
 * @file
 * InputProcessor base class implementation.
 *
 * @ingroup cppconsui
 */

#include "InputProcessor.h"
#include "KeyConfig.h"

namespace CppConsUI
{

InputProcessor::InputProcessor()
: input_child(NULL)
{
}

bool InputProcessor::processInput(const TermKeyKey &key)
{
  // process overriding key combinations first
  if (process(BINDABLE_OVERRIDE, key))
    return true;

  // hand of input to a child
  if (input_child && input_child->processInput(key))
    return true;

  // process other key combinations
  if (process(BINDABLE_NORMAL, key))
    return true;

  // do non-combo input processing
  TermKeyKey keyn = Keys::refineKey(key);
  if (keyn.type == TERMKEY_TYPE_UNICODE && processInputText(keyn))
    return true;

  return false;
}

void InputProcessor::setInputChild(InputProcessor &child)
{
  input_child = &child;
}

void InputProcessor::clearInputChild()
{
  input_child = NULL;
}

void InputProcessor::declareBindable(const char *context, const char *action,
    const sigc::slot<void> &function, BindableType type)
{
  keybindings[context][action] = Bindable(function, type);
}

bool InputProcessor::process(BindableType type, const TermKeyKey &key)
{
  for (Bindables::iterator i = keybindings.begin(); i != keybindings.end();
      i++) {
    // get keys for this context
    const KeyConfig::KeyBindContext *keys
      = KEYCONFIG->getKeyBinds(i->first.c_str());
    if (!keys)
      continue;
    KeyConfig::KeyBindContext::const_iterator j = keys->find(key);
    if (j == keys->end())
      continue;

    BindableContext::iterator k = i->second.find(j->second);
    if (k != i->second.end() && k->second.type == type) {
      k->second.function();
      return true;
    }
  }

  return false;
}

bool InputProcessor::processInputText(const TermKeyKey & /*key*/)
{
  return false;
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
