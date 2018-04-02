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
// along with CenterIM.  If not, see <http://www.gnu.org/licenses/>.

/// @file
/// InputProcessor base class.
///
/// @ingroup cppconsui

#ifndef INPUTPROCESSOR_H
#define INPUTPROCESSOR_H

#include "CppConsUI.h"

#include <sigc++/sigc++.h>
#include <sigc++/signal.h>

#include <termkey.h>

#include <map>
#include <string>

namespace CppConsUI {

/// Base class that takes care of input processing.
///
/// It allows to define:
/// - key-action bindings,
/// - a chain of input processors (top to bottom).
class InputProcessor {
public:
  /// Defines when a key binding will be processed comparing with the child
  /// input processor, @see ProcessInput.
  enum BindableType {
    /// Key bindings will be processed after the child input processor.
    BINDABLE_NORMAL,

    /// Key bindings will be processed before the child input processor.
    BINDABLE_OVERRIDE,
  };

  InputProcessor();
  virtual ~InputProcessor() {}

  /// There are 4 steps when processing input:
  /// <ol>
  /// <li>
  /// <i>Overriding key combos</i><br>
  /// Input is processed by checking for overriding key combinations. If a match
  /// is found, the function for that combo is executed.
  /// </li>
  /// <li>
  /// <i>Input child processing</i><br>
  /// If an input child is assigned, processing is done recursively by this
  /// child object.
  /// </li>
  /// <li>
  /// <i>Other key combos</i><br>
  /// Input is processed by checking for normal key combinations. If a match is
  /// found, the signal for that combo is sent.
  /// </li>
  /// <li>
  /// <i>Raw input processing</i><br>
  /// Non key combo raw input processing by objects. Used for e.g. input
  /// widgets.
  /// </li>
  /// </ol>
  ///
  /// @return True if the input was successfully processed, false otherwise.
  virtual bool processInput(const TermKeyKey &key);

protected:
  /// Bindable struct holds a function and a bindable type that is associated to
  /// some {context:action} pair.
  struct Bindable {
    Bindable() : type(BINDABLE_NORMAL) {}
    Bindable(const sigc::slot<void> &function, BindableType type)
      : function(function), type(type)
    {
    }
    virtual ~Bindable() {}
    // CONSUI_DISABLE_COPY(Bindable);

    sigc::slot<void> function;
    BindableType type;
  };

  /// Holds all actions and Bindables for a context, {action: Bindable}.
  typedef std::map<std::string, Bindable> BindableContext;

  /// Holds all Key contexts for this class, {context: KeyContext}.
  typedef std::map<std::string, BindableContext> Bindables;

  /// The set of declared Bindables.
  Bindables keybindings_;

  /// A child that will get to process the input.
  InputProcessor *input_child_;

  /// Set the child object that must process input before this object.
  virtual void setInputChild(InputProcessor &child);
  virtual void clearInputChild();
  virtual InputProcessor *getInputChild() { return input_child_; }

  /// Binds a (context, action) pair with a function.
  ///
  /// The bind can be normal or override, depending on whether it needs to be
  /// called after or before the @ref input_child_.
  virtual void declareBindable(const char *context, const char *action,
    const sigc::slot<void> &function, BindableType type);

  /// Tries to match an appropriate bound action to the input and process it.
  /// @return True if a match was found and processed.
  virtual bool process(BindableType type, const TermKeyKey &key);

  virtual bool processInputText(const TermKeyKey &key);

private:
  CONSUI_DISABLE_COPY(InputProcessor);
};

} // namespace CppConsUI

#endif // INPUTPROCESSOR_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
