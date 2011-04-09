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
 * InputProcessor base class implementation.
 *
 * @ingroup cppconsui
 */

#include "InputProcessor.h"
#include "KeyConfig.h"
#include "Keys.h"

InputProcessor::InputProcessor()
: input_child(NULL)
{
}

bool InputProcessor::ProcessInput(const TermKeyKey& key)
{
  // process overriding key combinations first
  if (Process(BINDABLE_OVERRIDE, key))
    return true;

  // hand of input to a child
  if (input_child && input_child->ProcessInput(key))
    return true;

  // process other key combinations
  if (Process(BINDABLE_NORMAL, key))
    return true;

  // do non-combo input processing
  TermKeyKey keyn = Keys::RefineKey(key);
  if (keyn.type == TERMKEY_TYPE_UNICODE && ProcessInputText(keyn))
    return true;

  return false;
}

void InputProcessor::SetInputChild(InputProcessor& child)
{
  input_child = &child;
}

void InputProcessor::ClearInputChild()
{
  input_child = NULL;
}

void InputProcessor::DeclareBindable(const char *context, const char *action,
    const sigc::slot<void>& function, BindableType type)
{
  keybindings[context][action] = Bindable(function, type);
}

bool InputProcessor::Process(BindableType type, const TermKeyKey& key)
{
  for (Bindables::iterator i = keybindings.begin(); i != keybindings.end();
      i++) {
    // get keys for this context
    const KeyConfig::KeyBindContext *keys
      = KEYCONFIG->GetKeyBinds(i->first.c_str());
    if (!keys)
      continue;

    /// @todo make this quicker
    for (KeyConfig::KeyBindContext::const_iterator j = keys->begin();
        j != keys->end(); j++) {
      if (Keys::Compare(key, j->first)) {
        BindableContext::iterator k = i->second.find(j->second);
        if (k != i->second.end() && k->second.type == type) {
          k->second.function();
          return true;
        }
      }
    }
  }

  return false;
}

bool InputProcessor::ProcessInputText(const TermKeyKey& key)
{
  return false;
}
