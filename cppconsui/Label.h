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

/** @file Label.h Label class
 *	@ingroup cppconsui
 */
#ifndef __LABEL_H__
#define __LABEL_H__

#include "Widget.h"

/**
 * A widget that displays a small to medium amount of text.
 */
class Label
: public Widget
{
	public:
		Label(Widget& parent, int x, int y, int w, int h, const gchar *text_);
		Label(Widget& parent, int x, int y, const gchar *text_);
		virtual ~Label();

		// Widget
		virtual void Draw();

		void DrawEx(bool color);

		/** Sets a new text and redraws the Widget */
		virtual void SetText(const gchar *text_);
		const gchar* GetText();

	protected:
		gchar *text; 

	private:
		/** manages the text allocation and reallocation
		 * without any Widget signals involved
		 */ 
		void RealSetText(const gchar *text_);
		Label();
		Label(const Label&);
		Label& operator=(const Label&);
};

#endif /* __LABEL_H__ */
