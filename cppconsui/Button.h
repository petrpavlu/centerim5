/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 *
 * This file is part of CenterIM.
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
/** @file Button.h Button class
 * @ingroup cppconsui
 */

#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "Label.h"

#include <glibmm/ustring.h>
/** This class implements a simple button behaviour
 *
 * The button doesn't keep states like pressed or not
 * and it can call back one (or more) functions when
 * pressed.
 */
class Button
: public Label
{
	public:
		Button(Widget& parent, int x, int y, int w, int h, Glib::ustring &text, sigc::slot<void> callback);
		Button(Widget& parent, int x, int y, int w, int h, const gchar *text, sigc::slot<void> callback);
		Button(Widget& parent, int x, int y, int w, int h, const gchar *text);
		Button(Widget& parent, int x, int y, const gchar *text, sigc::slot<void> callback);
		Button(Widget& parent, int x, int y, const gchar *text);
		virtual ~Button();

		virtual void Draw(void);

		/// @todo rename to setcallback?
		void SetFunction(sigc::slot<void> callback);

		/// @todo keep signal or the callback?
		sigc::signal<void, Button*> signal_activate;

	protected:

	private:
		Button(void);
		Button(const Button&);

		Button& operator=(const Button&);

		void OnActivate(void);

		sigc::slot<void> callback; ///< the function called when button is pressed
	
		/** it handles the automatic registration of defined keys */
		DECLARE_SIG_REGISTERKEYS();
		static bool RegisterKeys();
		void DeclareBindables();
};

#endif /* __BUTTON_H__ */
