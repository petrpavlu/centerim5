// Copyright (C) 2009-2015 Petr Pavlu <setup@dagobah.cz>
//
// This file is part of CenterIM.
//
// CenterIM is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// CenterIM is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/// @file
/// InputProcessor base class implementation.
///
/// @ingroup cppconsui

#include "InputProcessor.h"
#include "KeyConfig.h"

namespace CppConsUI {

InputProcessor::InputProcessor() : input_child_(nullptr)
{
}

bool InputProcessor::processInput(const TermKeyKey &key)
{
  // Process overriding key combinations first.
  if (process(BINDABLE_OVERRIDE, key))
    return true;

  // Hand of input to a child.
  if (input_child_ != nullptr && input_child_->processInput(key))
    return true;

  // Process other key combinations.
  if (process(BINDABLE_NORMAL, key))
    return true;

  // Do non-combo input processing.
  TermKeyKey keyn = Keys::refineKey(key);
  if (keyn.type == TERMKEY_TYPE_UNICODE && processInputText(keyn))
    return true;

  return false;
}

void InputProcessor::setInputChild(InputProcessor &child)
{
  input_child_ = &child;
}

void InputProcessor::clearInputChild()
{
  input_child_ = nullptr;
}

void InputProcessor::declareBindable(const char *context, const char *action,
  const sigc::slot<void> &function, BindableType type)
{
  keybindings_[context][action] = Bindable(function, type);
}

bool InputProcessor::process(BindableType type, const TermKeyKey &key)
{
  for (std::pair<const std::string, BindableContext> &keybind : keybindings_) {
    // Get keys for this context.
    const KeyConfig::KeyBindContext *keys =
      KEYCONFIG->getKeyBinds(keybind.first.c_str());
    if (keys == nullptr)
      continue;
    KeyConfig::KeyBindContext::const_iterator j = keys->find(key);
    if (j == keys->end())
      continue;

    BindableContext::iterator k = keybind.second.find(j->second);
    if (k != keybind.second.end() && k->second.type == type) {
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

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
