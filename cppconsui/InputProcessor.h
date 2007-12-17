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

#ifndef __INPUTPROCESSOR_H__
#define __INPUTPROCESSOR_H__

#include <sigc++/sigc++.h>
#include <sigc++/signal.h>

#include <vector>
#include <string>

class InputProcessor
{
	public:
		InputProcessor();
		~InputProcessor();

		/* Notes on how input is processed:
		 *
		 * There are 4 steps when processing input:
		 *  1: overriding key combos
		 *  2: input child processing
		 *  3: other key combos
		 *  4: raw input processing
		 *
		 * Overriding key combos:
		 * **********************
		 * Input is processed by checking for overriding
		 * key combinations. If a match is found, the signal for
		 * that combo is sent, and the function returns 
		 * the number of bytes used by that combo.
		 *
		 * Input child processing:
		 * ***********************
		 * I an input child is assigned, processing is done
		 * recursively by this child object. If this returns a
		 * non-zero value indicating that more bytes are needed
		 * or bytes were used the functions returns that value.
		 *
		 * Other key combos:
		 * *****************
		 * Input is processed by checking for normal
		 * key combinations. If a match is found, the signal for
		 * that combo is sent, and the function returns 
		 * the number of bytes used by that combo.
		 *
		 * Raw input processing:
		 * *********************
		 * Non key combo raw input processing by objects. Used
		 * for e.g. input widgets.
		 *
		 * TODO update and make accurate this description:
		 * return values could be negative! Explain why/when
		 * */
		int ProcessInput(const char *input, const int bytes);

	protected:
		virtual int ProcessInputText(const char *input, const int bytes);

		/* Set the child object which must process input before this object
		 * */
		void SetInputChild(InputProcessor *inputchild);

		void AddCombo(const char *key, sigc::slot<void> action, bool override = false);
		void RebindCombo(const char *oldkey, const char *newkey);
		void ClearCombos(void);

	private:
		/* Notes on how key combinations are stored and searched
		 *
		 * Key combinations are stored in a vector (array) stably
		 * sorted on the first byte of the key value only.
		 *
		 * This allows for fast searching using binary search. Inserts
		 * are slower, but since key combinations are manipulated
		 * very irregular this is not a problem.
		 *
		 * When adding a new combination to the vector, care must be
		 * taken to look for `shadowing'. This occurs when when to
		 * key combinations have the same prefix. In this case the
		 * first definition is used. The user should be notified of
		 * an error in the configuration.
		 * */
		enum ComboType {Normal, Override, Both};
		typedef struct KeyCombo {
			std::string key;
			sigc::slot<void> action;
			ComboType type;
		};
		typedef std::vector<KeyCombo> KeyCombos;

		int Process(ComboType type, const char *input, const int bytes);
		int Match(const std::string skey, const char *input, const int bytes);

		KeyCombos::iterator BinSearchFirst(const char *key);
		KeyCombos::iterator BinSearchLast(const char *key);

		KeyCombos combos;
		InputProcessor *inputchild;
};

#endif /* __INPUTPROCESSOR_H__ */
