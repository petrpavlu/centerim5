/** @file TextEdit.cpp TextEdit class implementation
 * @ingroup cppconsui
 */

/**
 * @todo
 *
 * - Make interface more general. Add SetText(), SetPosition() and similar
 *   functions.
 * - Functions MoveForwardWordFromCursor() and MoveBackwardWordFromCursor()
 *   should perform a move from a given position, not from the cursor.
 * - Overrally improve the code.
 * - Add description how it works.
 * - Add option for different wrapping.
 * - Make gap size configurable.
 */

/*
 * Gap buffer implementation based on code from Hsin Tsao
 * (stsao@lazyhacker.com).
 */

#include "TextEdit.h"

#include "Keys.h"

#include <glib.h>
#include <cstring>
#include "gettext.h"

#define CONTEXT_TEXTENTRY "textedit"

TextEdit::TextEdit(int w, int h)
: Widget(w, h)
, editable(true)
, buffer(NULL)
, gap_size(20) /// @todo
{
	InitBuffer(gap_size);

	can_focus = true;
	DeclareBindables();
}

TextEdit::~TextEdit()
{
	ClearScreenLines();

	if (buffer)
		g_free(buffer);
}

void TextEdit::DeclareBindables()
{
	// cursor movement
	DeclareBindable(CONTEXT_TEXTENTRY, "cursor-right",
			sigc::bind(sigc::mem_fun(this, &TextEdit::ActionMoveCursor), MOVE_LOGICAL_POSITIONS, 1),
			InputProcessor::Bindable_Override);

	DeclareBindable(CONTEXT_TEXTENTRY, "cursor-left",
			sigc::bind(sigc::mem_fun(this, &TextEdit::ActionMoveCursor), MOVE_LOGICAL_POSITIONS, -1),
			InputProcessor::Bindable_Override);

	DeclareBindable(CONTEXT_TEXTENTRY, "cursor-down",
			sigc::bind(sigc::mem_fun(this, &TextEdit::ActionMoveCursor), MOVE_DISPLAY_LINES, 1),
			InputProcessor::Bindable_Override);

	DeclareBindable(CONTEXT_TEXTENTRY, "cursor-up",
			sigc::bind(sigc::mem_fun(this, &TextEdit::ActionMoveCursor), MOVE_DISPLAY_LINES, -1),
			InputProcessor::Bindable_Override);

	DeclareBindable(CONTEXT_TEXTENTRY, "cursor-right-word",
			sigc::bind(sigc::mem_fun(this, &TextEdit::ActionMoveCursor), MOVE_WORDS, 1),
			InputProcessor::Bindable_Override);

	DeclareBindable(CONTEXT_TEXTENTRY, "cursor-left-word",
			sigc::bind(sigc::mem_fun(this, &TextEdit::ActionMoveCursor), MOVE_WORDS, -1),
			InputProcessor::Bindable_Override);

	DeclareBindable(CONTEXT_TEXTENTRY, "cursor-end",
			sigc::bind(sigc::mem_fun(this, &TextEdit::ActionMoveCursor), MOVE_DISPLAY_LINE_ENDS, 1),
			InputProcessor::Bindable_Override);

	DeclareBindable(CONTEXT_TEXTENTRY, "cursor-begin",
			sigc::bind(sigc::mem_fun(this, &TextEdit::ActionMoveCursor), MOVE_DISPLAY_LINE_ENDS, -1),
			InputProcessor::Bindable_Override);

	// deleting text
	DeclareBindable(CONTEXT_TEXTENTRY, "delete-char",
			sigc::bind(sigc::mem_fun(this, &TextEdit::ActionDelete), DELETE_CHARS, 1),
			InputProcessor::Bindable_Override);

	DeclareBindable(CONTEXT_TEXTENTRY, "backspace",
			sigc::bind(sigc::mem_fun(this, &TextEdit::ActionDelete), DELETE_CHARS, -1),
			InputProcessor::Bindable_Override);

	/*
	DeclareBindable(CONTEXT_TEXTENTRY, "delete-word-end",
			sigc::bind(sigc::mem_fun(this, &TextEdit::ActionDelete), DELETE_WORD_ENDS, 1),
			InputProcessor::Bindable_Override);

	DeclareBindable(CONTEXT_TEXTENTRY, "delete-word-begin",
			sigc::bind(sigc::mem_fun(this, &TextEdit::ActionDelete), DELETE_WORD_ENDS, -1),
			InputProcessor::Bindable_Override);

	// overwrite
	DeclareBindable(CONTEXT_TEXTENTRY, "toggle-overwrite",
			sigc::mem_fun(this, &TextEdit::ActionToggleOverwrite),
			InputProcessor::Bindable_Override);
	*/
}

DEFINE_SIG_REGISTERKEYS(TextEdit, RegisterKeys);
bool TextEdit::RegisterKeys()
{
	RegisterKeyDef(CONTEXT_TEXTENTRY, "cursor-right",
			_("Move the cursor to the right."),
			Keys::SymbolTermKey(TERMKEY_SYM_RIGHT));
	RegisterKeyDef(CONTEXT_TEXTENTRY, "cursor-left",
			_("Move the cursor to the left."),
			Keys::SymbolTermKey(TERMKEY_SYM_LEFT));
	RegisterKeyDef(CONTEXT_TEXTENTRY, "cursor-down",
			_("Move the cursor one line down."),
			Keys::SymbolTermKey(TERMKEY_SYM_DOWN));
	RegisterKeyDef(CONTEXT_TEXTENTRY, "cursor-up",
			_("Move the cursor to the right by one word."),
			Keys::SymbolTermKey(TERMKEY_SYM_UP));
	RegisterKeyDef(CONTEXT_TEXTENTRY, "cursor-right-word",
			_("Move the cursor to the right by one word."),
			Keys::SymbolTermKey(TERMKEY_SYM_RIGHT, TERMKEY_KEYMOD_CTRL));
	RegisterKeyDef(CONTEXT_TEXTENTRY, "cursor-left-word",
			_("Move the cursor to the left by one word."),
			Keys::SymbolTermKey(TERMKEY_SYM_LEFT, TERMKEY_KEYMOD_CTRL));
	RegisterKeyDef(CONTEXT_TEXTENTRY, "cursor-end",
			_("Move the cursor to the end of the text."),
			Keys::SymbolTermKey(TERMKEY_SYM_END));
	RegisterKeyDef(CONTEXT_TEXTENTRY, "cursor-begin",
			_("Move the cursor to the beginning of the text."),
			Keys::SymbolTermKey(TERMKEY_SYM_HOME));
	RegisterKeyDef(CONTEXT_TEXTENTRY, "delete-char",
			_("Delete character under cursor."),
			Keys::SymbolTermKey(TERMKEY_SYM_DELETE));
	RegisterKeyDef(CONTEXT_TEXTENTRY, "backspace",
			_("Delete character before cursor."),
			Keys::SymbolTermKey(TERMKEY_SYM_BACKSPACE));

	/// @todo enable
	/*
	RegisterKeyDef(CONTEXT_TEXTENTRY, "delete-word-end",
			_("Delete text until the end of the word at the cursor."),
			Keys::SymbolTermKey(TERMKEY_SYM_DELETE, TERMKEY_KEYMOD_CTRL));
	RegisterKeyDef(CONTEXT_TEXTENTRY, "delete-word-begin",
			_("Delete text until the beginning of the word at the cursor."),
			Keys::SymbolTermKey(TERMKEY_SYM_BACKSPACE, TERMKEY_KEYMOD_CTRL));
	RegisterKeyDef(CONTEXT_TEXTENTRY, "toggle-overwrite",
			_("Enable/Disable overwrite mode."),
			Keys::SymbolTermKey(TERMKEY_SYM_INSERT));
	*/

	return true;
}

void TextEdit::Clear()
{
	InitBuffer(gap_size);
	UpdateScreenLines();
	signal_redraw(*this);
}

gchar *TextEdit::AsString(const gchar *separator)
{
	g_assert(separator);

	if (!BufferSize())
		return NULL;

	int sep_len = strlen(separator);

	// count lines
	/** @todo Calculate lines number continuously during text
	 * inserting/removing. */
	int lines = 0;
	const gchar *p = buffer;
	while (p < bufend) {
		if (*p == '\n')
			lines++;
		p = NextChar(p);
	}
	/* Allocate memory for all chars (without internal line separator `\n' but
	 * with enough room for a given separator). */
	gchar *res = g_new(char, BufferSize() + lines * (sep_len - 1) + 1);

	p = buffer;
	const gchar *q;
	gchar *r = res;
	while (p < bufend) {
		q = p;
		p = NextChar(p);

		if (*q == '\n') {
			strncpy(r, separator, sep_len);
			r += sep_len;
		}
		// the gap was just skipped
		else if (p == gapend) {
			strncpy(r, q, gapstart - q);
			r += gapstart - q;
		}
		else {
			strncpy(r, q, p - q);
			r += p - q;
		}
	}
	*r = '\0';
	return res;
}

void TextEdit::Draw()
{
	if (!area)
		return;

	area->erase();

	if (screen_lines.empty())
		return;

	int realh = area->getmaxy();

	std::vector<ScreenLine *>::iterator i;
	int j;
	for (i = screen_lines.begin() + view_top, j = 0; i != screen_lines.end() && j < realh; i++, j++) {
		/// @todo
		if (gapstart >= (*i)->start && gapstart < (*i)->end) {
			int p;
			p = area->mvaddstring(0, j, (*i)->width, (*i)->start, gapstart);
			area->mvaddstring(p, j, (*i)->width - p, gapend);
		}
		else
			area->mvaddstring(0, j, (*i)->width, (*i)->start);
	}

	if (has_focus) {
		const gchar *line = screen_lines[current_sc_line]->start;
		int sc_x = Width(line, current_sc_linepos);
		int sc_y = current_sc_line - view_top;
		area->mvchgat(sc_x, sc_y, 1, Curses::Attr::REVERSE, 0, NULL);
	}
}

void TextEdit::MoveResize(int newx, int newy, int neww, int newh)
{
	Widget::MoveResize(newx, newy, neww, newh);

	UpdateScreenLines();
	UpdateScreenCursor();

	signal_redraw(*this);
}

bool TextEdit::ProcessInputText(const TermKeyKey &key)
{
	if (editable) {
		InsertTextAtCursor(key.utf8);
		return true;
	}
	return false;
}

void TextEdit::InitBuffer(int size)
{
	g_assert(size > 0);

	if (buffer)
		g_free(buffer);

	buffer = g_new(gchar, size);

	current_pos = 0;
	point = gapstart = buffer;

	// initially gapend is outside of buffer
	bufend = gapend = buffer + size;

	text_length = 0;
	current_pos = 0;
	current_sc_line = 0;
	current_sc_linepos = 0;

	view_top = 0;
}

int TextEdit::SizeOfGap()
{
	return gapend - gapstart;
}

int TextEdit::BufferSize()
{
	return (bufend - buffer) - (gapend - gapstart);
}

void TextEdit::ExpandGap(int size)
{
	if (size > SizeOfGap()) {
		size += gap_size;

		gchar *origbuffer = buffer;

		buffer = g_renew(char, buffer, (bufend - buffer) + size);

		point += buffer - origbuffer;
		bufend += buffer - origbuffer;
		gapstart += buffer - origbuffer;
		gapend += buffer - origbuffer;

		g_memmove(gapend + size, gapend, bufend - gapend);

		gapend += size;
		bufend += size;
	}
}

void TextEdit::MoveGapToCursor()
{
	if (point == gapstart)
		return;

	if (point == gapend) {
		point = gapstart;
		return;
	}

	// move gap towards the left
	if (point < gapstart) {
		// move the point over by gapsize
		g_memmove(point + (gapend - gapstart), point, gapstart - point);
		gapend -= (gapstart - point);
		gapstart = point;
	}
	else {
		/* Since point is after the gap, find distance between gapend and
		 * point and that's how much we move from gapend to gapstart. */
		g_memmove(gapstart, gapend, point - gapend);
		gapstart += point - gapend;
		gapend = point;
		point = gapstart;
	}
}

gchar *TextEdit::PrevChar(const gchar *p) const
{
	if (p >= gapend) {
		if ((p = g_utf8_find_prev_char(gapend, p)))
			return (gchar *) p;
		else
			p = gapstart;
	}

	if ((p = g_utf8_find_prev_char(buffer, p)))
		return (gchar *) p;
	else
		return (gchar *) buffer;
}

gchar *TextEdit::NextChar(const gchar *p) const
{
	// this should happen only if (gapstart == buffer)
	if (p == gapstart)
		p = gapend;

	if (p < gapstart) {
		if ((p = g_utf8_find_next_char(p, gapstart)))
			return (gchar *) p;
		else
			return (gchar *) gapend;
	}

	if ((p = g_utf8_find_next_char(p, bufend)))
		return (gchar *) p;
	else
		return (gchar *) bufend;
}

int TextEdit::Width(const gchar *start, int chars) const
{
	g_assert(start);

	int width = 0;

	while (chars--) {
		width += Curses::onscreen_width(g_utf8_get_char(start));
		start = NextChar(start);
	}
	return width;
}

gchar *TextEdit::GetScreenLine(gchar *text, int max_width, int *res_width, int *res_length) const
{
	g_assert(text);
	g_assert(text < bufend);
	g_assert(max_width > 0);
	g_assert(res_width);
	g_assert(res_length);

	gchar *cur = text;
	gchar *res = text;
	int prev_width = 0;
	int cur_width = 0;
	int cur_length = 0;
	gunichar uni;
	bool space = false;
	*res_width = 0;
	*res_length = 0;

	if (cur == gapstart)
		cur = gapend;

	while (cur < bufend) {
		prev_width = cur_width;
		uni = g_utf8_get_char(cur);
		cur_width += Curses::onscreen_width(uni);
		cur_length++;

		if (prev_width > max_width)
			break;

		// possibly too long word
		if (cur_width > max_width && !*res_width) {
			*res_width = prev_width;
			*res_length = cur_length - 1;
			res = cur;
		}

		// end of line (paragraph on screen) found
		if (*cur == '\n') {
			*res_width = prev_width;
			*res_length = cur_length;
			return NextChar(cur);
		}

		if (g_unichar_type(uni) == G_UNICODE_SPACE_SEPARATOR)
			space = true;
		else if (space) {
			/* Found start of a word and everything before that can fit into
			 * a screen line. */
			*res_width = prev_width;
			*res_length = cur_length - 1;
			res = cur;
			space = false;
		}

		cur = NextChar(cur);
	}

	// end of buffer reached
	if (cur == bufend && cur_width <= max_width) {
		*res_width = cur_width;
		*res_length = cur_length;
		return cur;
	}

	/* Fix for very small max_width and characters wider that 1 cell. For
	 * example max_width = 1 and text = "W" where W is a wide character
	 * (2 cells width). In that case we can not draw anything but we want to
	 * skip to another character. */
	if (res == text) {
		*res_length = 1;
		res = NextChar(res);
	}

	return res;
}

void TextEdit::UpdateScreenLines()
{
	ClearScreenLines();

	int realw;
	if (!area || (realw = area->getmaxx()) <= 1)
		return;

	gchar *p = buffer;
	gchar *s;
	int width;
	int length;

	while (p < bufend) {
		s = p;
		// lower max width by one to make a room for the cursor
		p = GetScreenLine(p, realw - 1, &width, &length);
		screen_lines.push_back(new ScreenLine(s, p, length, width));
	}
	/* An empty line has to be manually inserted if there is a '\n' character
	 * just in front of bufend. */
	if (BufferSize() && *PrevChar(p) == '\n')
		screen_lines.push_back(new ScreenLine(p, p, 0, 0));
}

void TextEdit::ClearScreenLines()
{
	for (std::vector<ScreenLine *>::iterator i = screen_lines.begin(); i != screen_lines.end(); i++)
		delete *i;
	screen_lines.clear();
}

void TextEdit::UpdateScreenCursor()
{
	int acu_length = 0;
	int length;
	current_sc_line = 0;
	current_sc_linepos = 0;

	if (!area)
		return;

	for (std::vector<ScreenLine *>::iterator i = screen_lines.begin();
			i != screen_lines.end(); i++) {
		length = (*i)->length;
		// last line is treated specially
		if (acu_length <= current_pos &&
				(*i == screen_lines.back() ? current_pos <= acu_length + length
				: current_pos < acu_length + length)) {
			current_sc_linepos = current_pos - acu_length;
			break;
		}
		current_sc_line++;
		acu_length += length;
	}

	// fix cursor visibility
	int realh = area->getmaxy();
	if (view_top <= current_sc_line && current_sc_line < view_top + realh)
		return;
	while (view_top > current_sc_line)
		view_top--;
	while (view_top + realh <= current_sc_line)
		view_top++;
}

void TextEdit::InsertTextAtCursor(const gchar *new_text, int new_text_bytes)
{
	g_assert(new_text);

	if (new_text_bytes == -1)
		new_text_bytes = strlen(new_text);

	// Here we do need to move the gap if the point
	// is not already at the start of the gap.
	MoveGapToCursor();

	// check to make sure that the gap has room
	if (new_text_bytes > SizeOfGap())
		ExpandGap(new_text_bytes);

	int n_chars = g_utf8_strlen(new_text, new_text_bytes);
	text_length += n_chars;
	current_pos += n_chars;

	while (new_text_bytes) {
		*gapstart++ = *new_text++;
		new_text_bytes--;
	}
	point = gapstart;

	UpdateScreenLines();
	UpdateScreenCursor();
	signal_redraw(*this);

	signal_text_changed();
}

void TextEdit::DeleteFromCursor(DeleteType type, int direction)
{
	if (!editable)
		return;

	int count = 0;

	switch (type) {
		case DELETE_CHARS:
			count = MoveLogically(current_pos, direction) - current_pos;
			break;
			/*
		case DELETE_WORD_ENDS:
			if (direction > 0)
				end_pos = MoveForwardWord(end_pos);
			else if (direction < 0)
				start_pos = MoveBackwardWord(start_pos);
			DeleteText(start_pos, end_pos);
			break;
			*/
		default:
			g_assert_not_reached();
	}

	if (count) {
		MoveGapToCursor();

		while (count > 0) {
			gapend = NextChar(gapend);
			text_length--;
			count--;
		}

		while (count < 0) {
			gapstart = PrevChar(gapstart);
			current_pos--;
			text_length--;
			count++;
		}
		point = gapstart;

		UpdateScreenLines();
		UpdateScreenCursor();
		signal_redraw(*this);

		signal_text_changed();
	}
}

void TextEdit::MoveCursor(CursorMovement step, int direction)
{
	if (screen_lines.empty())
		return;

	int old_pos = current_pos;
	switch (step) {
		case MOVE_LOGICAL_POSITIONS:
			current_pos = MoveLogically(current_pos, direction);
			break;
		case MOVE_WORDS:
			if (direction > 0)
				current_pos = MoveForwardWordFromCursor();
			else if (direction < 0)
				current_pos = MoveBackwardWordFromCursor();
			break;
		case MOVE_DISPLAY_LINES:
			if (direction > 0) {
				/* First move to end of current line then move to
				 * current_sc_linepos on next line (and make sure there is
				 * such position). */
				if (current_sc_line + 1 < (int) screen_lines.size())
					current_pos += (screen_lines[current_sc_line]->length - current_sc_linepos)
						+ MIN(current_sc_linepos, screen_lines[current_sc_line + 1]->length);
			}
			else if (direction < 0) {
				/* First move to start of current line then move to
				 * current_sc_linepos on previous line (and make sure there is
				 * such position). */
				if (current_sc_line > 0)
					current_pos -= current_sc_linepos + MAX(1,
							screen_lines[current_sc_line - 1]->length - current_sc_linepos);
			}
			break;
		case MOVE_DISPLAY_LINE_ENDS:
			if (direction > 0) {
				/* Last line needs to be handled specially when moving to end
				 * of line. */
				if (current_sc_line + 1 == (int) screen_lines.size())
					current_pos += screen_lines[current_sc_line]->length - current_sc_linepos;
				else
					current_pos += screen_lines[current_sc_line]->length - current_sc_linepos - 1;
			}
			else if (direction < 0)
				current_pos -= current_sc_linepos;
			break;
		default:
			g_assert_not_reached();
	}

	// update point
	while (old_pos > current_pos) {
		point = PrevChar(point);
		old_pos--;
	}
	while (old_pos < current_pos) {
		point = NextChar(point);
		old_pos++;
	}

	UpdateScreenCursor();
}

void TextEdit::ToggleOverwrite()
{
	overwrite_mode = !overwrite_mode;
}

int TextEdit::MoveLogically(int start, int direction)
{
	if (direction > 0 && start < text_length)
		return start + 1;
	else if (direction < 0 && start > 0)
		return start - 1;
	return start;
}

int TextEdit::MoveForwardWordFromCursor()
{
	int new_pos = current_pos;
	gchar *cur = point;
	bool white = false;

	// search for the first nonwhite character after white characters
	while (new_pos < text_length) {
		if (g_unichar_type(g_utf8_get_char(cur)) == G_UNICODE_SPACE_SEPARATOR || *cur == '\n')
			white = true;
		else if (white)
			break;
		new_pos++;
		cur = NextChar(cur);
	}

	return new_pos;
}

int TextEdit::MoveBackwardWordFromCursor()
{
	int new_pos = current_pos;
	gchar *cur = point;
	bool nonwhite = false;

	if (new_pos <= 0)
		return 0;

	// always move at least one character back
	cur = PrevChar(cur);
	new_pos--;

	// search for the first white character before nonwhite characters
	while (new_pos >= 0) {
		if (g_unichar_type(g_utf8_get_char(cur)) != G_UNICODE_SPACE_SEPARATOR && *cur != '\n')
			nonwhite = true;
		else if (nonwhite)
			break;

		if (new_pos > 0)
			cur = PrevChar(cur);
		new_pos--;
	}

	return ++new_pos;
}

void TextEdit::ActionMoveCursor(CursorMovement step, int direction)
{
	MoveCursor(step, direction);
	signal_redraw(*this);
}

void TextEdit::ActionDelete(DeleteType type, int direction)
{
	DeleteFromCursor(type, direction);
	signal_redraw(*this);
}

void TextEdit::ActionToggleOverwrite()
{
	ToggleOverwrite();
}

TextEdit::ScreenLine::ScreenLine(const gchar *start, const gchar *end, int length, int width)
: start(start), end(end), length(length), width(width)
{
}
