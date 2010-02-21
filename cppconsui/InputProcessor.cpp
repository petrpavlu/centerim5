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

#include "InputProcessor.h"
#include "KeyConfig.h"
#include "Keys.h"

InputProcessor::Bindable::Bindable()
: desc(NULL), type(Bindable_Normal)
{}

InputProcessor::Bindable::Bindable(const gchar *desc_,
		sigc::slot<void> function_,
		BindableType type_)
: function(function_), type(type_)
{
	g_assert(desc_);
	desc = g_strdup(desc_);
}

InputProcessor::Bindable::Bindable(const Bindable &other)
: desc(NULL)
{
	if (other.desc)
		desc = g_strdup(other.desc);
	function = other.function;
	type = other.type;
}

InputProcessor::Bindable::Bindable &InputProcessor::Bindable::operator=(const Bindable &other)
{
	if (desc) {
		g_free(desc);
		desc = NULL;
	}

	if (other.desc)
		desc = g_strdup(other.desc);
	function = other.function;
	type = other.type;

	return *this;
}

InputProcessor::Bindable::~Bindable()
{
	if (desc)
		g_free(desc);
}

InputProcessor::InputProcessor()
: inputchild(NULL)
{ ; }

InputProcessor::~InputProcessor()
{
}

bool InputProcessor::ProcessInput(const TermKeyKey &key)
{
	/* Process overriding key combinations first */
	if (Process(Bindable_Override, key))
		return true;
	
	/* Hand of input to a child */
	if (inputchild && inputchild->ProcessInput(key))
		return true;

	/* Process other key combinations */
	if (Process(Bindable_Normal, key))
		return true;

	/* Do non-combo input processing */
	if (key.type == TERMKEY_TYPE_UNICODE && ProcessInputText(key))
		return true;

	return false;
}

bool InputProcessor::ProcessInputText(const TermKeyKey &key)
{
	return false;
}

void InputProcessor::SetInputChild(InputProcessor &child)
{
	inputchild = &child;
}

void InputProcessor::ClearInputChild()
{
	inputchild = NULL;
}

bool InputProcessor::Process(BindableType type, const TermKeyKey &key)
{
	for (Bindables::iterator i = keybindings.begin(); i != keybindings.end(); i++) {
		// get keys for this context
		const KeyConfig::KeyContext *keys = KEYCONFIG->GetContext(i->first.c_str());
		if (!keys)
			continue;

		/// @todo make this quicker
		for (KeyConfig::KeyContext::const_iterator j = keys->begin(); j != keys->end(); j++) {
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

sigc::connection InputProcessor::AddRegisterCallback(const sigc::slot<bool> &function)
{
	return KEYCONFIG->AddRegisterCallback(function);
}

void InputProcessor::RegisterKeyDef(const char *context,
		const char *action, const TermKeyKey &key)
{
	KEYCONFIG->Bind(context, action, key);
}

void InputProcessor::DeclareBindable(const char *context, const char *action,
		const char *desc, sigc::slot<void> function, BindableType type)
{
	keybindings[context][action] = Bindable(desc, function, type);
}
