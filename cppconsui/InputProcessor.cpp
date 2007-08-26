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
	i = Process(Override, input, bytes);
	if (i) return i;
	
	/* Hand of input to a child */
	if (inputchild)
		i = inputchild->ProcessInput(input, bytes);
	if (i) return i;

	/* Process other key combinations */
	i = Process(Normal, input, bytes);
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

int InputProcessor::Process(InputProcessor::ComboType type, const char *input, const int bytes)
{
	KeyCombos::iterator i, begin, end;
	KeyCombo combo;
	int m;

	begin = BinSearchFirst(input);
	end = BinSearchLast(input);

	for (i = begin; i != end; i++) {
		combo = *i;
		if (combo.type == type || type == Both) {
			m = Match(combo.key, input, bytes);
			if (m < 0) {
				/* could match, but need btes more input to be sure */
				return m;
			} else if (m > 0) {
				/* found a match */
				combo.sig->emit();
				return m;
			} else {
				/* do nothing */
			}
		}
	}

	return 0;
}

int InputProcessor::Match(const std::string skey, const char *input, const int bytes)
{
	const char *key = skey.c_str();
	int n;
	
	n = skey.size();
	if (bytes < n) n = bytes;

	if (strncmp(key, input, n) == 0) {
		if (bytes < skey.size())
			return bytes - skey.size(); /* need more input to determine a match */
		else
			return n; /* complete match found */
	} else {
		return 0;
	}
}

InputProcessor::KeyCombos::iterator InputProcessor::BinSearchFirst(const char *key)
{
	int m, n, h;

	m = -1;
	n = combos.size();

	while (m+1 < n) {
		h = (m+n) / 2;

		if (combos[h].key[0] >= key[0])
			n = h;
		else
			m = h;
	}

	return combos.begin() + n;
}

InputProcessor::KeyCombos::iterator InputProcessor::BinSearchLast(const char *key)
{
	int m, n, h;

	m = -1;
	n = combos.size();

	while (m+1 < n) {
		h = (m+n) / 2;

		if (combos[h].key[0] > key[0])
			n = h;
		else
			m = h;
	}

	return combos.begin() + n;
}
