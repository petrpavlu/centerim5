/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010 by CenterIM developers
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

/**
 * @file
 * Panel class.
 *
 * @ingroup cppconsui
 */

#ifndef __PANEL_H__
#define __PANEL_H__

#include "LineStyle.h"
#include "Label.h"
#include "Widget.h"

/**
 * A widget representing a rectangular border with an optional label on the
 * top border line.
 *
 * @todo Add some constructor directly with the label text.
 * @todo Label drawing.
 */
class Panel
: public Widget
{
	public:
		Panel(int w, int h, LineStyle::Type ltype = LineStyle::DEFAULT);
		virtual ~Panel();

		// Widget
		virtual void Draw();

		/**
		 * Sets the text of the Label.
		 */
		void SetText(const gchar *str);
		/**
		 * Returns Label text.
		 */
		const gchar *GetText();

		/**
		 * Sets a new border style.
		 */
		void SetBorderStyle(LineStyle::Type ltype);
		/**
		 * Returns a current border style.
		 */
		LineStyle::Type GetBorderStyle();

	protected:
		LineStyle linestyle;
		Label *label;

	private:
		Panel();
		Panel(const Panel&);
		Panel& operator=(const Panel&);
};

#endif /* __PANEL_H__ */
