/* GTK - The GIMP Toolkit
 * gtktextlayout.c - calculate the layout of the text
 *
 * Copyright (c) 1992-1994 The Regents of the University of California.
 * Copyright (c) 1994-1997 Sun Microsystems, Inc.
 * Copyright (c) 2000 Red Hat, Inc.
 * Tk->Gtk port by Havoc Pennington
 * Pango support by Owen Taylor
 *
 * This file can be used under your choice of two licenses, the LGPL
 * and the original Tk license.
 *
 * LGPL:
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
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Original Tk license:
 *
 * This software is copyrighted by the Regents of the University of
 * California, Sun Microsystems, Inc., and other parties.  The
 * following terms apply to all files associated with the software
 * unless explicitly disclaimed in individual files.
 *
 * The authors hereby grant permission to use, copy, modify,
 * distribute, and license this software and its documentation for any
 * purpose, provided that existing copyright notices are retained in
 * all copies and that this notice is included verbatim in any
 * distributions. No written agreement, license, or royalty fee is
 * required for any of the authorized uses.  Modifications to this
 * software may be copyrighted by their authors and need not follow
 * the licensing terms described here, provided that the new terms are
 * clearly indicated on the first page of each file where they apply.
 *
 * IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY
 * PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
 * DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION,
 * OR ANY DERIVATIVES THEREOF, EVEN IF THE AUTHORS HAVE BEEN ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
 * NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS,
 * AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
 * MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 * GOVERNMENT USE: If you are acquiring this software on behalf of the
 * U.S. government, the Government shall have only "Restricted Rights"
 * in the software and related documentation as defined in the Federal
 * Acquisition Regulations (FARs) in Clause 52.227.19 (c) (2).  If you
 * are acquiring the software on behalf of the Department of Defense,
 * the software shall be classified as "Commercial Computer Software"
 * and the Government shall have only "Restricted Rights" as defined
 * in Clause 252.227-7013 (c) (1) of DFARs.  Notwithstanding the
 * foregoing, the authors grant the U.S. Government and others acting
 * in its behalf permission to use and distribute the software in
 * accordance with the terms specified in this license.
 *
 */
/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

//#define GTK_TEXT_USE_INTERNAL_UNSUPPORTED_API
//#include "config.h"
//#include "gtkmarshalers.h"
#include "CppConsUI.h"
#include "TextLayout.h"
#include "TextBTree.h"
#include "TextTypes.h"
//#include "gtktextiterprivate.h"
//#include "gtktextutil.h"
//#include "gtkintl.h"
//#include "gtkalias.h"

//#include <stdlib.h>
#include <string.h>

//#define GTK_TEXT_LAYOUT_GET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GTK_TYPE_TEXT_LAYOUT, TextLayoutPrivate))

//typedef struct _TextLayoutPrivate TextLayoutPrivate;

/*
typedef struct
{
  * Cache the line that the cursor is positioned on, as the keyboard
     direction only influences the direction of the cursor line.
  *
  TextLine *cursor_line;
} TextLayoutPrivate;*/

enum {
  ARG_0,
  LAST_ARG
};

/*
void TextLayout::class_init (TextLayoutClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gtk_text_layout_finalize;

  klass->wrap = gtk_text_layout_real_wrap;
  klass->invalidate = gtk_text_layout_real_invalidate;
  klass->invalidate_cursors = gtk_text_layout_real_invalidate_cursors;
  klass->free_line_data = gtk_text_layout_real_free_line_data;

  signals[INVALIDATED] =
    g_signal_new (I_("invalidated"),
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (TextLayoutClass, invalidated),
                  NULL, NULL,
                  _gtk_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);

  signals[CHANGED] =
    g_signal_new (I_("changed"),
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (TextLayoutClass, changed),
                  NULL, NULL,
                  _gtk_marshal_VOID__INT_INT_INT,
                  G_TYPE_NONE,
                  3,
                  G_TYPE_INT,
                  G_TYPE_INT,
                  G_TYPE_INT);

  signals[ALLOCATE_CHILD] =
    g_signal_new (I_("allocate-child"),
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (TextLayoutClass, allocate_child),
                  NULL, NULL,
                  _gtk_marshal_VOID__OBJECT_INT_INT,
                  G_TYPE_NONE,
                  3,
                  GTK_TYPE_OBJECT,
                  G_TYPE_INT,
                  G_TYPE_INT);
  
  g_type_class_add_private (object_class, sizeof (TextLayoutPrivate));
}*/

TextLayout::TextLayout (void)
{
  cursor_visible = true;
}

/*
TextLayout*
TextLayout::new (void)
{
  return g_object_new (GTK_TYPE_TEXT_LAYOUT, NULL);
}*/

void TextLayout::free_style_cache (void)
{
  if (one_style_cache)
    {
      one_style_cache->unref ();
      one_style_cache = NULL;
    }
}

TextLayout::~TextLayout()
{
//  TextLayout *layout;

//  layout = GTK_TEXT_LAYOUT (object);

  set_buffer (NULL);

  if (default_style)
    default_style->unref ();
  default_style = NULL;

  /*
  if (ltr_context)
    {
      //TODOg_object_unref (ltr_context);
      ltr_context = NULL;
    }
  if (rtl_context)
    {
      g_object_unref (rtl_context);
      rtl_context = NULL;
    }*/
  
  if (one_display_cache) 
    {
      TextLineDisplay *tmp_display = one_display_cache;
      one_display_cache = NULL;
      free_line_display (tmp_display);
    }

  if (preedit_string)
    {
      g_free (preedit_string);
      preedit_string = NULL;
    }

  /*if (preedit_attrs)
    {
      pango_attr_list_unref (preedit_attrs);
      preedit_attrs = NULL;
    }*/


  //TODO G_OBJECT_CLASS (gtk_text_layout_parent_class)->finalize (object);
}

void TextLayout::set_buffer ( TextBuffer *buffer)
{
//  //g_return_if_fail (GTK_IS_TEXT_LAYOUT (layout));
  g_return_if_fail (buffer == NULL /*|| GTK_IS_TEXT_BUFFER (buffer)*/);

  if (this->buffer == buffer)
    return;

  free_style_cache ();

  if (this->buffer)
    {
      this->buffer->get_btree()->remove_view (this);

      /*TODO
      g_signal_handlers_disconnect_by_func (layout->buffer, 
                                            G_CALLBACK (gtk_text_layout_mark_set_handler), 
                                            layout);
      g_signal_handlers_disconnect_by_func (layout->buffer, 
                                            G_CALLBACK (gtk_text_layout_buffer_insert_text), 
                                            layout);
      g_signal_handlers_disconnect_by_func (layout->buffer, 
                                            G_CALLBACK (gtk_text_layout_buffer_delete_range), 
                                            layout);

      g_object_unref (layout->buffer);
      */
      this->buffer = NULL;
    }

  if (buffer)
    {
      this->buffer = buffer;

      //TODO g_object_ref (buffer);

      buffer->get_btree()->add_view (this);

      /* Bind to all signals that move the insert mark. */
      /*TODO
      g_signal_connect_after (layout->buffer, "mark-set",
                              G_CALLBACK (gtk_text_layout_mark_set_handler), layout);
      g_signal_connect_after (layout->buffer, "insert-text",
                              G_CALLBACK (gtk_text_layout_buffer_insert_text), layout);
      g_signal_connect_after (layout->buffer, "delete-range",
                              G_CALLBACK (gtk_text_layout_buffer_delete_range), layout);
	*/
      update_cursor_line ();
    }
}

void TextLayout::default_style_changed (void)
{
//  //g_return_if_fail (GTK_IS_TEXT_LAYOUT (layout));

  DV (g_print ("invalidating all due to default style change (%s)\n", G_STRLOC));
  invalidate_all ();
}

void TextLayout::set_default_style (
                                   TextAttributes *values)
{
//  //g_return_if_fail (GTK_IS_TEXT_LAYOUT (layout));
  g_return_if_fail (values != NULL);

  if (values == default_style)
    return;

  values->ref ();

  if (default_style)
    default_style->unref ();

  default_style = values;

  default_style_changed ();
}

/*void
TextLayout::set_contexts (
                              PangoContext  *ltr_context,
                              PangoContext  *rtl_context)
{
//  //g_return_if_fail (GTK_IS_TEXT_LAYOUT (layout));

  if (layout->ltr_context != ltr_context)
    {
      if (layout->ltr_context)
	g_object_unref (layout->ltr_context);

      layout->ltr_context = ltr_context;
      g_object_ref (layout->ltr_context);
    }

  if (layout->rtl_context != rtl_context)
    {
      if (layout->rtl_context)
	g_object_unref (layout->rtl_context);

      layout->rtl_context = rtl_context;
      g_object_ref (layout->rtl_context);
    }

  DV (g_print ("invalidating all due to new pango contexts (%s)\n", G_STRLOC));
  invalidate_all ();
}*/

/**
 * gtk_text_layout_set_overwrite_mode:
 * @layout: a #TextLayout
 * @overwrite: overwrite mode
 *
 * Sets overwrite mode
 **/
void
TextLayout::set_overwrite_mode (
				    bool       overwrite)
{
  overwrite = overwrite != 0;
  if (overwrite != overwrite_mode)
    {
      overwrite_mode = overwrite;
      invalidate_cursor_line (true);
    }
}

/**
 * gtk_text_layout_set_cursor_direction:
 * @direction: the new direction(s) for which to draw cursors.
 *             %GTK_TEXT_DIR_NONE means draw cursors for both
 *             left-to-right insertion and right-to-left insertion.
 *             (The two cursors will be visually distinguished.)
 * 
 * Sets which text directions (left-to-right and/or right-to-left) for
 * which cursors will be drawn for the insertion point. The visual
 * point at which new text is inserted depends on whether the new
 * text is right-to-left or left-to-right, so it may be desired to
 * make the drawn position of the cursor depend on the keyboard state.
 **/
void
TextLayout::set_cursor_direction (
				      TextDirection direction)
{
  if (direction != cursor_direction)
    {
      cursor_direction = direction;
      invalidate_cursor_line (true);
    }
}

/**
 * gtk_text_layout_set_keyboard_direction:
 * @keyboard_dir: the current direction of the keyboard.
 *
 * Sets the keyboard direction; this is used as for the bidirectional
 * base direction for the line with the cursor if the line contains
 * only neutral characters.
 **/
void
TextLayout::set_keyboard_direction (
					TextDirection keyboard_dir)
{
  if (keyboard_dir != keyboard_direction)
    {
      keyboard_direction = keyboard_dir;
      invalidate_cursor_line (true);
    }
}

/**
 * gtk_text_layout_get_buffer:
 * @layout: a #TextLayout
 *
 * Gets the text buffer used by the layout. See
 * gtk_text_layout_set_buffer().
 *
 * Return value: the text buffer used by the layout.
 **/
TextBuffer *
TextLayout::get_buffer (void)
{
//  g_return_val_if_fail (GTK_IS_TEXT_LAYOUT (layout), NULL);

  return buffer;
}

void
TextLayout::set_screen_width (gint width)
{
//  //g_return_if_fail (GTK_IS_TEXT_LAYOUT (layout));
  g_return_if_fail (width >= 0);
  g_return_if_fail (wrap_loop_count == 0);

  if (screen_width == width)
    return;

  screen_width = width;

  DV (g_print ("invalidating all due to new screen width (%s)\n", G_STRLOC));
  invalidate_all ();
}

/**
 * gtk_text_layout_set_cursor_visible:
 * @layout: a #TextLayout
 * @cursor_visible: If %false, then the insertion cursor will not
 *   be shown, even if the text is editable.
 *
 * Sets whether the insertion cursor should be shown. Generally,
 * widgets using #TextLayout will hide the cursor when the
 * widget does not have the input focus.
 **/
void
TextLayout::set_cursor_visible ( bool       cursor_visible)
{
  cursor_visible = (cursor_visible != false);

  if (this->cursor_visible != cursor_visible)
    {
      TextIter iter;
      gint y, height;

      this->cursor_visible = cursor_visible;

      /* Now queue a redraw on the paragraph containing the cursor
       */
      buffer->get_iter_at_mark (&iter, buffer->get_insert ());

      get_line_yrange (&iter, &y, &height);
      emit_changed (y, height, height);

      invalidate_cache (iter.get_text_line (), true);
    }
}

/**
 * gtk_text_layout_get_cursor_visible:
 * @layout: a #TextLayout
 *
 * Returns whether the insertion cursor will be shown.
 *
 * Return value: if %false, the insertion cursor will not be
    shown, even if the text is editable.
 **/
bool
TextLayout::get_cursor_visible (void)
{
  return cursor_visible;
}

/**
 * gtk_text_layout_set_preedit_string:
 * @layout: a #PangoLayout
 * @preedit_string: a string to display at the insertion point
 * @preedit_attrs: a #PangoAttrList of attributes that apply to @preedit_string
 * @cursor_pos: position of cursor within preedit string in chars
 * 
 * Set the preedit string and attributes. The preedit string is a
 * string showing text that is currently being edited and not
 * yet committed into the buffer.
 **/
/*
void
TextLayout::set_preedit_string (TextLayout *layout,
				    const gchar   *preedit_string,
				    PangoAttrList *preedit_attrs,
				    gint           cursor_pos)
{
  //g_return_if_fail (GTK_IS_TEXT_LAYOUT (layout));
  g_return_if_fail (preedit_attrs != NULL || preedit_string == NULL);

  g_free (layout->preedit_string);

  if (layout->preedit_attrs)
    pango_attr_list_unref (layout->preedit_attrs);

  if (preedit_string)
    {
      layout->preedit_string = g_strdup (preedit_string);
      layout->preedit_len = strlen (layout->preedit_string);
      pango_attr_list_ref (preedit_attrs);
      layout->preedit_attrs = preedit_attrs;

      cursor_pos = CLAMP (cursor_pos, 0, g_utf8_strlen (layout->preedit_string, -1));
      layout->preedit_cursor = g_utf8_offset_to_pointer (layout->preedit_string, cursor_pos) - layout->preedit_string;
    }
  else
    {
      layout->preedit_string = NULL;
      layout->preedit_len = 0;
      layout->preedit_attrs = NULL;
      layout->preedit_cursor = 0;
    }

  gtk_text_layout_invalidate_cursor_line (layout, false);
}*/

void
TextLayout::get_size ( gint *width, gint *height)
{
//  //g_return_if_fail (GTK_IS_TEXT_LAYOUT (layout));

  if (width)
    *width = this->width;

  if (height)
    *height = this->height;
}

void
TextLayout::invalidated (void)
{
  //TODOg_signal_emit (layout, signals[INVALIDATED], 0);
}

void
TextLayout::emit_changed (
			      gint           y,
			      gint           old_height,
			      gint           new_height)
{
  //TODO g_signal_emit (layout, signals[CHANGED], 0, y, old_height, new_height);
}

void TextLayout::__changed (
                     gint           y,
                     gint           old_height,
                     gint           new_height,
                     bool       cursors_only)
{
  /* Check if the range intersects our cached line display,
   * and invalidate the cached line if so.
   */
  if (one_display_cache)
    {
      TextLine *line = one_display_cache->line;
      gint cache_y = buffer->get_btree()->find_line_top ( line, this);
      gint cache_height = one_display_cache->height;

      if (cache_y + cache_height > y && cache_y < y + old_height)
	invalidate_cache (line, cursors_only);
    }

  emit_changed (y, old_height, new_height);
}

void
TextLayout::changed (
                         gint           y,
                         gint           old_height,
                         gint           new_height)
{
  __changed (y, old_height, new_height, false);
}

void
TextLayout::cursors_changed (
                                 gint           y,
				 gint           old_height,
				 gint           new_height)
{
  __changed (y, old_height, new_height, true);
}

void
TextLayout::free_line_data (
                                TextLine       *line,
                                TextLineData   *line_data)
{
  //GTK_TEXT_LAYOUT_GET_CLASS (layout)->free_line_data (layout, line, line_data);
  free_line_data (line, line_data);
}

void
TextLayout::invalidate (
                            const TextIter *start_index,
                            const TextIter *end_index)
{
  //GTK_TEXT_LAYOUT_GET_CLASS (layout)->invalidate (layout, start_index, end_index);
  invalidate (start_index, end_index);
}

void
TextLayout::invalidate_cursors (
				    const TextIter *start_index,
				    const TextIter *end_index)
{
  //GTK_TEXT_LAYOUT_GET_CLASS (layout)->invalidate_cursors (layout, start_index, end_index);
  invalidate_cursors (start_index, end_index);
}

TextLineData*
TextLayout::wrap (
                      TextLine  *line,
                      /* may be NULL */
                      TextLineData *line_data)
{
  //return GTK_TEXT_LAYOUT_GET_CLASS (layout)->wrap (layout, line, line_data);
  return wrap (line, line_data);
}

GSList*
TextLayout::get_lines (
                           /* [top_y, bottom_y) */
                           gint top_y,
                           gint bottom_y,
                           gint *first_line_y)
{
  TextLine *first_btree_line;
  TextLine *last_btree_line;
  TextLine *line;
  GSList *retval;

//  g_return_val_if_fail (GTK_IS_TEXT_LAYOUT (layout), NULL);
  g_return_val_if_fail (bottom_y > top_y, NULL);

  retval = NULL;

  first_btree_line =
    buffer->get_btree()->find_line_by_y (
                                   this, top_y, first_line_y);
  if (first_btree_line == NULL)
    {
      /* off the bottom */
      return NULL;
    }

  /* -1 since bottom_y is one past */
  last_btree_line =
    buffer->get_btree()->find_line_by_y (
                                    this, bottom_y - 1, NULL);

  if (!last_btree_line)
    last_btree_line =
      buffer->get_btree()->get_end_iter_line ();

  g_assert (last_btree_line != NULL);

  line = first_btree_line;
  while (true)
    {
      retval = g_slist_prepend (retval, line);

      if (line == last_btree_line)
        break;

      line = line->next_excluding_last ();
    }

  retval = g_slist_reverse (retval);

  return retval;
}

void TextLayout::invalidate_cached_style (void)
{
  free_style_cache ();
}

/* These should be called around a loop which wraps a CONTIGUOUS bunch
 * of display lines. If the lines aren't contiguous you can't call
 * these.
 */
void
TextLayout::wrap_loop_start (void)
{
//  //g_return_if_fail (GTK_IS_TEXT_LAYOUT (layout));
  g_return_if_fail (one_style_cache == NULL);

  wrap_loop_count += 1;
}

void
TextLayout::wrap_loop_end (void)
{
  g_return_if_fail (wrap_loop_count > 0);

  wrap_loop_count -= 1;

  if (wrap_loop_count == 0)
    {
      /* We cache a some stuff if we're iterating over some lines wrapping
       * them. This cleans it up.
       */
      /* Nuke our cached style */
      invalidate_cached_style ();
      g_assert (one_style_cache == NULL);
    }
}

void TextLayout::invalidate_all (void)
{
  TextIter start;
  TextIter end;

  if (buffer == NULL)
    return;

  buffer->get_bounds (&start, &end);

  invalidate (&start, &end);
}

void
TextLayout::invalidate_cache (
                                  TextLine   *line,
				  bool       cursors_only)
{
  if (one_display_cache && line == one_display_cache->line)
    {
      TextLineDisplay *display = one_display_cache;

      if (cursors_only)
	{
	  g_slist_foreach (display->cursors, (GFunc)g_free, NULL);
	  g_slist_free (display->cursors);
	  display->cursors = NULL;
	  display->cursors_invalid = true;
	  display->has_block_cursor = false;
	}
      else
	{
	  one_display_cache = NULL;
	  free_line_display (display);
	}
    }
}

/* Now invalidate the paragraph containing the cursor
 */
void
TextLayout::invalidate_cursor_line (
					bool cursors_only)
{
//  TextLayoutPrivate *priv = GTK_TEXT_LAYOUT_GET_PRIVATE (layout);
  TextLineData *line_data;

  if (cursor_line == NULL)
    return;

  line_data = cursor_line->get_data (this);
  if (line_data)
    {
      if (cursors_only)
	  invalidate_cache (cursor_line, true);
      else
	{
	  invalidate_cache (cursor_line, false);
	  cursor_line->invalidate_wrap (line_data);
	}

      invalidated ();
    }
}

void TextLayout::update_cursor_line(void)
{
//  TextLayoutPrivate *priv = GTK_TEXT_LAYOUT_GET_PRIVATE (layout);
  TextIter iter;

  buffer->get_iter_at_mark (&iter, buffer->get_insert ());

  cursor_line = iter.get_text_line ();
}

void TextLayout::real_invalidate ( TextIter *start, TextIter *end)
{
  TextLine *line;
  TextLine *last_line;

//  //g_return_if_fail (GTK_IS_TEXT_LAYOUT (layout));
  g_return_if_fail (wrap_loop_count == 0);

  /* Because we may be invalidating a mark, it's entirely possible
   * that TextIter::equal (start, end) in which case we
   * should still invalidate the line they are both on. i.e.
   * we always invalidate the line with "start" even
   * if there's an empty range.
   */
  
#if 0
  gtk_text_view_index_spew (start_index, "invalidate start");
  gtk_text_view_index_spew (end_index, "invalidate end");
#endif

  last_line = end->get_text_line ();
  line = start->get_text_line ();

  while (true)
    {
      TextLineData *line_data = line->get_data (this);

      invalidate_cache (line, false);
      
      if (line_data)
        line->invalidate_wrap (line_data);

      if (line == last_line)
        break;

      line = line->next_excluding_last ();
    }

  invalidated ();
}

void
TextLayout::real_invalidate_cursors ( TextIter *start, TextIter *end)
{
  /* Check if the range intersects our cached line display,
   * and invalidate the cached line if so.
   */
  if (one_display_cache)
    {
      TextIter line_start, line_end;
      TextLine *line = one_display_cache->line;

      buffer->get_btree()->get_iter_at_line ( &line_start, line, 0);
      line_end = line_start;
      if (!line_end.ends_line ())
	line_end.forward_to_line_end ();

      if (TextIter::compare (start, end) > 0)
	{
	  TextIter *tmp = start;
	  start = end;
	  end = tmp;
	}

      if (TextIter::compare (&line_start, end) <= 0 &&
	  TextIter::compare (start, &line_end) <= 0)
	{
	  invalidate_cache (line, true);
	}
    }

  invalidated ();
}

void TextLayout::real_free_line_data ( TextLine       *line, TextLineData   *line_data)
{
  invalidate_cache (line, false);

  g_free (line_data);
}

/**
 * gtk_text_layout_is_valid:
 * @layout: a #TextLayout
 *
 * Check if there are any invalid regions in a #TextLayout's buffer
 *
 * Return value: %true if any invalid regions were found
 **/
bool
TextLayout::is_valid (void)
{
//  g_return_val_if_fail (GTK_IS_TEXT_LAYOUT (layout), false);

  return buffer->get_btree()->is_valid (this);
}

void TextLayout::update_layout_size (void)
{
  buffer->get_btree()->get_view_size ( this,
				&width, &height);
}

/**
 * gtk_text_layout_validate_yrange:
 * @layout: a #TextLayout
 * @anchor: iter pointing into a line that will be used as the
 *          coordinate origin
 * @y0_: offset from the top of the line pointed to by @anchor at
 *       which to begin validation. (The offset here is in pixels
 *       after validation.)
 * @y1_: offset from the top of the line pointed to by @anchor at
 *       which to end validation. (The offset here is in pixels
 *       after validation.)
 *
 * Ensure that a region of a #TextLayout is valid. The ::changed
 * signal will be emitted if any lines are validated.
 **/
void
TextLayout::validate_yrange (
                                 TextIter   *anchor,
                                 gint           y0,
                                 gint           y1)
{
  TextLine *line;
  TextLine *first_line = NULL;
  TextLine *last_line = NULL;
  gint seen;
  gint delta_height = 0;
  gint first_line_y = 0;        /* Quiet GCC */
  gint last_line_y = 0;         /* Quiet GCC */

  //g_return_if_fail (GTK_IS_TEXT_LAYOUT (layout));

  if (y0 > 0)
    y0 = 0;
  if (y1 < 0)
    y1 = 0;
  
  /* Validate backwards from the anchor line to y0
   */
  line = anchor->get_text_line ();
  line = line->previous_line ();
  seen = 0;
  while (line && seen < -y0)
    {
      TextLineData *line_data = line->get_data (this);
      if (!line_data || !line_data->valid)
        {
          gint old_height, new_height;
	  
	  old_height = line_data ? line_data->height : 0;

          buffer->get_btree()->validate_line ( line, this);
          line_data = line->get_data (this);

	  new_height = line_data ? line_data->height : 0;

          delta_height += new_height - old_height;
          
          first_line = line;
          first_line_y = -seen - new_height;
          if (!last_line)
            {
              last_line = line;
              last_line_y = -seen;
            }
        }

      seen += line_data ? line_data->height : 0;
      line = line->previous_line ();
    }

  /* Validate forwards to y1 */
  line = anchor->get_text_line ();
  seen = 0;
  while (line && seen < y1)
    {
      TextLineData *line_data = line->get_data (this);
      if (!line_data || !line_data->valid)
        {
          gint old_height, new_height;
	  
	  old_height = line_data ? line_data->height : 0;

          buffer->get_btree()->validate_line ( line, this);
          line_data = line->get_data (this);
	  new_height = line_data ? line_data->height : 0;

          delta_height += new_height - old_height;
          
          if (!first_line)
            {
              first_line = line;
              first_line_y = seen;
            }
          last_line = line;
          last_line_y = seen + new_height;
        }

      seen += line_data ? line_data->height : 0;
      line = line->next_excluding_last ();
    }

  /* If we found and validated any invalid lines, update size and
   * emit the changed signal
   */
  if (first_line)
    {
      gint line_top;

      update_layout_size ();

      line_top = buffer->get_btree()->find_line_top ( first_line, this);

      emit_changed (
				    line_top,
				    last_line_y - first_line_y - delta_height,
				    last_line_y - first_line_y);
    }
}

/**
 * gtk_text_layout_validate:
 * @tree: a #TextLayout
 * @max_pixels: the maximum number of pixels to validate. (No more
 *              than one paragraph beyond this limit will be validated)
 *
 * Validate regions of a #TextLayout. The ::changed signal will
 * be emitted for each region validated.
 **/
void
TextLayout::validate (
                          gint           max_pixels)
{
  gint y, old_height, new_height;

  //g_return_if_fail (GTK_IS_TEXT_LAYOUT (layout));

  while (max_pixels > 0 &&
         buffer->get_btree()->validate (
                                   this,  max_pixels,
                                   &y, &old_height, &new_height))
    {
      max_pixels -= new_height;

      update_layout_size ();
      emit_changed (y, old_height, new_height);
    }
}

TextLineData* TextLayout::real_wrap (
                           TextLine     *line,
                           /* may be NULL */
                           TextLineData *line_data)
{
  TextLineDisplay *display;

//  g_return_val_if_fail (GTK_IS_TEXT_LAYOUT (layout), NULL);
  g_return_val_if_fail (line != NULL, NULL);
  
  if (line_data == NULL)
    {
      line_data = new TextLineData (this, line);
      line->add_data (line_data);
    }

  display = get_line_display (line, true);
  line_data->width = display->width;
  line_data->height = display->height;
  line_data->valid = true;
  free_line_display (display);

  return line_data;
}

/*
 * Layout utility functions
 */

/* If you get the style with get_style () you need to call
   release_style () to free it. */
TextAttributes* TextLayout::get_style ( GPtrArray     *tags)
{
  TextAttributes *style;

  /* If we have the one-style cache, then it means
     that we haven't seen a toggle since we filled in the
     one-style cache.
  */
  if (one_style_cache != NULL)
    {
      one_style_cache->ref();
      return one_style_cache;
    }

  g_assert (one_style_cache == NULL);

  /* No tags, use default style */
  if (tags == NULL || tags->len == 0)
    {
      /* One ref for the return value, one ref for the
         layout->one_style_cache reference */
      default_style->ref();
      default_style->ref();
      one_style_cache = default_style;

      return default_style;
    }

  style = new TextAttributes();//gtk_text_attributes_new ();

  TextAttributes::copy_values (default_style,
                                   style);

  style->fill_from_tags (
                                       (TextTag**) tags->pdata,
                                       tags->len);

  g_assert (style->refcount == 1);

  /* Leave this style as the last one seen */
  g_assert (one_style_cache == NULL);
  style->ref (); /* ref held by layout->one_style_cache */
  one_style_cache = style;

  /* Returning yet another refcount */
  return style;
}

void TextLayout::release_style ( TextAttributes *style)
{
  g_return_if_fail (style != NULL);
  g_return_if_fail (style->refcount > 0);

  style->unref ();
}

/*
 * Lines
 */

/* This function tries to optimize the case where a line
   is completely invisible */
bool TextLayout::totally_invisible_line ( TextLine   *line, TextIter   *iter)
{
  TextLineSegment *seg;
  int bytes = 0;

  /* Check if the first char is visible, if so we are partially visible.  
   * Note that we have to check this since we don't know the current 
   * invisible/noninvisible toggle state; this function can use the whole btree 
   * to get it right.
   */
  buffer->get_btree()->get_iter_at_line ( iter, line, 0);
  
  if (!buffer->get_btree()->char_is_invisible (iter))
    return false;

  bytes = 0;
  seg = line->segments;

  while (seg != NULL)
    {
      if (seg->byte_count > 0)
        bytes += seg->byte_count;

      /* Note that these two tests can cause us to bail out
       * when we shouldn't, because a higher-priority tag
       * may override these settings. However the important
       * thing is to only invisible really-invisible lines, rather
       * than to invisible all really-invisible lines.
       */

      else if (seg->type == text_segment_toggle_on)
        {
          invalidate_cached_style ();

          /* Bail out if an elision-unsetting tag begins */
          if (seg->body.toggle.info->tag->invisible_set &&
              !seg->body.toggle.info->tag->values->invisible)
            break;
        }
      else if (seg->type == text_segment_toggle_off)
        {
          invalidate_cached_style ();

          /* Bail out if an elision-setting tag ends */
          if (seg->body.toggle.info->tag->invisible_set &&
              seg->body.toggle.info->tag->values->invisible)
            break;
        }

      seg = seg->next;
    }

  if (seg != NULL)       /* didn't reach line end */
    return false;

  return true;
}

/*static void
set_para_values (TextLayout      *layout,
                 PangoDirection      base_dir,
                 TextAttributes  *style,
                 TextLineDisplay *display)
{
  PangoAlignment pango_align = PANGO_ALIGN_LEFT;
  PangoWrapMode pango_wrap = PANGO_WRAP_WORD;

  switch (base_dir)
    {
    * If no base direction was found, then use the style direction *
    case PANGO_DIRECTION_NEUTRAL :
      display->direction = style->direction;

      * Override the base direction *
      if (display->direction == GTK_TEXT_DIR_RTL)
        base_dir = PANGO_DIRECTION_RTL;
      else
        base_dir = PANGO_DIRECTION_LTR;
      
      break;
    case PANGO_DIRECTION_RTL :
      display->direction = GTK_TEXT_DIR_RTL;
      break;
    default:
      display->direction = GTK_TEXT_DIR_LTR;
      break;
    }
  
  if (display->direction == GTK_TEXT_DIR_RTL)
    display->layout = pango_layout_new (layout->rtl_context);
  else
    display->layout = pango_layout_new (layout->ltr_context);

  switch (style->justification)
    {
    case GTK_JUSTIFY_LEFT:
      pango_align = (base_dir == PANGO_DIRECTION_LTR) ? PANGO_ALIGN_LEFT : PANGO_ALIGN_RIGHT;
      break;
    case GTK_JUSTIFY_RIGHT:
      pango_align = (base_dir == PANGO_DIRECTION_LTR) ? PANGO_ALIGN_RIGHT : PANGO_ALIGN_LEFT;
      break;
    case GTK_JUSTIFY_CENTER:
      pango_align = PANGO_ALIGN_CENTER;
      break;
    case GTK_JUSTIFY_FILL:
      pango_align = (base_dir == PANGO_DIRECTION_LTR) ? PANGO_ALIGN_LEFT : PANGO_ALIGN_RIGHT;
      pango_layout_set_justify (display->layout, true);
      break;
    default:
      g_assert_not_reached ();
      break;
    }

  pango_layout_set_alignment (display->layout, pango_align);
  pango_layout_set_spacing (display->layout,
                            style->pixels_inside_wrap * PANGO_SCALE);

  if (style->tabs)
    pango_layout_set_tabs (display->layout, style->tabs);

  display->top_margin = style->pixels_above_lines;
  display->height = style->pixels_above_lines + style->pixels_below_lines;
  display->bottom_margin = style->pixels_below_lines;
  display->left_margin = style->left_margin;
  display->right_margin = style->right_margin;
  
  display->x_offset = display->left_margin;

  pango_layout_set_indent (display->layout,
                           style->indent * PANGO_SCALE);

  switch (style->wrap_mode)
    {
    case GTK_WRAP_CHAR:
      pango_wrap = PANGO_WRAP_CHAR;
      break;
    case GTK_WRAP_WORD:
      pango_wrap = PANGO_WRAP_WORD;
      break;

    case GTK_WRAP_WORD_CHAR:
      pango_wrap = PANGO_WRAP_WORD_CHAR;
      break;

    case GTK_WRAP_NONE:
      break;
    }

  if (style->wrap_mode != GTK_WRAP_NONE)
    {
      int layout_width = (layout->screen_width - display->left_margin - display->right_margin);
      pango_layout_set_width (display->layout, layout_width * PANGO_SCALE);
      pango_layout_set_wrap (display->layout, pango_wrap);
    }

  display->total_width = MAX (layout->screen_width, layout->width) - display->left_margin - display->right_margin;
  
  if (style->pg_bg_color)
    display->pg_bg_color = gdk_color_copy (style->pg_bg_color);
  else
    display->pg_bg_color = NULL;  
}*/

/*static PangoAttribute *
gtk_text_attr_appearance_copy (const PangoAttribute *attr)
{
  const TextAttrAppearance *appearance_attr = (const TextAttrAppearance *)attr;

  return gtk_text_attr_appearance_new (&appearance_attr->appearance);
}

static void
gtk_text_attr_appearance_destroy (PangoAttribute *attr)
{
  TextAttrAppearance *appearance_attr = (TextAttrAppearance *)attr;

  if (appearance_attr->appearance.bg_stipple)
    g_object_unref (appearance_attr->appearance.bg_stipple);
  if (appearance_attr->appearance.fg_stipple)
    g_object_unref (appearance_attr->appearance.fg_stipple);

  g_slice_free (TextAttrAppearance, appearance_attr);
}

static bool
gtk_text_attr_appearance_compare (const PangoAttribute *attr1,
                                  const PangoAttribute *attr2)
{
  const TextAppearance *appearance1 = &((const TextAttrAppearance *)attr1)->appearance;
  const TextAppearance *appearance2 = &((const TextAttrAppearance *)attr2)->appearance;

  return (gdk_color_equal (&appearance1->fg_color, &appearance2->fg_color) &&
          gdk_color_equal (&appearance1->bg_color, &appearance2->bg_color) &&
          appearance1->fg_stipple ==  appearance2->fg_stipple &&
          appearance1->bg_stipple ==  appearance2->bg_stipple &&
          appearance1->underline == appearance2->underline &&
          appearance1->strikethrough == appearance2->strikethrough &&
          appearance1->draw_bg == appearance2->draw_bg);
}*

/**
 * gtk_text_attr_appearance_new:
 * @desc:
 *
 * Create a new font description attribute. (This attribute
 * allows setting family, style, weight, variant, stretch,
 * and size simultaneously.)
 *
 * Return value:
 **/
/*static PangoAttribute *
gtk_text_attr_appearance_new (const TextAppearance *appearance)
{
  static PangoAttrClass klass = {
    0,
    gtk_text_attr_appearance_copy,
    gtk_text_attr_appearance_destroy,
    gtk_text_attr_appearance_compare
  };

  TextAttrAppearance *result;

  if (!klass.type)
    klass.type = gtk_text_attr_appearance_type =
      pango_attr_type_register ("TextAttrAppearance");

  result = g_slice_new (TextAttrAppearance);
  result->attr.klass = &klass;

  result->appearance = *appearance;

  if (appearance->bg_stipple)
    g_object_ref (appearance->bg_stipple);
  if (appearance->fg_stipple)
    g_object_ref (appearance->fg_stipple);

  return (PangoAttribute *)result;
}

static void
add_generic_attrs (TextLayout      *layout,
                   TextAppearance  *appearance,
                   gint                byte_count,
                   PangoAttrList      *attrs,
                   gint                start,
                   bool            size_only,
                   bool            is_text)
{
  PangoAttribute *attr;

  if (appearance->underline != PANGO_UNDERLINE_NONE)
    {
      attr = pango_attr_underline_new (appearance->underline);
      
      attr->start_index = start;
      attr->end_index = start + byte_count;
      
      pango_attr_list_insert (attrs, attr);
    }

  if (appearance->strikethrough)
    {
      attr = pango_attr_strikethrough_new (appearance->strikethrough);
      
      attr->start_index = start;
      attr->end_index = start + byte_count;
      
      pango_attr_list_insert (attrs, attr);
    }

  if (appearance->rise != 0)
    {
      attr = pango_attr_rise_new (appearance->rise);
      
      attr->start_index = start;
      attr->end_index = start + byte_count;
      
      pango_attr_list_insert (attrs, attr);
    }
  
  if (!size_only)
    {
      attr = gtk_text_attr_appearance_new (appearance);
      
      attr->start_index = start;
      attr->end_index = start + byte_count;

      ((TextAttrAppearance *)attr)->appearance.is_text = is_text;
      
      pango_attr_list_insert (attrs, attr);
    }
}

static void
add_text_attrs (TextLayout      *layout,
                TextAttributes  *style,
                gint                byte_count,
                PangoAttrList      *attrs,
                gint                start,
                bool            size_only)
{
  PangoAttribute *attr;

  attr = pango_attr_font_desc_new (style->font);
  attr->start_index = start;
  attr->end_index = start + byte_count;

  pango_attr_list_insert (attrs, attr);

  if (style->font_scale != 1.0)
    {
      attr = pango_attr_scale_new (style->font_scale);

      attr->start_index = start;
      attr->end_index = start + byte_count;
      
      pango_attr_list_insert (attrs, attr);
    }
}

static void
add_pixbuf_attrs (TextLayout      *layout,
                  TextLineDisplay *display,
                  TextAttributes  *style,
                  TextLineSegment *seg,
                  PangoAttrList      *attrs,
                  gint                start)
{
  PangoAttribute *attr;
  PangoRectangle logical_rect;
  TextPixbuf *pixbuf = &seg->body.pixbuf;
  gint width, height;

  width = gdk_pixbuf_get_width (pixbuf->pixbuf);
  height = gdk_pixbuf_get_height (pixbuf->pixbuf);

  logical_rect.x = 0;
  logical_rect.y = -height * PANGO_SCALE;
  logical_rect.width = width * PANGO_SCALE;
  logical_rect.height = height * PANGO_SCALE;

  attr = pango_attr_shape_new_with_data (&logical_rect, &logical_rect,
					 pixbuf->pixbuf, NULL, NULL);
  attr->start_index = start;
  attr->end_index = start + seg->byte_count;
  pango_attr_list_insert (attrs, attr);

  display->shaped_objects =
    g_slist_append (display->shaped_objects, pixbuf->pixbuf);
}

static void
add_child_attrs (TextLayout      *layout,
                 TextLineDisplay *display,
                 TextAttributes  *style,
                 TextLineSegment *seg,
                 PangoAttrList      *attrs,
                 gint                start)
{
  PangoAttribute *attr;
  PangoRectangle logical_rect;
  gint width, height;
  GSList *tmp_list;
  GtkWidget *widget;

  width = 1;
  height = 1;
  
  tmp_list = seg->body.child.widgets;
  while (tmp_list != NULL)
    {
      GtkWidget *child = tmp_list->data;

      if (_gtk_anchored_child_get_layout (child) == layout)
        {
          * Found it *
          GtkRequisition req;

          gtk_widget_get_child_requisition (child, &req);
          
          width = req.width;
          height = req.height;

	  widget = child;
          
          break;
        }
      
      tmp_list = g_slist_next (tmp_list);
    }

  if (tmp_list == NULL)
    {
      * If tmp_list == NULL then there is no widget at this anchor in
       * this display; not an error. We make up an arbitrary size
       * to use, just so the programmer can see the blank spot.
       * We also put a NULL in the shaped objects list, to keep
       * the correspondence between the list and the shaped chars in
       * the layout. A bad hack, yes.
       *

      width = 30;
      height = 20;

      widget = NULL;
    }

  display->shaped_objects = g_slist_append (display->shaped_objects, widget);
  
  logical_rect.x = 0;
  logical_rect.y = -height * PANGO_SCALE;
  logical_rect.width = width * PANGO_SCALE;
  logical_rect.height = height * PANGO_SCALE;

  attr = pango_attr_shape_new_with_data (&logical_rect, &logical_rect,
					 widget, NULL, NULL);
  attr->start_index = start;
  attr->end_index = start + seg->byte_count;
  pango_attr_list_insert (attrs, attr);
}*/

/**
 * get_block_cursor:
 * @layout: a #TextLayout
 * @display: a #TextLineDisplay
 * @insert_iter: iter pointing to the cursor location
 * @insert_index: cursor offset in the @display's layout, it may
 * be different from @insert_iter's offset in case when preedit
 * string is present.
 * @pos: location to store cursor position
 * @cursor_at_line_end: whether cursor is at the end of line
 *
 * Checks whether layout should display block cursor at given position.
 * For this layout must be in overwrite mode and text at @insert_iter 
 * must be editable.
 **/
//TODO rename
bool TextLayout::__get_block_cursor (
		  TextLineDisplay *display,
		  TextIter  *insert_iter,
		  gint                insert_index,
		  Rect       *pos,
		  bool           *cursor_at_line_end)
{
  //PangoRectangle pango_pos;

	/*TODO implement
  if (overwrite_mode &&
      insert_iter->editable (true) &&
       _gtk_text_util_get_block_cursor_location (display->layout,
						insert_index,
						&pango_pos,
    					        cursor_at_line_end))
    {
      if (pos)
	{
	  pos->x = PANGO_PIXELS (pango_pos.x);
	  pos->y = PANGO_PIXELS (pango_pos.y);
	  pos->width = PANGO_PIXELS (pango_pos.width);
	  pos->height = PANGO_PIXELS (pango_pos.height);
	}

      return true;
    }
  else
    return false;
    */
}

void TextLayout::add_cursor (
            TextLineDisplay *display,
            TextLineSegment *seg,
            gint                start)
{
//  PangoRectangle strong_pos, weak_pos;
  TextCursorDisplay *cursor = NULL; /* Quiet GCC */
  bool add_weak = false;
  bool add_strong = false;
  
  /* Hide insertion cursor when we have a selection or the layout
   * user has hidden the cursor.
   */
  if (buffer->get_btree()->mark_is_insert (
                                     seg->body.mark.obj) &&
      (!cursor_visible ||
       buffer->get_selection_bounds (NULL, NULL)))
    return;

  if (overwrite_mode &&
      buffer->get_btree()->mark_is_insert (
				      seg->body.mark.obj))
    {
      TextIter iter;
      bool cursor_at_line_end;

      buffer->get_btree()->get_iter_at_mark (
					&iter, seg->body.mark.obj);

      if (__get_block_cursor (display, &iter, start,
			    &display->block_cursor,
			    &cursor_at_line_end))
	{
	  display->has_block_cursor = true;
	  display->cursor_at_line_end = cursor_at_line_end;
	  return;
	}
    }

  //TODOpango_layout_get_cursor_pos (display->layout, start, &strong_pos, &weak_pos);

  if (cursor_direction == GTK_TEXT_DIR_NONE)
    {
      add_strong = true;
      add_weak = true;
    }
  else if (display->direction == cursor_direction)
    add_strong = true;
  else
    add_weak = true;

  if (add_strong)
    {
      cursor = g_new (TextCursorDisplay, 1);

      //TODOcursor->x = PANGO_PIXELS (strong_pos.x);
      /*cursor->y = PANGO_PIXELS (strong_pos.y);
      cursor->height = PANGO_PIXELS (strong_pos.height);
      cursor->is_strong = true;
      cursor->is_weak = (cursor_direction == GTK_TEXT_DIR_NONE) ? false : true;*/
      display->cursors = g_slist_prepend (display->cursors, cursor);
    }
  
  if (add_weak)
    {
      /*TODOif (weak_pos.x == strong_pos.x && add_strong)
	cursor->is_weak = true;
      else*/
	{
	  cursor = g_new (TextCursorDisplay, 1);
	  
	  /*TODO cursor->x = PANGO_PIXELS (weak_pos.x);
	  cursor->y = PANGO_PIXELS (weak_pos.y);
	  cursor->height = PANGO_PIXELS (weak_pos.height);*/
	  //TODOcursor->is_strong = (layout->cursor_direction == GTK_TEXT_DIR_NONE) ? false : true;
	  cursor->is_weak = true;
	  display->cursors = g_slist_prepend (display->cursors, cursor);
	}
    }
}
/*
static bool
is_shape (PangoLayoutRun *run)
{
  GSList *tmp_list = run->item->analysis.extra_attrs;
    
  while (tmp_list)
    {
      PangoAttribute *attr = tmp_list->data;

      if (attr->klass->type == PANGO_ATTR_SHAPE)
        return true;

      tmp_list = tmp_list->next;
    }

  return false;
}*/

/*static void
allocate_child_widgets (TextLayout      *text_layout,
                        TextLineDisplay *display)
{
  GSList *shaped = display->shaped_objects;
  PangoLayout *layout = display->layout;
  PangoLayoutIter *iter;
  
  iter = pango_layout_get_iter (layout);
  
  do
    {
      PangoLayoutRun *run = pango_layout_iter_get_run_readonly (iter);

      if (run && is_shape (run))
        {
          GObject *shaped_object = shaped->data;
          shaped = shaped->next;

          * shaped_object is NULL for child anchors with no
           * widgets stored at them
           *
          if (GTK_IS_WIDGET (shaped_object))
            {
              PangoRectangle extents;

              * We emit "allocate_child" with the x,y of
               * the widget with respect to the top of the line
               * and the left side of the buffer
               *
              
              pango_layout_iter_get_run_extents (iter,
                                                 NULL,
                                                 &extents);
              
              g_signal_emit (text_layout,
                             signals[ALLOCATE_CHILD],
                             0,
                             shaped_object,
                             PANGO_PIXELS (extents.x) + display->x_offset,
                             PANGO_PIXELS (extents.y) + display->top_margin);
            }
        }
    }
  while (pango_layout_iter_next_run (iter));
  
  pango_layout_iter_free (iter);
}

static void
convert_color (GdkColor       *result,
	       PangoAttrColor *attr)
{
  result->red = attr->color.red;
  result->blue = attr->color.blue;
  result->green = attr->color.green;
}*/

/* This function is used to convert the preedit string attributes, which are
 * standard PangoAttributes, into the custom attributes used by the text
 * widget and insert them into a attr list with a given offset.
 */
/*
static void
add_preedit_attrs (TextLayout     *layout,
		   TextAttributes *style,
		   PangoAttrList     *attrs,
		   gint               offset,
		   bool           size_only)
{
  PangoAttrIterator *iter = pango_attr_list_get_iterator (layout->preedit_attrs);

  do
    {
      TextAppearance appearance = style->appearance;
      PangoFontDescription *font_desc = pango_font_description_copy_static (style->font);
      PangoAttribute *insert_attr;
      GSList *extra_attrs = NULL;
      GSList *tmp_list;
      PangoLanguage *language;
      gint start, end;

      pango_attr_iterator_range (iter, &start, &end);

      if (end == G_MAXINT)
	end = layout->preedit_len;
      
      if (end == start)
	continue;

      pango_attr_iterator_get_font (iter, font_desc, &language, &extra_attrs);
      
      tmp_list = extra_attrs;
      while (tmp_list)
	{
	  PangoAttribute *attr = tmp_list->data;
	  
	  switch (attr->klass->type)
	    {
	    case PANGO_ATTR_FOREGROUND:
	      convert_color (&appearance.fg_color, (PangoAttrColor *)attr);
	      break;
	    case PANGO_ATTR_BACKGROUND:
	      convert_color (&appearance.bg_color, (PangoAttrColor *)attr);
	      appearance.draw_bg = true;
	      break;
	    case PANGO_ATTR_UNDERLINE:
	      appearance.underline = ((PangoAttrInt *)attr)->value;
	      break;
	    case PANGO_ATTR_STRIKETHROUGH:
	      appearance.strikethrough = ((PangoAttrInt *)attr)->value;
	      break;
            case PANGO_ATTR_RISE:
              appearance.rise = ((PangoAttrInt *)attr)->value;
              break;
	    default:
	      break;
	    }
	  
	  pango_attribute_destroy (attr);
	  tmp_list = tmp_list->next;
	}
      
      g_slist_free (extra_attrs);
      
      insert_attr = pango_attr_font_desc_new (font_desc);
      insert_attr->start_index = start + offset;
      insert_attr->end_index = end + offset;
      
      pango_attr_list_insert (attrs, insert_attr);

      if (language)
	{
	  insert_attr = pango_attr_language_new (language);
	  insert_attr->start_index = start + offset;
	  insert_attr->end_index = end + offset;
	  
	  pango_attr_list_insert (attrs, insert_attr);
	}

      add_generic_attrs (layout, &appearance, end - start,
                         attrs, start + offset,
                         size_only, true);
      
      pango_font_description_free (font_desc);
    }
  while (pango_attr_iterator_next (iter));

  pango_attr_iterator_destroy (iter);
}*/

/* Iterate over the line and fill in display->cursors.
 * It's a stripped copy of gtk_text_layout_get_line_display() */
void TextLayout::update_text_display_cursors (
			     TextLine        *line,
			     TextLineDisplay *display)
{
  TextLineSegment *seg;
  TextIter iter;
  gint layout_byte_offset, buffer_byte_offset;
  GSList *cursor_byte_offsets = NULL;
  GSList *cursor_segs = NULL;
  GSList *tmp_list1, *tmp_list2;

  if (!display->cursors_invalid)
    return;

  display->cursors_invalid = false;

  buffer->get_btree()->get_iter_at_line ( &iter, line, 0);

  /* Special-case optimization for completely
   * invisible lines; makes it faster to deal
   * with sequences of invisible lines.
   */
  if (totally_invisible_line (line, &iter))
    return;

  /* Iterate over segments */
  layout_byte_offset = 0; /* position in the layout text (includes preedit, does not include invisible text) */
  buffer_byte_offset = 0; /* position in the buffer line */
  seg = iter.get_any_segment ();
  while (seg != NULL)
    {
      /* Displayable segments */
      if (seg->type == text_segment_char /*||
          seg->type == &text_pixbuf_type ||
          seg->type == &text_child_type*/)
        {
          buffer->get_btree()->get_iter_at_line (
                                            &iter, line,
                                            buffer_byte_offset);

	  if (!buffer->get_btree()->char_is_invisible (&iter))
            layout_byte_offset += seg->byte_count;

	  buffer_byte_offset += seg->byte_count;
        }

      /* Marks */
      else if (seg->type == text_segment_right_mark ||
               seg->type == text_segment_left_mark)
        {
	  gint cursor_offset = 0;

	  /* At the insertion point, add the preedit string, if any */

	  if (buffer->get_btree()->mark_is_insert (
					      seg->body.mark.obj))
	    {
	      display->insert_index = layout_byte_offset;

	      if (preedit_len > 0)
		{
		  layout_byte_offset += preedit_len;
                  /* DO NOT increment the buffer byte offset for preedit */
		  cursor_offset = preedit_cursor - preedit_len;
		}
	    }

          /* Display visible marks */

          if (seg->body.mark.visible)
            {
              cursor_byte_offsets = g_slist_prepend (cursor_byte_offsets,
                                                     GINT_TO_POINTER (layout_byte_offset + cursor_offset));
              cursor_segs = g_slist_prepend (cursor_segs, seg);
            }
        }

      /* Toggles */
      else if (seg->type == text_segment_toggle_on ||
               seg->type == text_segment_toggle_off)
        {
        }

      else
        g_error ("Unknown segment type: %s", seg->name);

      seg = seg->next;
    }

  tmp_list1 = cursor_byte_offsets;
  tmp_list2 = cursor_segs;
  /*TODO
  while (tmp_list1)
    {
      add_cursor (display, tmp_list2->data,
                  GPOINTER_TO_INT (tmp_list1->data));
      tmp_list1 = tmp_list1->next;
      tmp_list2 = tmp_list2->next;
    }*/
  g_slist_free (cursor_byte_offsets);
  g_slist_free (cursor_segs);
}

/* Same as buffer->get_btree()->get_tags(), except it returns GPtrArray,
 * to be used in gtk_text_layout_get_line_display(). */
//TODO move to textiter class?
GPtrArray * TextLayout::get_tags_array_at_iter (TextIter *iter)
{
  TextTag **tags;
  GPtrArray *array = NULL;
  gint n_tags;

  tags = buffer->get_btree()->get_tags (iter, &n_tags);

  if (n_tags > 0)
    {
      array = g_ptr_array_sized_new (n_tags);
      g_ptr_array_set_size (array, n_tags);
      memcpy (array->pdata, tags, n_tags * sizeof (TextTag*));
    }

  g_free (tags);
  return array;
}

/* Add the tag to the array if it's not there already, and remove
 * it otherwise. It keeps the array sorted by tags priority. */
static GPtrArray *
tags_array_toggle_tag (GPtrArray  *array,
		       TextTag *tag)
{
  gint pos;
  TextTag **tags;

  if (array == NULL)
    array = g_ptr_array_new ();

  tags = (TextTag**) array->pdata;

  for (pos = 0; pos < array->len && tags[pos]->priority < tag->priority; pos++) ;

  if (pos < array->len && tags[pos] == tag)
    g_ptr_array_remove_index (array, pos);
  else
    {
      g_ptr_array_set_size (array, array->len + 1);
      if (pos < array->len - 1)
	memmove (array->pdata + pos + 1, array->pdata + pos,
		 (array->len - pos - 1) * sizeof (TextTag*));
      array->pdata[pos] = tag;
    }

  return array;
}

TextLineDisplay * TextLayout::get_line_display (
                                  TextLine   *line,
                                  bool       size_only)
{
//  TextLayoutPrivate *priv = GTK_TEXT_LAYOUT_GET_PRIVATE (layout);
  TextLineDisplay *display;
  TextLineSegment *seg;
  TextIter iter;
  TextAttributes *style;
  gchar *text;
  //PangoAttrList *attrs;
  gint text_allocated, layout_byte_offset, buffer_byte_offset;
  //PangoRectangle extents;
  bool para_values_set = false;
  GSList *cursor_byte_offsets = NULL;
  GSList *cursor_segs = NULL;
  GSList *tmp_list1, *tmp_list2;
  bool saw_widget = false;
  //PangoDirection base_dir;
  GPtrArray *tags;
  bool initial_toggle_segments;
  
  g_return_val_if_fail (line != NULL, NULL);

  if (one_display_cache)
    {
      if (line == one_display_cache->line &&
          (size_only || !one_display_cache->size_only))
	{
	  if (!size_only)
            update_text_display_cursors (line, one_display_cache);
	  return one_display_cache;
	}
      else
        {
          TextLineDisplay *tmp_display = one_display_cache;
          one_display_cache = NULL;
          free_line_display (tmp_display);
        }
    }

  DV (g_print ("creating one line display cache (%s)\n", G_STRLOC));

  display = g_new0 (TextLineDisplay, 1);

  display->size_only = size_only;
  display->line = line;
  display->insert_index = -1;

  buffer->get_btree()->get_iter_at_line (
                                    &iter, line, 0);

  /* Special-case optimization for completely
   * invisible lines; makes it faster to deal
   * with sequences of invisible lines.
   */
  if (totally_invisible_line (line, &iter))
    {
      if (display->direction == GTK_TEXT_DIR_RTL)
      {}//TODOdisplay->layout = pango_layout_new (layout->rtl_context);
      else
      {}//TODOdisplay->layout = pango_layout_new (layout->ltr_context);
      
      return display;
    }

  /* Find the bidi base direction */
  /*base_dir = line->dir_propagated_forward;
  if (base_dir == PANGO_DIRECTION_NEUTRAL)
    base_dir = line->dir_propagated_back;*/

  if (line == cursor_line /*&&
      line->dir_strong == PANGO_DIRECTION_NEUTRAL*/)
    {
      //TODObase_dir = (keyboard_direction == GTK_TEXT_DIR_LTR) ?
	// PANGO_DIRECTION_LTR : PANGO_DIRECTION_RTL;
    }
  
  /* Allocate space for flat text for buffer
   */
  text_allocated = line->byte_count ();
  text = (gchar*)g_malloc (text_allocated);

  //attrs = pango_attr_list_new ();

  /* Iterate over segments, creating display chunks for them, and updating the tags array. */
  layout_byte_offset = 0; /* current length of layout text (includes preedit, does not include invisible text) */
  buffer_byte_offset = 0; /* position in the buffer line */
  seg = iter.get_any_segment ();
  tags = get_tags_array_at_iter (&iter);
  initial_toggle_segments = true;
  while (seg != NULL)
    {
      /* Displayable segments */
      if (seg->type == text_segment_char /*||
          seg->type == &text_pixbuf_type ||
          seg->type == &text_child_type*/)
        {
          style = get_style (tags);
	  initial_toggle_segments = false;

          /* We have to delay setting the paragraph values until we
           * hit the first pixbuf or text segment because toggles at
           * the beginning of the paragraph should affect the
           * paragraph-global values
           */
          if (!para_values_set)
            {
              //TODOset_para_values (base_dir, style, display);
              //para_values_set = true;
            }

          /* First see if the chunk is invisible, and ignore it if so. Tk
           * looked at tabs, wrap mode, etc. before doing this, but
           * that made no sense to me, so I am just skipping the
           * invisible chunks
           */
          if (!style->invisible)
            {
              if (seg->type == text_segment_char)
                {
                  /* We don't want to split segments because of marks,
                   * so we scan forward for more segments only
                   * separated from us by marks. In theory, we should
                   * also merge segments with identical styles, even
                   * if there are toggles in-between
                   */

                  gint bytes = 0;
 		  TextLineSegment *prev_seg = NULL;
  
 		  while (seg)
                    {
                      if (seg->type == text_segment_char)
                        {
                          memcpy (text + layout_byte_offset, seg->body.chars, seg->byte_count);
                          layout_byte_offset += seg->byte_count;
                          buffer_byte_offset += seg->byte_count;
                          bytes += seg->byte_count;
                        }
 		      else if (seg->type == text_segment_right_mark ||
 			       seg->type == text_segment_left_mark)
                        {
 			  /* If we have preedit string, break out of this loop - we'll almost
 			   * certainly have different attributes on the preedit string
 			   */

 			  if (preedit_len > 0 &&
 			      buffer->get_btree()->mark_is_insert (
 							     seg->body.mark.obj))
			    break;

 			  if (seg->body.mark.visible)
 			    {
			      cursor_byte_offsets = g_slist_prepend (cursor_byte_offsets, GINT_TO_POINTER (layout_byte_offset));
			      cursor_segs = g_slist_prepend (cursor_segs, seg);
			      if (buffer->get_btree()->mark_is_insert (
								  seg->body.mark.obj))
				display->insert_index = layout_byte_offset;
			    }
                        }
		      else
			break;

 		      prev_seg = seg;
                      seg = seg->next;
                    }

 		  seg = prev_seg; /* Back up one */
		  /*TODO
                  add_generic_attrs (&style->appearance,
                                     bytes,
                                     attrs, layout_byte_offset - bytes,
                                     size_only, true);
                  add_text_attrs (layout, style, bytes, attrs,
                                  layout_byte_offset - bytes, size_only);
		  */
                }
              /*TODO else if (seg->type == &text_pixbuf_type)
                {
                  add_generic_attrs (
                                     &style->appearance,
                                     seg->byte_count,
                                     attrs, layout_byte_offset,
                                     size_only, false);
                  add_pixbuf_attrs (layout, display, style,
                                    seg, attrs, layout_byte_offset);
		
                  memcpy (text + layout_byte_offset, text_unknown_char_utf8,
                          seg->byte_count);
                  layout_byte_offset += seg->byte_count;
                  buffer_byte_offset += seg->byte_count;
                }
              else if (seg->type == &text_child_type)
                {
                  saw_widget = true;
                  
                  add_generic_attrs (&style->appearance,
                                     seg->byte_count,
                                     attrs, layout_byte_offset,
                                     size_only, false);
                  add_child_attrs (layout, display, style,
                                   seg, attrs, layout_byte_offset);
	
                  memcpy (text + layout_byte_offset, text_unknown_char_utf8,
                          seg->byte_count);
                  layout_byte_offset += seg->byte_count;
                  buffer_byte_offset += seg->byte_count;
                }*/
              else
                {
                  /* We don't know this segment type */
                  g_assert_not_reached ();
                }
              
            } /* if (segment was visible) */
          else
            {
              /* Invisible segment */
              buffer_byte_offset += seg->byte_count;
            }

          release_style (style);
        }

      /* Toggles */
      else if (seg->type == text_segment_toggle_on ||
               seg->type == text_segment_toggle_off)
        {
          /* Style may have changed, drop our
             current cached style */
          invalidate_cached_style ();
	  /* Add the tag only after we have seen some non-toggle non-mark segment,
	   * otherwise the tag is already accounted for by buffer->get_btree()->get_tags(). */
	  if (!initial_toggle_segments)
	    tags = tags_array_toggle_tag (tags, seg->body.toggle.info->tag);
        }

      /* Marks */
      else if (seg->type == text_segment_right_mark ||
               seg->type == text_segment_left_mark)
        {
	  gint cursor_offset = 0;
 	  
	  /* At the insertion point, add the preedit string, if any */
	  
	  if (buffer->get_btree()->mark_is_insert (
					     seg->body.mark.obj))
	    {
	      display->insert_index = layout_byte_offset;
	      
	      if (preedit_len > 0)
		{
		  text_allocated += preedit_len;
		  text = (gchar*)g_realloc (text, text_allocated);

		  style = get_style (tags);
		  //add_preedit_attrs (layout, style, attrs, layout_byte_offset, size_only);
		  release_style (style);
                  
		  memcpy (text + layout_byte_offset, preedit_string, preedit_len);
		  layout_byte_offset += preedit_len;
                  /* DO NOT increment the buffer byte offset for preedit */
                  
		  cursor_offset = preedit_cursor - preedit_len;
		}
	    }
	  

          /* Display visible marks */

          if (seg->body.mark.visible)
            {
              cursor_byte_offsets = g_slist_prepend (cursor_byte_offsets,
                                                     GINT_TO_POINTER (layout_byte_offset + cursor_offset));
              cursor_segs = g_slist_prepend (cursor_segs, seg);
            }
        }

      else
        g_error ("Unknown segment type: %s", seg->name);

      seg = seg->next;
    }
  
  if (!para_values_set)
    {
      style = get_style (tags);
      //TODOset_para_values (layout, base_dir, style, display);
      release_style (style);
    }
  
  /* Pango doesn't want the trailing paragraph delimiters */

  {
    /* Only one character has type G_UNICODE_PARAGRAPH_SEPARATOR in
     * Unicode 3.0; update this if that changes.
     */
#define PARAGRAPH_SEPARATOR 0x2029
    gunichar ch = 0;

    if (layout_byte_offset > 0)
      {
        const char *prev = g_utf8_prev_char (text + layout_byte_offset);
        ch = g_utf8_get_char (prev);
        if (ch == PARAGRAPH_SEPARATOR || ch == '\r' || ch == '\n')
          layout_byte_offset = prev - text; /* chop off */

        if (ch == '\n' && layout_byte_offset > 0)
          {
            /* Possibly chop a CR as well */
            prev = g_utf8_prev_char (text + layout_byte_offset);
            if (*prev == '\r')
              --layout_byte_offset;
          }
      }
  }
  
  //TODOpango_layout_set_text (display->layout, text, layout_byte_offset);
  //pango_layout_set_attributes (display->layout, attrs);

  tmp_list1 = cursor_byte_offsets;
  tmp_list2 = cursor_segs;
  while (tmp_list1)
    {
      /*TODO add_cursor (display, tmp_list2->data,
                  GPOINTER_TO_INT (tmp_list1->data));*/
      tmp_list1 = tmp_list1->next;
      tmp_list2 = tmp_list2->next;
    }
  g_slist_free (cursor_byte_offsets);
  g_slist_free (cursor_segs);

  //TODOpango_layout_get_extents (display->layout, NULL, &extents);

  //TODOdisplay->width = PIXEL_BOUND (extents.width) + display->left_margin + display->right_margin;
  //TODOdisplay->height += PANGO_PIXELS (extents.height);

  /* If we aren't wrapping, we need to do the alignment of each
   * paragraph ourselves.
   */
  /*TODO if (pango_layout_get_width (display->layout) < 0)
    {
      gint excess = display->total_width - display->width;

      switch (pango_layout_get_alignment (display->layout))
	{
	case PANGO_ALIGN_LEFT:
	  break;
	case PANGO_ALIGN_CENTER:
	  display->x_offset += excess / 2;
	  break;
	case PANGO_ALIGN_RIGHT:
	  display->x_offset += excess;
	  break;
	}
    }*/
  
  /* Free this if we aren't in a loop */
  if (wrap_loop_count == 0)
    invalidate_cached_style ();

  g_free (text);
  //pango_attr_list_unref (attrs);
  if (tags != NULL)
    g_ptr_array_free (tags, true);

  one_display_cache = display;

  if (saw_widget)
  {}//allocate_child_widgets (display);
  
  return display;
}

void
TextLayout::free_line_display (
                                   TextLineDisplay *display)
{
  if (display != one_display_cache)
    {
      /*if (display->layout)
        g_object_unref (display->layout);*/

      if (display->cursors)
        {
          g_slist_foreach (display->cursors, (GFunc)g_free, NULL);
          g_slist_free (display->cursors);
        }
      g_slist_free (display->shaped_objects);
      
      if (display->pg_bg_color)
      {}//TODOgdk_color_free (display->pg_bg_color);

      g_free (display);
    }
}

/* Functions to convert iter <=> index for the line of a TextLineDisplay
 * taking into account the preedit string and invisible text if necessary.
 */
gint TextLayout::line_display_iter_to_index (
			    TextLineDisplay *display,
			    TextIter  *iter)
{
  gint index;

  g_return_val_if_fail (iter->get_text_line () == display->line, 0);

  index = iter->get_visible_line_index ();
  
  if (preedit_len > 0 && display->insert_index >= 0)
    {
      if (index >= display->insert_index)
	index += preedit_len;
    }

  return index;
}

void TextLayout::line_display_index_to_iter (
			    TextLineDisplay *display,
			    TextIter        *iter,
			    gint                index,
			    gint                trailing)
{
  g_return_if_fail (!buffer->get_btree()->line_is_last (display->line));
  
  if (preedit_len > 0 && display->insert_index >= 0)
    {
      if (index >= display->insert_index + preedit_len)
	index -= preedit_len;
      else if (index > display->insert_index)
	{
	  index = display->insert_index;
	  trailing = 0;
	}
    }

  buffer->get_btree()->get_iter_at_line (
                                    iter, display->line, 0);

  iter->set_visible_line_index (index);
  
  if (iter->get_text_line () != display->line)
    {
      /* Clamp to end of line - really this clamping should have been done
       * before here, maybe in Pango, this is a broken band-aid I think
       */
      buffer->get_btree()->get_iter_at_line (
                                        iter, display->line, 0);

      if (!iter->ends_line ())
        iter->forward_to_line_end ();
    }
  
  iter->forward_chars (trailing);
}

//TODO rename?
void TextLayout::__get_line_at_y (
               gint           y,
               TextLine  **line,
               gint          *line_top)
{
  if (y < 0)
    y = 0;
  if (y > height)
    y = height;

  *line = buffer->get_btree()->find_line_by_y (
                                         this, y, line_top);
  if (*line == NULL)
    {
      *line = buffer->get_btree()->get_end_iter_line ();
      
      if (line_top)
        *line_top =
          buffer->get_btree()->find_line_top (
                                        *line, this);
    }
}

/**
 * gtk_text_layout_get_line_at_y:
 * @layout: a #GtkLayout
 * @target_iter: the iterator in which the result is stored
 * @y: the y positition
 * @line_top: location to store the y coordinate of the
 *            top of the line. (Can by %NULL.)
 *
 * Get the iter at the beginning of the line which is displayed
 * at the given y.
 **/
void
TextLayout::get_line_at_y (
                               TextIter   *target_iter,
                               gint           y,
                               gint          *line_top)
{
  TextLine *line;

  //g_return_if_fail (GTK_IS_TEXT_LAYOUT (layout));
  g_return_if_fail (target_iter != NULL);

  __get_line_at_y (y, &line, line_top);
  buffer->get_btree()->get_iter_at_line (
                                   target_iter, line, 0);
}

void
TextLayout::get_iter_at_pixel (
                                   TextIter   *target_iter,
                                   gint           x, 
				   gint           y)
{
  gint trailing;

  get_iter_at_position (target_iter, &trailing, x, y);

  target_iter->forward_chars (trailing);  
}

void TextLayout::get_iter_at_position (
					   TextIter       *target_iter,
					   gint              *trailing,
					   gint               x,
					   gint               y)
{
  TextLine *line;
  gint byte_index;
  gint line_top;
  TextLineDisplay *display;

  //g_return_if_fail (GTK_IS_TEXT_LAYOUT (layout));
  g_return_if_fail (target_iter != NULL);

  __get_line_at_y (y, &line, &line_top);

  display = get_line_display (line, false);

  x -= display->x_offset;
  y -= line_top + display->top_margin;

  /* If we are below the layout, position the cursor at the last character
   * of the line.
   */
  if (y > display->height - display->top_margin - display->bottom_margin)
    {
      byte_index = line->byte_count ();
      *trailing = 0;
    }
  else
    {
       /* Ignore the "outside" return value from pango. Pango is doing
        * the right thing even if we are outside the layout in the
        * x-direction.
        */
      //TODOpango_layout_xy_to_index (display->layout, x * PANGO_SCALE, y * PANGO_SCALE,
            //                    &byte_index, trailing);
    }

  line_display_index_to_iter (display, target_iter, byte_index, 0);

  free_line_display (display);
}


/**
 * gtk_text_layout_get_cursor_locations:
 * @layout: a #TextLayout
 * @iter: a #TextIter
 * @strong_pos: location to store the strong cursor position (may be %NULL)
 * @weak_pos: location to store the weak cursor position (may be %NULL)
 *
 * Given an iterator within a text layout, determine the positions of the
 * strong and weak cursors if the insertion point is at that
 * iterator. The position of each cursor is stored as a zero-width
 * rectangle. The strong cursor location is the location where
 * characters of the directionality equal to the base direction of the
 * paragraph are inserted.  The weak cursor location is the location
 * where characters of the directionality opposite to the base
 * direction of the paragraph are inserted.
 **/
void
TextLayout::get_cursor_locations (
                                      TextIter    *iter,
                                      Rect   *strong_pos,
                                      Rect   *weak_pos)
{
  TextLine *line;
  TextLineDisplay *display;
  gint line_top;
  gint index;
  TextIter insert_iter;

  //PangoRectangle pango_strong_pos;
  //PangoRectangle pango_weak_pos;

//  g_return_if_fail (layout != NULL);
  g_return_if_fail (iter != NULL);

  line = iter->get_text_line ();
  display = get_line_display (line, false);
  index = line_display_iter_to_index (display, iter);
  
  line_top = buffer->get_btree()->find_line_top (
                                           line, this);
  
  buffer->get_iter_at_mark (&insert_iter,
                                    buffer->get_insert ());

  if (TextIter::equal (iter, &insert_iter))
    index += preedit_cursor - preedit_len;
  
  /*TODO pango_layout_get_cursor_pos (display->layout, index,
			       strong_pos ? &pango_strong_pos : NULL,
		       weak_pos ? &pango_weak_pos : NULL);*/

  if (strong_pos)
    {
	    /*TODO
      strong_pos->x = display->x_offset + pango_strong_pos.x / PANGO_SCALE;
      strong_pos->y = line_top + display->top_margin + pango_strong_pos.y / PANGO_SCALE;
      strong_pos->width = 0;
      strong_pos->height = pango_strong_pos.height / PANGO_SCALE;
      	*/
    }

  if (weak_pos)
    {
	    /*TODO
      weak_pos->x = display->x_offset + pango_weak_pos.x / PANGO_SCALE;
      weak_pos->y = line_top + display->top_margin + pango_weak_pos.y / PANGO_SCALE;
      weak_pos->width = 0;
      weak_pos->height = pango_weak_pos.height / PANGO_SCALE;
      	*/
    }

  free_line_display (display);
}

/**
 * _gtk_text_layout_get_block_cursor:
 * @layout: a #TextLayout
 * @pos: a #Rect to store block cursor position
 *
 * If layout is to display a block cursor, calculates its position
 * and returns %true. Otherwise it returns %false. In case when
 * cursor is visible, it simply returns the position stored in
 * the line display, otherwise it has to compute the position
 * (see get_block_cursor()).
 **/
bool TextLayout::get_block_cursor ( Rect  *pos)
{
  TextLine *line;
  TextLineDisplay *display;
  TextIter iter;
  Rect rect;
  bool block = false;

//  g_return_val_if_fail (layout != NULL, false);

  buffer->get_iter_at_mark (&iter,
                                    buffer->get_insert ());
  line = iter.get_text_line ();
  display = get_line_display (line, false);

  if (display->has_block_cursor)
    {
      block = true;
      rect = display->block_cursor;
    }
  else
    {
      gint index = display->insert_index;

      if (index < 0)
        index = iter.get_line_index ();

      if (__get_block_cursor (display, &iter, index, &rect, NULL))
	block = true;
    }

  if (block && pos)
    {
      gint line_top;

      line_top = buffer->get_btree()->find_line_top (
						line, this);

      *pos = rect;
      pos->x += display->x_offset;
      pos->y += line_top + display->top_margin;
    }

  free_line_display (display);
  return block;
}

/**
 * gtk_text_layout_get_line_yrange:
 * @layout: a #TextLayout
 * @iter:   a #TextIter
 * @y:      location to store the top of the paragraph in pixels,
 *          or %NULL.
 * @height  location to store the height of the paragraph in pixels,
 *          or %NULL.
 *
 * Find the range of y coordinates for the paragraph containing
 * the given iter.
 **/
void
TextLayout::get_line_yrange ( TextIter *iter,
                                 gint              *y,
                                 gint              *height)
{
  TextLine *line;

  //g_return_if_fail (GTK_IS_TEXT_LAYOUT (layout));
  g_return_if_fail (iter->get_btree() == buffer->get_btree());

  line = iter->get_text_line ();

  if (y)
    *y = buffer->get_btree()->find_line_top ( line, this);
  if (height)
    {
      TextLineData *line_data = line->get_data (this);
      if (line_data)
        *height = line_data->height;
      else
        *height = 0;
    }
}

/**
 * _gtk_text_layout_get_line_xrange:
 * @layout: a #TextLayout
 * @iter:   a #TextIter
 * @x:      location to store the top of the paragraph in pixels,
 *          or %NULL.
 * @width  location to store the height of the paragraph in pixels,
 *          or %NULL.
 *
 * Find the range of X coordinates for the paragraph containing
 * the given iter. Private for 2.0 due to API freeze, could
 * be made public for 2.2.
 **/
void
TextLayout::get_line_xrange (
                                  TextIter *iter,
                                  gint              *x,
                                  gint              *width)
{
  TextLine *line;

  //g_return_if_fail (GTK_IS_TEXT_LAYOUT (layout));
  g_return_if_fail (iter->get_btree () == buffer->get_btree());

  line = iter->get_text_line ();

  if (x)
    *x = 0; /* FIXME This is wrong; should represent the first available cursor position */
  
  if (width)
    {
      TextLineData *line_data = line->get_data (this);
      if (line_data)
        *width = line_data->width;
      else
        *width = 0;
    }
}

void TextLayout::get_iter_location ( TextIter *iter, Rect      *rect)
{
  //PangoRectangle pango_rect;
  TextLine *line;
  TextBTree *tree;
  TextLineDisplay *display;
  gint byte_index;
  gint x_offset;
  
  //g_return_if_fail (GTK_IS_TEXT_LAYOUT (layout));
  g_return_if_fail (iter->get_btree() == buffer->get_btree ());
  g_return_if_fail (rect != NULL);

  tree = iter->get_btree();
  line = iter->get_text_line ();

  display = get_line_display (line, false);

  rect->y = tree->find_line_top (line, this);

  //TODOx_offset = display->x_offset * PANGO_SCALE;

  byte_index = iter->get_line_index ();
  
  //TODOpango_layout_index_to_pos (display->layout, byte_index, &pango_rect);
  
  /*TODO rect->x = PANGO_PIXELS (x_offset + pango_rect.x);
  rect->y += PANGO_PIXELS (pango_rect.y) + display->top_margin;
  rect->width = PANGO_PIXELS (pango_rect.width);
  rect->height = PANGO_PIXELS (pango_rect.height);*/

  free_line_display (display);
}

/* FFIXX */

/* Find the iter for the logical beginning of the first display line whose
 * top y is >= y. If none exists, move the iter to the logical beginning
 * of the last line in the buffer.
 */
void TextLayout::find_display_line_below (
                         TextIter   *iter,
                         gint           y)
{
  TextLine *line, *next;
  TextLine *found_line = NULL;
  gint line_top;
  gint found_byte = 0;

  line = buffer->get_btree()->find_line_by_y (
                                        this, y, &line_top);
  if (!line)
    {
      line =
        buffer->get_btree()->get_end_iter_line ();

      line_top =
        buffer->get_btree()->find_line_top (
                                      line, this);
    }

  while (line && !found_line)
    {
      TextLineDisplay *display = get_line_display (line, false);
      //PangoLayoutIter *layout_iter;

      //TODO layout_iter = pango_layout_get_iter (display->layout);

      line_top += display->top_margin;

      /*TODO do
        {
          gint first_y, last_y;
          //TODO PangoLayoutLine *layout_line = pango_layout_iter_get_line_readonly (layout_iter);

          //TODOfound_byte = layout_line->start_index;
          
          if (line_top >= y)
            {
              found_line = line;
              break;
            }

          //TODO pango_layout_iter_get_line_yrange (layout_iter, &first_y, &last_y);
          //TODO line_top += (last_y - first_y) / PANGO_SCALE;
        }
      while (pango_layout_iter_next_line (layout_iter));*/

      //pango_layout_iter_free (layout_iter);
      
      line_top += display->bottom_margin;
      free_line_display (display);

      next = line->next_excluding_last ();
      if (!next)
        found_line = line;

      line = next;
    }

  buffer->get_btree()->get_iter_at_line (
                                   iter, found_line, found_byte);
}

/* Find the iter for the logical beginning of the last display line whose
 * top y is >= y. If none exists, move the iter to the logical beginning
 * of the first line in the buffer.
 */
void TextLayout::find_display_line_above ( TextIter   *iter, gint           y)
{
  TextLine *line;
  TextLine *found_line = NULL;
  gint line_top;
  gint found_byte = 0;

  line = buffer->get_btree()->find_line_by_y ( this, y, &line_top);
  if (!line)
    {
      line = buffer->get_btree()->get_end_iter_line ();
      
      line_top = buffer->get_btree()->find_line_top (line, this);
    }

  while (line && !found_line)
    {
      TextLineDisplay *display = get_line_display (line, false);
      //TODOPangoRectangle logical_rect;
      //PangoLayoutIter *layout_iter;
      gint tmp_top;

      //layout_iter = pango_layout_get_iter (display->layout);
      
      line_top -= display->top_margin + display->bottom_margin;
      //TODOpango_layout_iter_get_layout_extents (layout_iter, NULL, &logical_rect);
      //TODOline_top -= logical_rect.height / PANGO_SCALE;

      tmp_top = line_top + display->top_margin;

      /*TODO do
        {
          gint first_y, last_y;
          //TODOPangoLayoutLine *layout_line = pango_layout_iter_get_line_readonly (layout_iter);

          //found_byte = layout_line->start_index;

          //pango_layout_iter_get_line_yrange (layout_iter, &first_y, &last_y);
          
          //tmp_top -= (last_y - first_y) / PANGO_SCALE;

          if (tmp_top < y)
            {
              found_line = line;
	      //pango_layout_iter_free (layout_iter);
              goto done;
            }
        }
      while (pango_layout_iter_next_line (layout_iter));*/

      //pango_layout_iter_free (layout_iter);
      
      free_line_display (display);

      line = line->previous_line ();
    }

 done:
  
  if (found_line)
    buffer->get_btree()->get_iter_at_line (
                                     iter, found_line, found_byte);
  else
    buffer->get_iter_at_offset (iter, 0);
}

/**
 * gtk_text_layout_clamp_iter_to_vrange:
 * @layout: a #TextLayout
 * @iter:   a #TextIter
 * @top:    the top of the range
 * @bottom: the bottom the range
 *
 * If the iterator is not fully in the range @top <= y < @bottom,
 * then, if possible, move it the minimum distance so that the
 * iterator in this range.
 *
 * Returns: %true if the iterator was moved, otherwise %false.
 **/
bool
TextLayout::clamp_iter_to_vrange (
                                      TextIter   *iter,
                                      gint           top,
                                      gint           bottom)
{
  Rect iter_rect;

  get_iter_location (iter, &iter_rect);

  /* If the iter is at least partially above the range, put the iter
   * at the first fully visible line after the range.
   */
  if (iter_rect.y < top)
    {
      find_display_line_below (iter, top);

      return true;
    }
  /* Otherwise, if the iter is at least partially below the screen, put the
   * iter on the last logical position of the last completely visible
   * line on screen
   */
  else if (iter_rect.y + iter_rect.height > bottom)
    {
      find_display_line_above (iter, bottom);

      return true;
    }
  else
    return false;
}

/**
 * gtk_text_layout_move_iter_to_previous_line:
 * @layout: a #GtkLayout
 * @iter:   a #TextIter
 *
 * Move the iterator to the beginning of the previous line. The lines
 * of a wrapped paragraph are treated as distinct for this operation.
 **/
bool
TextLayout::move_iter_to_previous_line (
                                            TextIter   *iter)
{
  TextLine *line;
  TextLineDisplay *display;
  gint line_byte;
  //GSList *tmp_list;
  //PangoLayoutLine *layout_line;
  TextIter orig;
  bool update_byte = false;
  
//  g_return_val_if_fail (GTK_IS_TEXT_LAYOUT (layout), false);
  g_return_val_if_fail (iter != NULL, false);

  orig = *iter;


  line = iter->get_text_line ();
  display = get_line_display (line, false);
  line_byte = line_display_iter_to_index (display, iter);

  /* If display->height == 0 then the line is invisible, so don't
   * move onto it.
   */
  while (display->height == 0)
    {
      TextLine *prev_line;

      prev_line = line->previous_line ();

      if (prev_line == NULL)
        {
          line_display_index_to_iter (display, iter, 0, 0);
          goto out;
        }

      free_line_display (display);

      line = prev_line;
      display = get_line_display (prev_line, false);
      update_byte = true;
    }
  
  //TODOtmp_list = pango_layout_get_lines_readonly (display->layout);
  //layout_line = tmp_list->data;

  if (update_byte)
    {
      //line_byte = layout_line->start_index + layout_line->length;
    }

  /*TODO if (line_byte < layout_line->length || !tmp_list->next) * first line of paragraph *
    {
      TextLine *prev_line;

      prev_line = line->previous_line ();

      * first line of the whole buffer, do not move the iter and return false *
      if (prev_line == NULL)
        goto out;

      while (prev_line)
        {
          free_line_display (display);

          display = get_line_display (prev_line, false);

          if (display->height > 0)
            {
              //tmp_list = g_slist_last (pango_layout_get_lines_readonly (display->layout));
              layout_line = tmp_list->data;

              line_display_index_to_iter (display, iter,
                                          layout_line->start_index + layout_line->length, 0);
              break;
            }

          prev_line = prev_line->previous_line ();
        }
    }
  else
    {
      gint prev_offset = layout_line->start_index;

      tmp_list = tmp_list->next;
      while (tmp_list)
        {
          layout_line = tmp_list->data;

          if (line_byte < layout_line->start_index + layout_line->length ||
              !tmp_list->next)
            {
 	      line_display_index_to_iter (display, iter, prev_offset, 0);
              break;
            }

          prev_offset = layout_line->start_index;
          tmp_list = tmp_list->next;
        }
    }*/

 out:
  
  free_line_display (display);

  return
    !TextIter::equal (iter, &orig) &&
    !iter->is_end();
}

/**
 * gtk_text_layout_move_iter_to_next_line:
 * @layout: a #GtkLayout
 * @iter:   a #TextIter
 *
 * Move the iterator to the beginning of the next line. The
 * lines of a wrapped paragraph are treated as distinct for
 * this operation.
 **/
bool
TextLayout::move_iter_to_next_line ( TextIter   *iter)
{
  TextLine *line;
  TextLineDisplay *display;
  gint line_byte;
  TextIter orig;
  bool found = false;
  bool found_after = false;
  bool first = true;

//  g_return_val_if_fail (GTK_IS_TEXT_LAYOUT (layout), false);
  g_return_val_if_fail (iter != NULL, false);

  orig = *iter;
  
  line = iter->get_text_line ();

  while (line && !found_after)
    {
      GSList *tmp_list;

      display = get_line_display (line, false);

      if (display->height == 0)
        goto next;
      
      if (first)
	{
	  line_byte = line_display_iter_to_index (display, iter);
	  first = false;
	}
      else
	line_byte = 0;
	
      //tmp_list = pango_layout_get_lines_readonly (display->layout);
      while (tmp_list && !found_after)
        {
          //PangoLayoutLine *layout_line = tmp_list->data;

          if (found)
            {
	      //line_display_index_to_iter (display, iter,
              //                            layout_line->start_index, 0);
              found_after = true;
            }
          //TODO else if (line_byte < layout_line->start_index + layout_line->length || !tmp_list->next)
            //found = true;
          
          tmp_list = tmp_list->next;
        }

    next:
      
      free_line_display (display);

      line = line->next_excluding_last ();
    }

  if (!found_after)
    buffer->get_end_iter (iter);
  
  return
    !TextIter::equal (iter, &orig) &&
    !iter->is_end();
}

/**
 * gtk_text_layout_move_iter_to_line_end:
 * @layout: a #TextLayout
 * @direction: if negative, move to beginning of line, otherwise
               move to end of line.
 *
 * Move to the beginning or end of a display line.
 **/
bool TextLayout::move_iter_to_line_end ( TextIter   *iter, gint           direction)
{
  TextLine *line;
  TextLineDisplay *display;
  gint line_byte;
//  GSList *tmp_list;
  TextIter orig;
  
//  g_return_val_if_fail (GTK_IS_TEXT_LAYOUT (layout), false);
  g_return_val_if_fail (iter != NULL, false);

  orig = *iter;
  
  line = iter->get_text_line ();
  display = get_line_display (line, false);
  line_byte = line_display_iter_to_index (display, iter);

//TODO  tmp_list = pango_layout_get_lines_readonly (display->layout);
  /*while (tmp_list)
    {
      PangoLayoutLine *layout_line = tmp_list->data;

      if (line_byte < layout_line->start_index + layout_line->length || !tmp_list->next)
        {
 	  line_display_index_to_iter (layout, display, iter,
 				      direction < 0 ? layout_line->start_index : layout_line->start_index + layout_line->length,
 				      0);

          * FIXME: As a bad hack, we move back one position when we
	   * are inside a paragraph to avoid going to next line on a
	   * forced break not at whitespace. Real fix is to keep track
	   * of whether marks are at leading or trailing edge?  *
          if (direction > 0 && layout_line->length > 0 && 
	      !gtk_text_iter_ends_line (iter) && 
	      !buffer->get_btree()->char_is_invisible (iter))
            gtk_text_iter_backward_char (iter);
          break;
        }
      
      tmp_list = tmp_list->next;
    }*/

  free_line_display (display);

  return
    !TextIter::equal (iter, &orig) &&
    !iter->is_end();
}


/**
 * gtk_text_layout_iter_starts_line:
 * @layout: a #TextLayout
 * @iter: iterator to test
 *
 * Tests whether an iterator is at the start of a display line.
 **/
bool TextLayout::iter_starts_line (TextIter   *iter)
{
  TextLine *line;
  TextLineDisplay *display;
  gint line_byte;
//  GSList *tmp_list;
  
//  g_return_val_if_fail (GTK_IS_TEXT_LAYOUT (layout), false);
  g_return_val_if_fail (iter != NULL, false);

  line = iter->get_text_line ();
  display = get_line_display (line, false);
  line_byte = line_display_iter_to_index (display, iter);

  //tmp_list = pango_layout_get_lines_readonly (display->layout);
  /*TODO while (tmp_list)
    {
      PangoLayoutLine *layout_line = tmp_list->data;

      if (line_byte < layout_line->start_index + layout_line->length ||
          !tmp_list->next)
        {
          * We're located on this line or the para delimiters before
           * it
           *
          free_line_display (display);
          
          if (line_byte == layout_line->start_index)
            return true;
          else
            return false;
        }
      
      tmp_list = tmp_list->next;
    }*/

  g_assert_not_reached ();
  return false;
}

void
TextLayout::get_iter_at_line (
                                  TextIter    *iter,
                                  TextLine    *line,
                                  gint            byte_offset)
{
  buffer->get_btree()->get_iter_at_line (
                                    iter, line, byte_offset);
}


/**
 * gtk_text_layout_move_iter_to_x:
 * @layout: a #TextLayout
 * @iter:   a #TextIter
 * @x:      X coordinate
 *
 * Keeping the iterator on the same line of the layout, move it to the
 * specified X coordinate. The lines of a wrapped paragraph are
 * treated as distinct for this operation.
 **/
void
TextLayout::move_iter_to_x (
                                TextIter   *iter,
                                gint           x)
{
  TextLine *line;
  TextLineDisplay *display;
  gint line_byte;
  //TODOPangoLayoutIter *layout_iter;
  
  //g_return_if_fail (GTK_IS_TEXT_LAYOUT (layout));
  g_return_if_fail (iter != NULL);

  line = iter->get_text_line ();

  display = get_line_display (line, false);
  line_byte = line_display_iter_to_index (display, iter);

  //layout_iter = pango_layout_get_iter (display->layout);

  /*TODO do
    {
      //PangoLayoutLine *layout_line = pango_layout_iter_get_line_readonly (layout_iter);

      if (line_byte < layout_line->start_index + layout_line->length ||
          pango_layout_iter_at_last_line (layout_iter))
        {
          PangoRectangle logical_rect;
          gint byte_index, trailing;
          gint x_offset = display->x_offset * PANGO_SCALE;

          pango_layout_iter_get_line_extents (layout_iter, NULL, &logical_rect);

          pango_layout_line_x_to_index (layout_line,
                                        x * PANGO_SCALE - x_offset - logical_rect.x,
                                        &byte_index, &trailing);

 	  line_display_index_to_iter (layout, display, iter, byte_index, trailing);

          break;
        }
    }
  while (pango_layout_iter_next_line (layout_iter));*/

  //pango_layout_iter_free (layout_iter);
  
  free_line_display (display);
}

/**
 * gtk_text_layout_move_iter_visually:
 * @layout:  a #TextLayout
 * @iter:    a #TextIter
 * @count:   number of characters to move (negative moves left, positive moves right)
 *
 * Move the iterator a given number of characters visually, treating
 * it as the strong cursor position. If @count is positive, then the
 * new strong cursor position will be @count positions to the right of
 * the old cursor position. If @count is negative then the new strong
 * cursor position will be @count positions to the left of the old
 * cursor position.
 *
 * In the presence of bidirection text, the correspondence
 * between logical and visual order will depend on the direction
 * of the current run, and there may be jumps when the cursor
 * is moved off of the end of a run.
 **/

bool
TextLayout::move_iter_visually (
                                    TextIter   *iter,
                                    gint           count)
{
  TextLineDisplay *display = NULL;
  TextIter orig;
  TextIter lineiter;
  
//  g_return_val_if_fail (layout != NULL, false);
  g_return_val_if_fail (iter != NULL, false);

  orig = *iter;
  
  while (count != 0)
    {
      TextLine *line = iter->get_text_line ();
      gint line_byte;
      gint extra_back = 0;
      bool strong;

      int byte_count = line->byte_count ();

      int new_index;
      int new_trailing;

      if (!display)
	display = get_line_display (line, false);

      if (cursor_direction == GTK_TEXT_DIR_NONE)
	strong = true;
      else
	strong = display->direction == cursor_direction;

      line_byte = line_display_iter_to_index (display, iter);

      if (count > 0)
        {
          //pango_layout_move_cursor_visually (display->layout, strong, line_byte, 0, 1, &new_index, &new_trailing);
          count--;
        }
      else
        {
          //pango_layout_move_cursor_visually (display->layout, strong, line_byte, 0, -1, &new_index, &new_trailing);
          count++;
        }

      /* We need to handle the preedit string specially. Well, we don't really need to
       * handle it specially, since hopefully calling gtk_im_context_reset() will
       * remove the preedit string; but if we start off in front of the preedit
       * string (logically) and end up in or on the back edge of the preedit string,
       * we should move the iter one place farther.
       */
      if (preedit_len > 0 && display->insert_index >= 0)
	{
	  if (line_byte == display->insert_index + preedit_len &&
	      new_index < display->insert_index + preedit_len)
	    {
	      line_byte = display->insert_index;
	      extra_back = 1;
	    }
	}
      
      if (new_index < 0 || (new_index == 0 && extra_back))
        {
          do
            {
              line = line->previous_line ();

              if (!line)
                goto done;
              
              buffer->get_btree()->get_iter_at_line (
                                                &lineiter, line, 0);
            }
          while (totally_invisible_line (line, &lineiter));
          
 	  free_line_display (display);
 	  display = get_line_display (line, false);
          lineiter.forward_to_line_end ();
          new_index = lineiter.get_visible_line_index ();
        }
      else if (new_index > byte_count)
        {
          do
            {
              line = line->next_excluding_last ();
              if (!line)
                goto done;

              buffer->get_btree()->get_iter_at_line (
                                                &lineiter, line, 0);
            }
          while (totally_invisible_line (line, &lineiter));
  
 	  free_line_display (display);
 	  display = get_line_display (line, false);
          new_index = 0;
        }
      
       line_display_index_to_iter (display, iter, new_index, new_trailing);
       if (extra_back)
	 iter->backward_char ();
    }

  free_line_display (display);

 done:
  
  return
    !TextIter::equal (iter, &orig) &&
    !iter->is_end();
}

void
TextLayout::spew (void)
{
#if 0
  TextDisplayLine *iter;
  guint wrapped = 0;
  guint paragraphs = 0;
  TextLine *last_line = NULL;

  iter = layout->line_list;
  while (iter != NULL)
    {
      if (iter->line != last_line)
        {
          printf ("%5u  paragraph (%p)\n", paragraphs, iter->line);
          ++paragraphs;
          last_line = iter->line;
        }

      printf ("  %5u  y: %d len: %d start: %d bytes: %d\n",
              wrapped, iter->y, iter->length, iter->byte_offset,
              iter->byte_count);

      ++wrapped;
      iter = iter->next;
    }

  printf ("Layout %s recompute\n",
          layout->need_recompute ? "needs" : "doesn't need");

  printf ("Layout pars: %u lines: %u size: %d x %d Screen width: %d\n",
          paragraphs, wrapped, layout->width,
          layout->height, layout->screen_width);
#endif
}

/* Catch all situations that move the insertion point.
 */
void
TextLayout::mark_set_handler (    TextBuffer *buffer,
                                  const TextIter *location,
                                  TextMark       *mark,
                                  gpointer           data)
{
  //TextLayout *layout = GTK_TEXT_LAYOUT (data);
  TextLayout *layout = (TextLayout*)data; //TODO change func signature? (also next two

  if (mark == buffer->get_insert ())
    layout->update_cursor_line ();
}

void
TextLayout::buffer_insert_text (    TextBuffer *buffer,
				    TextIter   *iter,
				    gchar         *str,
				    gint           len,
				    gpointer       data)
{
  TextLayout *layout = (TextLayout*)data;

    layout->update_cursor_line ();
}

void
TextLayout::buffer_delete_range (    TextBuffer *buffer,
				     TextIter   *start,
				     TextIter   *end,
				     gpointer       data)
{
  TextLayout *layout = (TextLayout*)data;

  layout->update_cursor_line ();
}
