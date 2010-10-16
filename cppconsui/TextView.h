/*
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
 * TextView class.
 *
 * @ingroup cppconsui
 */

#ifndef __TEXTVIEW_H__
#define __TEXTVIEW_H__

#include "Widget.h"

#include <deque>

/**
 * Line oriented text view widget.
 */
class TextView
: public Widget
{
public:
	TextView(int w, int h, bool autoscroll_ = false);
	virtual ~TextView();

	/**
	 * Appends text after the last line.
	 */
	void Append(const gchar *text, int color = 0);
	/**
	 * Inserts text before specified line number. Text can contain multiple
	 * lines and should end with `\n' character just in front of `\0'
	 * character.
	 */
	void Insert(int line_num, const gchar *text, int color = 0);
	/**
	 * Removes a specified line.
	 */
	void Erase(int line_num);
	/**
	 * Removes specified range of lines. Parameter end_line represents the
	 * line after the last removed line, thus range of <start_line, end_line)
	 * lines is removed.
	 */
	void Erase(int start_line, int end_line);
	/**
	 * Removes all lines.
	 */
	void Clear();
	/**
	 * Returns string for a specified line number.
	 */
	const gchar *GetLine(int line_num) const;
	/**
	 * Returns count of all lines.
	 */
	int GetLinesNumber() const;
	/**
	 * Returns an on-screen line number where a text specified by line number
	 * is located. Note: this number can change if the widget is resized.
	 */
	int ViewPosForLine(int line_num) const;
	/**
	 * Sets an on-screen line number.
	 */
	void SetViewPos(int viewy);

	// Widget
	virtual void Draw();
	virtual void MoveResize(int newx, int newy, int neww, int newh);

protected:
	/**
	 * Struct Line saves a real line. All text added into TextView is split on
	 * `\n' character and stored into Line objects.
	 */
	struct Line
	{
		/**
		 * UTF-8 encoded text. Note: Newline character is not part of text.
		 */
		gchar *text;
		/**
		 * Text length in characters.
		 */
		int length;
		/**
		 * Color number.
		 */
		int color;

		Line(const gchar *text_, int bytes, int color_);
		virtual ~Line();
	};

	/**
	 * ScreenLine represents an on-screen line.
	 */
	struct ScreenLine
	{
		/**
		 * Parent Line that this ScreenLine shows.
		 */
		Line *parent;
		/**
		 * Pointer into parent's text where this ScreenLine starts.
		 */
		const gchar *text;
		/**
		 * On-screen width.
		 */
		int width;

		ScreenLine(Line &parent_, const gchar *text_, int width_);
	};

	typedef std::deque<Line *> Lines;
	typedef std::deque<ScreenLine *> ScreenLines;

	int view_top;
	bool autoscroll;

	/**
	 * Array of real lines.
	 */
	Lines lines;
	/**
	 * Array of on-screen lines.
	 */
	ScreenLines screen_lines;

	/**
	 * FIXME
	 */
	const gchar *ProceedLine(const gchar *text, int area_width,
			int *res_width) const;
	/**
	 * Recalculates on-screen lines for a specified line number.
	 */
	int UpdateScreenLines(int line_num, int start = 0);

	int EraseScreenLines(int line_num, int start = 0, int *deleted = NULL);

private:
	TextView(const TextView &);
	TextView& operator=(const TextView&);
};

#endif /* __TEXTVIEW_H__ */
