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
/** @file Panel.h Panel class
 * @ingroup cppconsui
 */

#ifndef __PANEL_H__
#define __PANEL_H__

#include "Panel.h"
#include "LineStyle.h"
#include "Label.h"
#include "Widget.h"

/** A widget representing a rectangular border with an optional
 * label on the top border line.
 * @todo borders with colors are nice 
 * @todo perhaps add some constructors directly with the label text
 */
class Panel
: public Widget
{
	public:
		Panel(Widget& parent, const int x, const int y, const int w, const int h);
		Panel(Widget& parent, const int x, const int y, const int w, const int h, LineStyle::Type ltype);

		virtual ~Panel();

		virtual void Draw();
		/** Sets the text of the label */
		void SetText(const gchar *str);
		Glib::ustring GetText();

		virtual void SetBorderStyle(LineStyle::Type ltype);
		virtual LineStyle::Type GetBorderStyle();

	protected:
		LineStyle *linestyle;
		Label *label;

	private:
		Panel();
		Panel(const Panel&);
		Panel& operator=(const Panel&);
};

#endif /* __PANEL_H__ */
