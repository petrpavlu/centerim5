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
#include "TextBrowser.h"

#include "TextWindow.h"

#include <glibmm/ustring.h>

TextWindow::TextWindow(int x_, int y_, int w_, int h_, LineStyle::Type ltype)
: Window(x_, y_, w_, h_, ltype)
{
	browser = new TextBrowser(*this, 1, 0, w - 2, h);
	SetFocusChild(browser);
	AddWidget(browser);
}

TextWindow::~TextWindow()
{
}

void TextWindow::SetLines(std::vector<Glib::ustring> &lines)
{
	browser->SetLines(lines);
}

void TextWindow::AddLines(std::vector<Glib::ustring> &lines)
{
	browser->AddLines(lines);
}

void TextWindow::AddLine(Glib::ustring line)
{
	browser->AddLine(line);
}

void TextWindow::Clear(void)
{
	browser->Clear();
}

int TextWindow::Size(void)
{
	return browser->Size();
}

void TextWindow::RemoveFront(void)
{
	browser->RemoveFront();
}

void TextWindow::RemoveBack(void)
{
	browser->RemoveBack();
}

void TextWindow::Resize(int neww, int newh)
{
	/* Let parent's Resize() renew data structures (including
	 * the area's of child widgets which will thus be done
	 * twice)
	 * */
	Window::Resize(neww, newh);

	/* resize all our widgets, in this case its only one widget
	 * here, w and h are the size of the container, which is 
	 * what we want. in most cases you would need to recalculate
	 * widget sizes based on window and/or container size.
	 * */
	browser->Resize(w - 2, h);
}
