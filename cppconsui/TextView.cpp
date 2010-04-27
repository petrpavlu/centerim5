#include "TextView.h"

#include "CppConsUI.h"
#include "ColorScheme.h"

TextView::TextView(int w, int h, bool autoscroll_)
: Widget(w, h)
, view_top(0)
, autoscroll(autoscroll_)
{
}

TextView::~TextView()
{
	Clear();
}

void TextView::Append(const gchar *text, int color)
{
	Insert(lines.size(), text, color);
}

void TextView::Insert(int line_num, const gchar *text, int color)
{
	g_assert(text);
	g_assert(line_num >= 0);
	g_assert(line_num <= lines.size());

	const gchar *p = text;
	const gchar *s = text;

	// parse lines
	while (*p) {
		GUnicodeType t = g_unichar_type(g_utf8_get_char(p));
		if ((*p == '\r' || *p == '\n' || t == G_UNICODE_LINE_SEPARATOR || t == G_UNICODE_PARAGRAPH_SEPARATOR)) {
			if (s < p) {
				Line *l = new Line(s, p - s, color);
				lines.insert(lines.begin() + line_num, l);
				UpdateScreenLines(line_num++);
			}
			s = p = g_utf8_next_char(p);
			continue;
		}

		p = g_utf8_next_char(p);
	}

	if (s < p) {
		Line *l = new Line(s, p - s, color);
		lines.insert(lines.begin() + line_num, l);
		UpdateScreenLines(line_num++);
	}

	signal_redraw(*this);
}

void TextView::Erase(int line_num)
{
	/// @todo
}

void TextView::Erase(int start_line, int end_line)
{
	/// @todo
}

void TextView::Clear()
{
	for (Lines::iterator i = lines.begin(); i != lines.end(); i++)
		delete *i;
	lines.clear();

	for (ScreenLines::iterator i = screen_lines.begin(); i != screen_lines.end(); i++)
		delete *i;
	screen_lines.clear();

	view_top = 0;
	signal_redraw(*this);
}

const gchar *TextView::GetLine(int line_num) const
{
	g_assert(line_num >= 0);
	g_assert(line_num < lines.size());

	return lines[line_num]->text;
}

int TextView::GetLinesNumber() const
{
	return lines.size();
}

int TextView::ViewPosForLine(int line_num) const
{
	g_assert(line_num > 0);
	g_assert(line_num < lines.size());

	/// @todo
	return 0;
}

void TextView::SetViewPos(int viewy)
{
	g_assert(viewy > 0);
	g_assert(viewy <= screen_lines.size());

	view_top = viewy;

	signal_redraw(*this);
}

void TextView::Draw()
{
	if (!area || lines.empty())
		return;

	area->erase();

	int realh = area->getmaxy();

	if (autoscroll && screen_lines.size()) {
		view_top = screen_lines.size() - area->getmaxy();
		if (view_top < 0)
			view_top = 0;
	}

	int attrs = COLORSCHEME->GetColorPair(GetColorScheme(), "textview", "text");
	area->attron(attrs);

	ScreenLines::iterator i;
	int j;
	for (i = screen_lines.begin() + view_top, j = 0; i != screen_lines.end() && j < realh; i++, j++) {
		int attrs2 = 0;
		if ((*i)->parent->color) {
			gchar *color = g_strdup_printf("color%d", (*i)->parent->color);
			attrs2 = COLORSCHEME->GetColorPair(GetColorScheme(), "textview", color);
			g_free(color);
			area->attroff(attrs);
			area->attron(attrs2);
		}

		area->mvaddstring(0, j, (*i)->width, (*i)->text);

		if (attrs2) {
			area->attroff(attrs2);
			area->attron(attrs);
		}
	}

	area->attroff(attrs);
}

void TextView::MoveResize(int newx, int newy, int neww, int newh)
{
	Widget::MoveResize(newx, newy, neww, newh);

	/// @todo optimize
	for (int i = 0; i < lines.size(); i++)
		UpdateScreenLines(i);
}

const gchar *TextView::ProceedLine(const gchar *text, int area_width, int *res_width) const
{
	g_assert(text);
	g_assert(area_width > 0);
	g_assert(res_width);

	const gchar *cur = text;
	const gchar *res = text;
	int prev_width = 0;
	int cur_width = 0;
	gunichar uni;
	bool space = false;
	*res_width = 0;

	while (*cur) {
		prev_width = cur_width;
		uni = g_utf8_get_char(cur);
		cur_width += g_unichar_iswide(uni) ? 2 : 1;

		if (prev_width > area_width)
			break;

		// possibly too long word
		if (cur_width > area_width && !*res_width) {
			*res_width = prev_width;
			res = cur;
		}

		if (g_unichar_type(uni) == G_UNICODE_SPACE_SEPARATOR)
			space = true;
		else if (space) {
			/* Found start of a word and everything before that can fit into
			 * a screen line. */
			*res_width = prev_width;
			res = cur;
			space = false;
		}

		cur = g_utf8_next_char(cur);
	}

	// end of text
	if (!*cur && cur_width <= area_width) {
		*res_width = cur_width;
		res = cur;
	}

	/* Fix for very small area_width and characters wider that 1 cell. For
	 * example area_width = 1 and text = "W" where W is a wide character
	 * (2 cells width). In that case we can not draw anything but we want to
	 * skip to another character. */
	if (res == text)
		res = g_utf8_next_char(res);

	return res;
}

void TextView::UpdateScreenLines(int line_num)
{
	g_assert(line_num >= 0);
	g_assert(line_num < lines.size());

	ScreenLines::iterator i;

	/* Find where new screen lines should be placed and remove previous screen
	 * lines created for this line. */
	i = screen_lines.begin();
	while (i != screen_lines.end()) {
		if ((*i)->parent == lines[line_num]) {
			delete *i;
			i = screen_lines.erase(i);
		}
		else if (line_num + 1 < lines.size() && (*i)->parent == lines[line_num + 1])
			break;
		else
			i++;
	}

	if (!area)
		return;

	// parse line into screen lines
	ScreenLines new_lines;
	const gchar *p = lines[line_num]->text;
	const gchar *s;
	int width;
	while (*p) {
		s = p;
		p = ProceedLine(p, area->getmaxx(), &width);
		new_lines.push_back(new ScreenLine(*lines[line_num], s, width));
	}

	screen_lines.insert(i, new_lines.begin(), new_lines.end());
}

TextView::Line::Line(const gchar *text_, int bytes, int color_)
: color(color_)
{
	g_assert(text_);
	g_assert(bytes > 0);

	text = g_strndup(text_, bytes);
	length = g_utf8_strlen(text, -1);
}

TextView::Line::~Line()
{
	g_assert(text);

	g_free(text);
}

TextView::ScreenLine::ScreenLine(Line &parent_, const gchar *text_, int width_)
: parent(&parent_), text(text_), width(width_)
{
}
