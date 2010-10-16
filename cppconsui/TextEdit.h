/** @file TextEdit.h TextEdit class
 * @ingroup cppconsui
 */

#ifndef __TEXTEDIT_H__
#define __TEXTEDIT_H__

#include "Widget.h"

#include <vector>

class TextEdit
: public Widget
{
	public:
		TextEdit(int w, int h);
		virtual ~TextEdit();

		void Clear();
		/** Returns inserted text. Lines are separated by a given separator.
		 * Caller is responsible for freeing returned data. */
		gchar *AsString(const gchar *separator = "\n");

		// Widget
		virtual void Draw();
		virtual void MoveResize(int newx, int newy, int neww, int newh);
		virtual bool ProcessInputText(const TermKeyKey &key);

		sigc::signal<void, TextEdit&> signal_text_changed;

	protected:
		void InitBuffer(int size);
		int SizeOfGap();
		int BufferSize();
		void ExpandGap(int size);
		void MoveGapToCursor();

		gchar *PrevChar(const gchar *p) const;
		gchar *NextChar(const gchar *p) const;
		int Width(const gchar *start, int chars) const;

		gchar *GetScreenLine(gchar *text, int max_width, int *res_width,
				int *res_length) const;
		void UpdateScreenLines();
		void ClearScreenLines();

		void UpdateScreenCursor();

		void InsertTextAtCursor(const gchar *new_text, int new_text_bytes = -1);
		void DeleteFromCursor(DeleteType type, int direction);
		void MoveCursor(CursorMovement step, int direction);

		void ToggleOverwrite();

		bool editable;
		bool overwrite_mode;

		int current_pos; ///< Character position from the start of buffer.
		gchar *point; ///< Cursor location in the buffer.

		int current_sc_line; ///< Current cursor line (derived from current_pos and screen_lines).
		int current_sc_linepos; ///< Current cursor character number (in the current line).
		int view_top;

		gchar *buffer; ///< Start of text buffer.
		gchar *bufend; ///< First location outside buffer.
		gchar *gapstart; ///< Start of gap.
		gchar *gapend; ///< First location after end of gap.
		int text_length; ///< Length in use, in chars.

		int gap_size; ///< Expand gap by this value.

		struct ScreenLine
		{
			const gchar *start; ///< Pointer to start of line (points into buffer).
			const gchar *end; ///< Pointer to first byte that is not part of line.
			int length; ///< Precalculated length.
			int width; ///< Precalculated on screen width.

			ScreenLine(const gchar *start, const gchar *end, int length, int width);
		};

		std::vector<ScreenLine *> screen_lines;

	private:
		TextEdit();
		TextEdit(const TextEdit&);
		TextEdit& operator=(const TextEdit&);

		int MoveLogically(int start, int direction);
		int MoveBackwardWordFromCursor();
		int MoveForwardWordFromCursor();

		void ActionMoveCursor(CursorMovement step, int direction);
		void ActionDelete(DeleteType type, int direction);
		void ActionToggleOverwrite();

		/** it handles the automatic registration of defined keys */
		DECLARE_SIG_REGISTERKEYS();
		static bool RegisterKeys();
		void DeclareBindables();
};

#endif /* __TEXTEDIT_H__ */
