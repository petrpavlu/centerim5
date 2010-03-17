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
		typedef enum {
			ResponseOK,
			ResponseCancel, /* Cancel button or close dialog. */
			ResponseYes,
			ResponseNo
		} ResponseType;

		Dialog(int x, int y, int w, int h, LineStyle::Type ltype);
		Dialog();
		virtual ~Dialog();

		virtual void Close();

		void AddButton(const gchar *label, ResponseType response);

		void Response(ResponseType response);

		/* Signal emitted when the user closes the dialog indicating. */
		sigc::signal<void, ResponseType> signal_response;

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
