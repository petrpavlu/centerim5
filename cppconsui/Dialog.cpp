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

#include "ConsuiCurses.h"

#include "Dialog.h"

#include "Panel.h"

Dialog::Dialog(int x, int y, int w, int h, LineStyle::Type ltype)
: Window(x, y, w, h, ltype)
{
	AddWidgets();
}

Dialog::Dialog()
: Window(0, 0, 1, 1)
{
	//TODO set correct position.
	MoveResize(10, 10, 60, 12);

	AddWidgets();
}

Dialog::~Dialog()
{
}

void Dialog::AddWidgets(void)
{
	buttons = new HorizontalListBox(0, height - 1, width, 1);
	seperator = new HorizontalLine(0, height - 2, width);
	AddWidget(*buttons);
	AddWidget(*seperator);
}

void Dialog::AddButton(const gchar *text, Dialog::ResponseType response)
{
	buttons->AddItem(text, sigc::bind(sigc::mem_fun(this, &Dialog::Response), response));
}

void Dialog::Close()
{
	Response(ResponseCancel);
}

void Dialog::Response(Dialog::ResponseType response)
{
	signal_response(response);

	Window::Close();
}
