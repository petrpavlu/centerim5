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

#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "Label.h"

#include <glibmm/ustring.h>

class Button
: public Label
{
	public:
		Button(Widget& parent, int x, int y, int w, int h, Glib::ustring &text, sigc::slot<void> callback);
		Button(Widget& parent, int x, int y, int w, int h, const char *text, sigc::slot<void> callback);
		Button(Widget& parent, int x, int y, int w, int h, const char *text);
		Button(Widget& parent, int x, int y, const char *text, sigc::slot<void> callback);
		Button(Widget& parent, int x, int y, const char *text);
		virtual ~Button();

		virtual void Draw(void);

		//TODO rename to setcallback?
		void SetFunction(sigc::slot<void> callback);

		//TODO keep signal or the callback?
		sigc::signal<void, Button*> signal_activate;

	protected:

	private:
		Button(void);
		Button(const Button&);

		Button& operator=(const Button&);

		void AddBindables(void);
		void OnActivate(void);

		sigc::slot<void> callback;
};

#endif /* __BUTTON_H__ */
