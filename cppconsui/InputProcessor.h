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

		/* Notes on how input is processed
		 *
		 * Input is processed by first checking for overriding
		 * key combinations. If a match is found, the signal for
		 * that combo is executed, and the function returns 
		 * the number of bytes of that combo.
		 *
		 * If no match is found the ProcessInput function of
		 * a child, if not NULL is called. If this returns a
		 * non-zero value the functions returns that value.
		 *
		 * If no input was eaten by the child, or no child was
		 * selected normal key combinations are checked.
		 * If a match is found, the signal for that combo is
		 * executed, and the function returns the number of
		 * bytes of that combo.
		 *
		 * TODO update and make accurate this description
		 * */
		int ProcessInput(const char *input, const int bytes);

	protected:
		virtual int ProcessInputText(const char *input, const int bytes);

		void SetInputChild(InputProcessor *inputchild);

		void AddCombo(const char *key, sigc::slot<void> action, bool override = false);
		void RebindCombo(const char *oldkey, const char *newkey);
		void ClearCombos(void);

	private:
		/* Notes on how key combinations are stored and searched
		 *
		 * Key combinations are stored in a vector (array) stabally
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
