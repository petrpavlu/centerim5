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

#include <cstring>

/** Holds a bound key definition
 */
class InputProcessor::Bindable {
public:
	Bindable(const KeyConfig::ustring& context_, 
			 const KeyConfig::ustring& action_,
			 sigc::slot<void> function_,
			 InputProcessor::BindableType type_)
	: keyvalue( KEYCONFIG->GetKeyValue(context_, action_) )
	, function(function_)
	, type(type_)
	{ ; }
	const KeyConfig::KeyValue& keyvalue;
	sigc::slot<void> function;
	InputProcessor::BindableType type;
};



/* ******************************* */
InputProcessor::InputProcessor()
: sig_reconfig(KEYCONFIG->AddReconfigCallback(
					sigc::mem_fun(this, &InputProcessor::rebuild_keymap)))
, inputchild(NULL)
{ ; }

InputProcessor::~InputProcessor()
{
	// disconnects the reconfig signal
	sig_reconfig.disconnect();
}

int InputProcessor::ProcessInput(const char *input, const int bytes)
{
	int i, needed = 0;

	/* Process overriding key combinations first */
	i = Process(Bindable_Override, input, bytes);
	if (i > 0) return i;
	if (i < 0) needed = i;
	
	/* Hand of input to a child */
	if (inputchild)
		i = inputchild->ProcessInput(input, bytes);
	if (i > 0) return i;
	if (i < 0) needed = i;

	/* Process other key combinations */
	i = Process(Bindable_Normal, input, bytes);
	if (i > 0) return i;
	if (i < 0) needed = i;

	/* Do non-combo input processing */
	i = ProcessInputText(input, bytes);
	if (i > 0) return i;
	if (i < 0) needed = i;

	return needed;
}

int InputProcessor::ProcessInputText(const char *input, const int bytes)
{
	return 0;
}

void InputProcessor::SetInputChild(InputProcessor *child)
{
	inputchild = child;
}

int InputProcessor::Process(InputProcessor::BindableType type, const char *input, const int bytes)
{
	g_assert(input != NULL);
	BindableMap::iterator begin, end, i;
	sigc::slot<void> function;
	int m;
	int needed = 0; // raise downward

	begin = keymap.lower_bound(input[0]);
	end = keymap.upper_bound(input[0]);

	for (i = begin; i != end; i++) {
		const Bindable& bindable = *(i->second);
		if (bindable.type == type) {
			m = Match(bindable.keyvalue.value, input, bytes);
			if (m < 0) {
				// could match, but need more input bytes
				if (m > needed || needed == 0)
					needed = m;
			} else if (m > 0) {
				bindable.function();
				return m;
			}
		}
	}

	// return minimal needed characters for a match
	return needed;
}

int InputProcessor::Match(const std::string &skeycombo, const char *input, const int bytes)
{
	const char *keycombo = skeycombo.c_str();
	int size;
	int min;

	size = skeycombo.size();
	min = size < bytes ? size : bytes;
	
	if (strncmp(keycombo, input, min) == 0) {
		if (bytes < size)
			// need more input to determine a match
			return bytes - size;
		else
			// complete match found
			return size;
	}

	return 0;
}

sigc::connection InputProcessor::AddRegisterCallback(const sigc::slot<bool>& function)
{
	return KEYCONFIG->AddRegisterCallback(function);
}
bool InputProcessor::RegisterKeyDef(const gchar *context, const gchar *action, const gchar* desc, const char *defvalue)
{
	return KEYCONFIG->RegisterKeyDef( KeyConfig::KeyDef(context, action, desc, defvalue) );
}


void InputProcessor::DeclareBindable(const gchar *context, const gchar *action,
	sigc::slot<void> function, BindableType type)
{
	keybindings.push_back(Bindable(context, action, function, type));
	MapBindable(keybindings.back());
}

void InputProcessor::MapBindable(const Bindable& bindable)
{
	const gchar* keyvalue = bindable.keyvalue.value;
	if (keyvalue[0] != '\0')
		keymap.insert(std::make_pair(keyvalue[0], &bindable));
}

void InputProcessor::ClearBindables(void)
{
	// @todo not very useful, isn't it ?
	keymap.clear();
	keybindings.clear();
}

bool InputProcessor::rebuild_keymap()
{
	// rebuild keymap
	keymap.clear();
	for (Bindables::const_iterator i = keybindings.begin(); i != keybindings.end(); i++) {
		MapBindable(*i);
	}
	return true;
}
