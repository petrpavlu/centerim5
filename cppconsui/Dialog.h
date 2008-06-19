/*
 * Copyright (C) 2008 by Mark Pustjens <pustjens@dds.nl>
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

#ifndef __DIALOG_H__
#define __DIALOG_H__

#include "TextEntry.h"
#include "Window.h"

#include "HorizontalListBox.h"
#include "HorizontalLine.h"

class Dialog
: public Window
{
	public:
		Dialog(int x, int y, int w, int h, Border *border);
		Dialog();
		virtual ~Dialog();

		virtual void Resize(int neww, int newh);

		void AddButton(const gchar *label, sigc::slot<void> function);

	protected:
		HorizontalListBox *buttons;
		HorizontalLine *seperator;

	private:
		Dialog(const Dialog&);

		Dialog& operator=(const Dialog&);

		//TODO better name?
		void AddWidgets(void);
};

#endif /* __DIALOG_H__ */
