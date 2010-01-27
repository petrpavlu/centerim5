/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Ported to c++ and modified for cppconsui/centerim.
 */

/** @file TextEntry.cpp TextEntry class implementation
 * @ingroup cppconsui
 */

#include "TextEntry.h"
#include "Keys.h"

#include <glib.h>
#include <cstring>

#define CONTEXT_TEXTENTRY "textentry"

TextEntry::TextEntry(Widget& parent, int x, int y, int w, int h, const gchar *text_)
: Label(parent, x, y, w, h, text_)
, current_pos(0)
, selection_bound(0)
, editable(true)
, obscured(false)
, flags(0)
, text_max_length(MAX_SIZE)
{
	RecalculateLengths();

	set_position(text_size); // @todo should it be text_size or text_length ?

	can_focus = true;
	DeclareBindables();
}

TextEntry::TextEntry(Widget& parent, int x, int y, const gchar *text_)
: Label(parent, x, y, text_)
, current_pos(0)
, selection_bound(0)
, editable(true)
, obscured(false)
, flags(0)
, text_max_length(MAX_SIZE)
{
	RecalculateLengths();

	set_position(text_size);

	Resize(width(text), 1);

	can_focus = true;
	DeclareBindables();
}

TextEntry::~TextEntry()
{
}

void TextEntry::DeclareBindables()
{
	/* cursor movement */
	DeclareBindable(CONTEXT_TEXTENTRY, "cursor-right",
					sigc::bind(sigc::mem_fun(this, &TextEntry::ActionMoveCursor), MOVE_VISUAL_POSITIONS, 1, false),
					InputProcessor::Bindable_Override);
	
	DeclareBindable(CONTEXT_TEXTENTRY, "cursor-left",
					sigc::bind(sigc::mem_fun(this, &TextEntry::ActionMoveCursor), MOVE_VISUAL_POSITIONS, -1, false),
					InputProcessor::Bindable_Override);
	
	DeclareBindable(CONTEXT_TEXTENTRY, "cursor-right-word",
					sigc::bind(sigc::mem_fun(this, &TextEntry::ActionMoveCursor), MOVE_WORDS, 1, false),
					InputProcessor::Bindable_Override);
	
	DeclareBindable(CONTEXT_TEXTENTRY, "cursor-left-word",
					sigc::bind(sigc::mem_fun(this, &TextEntry::ActionMoveCursor), MOVE_WORDS, -1, false),
					InputProcessor::Bindable_Override);
	
	DeclareBindable(CONTEXT_TEXTENTRY, "cursor-end-line",
					sigc::bind(sigc::mem_fun(this, &TextEntry::ActionMoveCursor), MOVE_DISPLAY_LINE_ENDS, 1, false),
					InputProcessor::Bindable_Override);
	
	DeclareBindable(CONTEXT_TEXTENTRY, "cursor-begin-line",
					sigc::bind(sigc::mem_fun(this, &TextEntry::ActionMoveCursor), MOVE_DISPLAY_LINE_ENDS, -1, false),
					InputProcessor::Bindable_Override);
#if 0	
	DeclareBindable(CONTEXT_TEXTENTRY, "cursor-end",
					sigc::bind(sigc::mem_fun(this, &TextEntry::ActionMoveCursor), MOVE_BUFFER_ENDS, 1, false),
					InputProcessor::Bindable_Override);
	
	DeclareBindable(CONTEXT_TEXTENTRY, "cursor-begin",
					sigc::bind(sigc::mem_fun(this, &TextEntry::ActionMoveCursor), MOVE_BUFFER_ENDS, -1, false),
					InputProcessor::Bindable_Override);
	
	/* selection extension variants */
	DeclareBindable(CONTEXT_TEXTENTRY, "selection-right",
					sigc::bind(sigc::mem_fun(this, &TextEntry::ActionMoveCursor), MOVE_VISUAL_POSITIONS, 1, true),
					InputProcessor::Bindable_Override);
	
	DeclareBindable(CONTEXT_TEXTENTRY, "selection-left",
					sigc::bind(sigc::mem_fun(this, &TextEntry::ActionMoveCursor), MOVE_VISUAL_POSITIONS, -1, true),
					InputProcessor::Bindable_Override);
	
	DeclareBindable(CONTEXT_TEXTENTRY, "selection-right-word",
					sigc::bind(sigc::mem_fun(this, &TextEntry::ActionMoveCursor), MOVE_WORDS, 1, true),
					InputProcessor::Bindable_Override);
	
	DeclareBindable(CONTEXT_TEXTENTRY, "selection-left-word",
					sigc::bind(sigc::mem_fun(this, &TextEntry::ActionMoveCursor), MOVE_WORDS, -1, true),
					InputProcessor::Bindable_Override);
	
	DeclareBindable(CONTEXT_TEXTENTRY, "selection-end-line",
					sigc::bind(sigc::mem_fun(this, &TextEntry::ActionMoveCursor), MOVE_DISPLAY_LINE_ENDS, 1, true),
					InputProcessor::Bindable_Override);
	
	DeclareBindable(CONTEXT_TEXTENTRY, "selection-begin-line",
					sigc::bind(sigc::mem_fun(this, &TextEntry::ActionMoveCursor), MOVE_DISPLAY_LINE_ENDS, -1, true),
					InputProcessor::Bindable_Override);
	
	DeclareBindable(CONTEXT_TEXTENTRY, "selection-end",
					sigc::bind(sigc::mem_fun(this, &TextEntry::ActionMoveCursor), MOVE_BUFFER_ENDS, 1, true),
					InputProcessor::Bindable_Override);
	
	DeclareBindable(CONTEXT_TEXTENTRY, "selection-begin",
					sigc::bind(sigc::mem_fun(this, &TextEntry::ActionMoveCursor), MOVE_BUFFER_ENDS, -1, true),
					InputProcessor::Bindable_Override);
	
	/* select all */
	DeclareBindable(CONTEXT_TEXTENTRY, "select-all",
					sigc::bind(sigc::mem_fun(this, &TextEntry::ActionSelectAll), true),
					InputProcessor::Bindable_Override);
	
	/* unselect all */
	DeclareBindable(CONTEXT_TEXTENTRY, "unselect-all",
					sigc::bind(sigc::mem_fun(this, &TextEntry::ActionSelectAll), false),
					InputProcessor::Bindable_Override);
#endif
	/* deleting text */
	DeclareBindable(CONTEXT_TEXTENTRY, "delete-char",
					sigc::bind(sigc::mem_fun(this, &TextEntry::ActionDelete), DELETE_CHARS, 1),
					InputProcessor::Bindable_Override);
	
	DeclareBindable(CONTEXT_TEXTENTRY, "backspace",
					sigc::mem_fun(this, &TextEntry::ActionBackspace),
					InputProcessor::Bindable_Override);
#if 0	
	DeclareBindable(CONTEXT_TEXTENTRY, "delete-word-end",
					sigc::bind(sigc::mem_fun(this, &TextEntry::ActionDelete), DELETE_WORD_ENDS, 1),
					InputProcessor::Bindable_Override);
	
	DeclareBindable(CONTEXT_TEXTENTRY, "delete-word-begin",
					sigc::bind(sigc::mem_fun(this, &TextEntry::ActionDelete), DELETE_WORD_ENDS, -1),
					InputProcessor::Bindable_Override);
	
	/* overwrite */
	DeclareBindable(CONTEXT_TEXTENTRY, "toggle-overwrite",
					sigc::mem_fun(this, &TextEntry::ActionToggleOverwrite),
					InputProcessor::Bindable_Override);
#endif
	/* non text editing bindables */
	DeclareBindable(CONTEXT_TEXTENTRY, "activate", sigc::mem_fun(this, &TextEntry::OnActivate),
					InputProcessor::Bindable_Override);
}

DEFINE_SIG_REGISTERKEYS(TextEntry, RegisterKeys);
bool TextEntry::RegisterKeys(void)
{
	/// @todo implement more of these
	RegisterKeyDef(CONTEXT_TEXTENTRY, "cursor-right", _("Move the cursor to the right."), KEYS->Key_right());
	RegisterKeyDef(CONTEXT_TEXTENTRY, "cursor-left", _("Move the cursor to the left."), KEYS->Key_left());
	RegisterKeyDef(CONTEXT_TEXTENTRY, "cursor-right-word", _("Move the cursor to the right by one word."), KEYS->Key_ctrl_right());
	RegisterKeyDef(CONTEXT_TEXTENTRY, "cursor-left-word", _("Move the cursor to the left by one word."), KEYS->Key_ctrl_left());
	RegisterKeyDef(CONTEXT_TEXTENTRY, "cursor-end-line", _("Move the cursor to the end of the line."), KEYS->Key_end());
	RegisterKeyDef(CONTEXT_TEXTENTRY, "cursor-begin-line", _("Move the cursor to the beginning of the line."), KEYS->Key_home());
	/*RegisterKeyDef(CONTEXT_TEXTENTRY, "cursor-end", _("Move the cursor to the end of the text."), KEYS->Key_ctrl_end());
	 RegisterKeyDef(CONTEXT_TEXTENTRY, "cursor-begin", _("Move the cursor to the beginning of the text."), KEYS->Key_ctrl_home());
	 RegisterKeyDef(CONTEXT_TEXTENTRY, "selection-right", _("Extend the selection to the right."), KEYS->Key_());
	 RegisterKeyDef(CONTEXT_TEXTENTRY, "selection-left", _("Extend the selection to the left."), KEYS->Key_());
	 RegisterKeyDef(CONTEXT_TEXTENTRY, "selection-right-word", _("Extend the selection to the right by one word."), KEYS->Key_());
	 RegisterKeyDef(CONTEXT_TEXTENTRY, "selection-left-word", _("Extend the selection to the left by one word."), KEYS->Key_());
	 RegisterKeyDef(CONTEXT_TEXTENTRY, "selection-end-line", _("Extend the selection to the end of the line"), KEYS->Key_());
	 RegisterKeyDef(CONTEXT_TEXTENTRY, "selection-begin-line", _("Extend the selection to the beginning of the line."), KEYS->Key_());
	 RegisterKeyDef(CONTEXT_TEXTENTRY, "selection-end", _("Extend the selection to the end of the text."), KEYS->Key_());
	 RegisterKeyDef(CONTEXT_TEXTENTRY, "selection-begin", _("Extend the selection to the beginning of the text."), KEYS->Key_());
	 RegisterKeyDef(CONTEXT_TEXTENTRY, "select-all", _("Select all text."), KEYS->Key_());
	 RegisterKeyDef(CONTEXT_TEXTENTRY, "unselect-all", _("Unselect all text."), KEYS->Key_());*/
	RegisterKeyDef(CONTEXT_TEXTENTRY, "delete-char",  _("Delete character under cursor."), KEYS->Key_del());
	RegisterKeyDef(CONTEXT_TEXTENTRY, "backspace", _("Delete character before cursor."), KEYS->Key_backspace());
	/*RegisterKeyDef(CONTEXT_TEXTENTRY, "delete-word-end", _("Delete text until the end of the word at the cursor."), KEYS->Key_());
	 RegisterKeyDef(CONTEXT_TEXTENTRY, "delete-word-begin", _("Delete text until the beginning of the word at the cursor."), KEYS->Key_());
	 RegisterKeyDef(CONTEXT_TEXTENTRY, "toggle-overwrite", _("Enable/Disable overwrite mode."), KEYS->Key_ins());*/
	
	RegisterKeyDef(CONTEXT_TEXTENTRY, "activate", _("Accept input and move focus."), KEYS->Key_enter());
	return true;
}

void TextEntry::Draw(void)
{
	Label::Draw();

	/// @todo we can do better than this
	/// @todo cursor blinking
	if (has_focus) {
		gchar *ptr = g_utf8_offset_to_pointer(text, current_pos);
		int i = width(text, ptr);
		colorscheme->SetColor(area, i, 0, 1, ColorScheme::Focus);
	}
}

int TextEntry::ProcessInputText(const char *input, const int bytes)
{
	gint pos = current_pos;
	gunichar c;
	char *new_input;
	int new_bytes;
	
	/* We don't insert control characters in the text, we just skip
	 * the input in this case. Note that this does not handle
	 * control characters occurring anywhere else than in the first
	 * byte. This means that pasting text with embedded control
	 * characters will not work. */
	/// @todo also skip function keys
	c = g_utf8_get_char_validated(input, bytes);
	if (g_unichar_iscntrl(c))
		return 0;

	/// @todo filter out invalid chars
	
	/* Filter out unwanted input */
	/// @todo move to separate function?
	if (flags) {
		new_input = g_new0(char, bytes);
		char *next;
		const char *str = input;
		const char *end = str + bytes;
		new_bytes = 0;

		for (str = input; str < end; str = next) {
			c = g_utf8_get_char_validated(str, end-str);
			next = g_utf8_next_char(str);

			if (!(flags & FlagAlphabetic) && g_unichar_isalpha(c)) {
				/* We dont want this character in the string, so skip it. */
			} else if (!(flags & FlagNumeric) && g_unichar_isdigit(c)) {
				/* Don't want it. */
			} else if (!(flags & FlagNoSpace) && g_unichar_isspace(c)) {
				/* Don't want it. */
			} else if (!(flags & FlagNoPunctuation) && g_unichar_ispunct(c)) {
				/* Don't want it. */
			} else {
				/* We want this character, so copy it to the new input string */

				/* Copy the single character. */
				g_utf8_strncpy(new_input+new_bytes, str, 1);
				/* Increase the number of bytes in the input*/
				new_bytes += next-str;
			}
		}   
	} else {
		new_input = g_strndup(input, bytes);
		new_bytes = bytes;
	}
		
	if (editable) {
		insert_text(new_input, new_bytes, &pos);
		set_position(pos);
	} else {
	}

	Redraw();

	/* Don't forget to free the new input string. */
	g_free(new_input);

	return bytes;
}

void TextEntry::Flags(int _flags)
{
	flags = _flags;
	//TODO validate text using new flags?
}

void TextEntry::Obscured(bool obscure)
{
	/// @todo how to actually obscure the text if we use label::draw?
	///  copy label::draw implementation here?
	obscured = obscure;
	Redraw();
}

void TextEntry::SetText(const gchar *text_)
{
	Label::SetText(text);

	// @todo length checking?
	// @todo move cursor somewhere?

	RecalculateLengths();
}

void TextEntry::backspace(void)
{
  gint prev_pos;

  //cim doesn't support input methods
  //_gtk_this_reset_im_context (this);

	/// @todo make sure that this->text cannot be NULL and remove the test from here
  if (!this->editable || !this->text)
    return;

  if (this->selection_bound != this->current_pos)
    {
      delete_selection ();
      return;
    }

  prev_pos = move_logically(this->current_pos, -1);

  if (prev_pos < this->current_pos)
    {
      //cim doesn't use pango
      //PangoLayout *layout = gtk_this_ensure_layout (this, FALSE);
      //PangoLogAttr *log_attrs;
      //gint n_attrs;

      //cim doesn't use pango
      //pango_layout_get_log_attrs (layout, &log_attrs, &n_attrs);
                                                     
      /*cim doesn't use XIM
      if (log_attrs[this->current_pos].backspace_deletes_character)
        {
          gchar *cluster_text;
          gchar *normalized_text;
          glong  len;

          cluster_text = gtk_editable_get_chars (editable,
                                                 prev_pos,
                                                 this->current_pos);
          normalized_text = g_utf8_normalize (cluster_text,
                                              strlen (cluster_text),
                                              G_NORMALIZE_NFD);
          len = g_utf8_strlen (normalized_text, -1);

          gtk_editable_delete_text (editable, prev_pos, this->current_pos);
          if (len > 1)
            {
              gint pos = this->current_pos;

              gtk_editable_insert_text (editable, normalized_text,
                                        g_utf8_offset_to_pointer (normalized_text, len - 1) - normalized_text,
                                        &pos);
              gtk_editable_set_position (editable, pos);
            }

          g_free (normalized_text);
          g_free (cluster_text);
        }
      else */
        {
          delete_text (prev_pos, this->current_pos);
        }

      //g_free (log_attrs);
    }

  /* cursor blinking is a character attribute */
  //gtk_entry_pend_cursor_blink (this);
}

void TextEntry::move_cursor (CursorMovement step, gint count, gboolean extend_selection)
{
  gint new_pos = this->current_pos;

  //cim doesn't use XIM
  //_gtk_this_reset_im_context (this);

  if (this->current_pos != this->selection_bound && !extend_selection)
    {
      /* If we have a current selection and aren't extending it, move to the
       * start/or end of the selection as appropriate
       */
      switch (step)
        {
        case MOVE_VISUAL_POSITIONS:
          {/*TODO port this?
            gint current_x = get_better_cursor_x (this, this->current_pos);
            gint bound_x = get_better_cursor_x (this, this->selection_bound);

            if (count <= 0)
              new_pos = current_x < bound_x ? this->current_pos : this->selection_bound;
            else
              new_pos = current_x > bound_x ? this->current_pos : this->selection_bound;
            break; */
          }
        case MOVE_LOGICAL_POSITIONS:
        case MOVE_WORDS:
          if (count < 0)
            new_pos = MIN (this->current_pos, this->selection_bound);
          else
            new_pos = MAX (this->current_pos, this->selection_bound);
          break;
        case MOVE_DISPLAY_LINE_ENDS:
        case MOVE_PARAGRAPH_ENDS:
        case MOVE_BUFFER_ENDS:
          new_pos = count < 0 ? 0 : this->text_length;
          break;
        case MOVE_DISPLAY_LINES:
        case MOVE_PARAGRAPHS:
        case MOVE_PAGES:
        case MOVE_HORIZONTAL_PAGES:
          break;
        }

    }
  else
    {
      switch (step)
        {
        case MOVE_LOGICAL_POSITIONS:                                                                                                              
          new_pos = move_logically (new_pos, count);
          break;
        case MOVE_VISUAL_POSITIONS:
          //TODO this moves according to ltr or rtl or mixed text. should we support this
	  //or wait for a gtktextbuffer port?
          //new_pos = move_visually (new_pos, count);
          new_pos = move_logically (new_pos, count);
          break;
        case MOVE_WORDS:
          while (count > 0)
            {
              new_pos = move_forward_word (new_pos, FALSE);
              count--;
            }
          while (count < 0)
            {
              new_pos = move_backward_word (new_pos, FALSE);
              count++;
            }
          break;
        case MOVE_DISPLAY_LINE_ENDS:
        case MOVE_PARAGRAPH_ENDS:
        case MOVE_BUFFER_ENDS:
          new_pos = count < 0 ? 0 : this->text_length;
          break;
        case MOVE_DISPLAY_LINES:
        case MOVE_PARAGRAPHS:
        case MOVE_PAGES:
        case MOVE_HORIZONTAL_PAGES:
          break;
        }
    }

  if (extend_selection)
    set_selection_bounds (this->selection_bound, new_pos);
  else
    set_position (new_pos);

  /* blinking is a character attribute */
  //gtk_entry_pend_cursor_blink (entry);
}

void TextEntry::insert_text (const gchar *new_text, gint new_text_length, gint *position)
{
  gint index;
  gint n_chars;

  if (new_text_length < 0)
    new_text_length = strlen (new_text);

  n_chars = g_utf8_strlen (new_text, new_text_length);
  if (text_max_length > 0 && n_chars + text_length > text_max_length)
    {
      //TODO: flash/blink display, or just beep? (a la screen)
		Curses::beep();
      //gdk_display_beep (gtk_widget_get_display (GTK_WIDGET (this)));
      n_chars = text_max_length - text_length;
      new_text_length = g_utf8_offset_to_pointer (new_text, n_chars) - new_text;
    }
  if (new_text_length + this->n_bytes + 1 > this->text_size) {
      gsize prev_size = this->text_size;

	while (new_text_length + this->n_bytes + 1 > this->text_size)
        {
          if (this->text_size == 0)
            this->text_size = MIN_SIZE;
          else {
              if (2 * (guint)this->text_size < MAX_SIZE &&
                  2 * (guint)this->text_size > this->text_size)
                this->text_size *= 2;
              else {
                  this->text_size = MAX_SIZE;
                  if (new_text_length > (gint)this->text_size - (gint)this->n_bytes - 1) {
                      new_text_length = (gint)this->text_size - (gint)this->n_bytes - 1;
                      new_text_length = g_utf8_find_prev_char (new_text, new_text + new_text_length + 1) - new_text;
                      n_chars = g_utf8_strlen (new_text, new_text_length);
                    }
                  break;
                }
            }
        }

	/// @todo implement widget visibility property?
      //if (this->visible)
      //  this->text = g_realloc (this->text, this->text_size);
      //else
        {
          /* Same thing, just slower and without leaving stuff in memory.  */
          gchar *et_new = (gchar*)g_malloc (this->text_size);
          memcpy (et_new, this->text, MIN (prev_size, this->text_size));
          trash_area (this->text, prev_size);
          g_free (this->text);
          this->text = et_new;
        }
    }

  index = g_utf8_offset_to_pointer (this->text, *position) - this->text;

  g_memmove (this->text + index + new_text_length, this->text + index, this->n_bytes - index);
  memcpy (this->text + index, new_text, new_text_length);

  this->n_bytes += new_text_length;
  this->text_length += n_chars;

  /* NUL terminate for safety and convenience */
  this->text[this->n_bytes] = '\0';

  if (this->current_pos > *position)
    this->current_pos += n_chars;

  if (this->selection_bound > *position)
    this->selection_bound += n_chars;

  /* cim5 doesn't have password hints
  g_object_get (gtk_widget_get_settings (GTK_WIDGET (this)),
                "gtk-this-password-hint-timeout", &password_hint_timeout,
                NULL);

  if (password_hint_timeout > 0 && n_chars == 1 && !this->visible &&
      (new_text_length < PASSWORD_HINT_MAX))
    {
      GtkEntryPasswordHint *password_hint = g_object_get_qdata (G_OBJECT (this),
                                                                quark_password_hint);

      if (! password_hint)
        {
          password_hint = g_new0 (GtkEntryPasswordHint, 1);
          g_object_set_qdata_full (G_OBJECT (this), quark_password_hint,
                                   password_hint,
                                   (GDestroyNotify) gtk_this_password_hint_free);
        }

      memset (&password_hint->password_hint, 0x0, PASSWORD_HINT_MAX);
      password_hint->password_hint_length = new_text_length;
      memcpy (&password_hint->password_hint, new_text, new_text_length);
      password_hint->password_hint_position = *position + n_chars;
    }
  else
    {
      g_object_set_qdata (G_OBJECT (this), quark_password_hint, NULL);
    }
  */

  *position += n_chars;

  recompute ();

  signal_text_changed();

}

void TextEntry::delete_text (gint start_pos, gint end_pos)
{
  if (start_pos < 0)
    start_pos = 0;
  if (end_pos < 0 || end_pos > this->text_length)
    end_pos = this->text_length;
  
  if (start_pos < end_pos)
    {
      gint start_index = g_utf8_offset_to_pointer (this->text, start_pos) - this->text;
      gint end_index = g_utf8_offset_to_pointer (this->text, end_pos) - this->text;
      gint current_pos;
      gint selection_bound;
                        
      g_memmove (this->text + start_index, this->text + end_index, this->n_bytes + 1 - end_index);
      this->text_length -= (end_pos - start_pos);
      this->n_bytes -= (end_index - start_index);

      /* In password-mode, make sure we don't leave anything sensitive after
       * the terminating zero.  Note, that the terminating zero already trashed
       * one byte.
       */
      /// @todo implement widget visibility?
      //if (!this->visible) 
      //  trash_area (this->text + this->n_bytes + 1, end_index - start_index - 1);

      current_pos = this->current_pos;
      if (current_pos > start_pos)
        current_pos -= MIN (current_pos, end_pos) - start_pos;

      selection_bound = this->selection_bound;
      if (selection_bound > start_pos)
        selection_bound -= MIN (selection_bound, end_pos) - start_pos;

      set_positions (current_pos, selection_bound);

      /* We might have deleted the selection
       */
      /* We don't use the clipboard
      update_primary_selection(); */

      recompute();

      signal_text_changed();
    }
}

void TextEntry::RecalculateLengths()
{
	text_size = strlen(text);
	n_bytes = text_size;
	text_length = g_utf8_strlen(text, text_size);
}

/**
 * Overwrite a memory that might contain sensitive information.
 */
void TextEntry::trash_area (gchar *area, gsize len)
{
  volatile gchar *varea = (volatile gchar *)area;
  while (len-- > 0)
    *varea++ = 0;
}

void TextEntry::recompute (void)
{
  /** @todo port this too :)
  gtk_entry_reset_layout();
  gtk_entry_check_cursor_blink();
  
  if (!entry->recompute_idle)
    {
      entry->recompute_idle = g_idle_add_full (G_PRIORITY_HIGH_IDLE + 15, * between resize and redraw *
                                               recompute_idle_func, entry, NULL);
    } 
  */
}   

/** All changes to entry->current_pos and entry->selection_bound
 * should go through this function.
 */     
void TextEntry::set_positions (gint current_pos, gint selection_bound)
{
  bool changed = false;
      
  //g_object_freeze_notify (G_OBJECT (this));
  
  if (current_pos != -1 &&
      this->current_pos != current_pos)
    {
      this->current_pos = current_pos;
      changed = true;           

      /// @todo would we like a cursor changed signal?
      //g_object_notify (G_OBJECT (this), "cursor-position");
    }

  if (selection_bound != -1 &&
      this->selection_bound != selection_bound)
    {
      this->selection_bound = selection_bound;
      changed = true;                         
      
      /// @todo would we like a selection changed signal?
      //g_object_notify (G_OBJECT (this), "selection-bound"); 
    }
                  
  //g_object_thaw_notify (G_OBJECT (this));

  if (changed)
    recompute ();
}

void TextEntry::set_position (gint position)
{
  if (position < 0 || position > this->text_length)
    position = this->text_length;
  
  if (position != this->current_pos ||
      position != this->selection_bound)
    {
      //cim doesn't use XIM
      //_gtk_this_reset_im_context (this);
      set_positions (position, position);
    }
}

void TextEntry::delete_selection (void)
{
  gint start, end;

  g_return_if_fail (editable);

  if (get_selection_bounds (&start, &end))
    delete_text (start, end);
}

void TextEntry::insert_at_cursor (const gchar *str)
{                 
  gint pos = this->current_pos;
                  
  if (this->editable)
    {             
      //cim doesn't use XIM
      //_gtk_this_reset_im_context (this);
  
      insert_text (str, -1, &pos);
      set_position (pos);
    }             
}

void TextEntry::delete_from_cursor (DeleteType type, gint count)
{
  gint start_pos = this->current_pos;
  gint end_pos = this->current_pos;

  //cim doesn't use XIM
  //_gtk_this_reset_im_context (this);

  if (!this->editable)
    return;

  if (this->selection_bound != this->current_pos)
    {
      delete_selection ();
      return;
    }

  switch (type)
    {
    case DELETE_CHARS:
      end_pos = move_logically (this->current_pos, count);
      delete_text (MIN (start_pos, end_pos), MAX (start_pos, end_pos));
      break;
    case DELETE_WORDS:
      if (count < 0)
        {
          /* Move to end of current word, or if not on a word, end of previous word */
          end_pos = move_backward_word (end_pos, FALSE);
          end_pos = move_forward_word (end_pos, FALSE);
        }
      else if (count > 0)
        {
          /* Move to beginning of current word, or if not on a word, begining of next word */
          start_pos = move_forward_word (start_pos, FALSE);
          start_pos = move_backward_word (start_pos, FALSE);
        }

      /* Fall through */
    case DELETE_WORD_ENDS:
      while (count < 0)
        {
          start_pos = move_backward_word (start_pos, FALSE);
          count++;
        }
      while (count > 0)
        {
          end_pos = move_forward_word (end_pos, FALSE);
          count--;
        }
      delete_text (start_pos, end_pos);
      break;
    case DELETE_DISPLAY_LINE_ENDS:
    case DELETE_PARAGRAPH_ENDS:
      if (count < 0)
        delete_text (0, this->current_pos);
      else
        delete_text (this->current_pos, -1);
      break;
    case DELETE_DISPLAY_LINES:
    case DELETE_PARAGRAPHS:
      delete_text (0, -1);
      break;
    case DELETE_WHITESPACE:
      delete_whitespace ();
      break;
    }
  
  /// @todo implement cursor blinking;
  //gtk_this_pend_cursor_blink (this);
}

void TextEntry::delete_whitespace (void)
{
  /** @todo this is untested */
  //gint n_attrs;
  gchar *start, *end;

  start = end = g_utf8_offset_to_pointer(text, this->current_pos); 

  while (start > 0 && g_unichar_isspace(g_utf8_get_char(start)))
    start = g_utf8_find_prev_char(text, start);

  while (end < text+n_bytes && g_unichar_isspace(g_utf8_get_char(end)))
    end = g_utf8_find_next_char(end, text+n_bytes);

  if (start != end)
    delete_text (g_utf8_pointer_to_offset(text, start), 
                 g_utf8_pointer_to_offset(text, end));
}

gint TextEntry::move_logically (gint start, gint count)
{
  gint new_pos = start;

  /* Prevent any leak of information */
  /** @todo implement widget visibility?
  if (!this->visible)
    {
      new_pos = CLAMP (start + count, 0, this->text_length);
    }
  else*/ if (this->text)
    {
      /*cim doesn't use pango
      PangoLayout *layout = gtk_this_ensure_layout (this, FALSE);
      PangoLogAttr *log_attrs;
      gint n_attrs;

      pango_layout_get_log_attrs (layout, &log_attrs, &n_attrs);

      while (count > 0 && new_pos < this->text_length)
        {
          do
            new_pos++;
          while (new_pos < this->text_length && !log_attrs[new_pos].is_cursor_position);

          count--;
        }
      while (count < 0 && new_pos > 0)
        {
          do
            new_pos--;
          while (new_pos > 0 && !log_attrs[new_pos].is_cursor_position);

          count++;
        }

      g_free (log_attrs); */
	gchar *ptr_pos = g_utf8_offset_to_pointer(text, new_pos);

	while (count > 0 && new_pos < text_length
			&& (ptr_pos = g_utf8_find_next_char(text+n_bytes, ptr_pos))) {
		new_pos++;
		count--;
	}
	while (count < 0 && new_pos > 0
			&& (ptr_pos = g_utf8_find_prev_char(text, ptr_pos))) {
		new_pos--;
		count++;
	}
    }

  return new_pos;
}

gint TextEntry::move_visually (gint start, gint count)
{
  /** @todo should cim5 support ltr and rtl and mixed text?
  gint index;
  PangoLayout *layout = gtk_entry_ensure_layout (entry, FALSE);
  const gchar *text;

  text = pango_layout_get_text (layout);

  index = g_utf8_offset_to_pointer (text, start) - text;

  while (count != 0)
    {
      int new_index, new_trailing;
      gboolean split_cursor;
      gboolean strong;

      g_object_get (gtk_widget_get_settings (GTK_WIDGET (entry)),
                    "gtk-split-cursor", &split_cursor,
                    NULL);

      if (split_cursor)
        strong = TRUE;
      else
        {
          GdkKeymap *keymap = gdk_keymap_get_for_display (gtk_widget_get_display (GTK_WIDGET (entry)));
          PangoDirection keymap_direction = gdk_keymap_get_direction (keymap);

          strong = keymap_direction == entry->resolved_dir;
        }

      if (count > 0)
        {
          pango_layout_move_cursor_visually (layout, strong, index, 0, 1, &new_index, &new_trailing);
          count--;
        }
      else
        {
          pango_layout_move_cursor_visually (layout, strong, index, 0, -1, &new_index, &new_trailing);
          count++;
        }

      if (new_index < 0)
        index = 0;
      else if (new_index != G_MAXINT)
        index = new_index;

      while (new_trailing--)
        index = g_utf8_next_char (text + index) - text;
    }
  return g_utf8_pointer_to_offset (text, text + index);*/
  return 0;
}

gboolean TextEntry::get_selection_bounds (gint *start, gint *end)
{ 
  *start = this->selection_bound;
  *end = this->current_pos;

  return (this->selection_bound != this->current_pos);
} 

void TextEntry::set_selection_bounds (gint start, gint end)
{ 
  if (start < 0)
    start = this->text_length;
  if (end < 0)
    end = this->text_length;
 
  //cim doesn't use XIM
  //_gtk_this_reset_im_context (this);

  set_positions (MIN (end, this->text_length),
		 MIN (start, this->text_length));
                        
  //cim doesn't use the clipboard
  //update_primary_selection ();
} 

gint TextEntry::move_forward_word (gint start, gboolean allow_whitespace)
{
  gint new_pos = start;
  /*TODO port this function

  * Prevent any leak of information *
  if (!entry->visible)
    {
      new_pos = entry->text_length;
    }
  else if (entry->text && (new_pos < entry->text_length))
    {
      PangoLayout *layout = gtk_entry_ensure_layout (entry, FALSE);
      PangoLogAttr *log_attrs;
      gint n_attrs;

      pango_layout_get_log_attrs (layout, &log_attrs, &n_attrs);

      * Find the next word boundary *
      new_pos++;
      while (new_pos < n_attrs - 1 && !(log_attrs[new_pos].is_word_end ||
                                        (log_attrs[new_pos].is_word_start && allow_whitespace)))
        new_pos++;

      g_free (log_attrs);
    }
  */
  return new_pos;
}

gint TextEntry::move_backward_word (gint start, gboolean allow_whitespace)
{
  gint new_pos = start;
  /** @todo port this function also in textview

  * Prevent any leak of information *
  if (!entry->visible)
    {
      new_pos = 0;
    }
  else if (entry->text && start > 0)
    {
      PangoLayout *layout = gtk_entry_ensure_layout (entry, FALSE);
      PangoLogAttr *log_attrs;
      gint n_attrs;

      pango_layout_get_log_attrs (layout, &log_attrs, &n_attrs);

      new_pos = start - 1;

      * Find the previous word boundary *
      while (new_pos > 0 && !(log_attrs[new_pos].is_word_start ||
                              (log_attrs[new_pos].is_word_end && allow_whitespace)))
        new_pos--;

      g_free (log_attrs);
    }
  */
  return new_pos;
}

void TextEntry::toggle_overwrite (void)
{ 
  overwrite_mode = !overwrite_mode;
}                 

void TextEntry::ActionMoveCursor(CursorMovement step, int count, bool extend_selection)
{
	move_cursor(step, count, extend_selection);
	Redraw();
}

void TextEntry::ActionSelectAll(bool select_all)
{
	if (select_all) {
		ActionMoveCursor(MOVE_BUFFER_ENDS, -1, false);
		ActionMoveCursor(MOVE_BUFFER_ENDS, 1, true);
	} else {
		ActionMoveCursor(MOVE_VISUAL_POSITIONS, 0, false);
	}
	Redraw();
}

void TextEntry::ActionDelete(DeleteType type, gint count)
{
	delete_from_cursor(type, count);
	Redraw();
}
   
void TextEntry::ActionBackspace(void)
{
	backspace();
	Redraw();
}

void TextEntry::ActionToggleOverwrite(void)
{
	toggle_overwrite();
}

//TODO custom handlers?
void TextEntry::OnActivate(void)
{
	if (parent) {
		parent->MoveFocus(FocusNext);
	}
}
