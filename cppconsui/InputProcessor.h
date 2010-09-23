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
#include <libtermkey/termkey.h>

#include <vector>
#include <map>
#include <string>

#define DECLARE_SIG_REGISTERKEYS() static sigc::connection sig_register
#define DEFINE_SIG_REGISTERKEYS(classname,staticmethod) sigc::connection classname::sig_register = InputProcessor::AddRegisterCallback(sigc::ptr_fun(& classname::staticmethod))

/** Ideas for the new implementation
 * The bindable is split into three parts:
 * - @ref KeyConfig::KeyDef "KeyDef" holding the context,action, description and default value
 * - @ref KeyConfig::KeyValue "KeyValue" holding a KeyDef and a configured value
 * - @ref InputProcessor::Bindable "Bindable" holding a reference to a KeyValue, a function slot and a type
 *
 * The KeyDef should be defined statically by each class and be used to build
 * the keybindings configuration section. @see KeyConfig
 * 
 * Each (or some) of the KeyDef defintions can be bound for the specific instance
 * of the class. 
 */


/** 
 * Base class that takes care of input processing
 *
 * It allows to define:
 * - key-action bindings
 * - a chain of input processors (top to bottom)
 */
class InputProcessor
{
	public:
		/** Defines when a key binding will be processed comparing with the child input processor @see ProcessInput
		 */
		enum BindableType {
			BINDABLE_NORMAL, ///< key bindings will be processed after the child input processor 
			BINDABLE_OVERRIDE ///< key bindings will be processed before the child input processor
		};
	
		InputProcessor();
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
		virtual bool ProcessInput(const TermKeyKey &key);

	protected:
		/*
		 * Internal part of bindable (see KeyConfig::Bindable for an external
		 * part). Holds a function and type for every bindable.
		 */
		class Bindable
		{
			public:
				Bindable();
				Bindable(sigc::slot<void> function_,
						BindableType type_);
				//Bindable(const Bindable &other);
				//Bindable &operator=(const Bindable &other);
				virtual ~Bindable() {}

				sigc::slot<void> function;
				BindableType type;
		};

		/** Holds all actions and Bindables for a context, { action: Bindable }. */
		typedef std::map<std::string, Bindable> BindableContext;
		/** Holds all Key contexts for this class, { context : KeyContext }. */
		typedef std::map<std::string, BindableContext> Bindables;

		virtual bool ProcessInputText(const TermKeyKey &key);

		/* Set the child object that must process input before this object
		 * */
		void SetInputChild(InputProcessor &child);
		void ClearInputChild();
		InputProcessor *GetInputChild(void) { return inputchild; }
		
		/** Wrapper for KEYCONFIG->AddRegisterCallback */
		static sigc::connection AddRegisterCallback(const sigc::slot<bool> &);
		/** it is just a wrapper for KEYCONFIG->Bind */
		static void RegisterKeyDef(const char *context, const char *action,
				const gchar *desc, const TermKeyKey &key);

		/** Binds a (context,action) pair with a function. 
		 *
		 * The bind can be normal or override, depending on wether it needs to be called
		 * after or before the @ref inputchild.
		 * @throws std::logic_error if the context,action pair hasn't been registered yet
		 */
		void DeclareBindable(const char *context, const char *action,
				sigc::slot<void> function, BindableType type);

	private:
		InputProcessor(const InputProcessor &);
		InputProcessor& operator=(const InputProcessor &);
	
		/** Tries to match an appropriate bound action to the input and apply it
		 * @return true if a match was found and processed
		 */
		bool Process(BindableType type, const TermKeyKey &key);
		
		/** the set of declared Bindables */
		Bindables keybindings;

		/* The child that will get to process the input */
		InputProcessor *inputchild;
};

#endif /* __INPUTPROCESSOR_H__ */
