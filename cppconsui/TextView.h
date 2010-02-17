#ifndef __TEXTVIEW_H__
#define __TEXTVIEW_H__

#include "Widget.h"

#include <vector>

/// Line oriented text view widget.
class TextView
: public Widget 
{
	public:
		TextView(Widget& parent_, int x_, int y_, int w_, int h_, bool autoscroll_ = false);
		virtual ~TextView();

		void Append(const gchar *text);
		/** Inserts text before specified line number. */
		void Insert(int line_num, const gchar *text);
		void Erase(int line_num);
		void Erase(int start_line, int end_line);
		void Clear();

		const gchar *GetLine(int line_num) const;
		int GetLinesNumber() const;
		int ViewPosForLine(int line_num) const;
		void SetViewPos(int viewy);

		// Widget
		virtual void Draw();
		virtual void MoveResize(int newx, int newy, int neww, int newh);

	protected:
		struct Line
		{
			gchar *text; ///< UTF-8 encoded text.
			int length; ///< Text length in characters.

			Line(const gchar *text_, int bytes);
			virtual ~Line();
		};

		struct ScreenLine
		{
			Line *parent;
			const gchar *text;
			int width;

			ScreenLine(Line &parent_, const gchar *text_, int width_);
		};

		int view_top;
		bool autoscroll;

		std::vector<Line *> lines;
		std::vector<ScreenLine *> screen_lines;

		const gchar *ProceedLine(const gchar *text, int area_width, int *res_width) const;
		void UpdateScreenLines(int line_num);

	private:
		TextView(const TextView &);
		TextView& operator=(const TextView&);
};

#endif /* __TEXTVIEW_H__ */
