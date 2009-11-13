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
/**
 * @file InputProcessor.h InputProcessor base class
 * @ingroup cppconsui
 */

#ifndef __INPUTPROCESSOR_H__
#define __INPUTPROCESSOR_H__

#include <sigc++/sigc++.h>
#include <sigc++/signal.h>

#include <glib.h>

#include <vector>
#include <map>
#include <string>

/** 
 * Base class that takes care of input processing
 *
 * It allows to define:
 * - key-action bindings
 * - a chain of input processors (top to bottom)
 * 
 * Common usage for a derived class:
 * - call DeclareBindable() for each action that your class needs to support (in the proper context)
 * - call BindAction() for each declared bindable that has a key bound to it
 * - call SetInputChild() if there is another object in the chain of input processing
 *
 * @todo At the moment, @ref InputProcessor::keybindings is used to keep both declared bindable and the bound keys. 
 * It makes it difficult to reconfigure all keys from a unique place, like when the keybindings 
 * configuration changes. 
 */
class InputProcessor
{
	public:
		/** Defines when a key binding will be processed comparing with the child input processor @see ProcessInput
		 */
		enum BindableType {
			Bindable_Normal, ///< key bindings will be processed after the child input processor 
			Bindable_Override ///< key bindings will be processed before the child input processor
		};
		/** Holder of a key definition */
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

				const gchar *context; ///< the context of the key definition, like (...)
				const gchar *action; ///< the name of the action, like (...)
				const gchar *description; ///< a description of the action
				const gchar *keycombo; ///< the value, i.e. the key(s) that trigger the action - it is "" if no key is assigned yet
			public:
			KeyCombo(){;} ///<@todo KeyCombo() constructor should be private and not defined
		};

		InputProcessor(void);
		virtual ~InputProcessor();

		/** 
		 * There are 4 steps when processing input:
		 * <OL>
		 * <LI><I>
		 *       Overriding key combos
		 * </I><BR>Input is processed by checking for overriding
		 * key combinations. If a match is found, the signal for
		 * that combo is sent, and the function returns 
		 * the number of bytes used by that combo.
		 * </LI>
		 * <LI><I>
		 *       Input child processing
		 * </I><BR>
		 * If an input child is assigned, processing is done
		 * recursively by this child object. If this returns a
		 * non-zero value indicating that more bytes are needed
		 * or bytes were used the functions returns that value.
		 * </LI>
		 * <LI><I>
		 *        Other key combos
		 * </I><BR>
		 * Input is processed by checking for normal
		 * key combinations. If a match is found, the signal for
		 * that combo is sent, and the function returns 
		 * the number of bytes used by that combo.
		 * </LI>
		 * <LI><I>
		 *        Raw input processing
		 * </I><BR>
		 * Non key combo raw input processing by objects. Used
		 * for e.g. input widgets.
		 * </LI>
		 * </OL>
		 * 
		 * @return When checking for matches and no match is found but
		 * a partial match was found, the number of bytes
		 * needed to be able to make a full match is returned
		 * if the number of input bytes was less than the number
		 * of bytes in the combo. In this case the return value
		 * is amount of bytes subtracted from 0.<BR>
		 * If more input bytes were given than a certain combo,
		 * even if the combo is a prefix of the input, it will
		 * not count as a match.
		 *
		 * */
		int ProcessInput(const char *input, const int bytes);

	protected:


		class Bindable;
		typedef std::vector<KeyCombo> KeyCombos;
	
	/** Notes on how key combinations are stored and searched
	 *
	 * Key combinations are stored in a multimap 
	 * indexed on the first byte of the key value only, this
	 * allows for partial matching.
	 *
	 * @todo When adding a new combination to the map, care must be
	 * taken to look for `shadowing'. This occurs when when two
	 * key combinations have the same prefix. In this case the
	 * first definition is used. The user should be notified of
	 * an error in the configuration. 
	 *
	 * @see DeclareBindable,BindAction
	 * */	
		typedef std::multimap<char, Bindable> Bindables;

		virtual int ProcessInputText(const char *input, const int bytes);

		/* Set the child object that must process input before this object
		 * */
		void SetInputChild(InputProcessor *child);
		InputProcessor* GetInputChild(void) { return inputchild; }

		/** Binds a keycombo to an action 
		 *
		 * Probably a better name for this method would have been BindKey. It needs to be called after a Bindable has been @ref DeclareBindable "declared".
		 * If the key has already been bound use the replace parameter. 
		 * @return if the binding succeded
		 * @todo check memory allocation, perhaps optimise things a bit. Also, perhaps it's a good idea to 
		 *   throw an error or something if the keycombo exists and replace is set to false (it shouldn't happen).
		 */
		bool BindAction(const gchar *context, const gchar *action, const char *keycombo, bool replace);
		bool RebindAction(const gchar *context, const gchar *action, const char *keycombo)
			{ return BindAction(context, action, keycombo, true); }

		/** Binds a (context,action) pair with a function. 
		 *
		 * The bind can be normal or override, depending on wether it needs to be called
		 * after or before the @ref inputchild.
		 * @todo should throw an error if the bindable has already been declared.
		 */
		void DeclareBindable(const gchar *context, const gchar *action,
			sigc::slot<void> function, const gchar *description, BindableType type);

		void ClearBindables(void);

	private:
		InputProcessor& operator=(const InputProcessor&);
	
		/** Tries to match an appropriate bound action to the input and apply it
		 * @return the best match:
		 * - positive (and equal to <i>bytes</i>) if an action was taken
		 * - the bigest negative number if a partial match was found
		 * - 0 if no match was found
		 */
		int Process(BindableType type, const char *input, const int bytes);
		/** Checks if they <i>bytes</i> chars from <i>input</i> matches fully or partially 
		 * the <i>skey</i>
		 * @return	
		 *		- <i>bytes</i> if perfectly matches
		 *		- -needed where needed is how many chars are needed for a match
		 *		- 0 if they not match
		 */
		int Match(const std::string &skey, const char *input, const size_t bytes);
		/** Checks if the input processor has declared a (context,action) pair,
		 * even if it not yet bound to a keycombo
		 */
		bool HaveBindable(const gchar *context, const gchar *action);

		///@todo Bindables GetBindables(void); maybe public

		Bindables keybindings;

		/* The child that will get to process the input */
		InputProcessor *inputchild;
};

#endif /* __INPUTPROCESSOR_H__ */
