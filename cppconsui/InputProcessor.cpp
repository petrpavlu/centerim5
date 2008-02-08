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

//TODO this *must* leak memory somewhere :)
class InputProcessor::Bindable
: public InputProcessor::KeyCombo
{
	public:
		Bindable(const gchar *context,
			const gchar *action,
			const gchar *description,
			const gchar *keycombo,
			sigc::slot<void> function,
			InputProcessor::BindableType type
		)
		: KeyCombo(context, action, description, keycombo)
		, function(function)
		, type(type)
		{ ; }

		Bindable() { ; }

		sigc::slot<void> function;
		InputProcessor::BindableType type;

};

InputProcessor::InputProcessor()
: inputchild(NULL)
{
}

InputProcessor::~InputProcessor()
{
}

int InputProcessor::ProcessInput(const char *input, const int bytes)
{
	int i;

	/* Process overriding key combinations first */
	i = Process(Bindable_Override, input, bytes);
	if (i) return i;
	
	/* Hand of input to a child */
	if (inputchild)
		i = inputchild->ProcessInput(input, bytes);
	if (i) return i;

	/* Process other key combinations */
	i = Process(Bindable_Normal, input, bytes);
	if (i) return i;

	/* Do non-combo input processing */
	return ProcessInputText(input, bytes);
}

int InputProcessor::ProcessInputText(const char *input, const int bytes)
{
	return 0;
}

void InputProcessor::SetInputChild(InputProcessor *inputchild_)
{
	inputchild = inputchild_;
}

int InputProcessor::Process(InputProcessor::BindableType type, const char *input, const int bytes)
{
	g_assert(input != NULL);

	Bindables::iterator begin, end, i;
	Bindable bindable;
	int m, minm = 0;

	begin = keybindings.lower_bound(input[0]);
	end = keybindings.upper_bound(input[0]);

	for (i = begin; i != end; i++) {
		bindable = (*i).second;
		if (bindable.type == type || type == Bindable_Override) {
			m = Match(bindable.keycombo, input, bytes);
			if (m < 0) {
				/* could match, but need btes more input to be sure */
				if (m > minm) minm = m;
			} else if (m > 0) {
				/* found a match, execute the action */
				bindable.function();
				return m;
			} else {
				/* do nothing */
			}
		}
	}

	return 0;
}

int InputProcessor::Match(const std::string &skeycombo, const char *input, const int bytes)
{
	const char *keycombo = skeycombo.c_str();
	int n;
	
	n = skeycombo.size();
	if (bytes < n) n = bytes;

	if (strncmp(keycombo, input, n) == 0) {
 	        if (bytes < (int)skeycombo.size())
			/* need more input to determine a match */
			return bytes - skeycombo.size();
		else
			/* complete match found */
			return n;
	} else {
		return 0;
	}
}

void InputProcessor::DeclareBindable(const gchar *context, const gchar *action,
	sigc::slot<void> function, const gchar *description, BindableType type)
{
	if (HaveBindable(context, action))
		return; //TODO maybe some error here

	keybindings.insert(std::pair<char, Bindable>('\0', Bindable(context, action, description, '\0', function, type)));
}

void InputProcessor::ClearBindables(void)
{
	keybindings.clear();
}

bool InputProcessor::BindAction(const gchar *context, const gchar *action, const char *keycombo, bool override)
{
	Bindables::iterator i;
	Bindable bindable;

	g_return_val_if_fail((keycombo != NULL) && strncmp("\0", keycombo, strlen(keycombo)), false);

	for (i = keybindings.begin(); i != keybindings.end(); i++) {
		bindable = (*i).second;

		if (strncmp(bindable.context, context, strlen(bindable.context)) == 0 &&
			strncmp(bindable.action, action, strlen(bindable.action)) == 0) {

			if (bindable.keycombo == NULL || override) {
				bindable.keycombo = g_strdup(keycombo);
				keybindings.erase(i);
				keybindings.insert(std::pair<char, Bindable>(keycombo[0], bindable));
				return true;
			} else 
				return false;
		}
	}

	return false;
}

bool InputProcessor::HaveBindable(const gchar *context, const gchar *action)
{
	Bindables::iterator i;
	Bindable bindable;

	for (i = keybindings.begin(); i != keybindings.end(); i++) {
		bindable = (*i).second;

		if (strncmp(bindable.context, context, strlen(bindable.context)) == 0 &&
			strncmp(bindable.action, action, strlen(bindable.action)) == 0) {

			return true;
		}
	}

	return false;
}
