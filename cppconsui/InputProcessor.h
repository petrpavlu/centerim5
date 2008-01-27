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

#include <glib.h>

#include <vector>
#include <map>
#include <string>

class InputProcessor
{
	public:
		enum BindableType {Bindable_Normal, Bindable_Override};

		class KeyCombo {
			public:
				KeyCombo(const gchar *context,
					const gchar *action,
					const gchar *description,
					const gchar *keycombo
				)
				: context(context)
				, action(action)
				, description(description)
				, keycombo(keycombo)
				{ ; }

				KeyCombo() { ; }

				const gchar *context;
				const gchar *action;
				const gchar *description;
				const gchar *keycombo;
		};

		InputProcessor(void);
		virtual ~InputProcessor();

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
		/* Notes on how key combinations are stored and searched
		 *
		 * Key combinations are stored in a multimap 
		 * indexed on the first byte of the key value only, this
		 * allows for partial matching.
		 *
		 * When adding a new combination to the map, care must be
		 * taken to look for `shadowing'. This occurs when when to
		 * key combinations have the same prefix. In this case the
		 * first definition is used. The user should be notified of
		 * an error in the configuration. TODO implement this
		 * */

		class Bindable;
		typedef std::vector<KeyCombo> KeyCombos;
		typedef std::multimap<char, Bindable> Bindables;

		virtual int ProcessInputText(const char *input, const int bytes);

		/* Set the child object which must process input before this object
		 * */
		void SetInputChild(InputProcessor *inputchild);
		InputProcessor* GetInputChild(void) { return inputchild; }

		/* Bind a keycombo to an action */
		bool BindAction(const gchar *context, const gchar *action, const char *keycombo, bool override);
		bool RebindAction(const gchar *context, const gchar *action, const char *keycombo)
			{ return BindAction(context, action, keycombo, true); }

		void DeclareBindable(const gchar *context, const gchar *action,
			sigc::slot<void> function, const gchar *description, BindableType type);
	private:
		InputProcessor& operator=(const InputProcessor&);

		int Process(BindableType type, const char *input, const int bytes);
		int Match(const std::string &skey, const char *input, const int bytes);
		bool HaveBindable(const gchar *context, const gchar *action);

		//TODO: Bindables GetBindables(void); maybe public

		Bindables keybindings;

		/* The child which will get to process the input */
		InputProcessor *inputchild;
};

#endif /* __INPUTPROCESSOR_H__ */
