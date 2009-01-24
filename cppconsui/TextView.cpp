/* -*- Mode: C; c-file-style: "gnu"; tab-width: 8 -*- */
/* GTK - The GIMP Toolkit
 * gtktextview.c Copyright (C) 2000 Red Hat, Inc.
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

#include "CppConsUI.h"
#include "TextView.h"

#include "Keys.h"

//#include "config.h"
#include <string.h>

#include <glib.h>

//#define GTK_TEXT_USE_INTERNAL_UNSUPPORTED_API
//#include "gtkbindings.h"
//#include "gtkdnd.h"
//#include "gtkimagemenuitem.h"
//#include "gtkintl.h"
//#include "gtkmain.h"
//#include "gtkmarshalers.h"
//#include "gtkmenu.h"
//#include "gtkmenuitem.h"
//#include "gtkseparatormenuitem.h"
//#include "gtksettings.h"
//#include "gtkstock.h"
#include "TextBuffer.h"
//#include "gtktextdisplay.h"
//#include "Widget.h"
#include "TextView.h"
//#include "gtkimmulticontext.h"
//#include "gdk/gdkkeysyms.h"
//#include "gtkprivate.h"
//#include "gtktextutil.h"
//#include "gtkwindow.h"
//#include "gtkalias.h"

/* How scrolling, validation, exposes, etc. work.
 *
 * The expose_event handler has the invariant that the onscreen lines
 * have been validated.
 *
 * There are two ways that onscreen lines can become invalid. The first
 * is to change which lines are onscreen. This happens when the value
 * of a scroll adjustment changes. So the code path begins in
 * gtk_text_view_value_changed() and goes like this:
 *   - gdk_window_scroll() to reflect the new adjustment value
 *   - validate the lines that were moved onscreen
 *   - gdk_window_process_updates() to handle the exposes immediately
 *
 * The second way is that you get the "invalidated" signal from the layout,
 * indicating that lines have become invalid. This code path begins in
 * invalidated_handler() and goes like this:
 *   - install high-priority idle which does the rest of the steps
 *   - if a scroll is pending from scroll_to_mark(), do the scroll,
 *     jumping to the gtk_text_view_value_changed() code path
 *   - otherwise, validate the onscreen lines
 *   - DO NOT process updates
 *
 * In both cases, validating the onscreen lines can trigger a scroll
 * due to maintaining the first_para on the top of the screen.
 * If validation triggers a scroll, we jump to the top of the code path
 * for value_changed, and bail out of the current code path.
 *
 * Also, in size_allocate, if we invalidate some lines from changing
 * the layout width, we need to go ahead and run the high-priority idle,
 * because GTK sends exposes right after doing the size allocates without
 * returning to the main loop. This is also why the high-priority idle
 * is at a higher priority than resizing.
 *
 */

#if 0
#define DEBUG_VALIDATION_AND_SCROLLING
#endif

//#ifdef DEBUG_VALIDATION_AND_SCROLLING
//#define DV(x) (x)
//#else
//#define DV(x)
//#endif

//#define SCREEN_WIDTH(widget) text_window_get_width (GTK_TEXT_VIEW (widget)->text_window)
//#define SCREEN_HEIGHT(widget) text_window_get_height (GTK_TEXT_VIEW (widget)->text_window)

#define SPACE_FOR_CURSOR 1


//#define GTK_TEXT_VIEW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GTK_TYPE_TEXT_VIEW, TextViewPrivate))


//TODO get rid of these
/*typedef enum
{
  SET_SCROLL_ADJUSTMENTS,
  POPULATE_POPUP,
  MOVE_CURSOR,
  PAGE_HORIZONTALLY,
  SET_ANCHOR,
  INSERT_AT_CURSOR,
  DELETE_FROM_CURSOR,
  BACKSPACE,
  CUT_CLIPBOARD,
  COPY_CLIPBOARD,
  PASTE_CLIPBOARD,
  TOGGLE_OVERWRITE,
  MOVE_VIEWPORT,
  SELECT_ALL,
  TOGGLE_CURSOR_VISIBLE,
  LAST_SIGNAL
} signal_thingies;*/

/*typedef enum
{
  PROP_0,
  PROP_PIXELS_ABOVE_LINES,
  PROP_PIXELS_BELOW_LINES,
  PROP_PIXELS_INSIDE_WRAP,
  PROP_EDITABLE,
  PROP_WRAP_MODE,
  PROP_JUSTIFICATION,
  PROP_LEFT_MARGIN,
  PROP_RIGHT_MARGIN,
  PROP_INDENT,
  PROP_TABS,
  PROP_CURSOR_VISIBLE,
  PROP_BUFFER,
  PROP_OVERWRITE,
  PROP_ACCEPTS_TAB
} prop_thingies;*/




//static guint signals[LAST_SIGNAL] = { 0 };

//G_DEFINE_TYPE (TextView, gtk_text_view, GTK_TYPE_CONTAINER)

/*
static void
TextView::add_move_binding (
                  guint           keyval,
                  guint           modmask,
                  CursorMovement step,
                  gint            count)
{
  g_assert ((modmask & GDK_SHIFT_MASK) == 0);

  gtk_binding_entry_add_signal (binding_set, keyval, modmask,
                                "move-cursor", 3,
                                G_TYPE_ENUM, step,
                                G_TYPE_INT, count,
                                G_TYPE_BOOLEAN, false);

  * Selection-extending version *
  gtk_binding_entry_add_signal (binding_set, keyval, modmask | GDK_SHIFT_MASK,
                                "move-cursor", 3,
                                G_TYPE_ENUM, step,
                                G_TYPE_INT, count,
                                G_TYPE_BOOLEAN, true);
}*/

void TextView::init (void)
{
  /*
   * Signals
   */

	//TODO implement set anchor bindable
	/*
  signals[SET_ANCHOR] =
    g_signal_new (I_("set-anchor"),
		  G_OBJECT_CLASS_TYPE (gobject_class),
		  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (TextViewClass, set_anchor),
		  NULL, NULL,
		  _gtk_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);
*/

   /* TODO implement cim clipboard stuff?
  signals[CUT_CLIPBOARD] =
    g_signal_new (I_("cut-clipboard"),
		  G_OBJECT_CLASS_TYPE (gobject_class),
		  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (TextViewClass, cut_clipboard),
		  NULL, NULL,
		  _gtk_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);
   */

  /**
   * TextView::copy-clipboard:
   * @text_view: the object which received the signal
   *
   * The ::copy-clipboard signal is a 
   * <link linkend="keybinding-signals">keybinding signal</link> 
   * which gets emitted to copy the selection to the clipboard.
   * 
   * The default bindings for this signal are
   * Ctrl-c and Ctrl-Insert.
  signals[COPY_CLIPBOARD] =
    g_signal_new (I_("copy-clipboard"),
		  G_OBJECT_CLASS_TYPE (gobject_class),
		  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (TextViewClass, copy_clipboard),
		  NULL, NULL,
		  _gtk_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);
   */

  /**
   * TextView::paste-clipboard:
   * @text_view: the object which received the signal
   *
   * The ::paste-clipboard signal is a 
   * <link linkend="keybinding-signals">keybinding signal</link> 
   * which gets emitted to paste the contents of the clipboard 
   * into the text view.
   * 
   * The default bindings for this signal are
   * Ctrl-v and Shift-Insert.
  signals[PASTE_CLIPBOARD] =
    g_signal_new (I_("paste-clipboard"),
		  G_OBJECT_CLASS_TYPE (gobject_class),
		  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (TextViewClass, paste_clipboard),
		  NULL, NULL,
		  _gtk_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);
   */

  /**
   * TextView::toggle-cursor-visible:
   * @text_view: the object which received the signal
   *
   * The ::toggle-cursor-visible signal is a 
   * <link linkend="keybinding-signals">keybinding signal</link> 
   * which gets emitted to toggle the visibility of the cursor.
   *
   * The default binding for this signal is F7.
   *
   * TODO implement for cim as well? or let cursor visibility depend on editablility of buffer?
  signals[TOGGLE_CURSOR_VISIBLE] =
    g_signal_new_class_handler (I_("toggle-cursor-visible"),
                                G_OBJECT_CLASS_TYPE (object_class),
                                G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                G_CALLBACK (gtk_text_view_toggle_cursor_visible),
                                NULL, NULL,
                                _gtk_marshal_VOID__VOID,
                                G_TYPE_NONE, 0);
   */ 

  /*
   * Key bindings
   */

	DeclareBindables();
	BindActions();


  /* Set up default style */
  wrap_mode = WRAP_NONE;
  pixels_above_lines = 0;
  pixels_below_lines = 0;
  pixels_inside_wrap = 0;
  //justify = GTK_JUSTIFY_LEFT;
  left_margin = 0;
  right_margin = 0;
  indent = 0;
  //tabs = NULL;
  editable = true;

  /*gtk_drag_dest_set (widget, 0, NULL, 0,
                     GDK_ACTION_COPY | GDK_ACTION_MOVE);*/

  //target_list = gtk_target_list_new (NULL, 0);
  //gtk_drag_dest_set_target_list (widget, target_list);
  //gtk_target_list_unref (target_list);

  virtual_cursor_x = -1;
  virtual_cursor_y = -1;

  /* This object is completely private. No external entity can gain a reference
   * to it; so we create it here and destroy it in finalize ().
   */
  //text_view->im_context = gtk_im_multicontext_new ();

  //TODO
  /*g_signal_connect (text_view->im_context, "commit",
                    G_CALLBACK (gtk_text_view_commit_handler), text_view);
  g_signal_connect (text_view->im_context, "preedit-changed",
 		    G_CALLBACK (gtk_text_view_preedit_changed_handler), text_view);
  g_signal_connect (text_view->im_context, "retrieve-surrounding",
 		    G_CALLBACK (gtk_text_view_retrieve_surrounding_handler), text_view);
  g_signal_connect (text_view->im_context, "delete-surrounding",
 		    G_CALLBACK (gtk_text_view_delete_surrounding_handler), text_view);*/

  cursor_visible = true;

  accepts_tab = true;

  //text_view->text_window = text_window_new (GTK_TEXT_WINDOW_TEXT,
  //                                          widget, 200, 200);

  //drag_start_x = -1;
  //drag_start_y = -1;

  pending_place_cursor_button = 0;

  /* We handle all our own redrawing */
  //gtk_widget_set_redraw_on_allocate (widget, false);
}

TextView::TextView(Widget& parent, int x, int y, int w, int h)
: Widget(parent, x, y, w, h)
{
  init();

  buffer = NULL;
}


/**
 * gtk_text_view_new_with_buffer:
 * @buffer: a #GtkTextBuffer
 *
 * Creates a new #TextView widget displaying the buffer
 * @buffer. One buffer can be shared among many widgets.
 * @buffer may be %NULL to create a default buffer, in which case
 * this function is equivalent to gtk_text_view_new(). The
 * text view adds its own reference count to the buffer; it does not
 * take over an existing reference.
 *
 * Return value: a new #TextView.
 **/

TextView::TextView(Widget& parent, int x, int y, int w, int h, TextBuffer *buffer)
: Widget(parent, x, y, w, h)
{
  init();

  set_buffer (buffer);

}

/**
 * gtk_text_view_set_buffer:
 * @text_view: a #TextView
 * @buffer: a #GtkTextBuffer
 *
 * Sets @buffer as the buffer being displayed by @text_view. The previous
 * buffer displayed by the text view is unreferenced, and a reference is
 * added to @buffer. If you owned a reference to @buffer before passing it
 * to this function, you must remove that reference yourself; #TextView
 * will not "adopt" it.
 **/
void
TextView::set_buffer ( TextBuffer *buffer)
{
  ////g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))
  //g_return_if_fail (buffer == NULL || GTK_IS_TEXT_BUFFER (buffer));

  if (this->buffer == buffer)
    return;

  if (this->buffer != NULL)
    {
      /* Destroy all anchored children */
      /*GSList *tmp_list;
      GSList *copy;

      copy = g_slist_copy (text_view->children);
      tmp_list = copy;
      while (tmp_list != NULL)
        {
          TextViewChild *vc = tmp_list->data;

          if (vc->anchor)
            {
              gtk_widget_destroy (vc->widget);
              * vc may now be invalid! *
            }

          tmp_list = g_slist_next (tmp_list);
        }
      */

      //g_slist_free (copy);

	    /*TODO
      g_signal_handlers_disconnect_by_func (text_view->buffer,
					    gtk_text_view_mark_set_handler,
					    text_view);
      g_signal_handlers_disconnect_by_func (text_view->buffer,
                                            gtk_text_view_target_list_notify,
                                            text_view);*/

      dnd_mark = NULL;
      first_para_mark = NULL;

      /*if (GTK_WIDGET_REALIZED (text_view))
	{
	  GtkClipboard *clipboard = gtk_widget_get_clipboard (GTK_WIDGET (text_view),
							      GDK_SELECTION_PRIMARY);
	  gtk_text_buffer_remove_selection_clipboard (text_view->buffer, clipboard);
	}*/
    }

  this->buffer = buffer;

  if (buffer != NULL)
    {
      TextIter start;

      //g_object_ref (buffer);

      if (layout)
        layout->set_buffer (buffer);

      buffer->get_iter_at_offset (&start, 0);

      dnd_mark = buffer->create_mark (
                                                         "cim_drag_target",
                                                         &start, false);

      first_para_mark = buffer->create_mark (
                                                                NULL,
                                                                &start, true);

      first_para_pixels = 0;

      /*TODO
      g_signal_connect (text_view->buffer, "mark-set",
			G_CALLBACK (gtk_text_view_mark_set_handler),
                        text_view);
      g_signal_connect (text_view->buffer, "notify::paste-target-list",
			G_CALLBACK (gtk_text_view_target_list_notify),
                        text_view);*/

      target_list_notify ();

      /*if (GTK_WIDGET_REALIZED (text_view))
	{
	  GtkClipboard *clipboard = gtk_widget_get_clipboard (GTK_WIDGET (text_view),
							      GDK_SELECTION_PRIMARY);
	  gtk_text_buffer_add_selection_clipboard (text_view->buffer, clipboard);
	}*/
    }

  //g_object_notify (G_OBJECT (text_view), "buffer");
  
  //if (GTK_WIDGET_VISIBLE (text_view))
  //  gtk_widget_queue_draw (GTK_WIDGET (text_view));

  //DV(g_print ("Invalidating due to set_buffer\n"));
  //TODO request redraw here?
  invalidate ();
}

/**
 * gtk_text_view_get_buffer:
 * @text_view: a #TextView
 *
 * Returns the #GtkTextBuffer being displayed by this text view.
 * The reference count on the buffer is not incremented; the caller
 * of this function won't own a new reference.
 *
 * Return value: a #GtkTextBuffer
 **/
TextBuffer*
TextView::get_buffer (void)
{
  if (buffer == NULL)
    {
      TextBuffer *b;
      b = new TextBuffer (NULL);
      set_buffer (b);
      //TODO g_object_unref (b);
    }

  return buffer;
}


/**
 * gtk_text_view_get_iter_at_location:
 * @text_view: a #TextView
 * @iter: a #TextIter
 * @x: x position, in buffer coordinates
 * @y: y position, in buffer coordinates
 *
 * Retrieves the iterator at buffer coordinates @x and @y. Buffer
 * coordinates are coordinates for the entire buffer, not just the
 * currently-displayed portion.  If you have coordinates from an
 * event, you have to convert those to buffer coordinates with
 * gtk_text_view_window_to_buffer_coords().
 **/
void
TextView::get_iter_at_location (
                                    TextIter *iter,
                                    gint         x,
                                    gint         y)
{
  ////g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))
  g_return_if_fail (iter != NULL);

  ensure_layout ();
  
  layout->get_iter_at_pixel ( iter, x, y);
}

/**
 * gtk_text_view_get_iter_at_position:
 * @text_view: a #TextView
 * @iter: a #TextIter
 * @trailing: location to store an integer indicating where
 *    in the grapheme the user clicked. It will either be
 *    zero, or the number of characters in the grapheme. 
 *    0 represents the trailing edge of the grapheme.
 * @x: x position, in buffer coordinates
 * @y: y position, in buffer coordinates
 *
 * Retrieves the iterator pointing to the character at buffer 
 * coordinates @x and @y. Buffer coordinates are coordinates for 
 * the entire buffer, not just the currently-displayed portion.  
 * If you have coordinates from an event, you have to convert 
 * those to buffer coordinates with 
 * gtk_text_view_window_to_buffer_coords().
 *
 * Note that this is different from gtk_text_view_get_iter_at_location(),
 * which returns cursor locations, i.e. positions <emphasis>between</emphasis>
 * characters.
 *
 * Since: 2.6
 **/
void
TextView::get_iter_at_position (
				    TextIter *iter,
				    gint        *trailing,
				    gint         x,
				    gint         y)
{
  ////g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))
  g_return_if_fail (iter != NULL);

  ensure_layout ();
  
  layout->get_iter_at_position ( iter, trailing, x, y);
}

/**
 * gtk_text_view_get_iter_location:
 * @text_view: a #TextView
 * @iter: a #TextIter
 * @location: bounds of the character at @iter
 *
 * Gets a rectangle which roughly contains the character at @iter.
 * The rectangle position is in buffer coordinates; use
 * gtk_text_view_buffer_to_window_coords() to convert these
 * coordinates to coordinates for one of the windows in the text view.
 **/
void
TextView::get_iter_location (
                                 TextIter *iter,
                                 Rect      *location)
{
  ////g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))
  g_return_if_fail (iter->get_buffer () == get_buffer ());

  ensure_layout ();
  
  layout->get_iter_location (iter, location);
}

/**
 * gtk_text_view_get_line_yrange:
 * @text_view: a #TextView
 * @iter: a #TextIter
 * @y: return location for a y coordinate
 * @height: return location for a height
 *
 * Gets the y coordinate of the top of the line containing @iter,
 * and the height of the line. The coordinate is a buffer coordinate;
 * convert to window coordinates with gtk_text_view_buffer_to_window_coords().
 **/
void
TextView::get_line_yrange (
                               TextIter *iter,
                               gint              *y,
                               gint              *height)
{
  ////g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))
  g_return_if_fail (iter->get_buffer () == get_buffer ());

  ensure_layout ();
  
  layout->get_line_yrange (
                                   iter,
                                   y,
                                   height);
}

/**
 * gtk_text_view_get_line_at_y:
 * @text_view: a #TextView
 * @target_iter: a #TextIter
 * @y: a y coordinate
 * @line_top: return location for top coordinate of the line
 *
 * Gets the #TextIter at the start of the line containing
 * the coordinate @y. @y is in buffer coordinates, convert from
 * window coordinates with gtk_text_view_window_to_buffer_coords().
 * If non-%NULL, @line_top will be filled with the coordinate of the top
 * edge of the line.
 **/
void
TextView::get_line_at_y (
                             TextIter *target_iter,
                             gint         y,
                             gint        *line_top)
{
  ////g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))

  ensure_layout ();
  
  layout->get_line_at_y (
                                 target_iter,
                                 y,
                                 line_top);
}

bool TextView::set_adjustment_clamped (Adjustment *adj, gdouble val)
{
  //DV (g_print ("  Setting adj to raw value %g\n", val));
  
  /* We don't really want to clamp to upper; we want to clamp to
     upper - page_size which is the highest value the scrollbar
     will let us reach. */
  if (val > (adj->upper - adj->page_size))
    val = adj->upper - adj->page_size;

  if (val < adj->lower)
    val = adj->lower;

  if (val != adj->value)
    {
      //DV (g_print ("  Setting adj to clamped value %g\n", val));
      adj->set_value (val);
      return true;
    }
  else
    return false;
}

/**
 * gtk_text_view_scroll_to_iter:
 * @text_view: a #TextView
 * @iter: a #TextIter
 * @within_margin: margin as a [0.0,0.5) fraction of screen size
 * @use_align: whether to use alignment arguments (if %false, 
 *    just get the mark onscreen)
 * @xalign: horizontal alignment of mark within visible area
 * @yalign: vertical alignment of mark within visible area
 *
 * Scrolls @text_view so that @iter is on the screen in the position
 * indicated by @xalign and @yalign. An alignment of 0.0 indicates
 * left or top, 1.0 indicates right or bottom, 0.5 means center. 
 * If @use_align is %false, the text scrolls the minimal distance to 
 * get the mark onscreen, possibly not scrolling at all. The effective 
 * screen for purposes of this function is reduced by a margin of size 
 * @within_margin.
 *
 * Note that this function uses the currently-computed height of the
 * lines in the text buffer. Line heights are computed in an idle 
 * handler; so this function may not have the desired effect if it's 
 * called before the height computations. To avoid oddness, consider 
 * using gtk_text_view_scroll_to_mark() which saves a point to be 
 * scrolled to after line validation.
 *
 * Return value: %true if scrolling occurred
 **/
bool
TextView::scroll_to_iter (
                              TextIter   *iter,
                              gdouble        within_margin,
                              bool       use_align,
                              gdouble        xalign,
                              gdouble        yalign)
{
  Rect rect;
  Rect screen;
  gint screen_bottom;
  gint screen_right;
  gint scroll_dest;
  //Widget *widget;
  bool retval = false;
  gint scroll_inc;
  gint screen_xoffset, screen_yoffset;
  gint current_x_scroll, current_y_scroll;

  /* FIXME why don't we do the validate-at-scroll-destination thing
   * from flush_scroll in this function? I think it wasn't done before
   * because changed_handler was screwed up, but I could be wrong.
   */
  
  //g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), false);
  g_return_val_if_fail (iter != NULL, false);
  g_return_val_if_fail (within_margin >= 0.0 && within_margin < 0.5, false);
  g_return_val_if_fail (xalign >= 0.0 && xalign <= 1.0, false);
  g_return_val_if_fail (yalign >= 0.0 && yalign <= 1.0, false);
  
  //widget = GTK_WIDGET (text_view);

  //DV(g_print(G_STRLOC"\n"));
  
  layout->get_iter_location ( iter, &rect);

  //DV (g_print (" target rect %d,%d  %d x %d\n", rect.x, rect.y, rect.width, rect.height));
  
  current_x_scroll = xoffset;
  current_y_scroll = yoffset;

  screen.x = current_x_scroll;
  screen.y = current_y_scroll;
  screen.width = w;
  screen.height = h;
  
  screen_xoffset = screen.width * within_margin;
  screen_yoffset = screen.height * within_margin;
  
  screen.x += screen_xoffset;
  screen.y += screen_yoffset;
  screen.width -= screen_xoffset * 2;
  screen.height -= screen_yoffset * 2;

  /* paranoia check */
  if (screen.width < 1)
    screen.width = 1;
  if (screen.height < 1)
    screen.height = 1;
  
  /* The -1 here ensures that we leave enough space to draw the cursor
   * when this function is used for horizontal scrolling. 
   */
  screen_right = screen.x + screen.width - 1;
  screen_bottom = screen.y + screen.height;
  
  /* The alignment affects the point in the target character that we
   * choose to align. If we're doing right/bottom alignment, we align
   * the right/bottom edge of the character the mark is at; if we're
   * doing left/top we align the left/top edge of the character; if
   * we're doing center alignment we align the center of the
   * character.
   */
  
  /* Vertical scroll */

  scroll_inc = 0;
  scroll_dest = current_y_scroll;
  
  if (use_align)
    {      
      scroll_dest = rect.y + (rect.height * yalign) - (screen.height * yalign);
      
      /* if scroll_dest < screen.y, we move a negative increment (up),
       * else a positive increment (down)
       */
      scroll_inc = scroll_dest - screen.y + screen_yoffset;
    }
  else
    {
      /* move minimum to get onscreen */
      if (rect.y < screen.y)
        {
          scroll_dest = rect.y;
          scroll_inc = scroll_dest - screen.y - screen_yoffset;
        }
      else if ((rect.y + rect.height) > screen_bottom)
        {
          scroll_dest = rect.y + rect.height;
          scroll_inc = scroll_dest - screen_bottom + screen_yoffset;
        }
    }  
  
  if (scroll_inc != 0)
    {
      retval = set_adjustment_clamped (get_vadjustment (),
                                       current_y_scroll + scroll_inc);

      DV (g_print (" vert increment %d\n", scroll_inc));
    }

  /* Horizontal scroll */
  
  scroll_inc = 0;
  scroll_dest = current_x_scroll;
  
  if (use_align)
    {      
      scroll_dest = rect.x + (rect.width * xalign) - (screen.width * xalign);

      /* if scroll_dest < screen.y, we move a negative increment (left),
       * else a positive increment (right)
       */
      scroll_inc = scroll_dest - screen.x + screen_xoffset;
    }
  else
    {
      /* move minimum to get onscreen */
      if (rect.x < screen.x)
        {
          scroll_dest = rect.x;
          scroll_inc = scroll_dest - screen.x - screen_xoffset;
        }
      else if ((rect.x + rect.width) > screen_right)
        {
          scroll_dest = rect.x + rect.width;
          scroll_inc = scroll_dest - screen_right + screen_xoffset;
        }
    }
  
  if (scroll_inc != 0)
    {
      retval = set_adjustment_clamped (get_hadjustment (),
                                       current_x_scroll + scroll_inc);

      DV (g_print (" horiz increment %d\n", scroll_inc));
    }
  
  if (retval)
    DV(g_print (">Actually scrolled ("G_STRLOC")\n"));
  else
    DV(g_print (">Didn't end up scrolling ("G_STRLOC")\n"));
  
  return retval;
}

void
TextView::free_pending_scroll (TextPendingScroll *scroll)
{
  if (!scroll->mark->get_deleted ())
    buffer->delete_mark (
                                 scroll->mark);
  //g_object_unref (scroll->mark);
  g_free (scroll);
}

void
TextView::cancel_pending_scroll ()
{
  if (pending_scroll)
    {
      free_pending_scroll (pending_scroll);
      pending_scroll = NULL;
    }
}
    
void
TextView::queue_scroll (
                            TextMark   *mark,
                            gdouble        within_margin,
                            bool       use_align,
                            gdouble        xalign,
                            gdouble        yalign)
{
  TextIter iter;
  TextPendingScroll *scroll;

  //DV(g_print(G_STRLOC"\n"));
  
  scroll = g_new (TextPendingScroll, 1);

  scroll->within_margin = within_margin;
  scroll->use_align = use_align;
  scroll->xalign = xalign;
  scroll->yalign = yalign;
  
  buffer->get_iter_at_mark (&iter, mark);

  scroll->mark = buffer->create_mark ( NULL,
                                              &iter,
                                              mark->get_left_gravity ());

  //g_object_ref (scroll->mark);
  
  cancel_pending_scroll ();

  pending_scroll = scroll;
}

bool
TextView::flush_scroll ()
{
  TextIter iter;
  TextPendingScroll *scroll;
  bool retval;
  //GtkWidget *widget;

  //widget = GTK_WIDGET (text_view);
  
  //DV(g_print(G_STRLOC"\n"));
  
  if (pending_scroll == NULL)
    {
      //DV (g_print ("in flush scroll, no pending scroll\n"));
      return false;
    }

  scroll = pending_scroll;

  /* avoid recursion */
  pending_scroll = NULL;
  
  buffer->get_iter_at_mark (&iter, scroll->mark);

  /* Validate area around the scroll destination, so the adjustment
   * can meaningfully point into that area. We must validate
   * enough area to be sure that after we scroll, everything onscreen
   * is valid; otherwise, validation will maintain the first para
   * in one place, but may push the target iter off the bottom of
   * the screen.
   */
  //DV(g_print (">Validating scroll destination ("G_STRLOC")\n"));
  //TODO check if the args are correct
  layout->validate_yrange ( &iter,
                                   - (height * 2),
                                   h * 2);
  
  //DV(g_print (">Done validating scroll destination ("G_STRLOC")\n"));

  /* Ensure we have updated width/height */
  update_adjustments ();
  
  retval = scroll_to_iter (
                                         &iter,
                                         scroll->within_margin,
                                         scroll->use_align,
                                         scroll->xalign,
                                         scroll->yalign);

  free_pending_scroll (scroll);

  return retval;
}

void
TextView::set_adjustment_upper (Adjustment *adj, gdouble upper)
{  
  if (upper != adj->upper)
    {
      gdouble min = MAX (0.0, upper - adj->page_size);
      bool value_changed = false;

      adj->upper = upper;

      if (adj->value > min)
        {
          adj->value = min;
          value_changed = true;
        }

      adj->adjustment_changed ();
      DV(g_print(">Changed adj upper to %g ("G_STRLOC")\n", upper));
      
      if (value_changed)
        {
          DV(g_print(">Changed adj value because upper decreased ("G_STRLOC")\n"));
	  adj->value_changed ();
        }
    }
}

void
TextView::update_adjustments ()
{
  gint width = 0, height = 0;

  DV(g_print(">Updating adjustments ("G_STRLOC")\n"));

  if (layout)
    layout->get_size (&width, &height);

  /* Make room for the cursor after the last character in the widest line */
  width += SPACE_FOR_CURSOR;

  if (this->width != width || this->height != height)
    {
      if (this->width != width)
	width_changed = true;

      this->width = width;
      this->height = height;

      set_adjustment_upper (get_hadjustment(),
                                          MAX (w, width));
      set_adjustment_upper (get_vadjustment(),
                                          MAX (h, height));
      
      /* hadj/vadj exist since we called get_hadjustment/get_vadjustment above */

      /* Set up the step sizes; we'll say that a page is
         our allocation minus one step, and a step is
         1/10 of our allocation. */
      //TODO convert to lines/cols
      hadjustment->step_increment =
        Width() / 10.0;
      hadjustment->page_increment =
        Width() * 0.9;
      
      vadjustment->step_increment =
        Height() / 10.0;
      vadjustment->page_increment =
        Height() * 0.9;

      hadjustment->adjustment_changed ();
      vadjustment->adjustment_changed ();
    }
}

void
TextView::update_layout_width ()
{
  DV(g_print(">Updating layout width ("G_STRLOC")\n"));
  
  ensure_layout ();

  layout->set_screen_width (
                                    MAX (1, w - SPACE_FOR_CURSOR));
}

/*
static void
TextView::update_im_spot_location ()
{
  GdkRect area;

  if (text_view->layout == NULL)
    return;
  
  gtk_text_view_get_cursor_location (text_view, &area);

  area.x -= text_view->xoffset;
  area.y -= text_view->yoffset;
    
  * Width returned by Pango indicates direction of cursor,
   * by it's sign more than the size of cursor.
   *
  area.width = 0;

  gtk_im_context_set_cursor_location (text_view->im_context, &area);
}*/
/*
static bool
do_update_im_spot_location (gpointer text_view)
{
  TextViewPrivate *priv;

  priv = GTK_TEXT_VIEW_GET_PRIVATE (text_view);
  priv->im_spot_idle = 0;

  gtk_text_view_update_im_spot_location (text_view);
  return false;
}

static void
queue_update_im_spot_location (TextView *text_view)
{
  TextViewPrivate *priv;

  priv = GTK_TEXT_VIEW_GET_PRIVATE (text_view);

  * Use priority a little higher than GTK_TEXT_VIEW_PRIORITY_VALIDATE,
   * so we don't wait until the entire buffer has been validated. *
  if (!priv->im_spot_idle)
    priv->im_spot_idle = gdk_threads_add_idle_full (GTK_TEXT_VIEW_PRIORITY_VALIDATE - 1,
						    do_update_im_spot_location,
						    text_view,
						    NULL);
}*/

void
TextView::flush_update_im_spot_location (void)
{
  //TextViewPrivate *priv;

  //priv = GTK_TEXT_VIEW_GET_PRIVATE (text_view);
/*
  if (im_spot_idle)
    {
      g_source_remove (priv->im_spot_idle);
      priv->im_spot_idle = 0;
      gtk_text_view_update_im_spot_location (text_view);
    }*/
}

/**
 * gtk_text_view_scroll_to_mark:
 * @text_view: a #TextView
 * @mark: a #TextMark
 * @within_margin: margin as a [0.0,0.5) fraction of screen size
 * @use_align: whether to use alignment arguments (if %false, just 
 *    get the mark onscreen)
 * @xalign: horizontal alignment of mark within visible area
 * @yalign: vertical alignment of mark within visible area
 *
 * Scrolls @text_view so that @mark is on the screen in the position
 * indicated by @xalign and @yalign. An alignment of 0.0 indicates
 * left or top, 1.0 indicates right or bottom, 0.5 means center. 
 * If @use_align is %false, the text scrolls the minimal distance to 
 * get the mark onscreen, possibly not scrolling at all. The effective 
 * screen for purposes of this function is reduced by a margin of size 
 * @within_margin.
 **/
void
TextView::scroll_to_mark (
                              TextMark *mark,
                              gdouble      within_margin,
                              bool     use_align,
                              gdouble      xalign,
                              gdouble      yalign)
{  
 // //g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))
 // g_return_if_fail (GTK_IS_TEXT_MARK (mark));
  g_return_if_fail (within_margin >= 0.0 && within_margin < 0.5);
  g_return_if_fail (xalign >= 0.0 && xalign <= 1.0);
  g_return_if_fail (yalign >= 0.0 && yalign <= 1.0);

  queue_scroll (mark,
                              within_margin,
                              use_align,
                              xalign,
                              yalign);

  /* If no validation is pending, we need to go ahead and force an
   * immediate scroll.
   */
  if (layout &&
      layout->is_valid ())
    flush_scroll ();
}

/**
 * gtk_text_view_scroll_mark_onscreen:
 * @text_view: a #TextView
 * @mark: a mark in the buffer for @text_view
 * 
 * Scrolls @text_view the minimum distance such that @mark is contained
 * within the visible area of the widget.
 **/
void
TextView::scroll_mark_onscreen (
                                    TextMark *mark)
{
  ////g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))
  //g_return_if_fail (GTK_IS_TEXT_MARK (mark));

  scroll_to_mark (mark, 0.0, false, 0.0, 0.0);
}

bool TextView::clamp_iter_onscreen (TextIter *iter)
{
  Rect visible_rect;
  get_visible_rect (&visible_rect);

  return layout->clamp_iter_to_vrange (iter,
                                               visible_rect.y,
                                               visible_rect.y + visible_rect.height);
}

/**
 * gtk_text_view_move_mark_onscreen:
 * @text_view: a #TextView
 * @mark: a #TextMark
 *
 * Moves a mark within the buffer so that it's
 * located within the currently-visible text area.
 *
 * Return value: %true if the mark moved (wasn't already onscreen)
 **/
bool
TextView::move_mark_onscreen (
                                  TextMark *mark)
{
  TextIter iter;

  //g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), false);
  g_return_val_if_fail (mark != NULL, false);

  buffer->get_iter_at_mark (&iter, mark);

  if (clamp_iter_onscreen (&iter))
    {
      buffer->move_mark (mark, &iter);
      return true;
    }
  else
    return false;
}

/**
 * gtk_text_view_get_visible_rect:
 * @text_view: a #TextView
 * @visible_rect: rectangle to fill
 *
 * Fills @visible_rect with the currently-visible
 * region of the buffer, in buffer coordinates. Convert to window coordinates
 * with gtk_text_view_buffer_to_window_coords().
 **/
void TextView::get_visible_rect ( Rect *visible_rect)
{
  //GtkWidget *widget;

  ////g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))

  //widget = GTK_WIDGET (text_view);

  if (visible_rect)
    {
      visible_rect->x = xoffset;
      visible_rect->y = yoffset;
      visible_rect->width = Width();
      visible_rect->height = Height();

      DV(g_print(" visible rect: %d,%d %d x %d\n",
                 visible_rect->x,
                 visible_rect->y,
                 visible_rect->width,
                 visible_rect->height));
    }
}

/**
 * gtk_text_view_set_wrap_mode:
 * @text_view: a #TextView
 * @wrap_mode: a #WrapMode
 *
 * Sets the line wrapping for the view.
 **/
void
TextView::set_wrap_mode (
                             WrapMode  wrap_mode)
{
  ////g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))

  if (this->wrap_mode != wrap_mode)
    {
      this->wrap_mode = wrap_mode;

      if (layout)
        {
          layout->default_style->wrap_mode = wrap_mode;
          layout->default_style_changed ();
        }
    }

 // g_object_notify (G_OBJECT (text_view), "wrap-mode");
}

/**
 * gtk_text_view_get_wrap_mode:
 * @text_view: a #TextView
 *
 * Gets the line wrapping for the view.
 *
 * Return value: the line wrap setting
 **/
WrapMode TextView::get_wrap_mode (void)
{
  //g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), GTK_WRAP_NONE);

  return wrap_mode;
}

/**
 * gtk_text_view_set_editable:
 * @text_view: a #TextView
 * @setting: whether it's editable
 *
 * Sets the default editability of the #TextView. You can override
 * this default setting with tags in the buffer, using the "editable"
 * attribute of tags.
 **/
void
TextView::set_editable (
                            bool     setting)
{
  ////g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))
  setting = setting != false;

  if (editable != setting)
    {
      if (!setting)
	{
	  //gtk_text_view_reset_im_context(text_view);
	  if (HasFocus())
	    //TODO move focus to next widget.
	    // gtk_im_context_focus_out (text_view->im_context);
	    ;
	}

      editable = setting;

      if (setting && HasFocus())
	//gtk_im_context_focus_in (text_view->im_context);
	;

      if (layout)
        {
	  layout->set_overwrite_mode ( overwrite_mode && editable);
          layout->default_style->editable = editable;
          layout->default_style_changed ();
        }

      //g_object_notify (G_OBJECT (text_view), "editable");
    }
}

/**
 * gtk_text_view_get_editable:
 * @text_view: a #TextView
 *
 * Returns the default editability of the #TextView. Tags in the
 * buffer may override this setting for some ranges of text.
 *
 * Return value: whether text is editable by default
 **/
bool
TextView::get_editable (void)
{
  //g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), false);

  return editable;
}

/**
 * gtk_text_view_set_pixels_above_lines:
 * @text_view: a #TextView
 * @pixels_above_lines: pixels above paragraphs
 * 
 * Sets the default number of blank pixels above paragraphs in @text_view.
 * Tags in the buffer for @text_view may override the defaults.
 **/
void TextView::set_pixels_above_lines ( gint         pixels_above_lines)
{
  ////g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))

  if (this->pixels_above_lines != pixels_above_lines)
    {
      this->pixels_above_lines = pixels_above_lines;

      if (layout)
        {
          layout->default_style->pixels_above_lines = pixels_above_lines;
          layout->default_style_changed ();
        }

 //     g_object_notify (G_OBJECT (text_view), "pixels-above-lines");
    }
}

/**
 * gtk_text_view_get_pixels_above_lines:
 * @text_view: a #TextView
 * 
 * Gets the default number of pixels to put above paragraphs.
 * 
 * Return value: default number of pixels above paragraphs
 **/
gint
TextView::get_pixels_above_lines (void)
{
  //g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), 0);

  return pixels_above_lines;
}

/**
 * gtk_text_view_set_pixels_below_lines:
 * @text_view: a #TextView
 * @pixels_below_lines: pixels below paragraphs 
 *
 * Sets the default number of pixels of blank space
 * to put below paragraphs in @text_view. May be overridden
 * by tags applied to @text_view's buffer. 
 **/
//TODO set to 0 always
void TextView::set_pixels_below_lines ( gint         pixels_below_lines)
{
  ////g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))

  if (this->pixels_below_lines != pixels_below_lines)
    {
      this->pixels_below_lines = pixels_below_lines;

      if (layout)
        {
          layout->default_style->pixels_below_lines = pixels_below_lines;
          layout->default_style_changed ();
        }

      //g_object_notify (G_OBJECT (text_view), "pixels-below-lines");
    }
}

/**
 * gtk_text_view_get_pixels_below_lines:
 * @text_view: a #TextView
 * 
 * Gets the value set by gtk_text_view_set_pixels_below_lines().
 * 
 * Return value: default number of blank pixels below paragraphs
 **/
gint
TextView::get_pixels_below_lines (void)
{
 // g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), 0);

  return pixels_below_lines;
}

/**
 * gtk_text_view_set_pixels_inside_wrap:
 * @text_view: a #TextView
 * @pixels_inside_wrap: default number of pixels between wrapped lines
 *
 * Sets the default number of pixels of blank space to leave between
 * display/wrapped lines within a paragraph. May be overridden by
 * tags in @text_view's buffer.
 **/
void TextView::set_pixels_inside_wrap ( gint         pixels_inside_wrap)
{
  ////g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))

  if (this->pixels_inside_wrap != pixels_inside_wrap)
    {
      this->pixels_inside_wrap = pixels_inside_wrap;

      if (layout)
        {
          layout->default_style->pixels_inside_wrap = pixels_inside_wrap;
          layout->default_style_changed ();
        }

      //g_object_notify (G_OBJECT (text_view), "pixels-inside-wrap");
    }
}

/**
 * gtk_text_view_get_pixels_inside_wrap:
 * @text_view: a #TextView
 * 
 * Gets the value set by gtk_text_view_set_pixels_inside_wrap().
 * 
 * Return value: default number of pixels of blank space between wrapped lines
 **/
gint
TextView::get_pixels_inside_wrap (void)
{
  //g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), 0);

  return pixels_inside_wrap;
}

/**
 * gtk_text_view_set_justification:
 * @text_view: a #TextView
 * @justification: justification
 *
 * Sets the default justification of text in @text_view.
 * Tags in the view's buffer may override the default.
 * 
 **/
/*
void
TextView::set_justification (
                                 Justification justification)
{
 // //g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))

  if (text_view->justify != justification)
    {
      text_view->justify = justification;

      if (text_view->layout)
        {
          text_view->layout->default_style->justification = justification;
          gtk_text_layout_default_style_changed (text_view->layout);
        }

      g_object_notify (G_OBJECT (text_view), "justification");
    }
}*/

/**
 * gtk_text_view_get_justification:
 * @text_view: a #TextView
 * 
 * Gets the default justification of paragraphs in @text_view.
 * Tags in the buffer may override the default.
 * 
 * Return value: default justification
 **/
/*
Justification
TextView::get_justification (TextView *text_view)
{
  g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), GTK_JUSTIFY_LEFT);

  return text_view->justify;
}*/

/**
 * gtk_text_view_set_left_margin:
 * @text_view: a #TextView
 * @left_margin: left margin in pixels
 * 
 * Sets the default left margin for text in @text_view.
 * Tags in the buffer may override the default.
 **/
////void TextView::set_left_margin (gint left_margin)
////{
////  //g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))
////
////  if (this->left_margin != left_margin)
////    {
////      this->left_margin = left_margin;
////
////      if (layout)
////        {
////          layout->default_style->left_margin = left_margin;
////          layout->default_style_changed ();
////        }
////
//// //     g_object_notify (G_OBJECT (text_view), "left-margin");
////    }
////}
////
/**
 * gtk_text_view_get_left_margin:
 * @text_view: a #TextView
 * 
 * Gets the default left margin size of paragraphs in the @text_view.
 * Tags in the buffer may override the default.
 * 
 * Return value: left margin in pixels
 **/
gint TextView::get_left_margin (void)
{
  //g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), 0);

  return left_margin;
}

/**
 * gtk_text_view_set_right_margin:
 * @text_view: a #TextView
 * @right_margin: right margin in pixels
 *
 * Sets the default right margin for text in the text view.
 * Tags in the buffer may override the default.
 **/
////void TextView::set_right_margin(gint right_margin)
////{
////  //g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))
////
////  if (this->right_margin != right_margin)
////    {
////      this->right_margin = right_margin;
////
////      if (layout)
////        {
////          layout->default_style->right_margin = right_margin;
////          layout->default_style_changed ();
////        }
////
//// //     g_object_notify (G_OBJECT (text_view), "right-margin");
////    }
////}
////
/**
 * gtk_text_view_get_right_margin:
 * @text_view: a #TextView
 * 
 * Gets the default right margin for text in @text_view. Tags
 * in the buffer may override the default.
 * 
 * Return value: right margin in pixels
 **/
gint
TextView::get_right_margin ()
{
//  g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), 0);

  return right_margin;
}

/**
 * gtk_text_view_set_indent:
 * @text_view: a #TextView
 * @indent: indentation in pixels
 *
 * Sets the default indentation for paragraphs in @text_view.
 * Tags in the buffer may override the default.
 **/
void
TextView::set_indent (
                          gint         indent)
{
  //g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))

  if (this->indent != indent)
    {
      this->indent = indent;

      if (layout)
        {
          layout->default_style->indent = indent;
          layout->default_style_changed ();
        }

      //TODOg_object_notify (G_OBJECT (text_view), "indent");
    }
}

/**
 * gtk_text_view_get_indent:
 * @text_view: a #TextView
 * 
 * Gets the default indentation of paragraphs in @text_view.
 * Tags in the view's buffer may override the default.
 * The indentation may be negative.
 * 
 * Return value: number of pixels of indentation
 **/
gint
TextView::get_indent ()
{
 // g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), 0);

  return indent;
}

/**
 * gtk_text_view_set_tabs:
 * @text_view: a #TextView
 * @tabs: tabs as a #PangoTabArray
 *
 * Sets the default tab stops for paragraphs in @text_view.
 * Tags in the buffer may override the default.
 **/
/*
void
TextView::set_tabs (TextView   *text_view,
                        PangoTabArray *tabs)
{
  //g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))

  if (text_view->tabs)
    pango_tab_array_free (text_view->tabs);

  text_view->tabs = tabs ? pango_tab_array_copy (tabs) : NULL;

  if (text_view->layout)
    {
      * some unkosher futzing in internal struct details... *
      if (text_view->layout->default_style->tabs)
        pango_tab_array_free (text_view->layout->default_style->tabs);

      text_view->layout->default_style->tabs =
        text_view->tabs ? pango_tab_array_copy (text_view->tabs) : NULL;

      gtk_text_layout_default_style_changed (text_view->layout);
    }

  g_object_notify (G_OBJECT (text_view), "tabs");
}*/

/**
 * gtk_text_view_get_tabs:
 * @text_view: a #TextView
 * 
 * Gets the default tabs for @text_view. Tags in the buffer may
 * override the defaults. The returned array will be %NULL if
 * "standard" (8-space) tabs are used. Free the return value
 * with pango_tab_array_free().
 * 
 * Return value: copy of default tab array, or %NULL if "standard" 
 *    tabs are used; must be freed with pango_tab_array_free().
 **/
/*PangoTabArray*
TextView::get_tabs (TextView *text_view)
{
  g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), NULL);

  return text_view->tabs ? pango_tab_array_copy (text_view->tabs) : NULL;
}*/

void
TextView::toggle_cursor_visible (void)
{
  set_cursor_visible (!cursor_visible);
}

/**
 * gtk_text_view_set_cursor_visible:
 * @text_view: a #TextView
 * @setting: whether to show the insertion cursor
 *
 * Toggles whether the insertion point is displayed. A buffer with no editable
 * text probably shouldn't have a visible cursor, so you may want to turn
 * the cursor off.
 **/
void
TextView::set_cursor_visible ( bool     setting)
{
  //g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))

  if (cursor_visible != setting)
    {
      cursor_visible = setting;

      if (HasFocus())
        {
          if (layout)
            {
              layout->set_cursor_visible (setting);
	      //check_cursor_blink (text_view);
            }
        }

      //g_object_notify (G_OBJECT (text_view), "cursor-visible");
    }
}

/**
 * gtk_text_view_get_cursor_visible:
 * @text_view: a #TextView
 *
 * Find out whether the cursor is being displayed.
 *
 * Return value: whether the insertion mark is visible
 **/
bool TextView::get_cursor_visible (void )
{
  //g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), false);

  return cursor_visible;
}


/**
 * gtk_text_view_place_cursor_onscreen:
 * @text_view: a #TextView
 *
 * Moves the cursor to the currently visible region of the
 * buffer, it it isn't there already.
 *
 * Return value: %true if the cursor had to be moved.
 **/
bool
TextView::place_cursor_onscreen ()
{
  TextIter insert;

 // g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), false);

  buffer->get_iter_at_mark (&insert,
                                    buffer->get_insert ());

  if (clamp_iter_onscreen (&insert))
    {
      buffer->place_cursor (&insert);
      return true;
    }
  else
    return false;
}

void TextView::remove_validate_idles ()
{
  if (first_validate_idle != 0)
    {
      //DV (g_print ("Removing first validate idle: %s\n", G_STRLOC));
      g_source_remove (first_validate_idle);
      first_validate_idle = 0;
    }

  if (incremental_validate_idle != 0)
    {
      g_source_remove (incremental_validate_idle);
      incremental_validate_idle = 0;
    }
}


TextView::~TextView (void)
{
 // TextView *text_view;
 // TextViewPrivate *priv;

 // text_view = GTK_TEXT_VIEW (object);
//  priv = GTK_TEXT_VIEW_GET_PRIVATE (text_view);

  remove_validate_idles ();
  set_buffer (NULL);
  destroy_layout ();

  if (scroll_timeout)
    {
      g_source_remove (scroll_timeout);
      scroll_timeout = 0;
    }

  if (im_spot_idle)
    {
      g_source_remove (im_spot_idle);
      im_spot_idle = 0;
    }

  //GTK_OBJECT_CLASS (gtk_text_view_parent_class)->destroy (object);
/*
}

static void
TextView::finalize (GObject *object)
{*/
  
  cancel_pending_scroll ();

  //if (tabs)
    //pango_tab_array_free (text_view->tabs);
  
  //if (hadjustment)
  //  g_object_unref (text_view->hadjustment);
  //if (text_view->vadjustment)
  //  g_object_unref (text_view->vadjustment);

  //text_window_free (text_view->text_window);

  /*if (text_view->left_window)
    text_window_free (text_view->left_window);

  if (text_view->top_window)
    text_window_free (text_view->top_window);

  if (text_view->right_window)
    text_window_free (text_view->right_window);

  if (text_view->bottom_window)
    text_window_free (text_view->bottom_window);

  g_object_unref (text_view->im_context);

  G_OBJECT_CLASS (gtk_text_view_parent_class)->finalize (object);
  */
}

/*
void
TextView::set_property (GObject         *object,
			    guint            prop_id,
			    const GValue    *value,
			    GParamSpec      *pspec)
{
  TextView *text_view;

  text_view = GTK_TEXT_VIEW (object);

  switch (prop_id)
    {
    case PROP_PIXELS_ABOVE_LINES:
      gtk_text_view_set_pixels_above_lines (text_view, g_value_get_int (value));
      break;

    case PROP_PIXELS_BELOW_LINES:
      gtk_text_view_set_pixels_below_lines (text_view, g_value_get_int (value));
      break;

    case PROP_PIXELS_INSIDE_WRAP:
      gtk_text_view_set_pixels_inside_wrap (text_view, g_value_get_int (value));
      break;

    case PROP_EDITABLE:
      gtk_text_view_set_editable (text_view, g_value_get_boolean (value));
      break;

    case PROP_WRAP_MODE:
      gtk_text_view_set_wrap_mode (text_view, g_value_get_enum (value));
      break;
      
    case PROP_JUSTIFICATION:
      gtk_text_view_set_justification (text_view, g_value_get_enum (value));
      break;

    case PROP_LEFT_MARGIN:
      gtk_text_view_set_left_margin (text_view, g_value_get_int (value));
      break;

    case PROP_RIGHT_MARGIN:
      gtk_text_view_set_right_margin (text_view, g_value_get_int (value));
      break;

    case PROP_INDENT:
      gtk_text_view_set_indent (text_view, g_value_get_int (value));
      break;

    case PROP_TABS:
      gtk_text_view_set_tabs (text_view, g_value_get_boxed (value));
      break;

    case PROP_CURSOR_VISIBLE:
      gtk_text_view_set_cursor_visible (text_view, g_value_get_boolean (value));
      break;

    case PROP_OVERWRITE:
      gtk_text_view_set_overwrite (text_view, g_value_get_boolean (value));
      break;

    case PROP_BUFFER:
      gtk_text_view_set_buffer (text_view, GTK_TEXT_BUFFER (g_value_get_object (value)));
      break;

    case PROP_ACCEPTS_TAB:
      gtk_text_view_set_accepts_tab (text_view, g_value_get_boolean (value));
      break;
      
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}*/

/*void
TextView::get_property (GObject         *object,
			    guint            prop_id,
			    GValue          *value,
			    GParamSpec      *pspec)
{
  TextView *text_view;

  text_view = GTK_TEXT_VIEW (object);

  switch (prop_id)
    {
    case PROP_PIXELS_ABOVE_LINES:
      g_value_set_int (value, text_view->pixels_above_lines);
      break;

    case PROP_PIXELS_BELOW_LINES:
      g_value_set_int (value, text_view->pixels_below_lines);
      break;

    case PROP_PIXELS_INSIDE_WRAP:
      g_value_set_int (value, text_view->pixels_inside_wrap);
      break;

    case PROP_EDITABLE:
      g_value_set_boolean (value, text_view->editable);
      break;
      
    case PROP_WRAP_MODE:
      g_value_set_enum (value, text_view->wrap_mode);
      break;

    case PROP_JUSTIFICATION:
      g_value_set_enum (value, text_view->justify);
      break;

    case PROP_LEFT_MARGIN:
      g_value_set_int (value, text_view->left_margin);
      break;

    case PROP_RIGHT_MARGIN:
      g_value_set_int (value, text_view->right_margin);
      break;

    case PROP_INDENT:
      g_value_set_int (value, text_view->indent);
      break;

    case PROP_TABS:
      g_value_set_boxed (value, text_view->tabs);
      break;

    case PROP_CURSOR_VISIBLE:
      g_value_set_boolean (value, text_view->cursor_visible);
      break;

    case PROP_BUFFER:
      g_value_set_object (value, buffer );
      break;

    case PROP_OVERWRITE:
      g_value_set_boolean (value, text_view->overwrite_mode);
      break;

    case PROP_ACCEPTS_TAB:
      g_value_set_boolean (value, text_view->accepts_tab);
      break;
      
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}*/

/*
void
TextView::size_request (GtkWidget      *widget,
                            Rect *requisition)
{
  TextView *text_view;
  GSList *tmp_list;
  gint focus_edge_width;
  gint focus_width;
  bool interior_focus;
  
  text_view = GTK_TEXT_VIEW (widget);

  gtk_widget_style_get (widget,
			"interior-focus", &interior_focus,
			"focus-line-width", &focus_width,
			NULL);

  if (interior_focus)
    focus_edge_width = 0;
  else
    focus_edge_width = focus_width;

  if (text_view->layout)
    {
      text_view->text_window->requisition.width = text_view->layout->width;
      text_view->text_window->requisition.height = text_view->layout->height;
    }
  else
    {
      text_view->text_window->requisition.width = 0;
      text_view->text_window->requisition.height = 0;
    }
  
  requisition->width = text_view->text_window->requisition.width + focus_edge_width * 2;
  requisition->height = text_view->text_window->requisition.height + focus_edge_width * 2;

  if (text_view->left_window)
    requisition->width += text_view->left_window->requisition.width;

  if (text_view->right_window)
    requisition->width += text_view->right_window->requisition.width;

  if (text_view->top_window)
    requisition->height += text_view->top_window->requisition.height;

  if (text_view->bottom_window)
    requisition->height += text_view->bottom_window->requisition.height;

  requisition->width += GTK_CONTAINER (text_view)->border_width * 2;
  requisition->height += GTK_CONTAINER (text_view)->border_width * 2;
  
  tmp_list = text_view->children;
  while (tmp_list != NULL)
    {
      TextViewChild *child = tmp_list->data;

      if (child->anchor)
        {
          Rect child_req;
          Rect old_req;

          gtk_widget_get_child_requisition (child->widget, &old_req);
          
          gtk_widget_size_request (child->widget, &child_req);

          gtk_widget_get_child_requisition (child->widget, &child_req);

          * Invalidate layout lines if required *
          if (text_view->layout &&
              (old_req.width != child_req.width ||
               old_req.height != child_req.height))
            gtk_text_child_anchor_queue_resize (child->anchor,
                                                text_view->layout);
        }
      else
        {
          Rect child_req;
          
          gtk_widget_size_request (child->widget, &child_req);
        }

      tmp_list = g_slist_next (tmp_list);
    }
}*/

/*void
TextView::compute_child_allocation (TextView      *text_view,
                                        TextViewChild *vc,
                                        GtkAllocation    *allocation)
{
  gint buffer_y;
  TextIter iter;
  Rect req;
  
  gtk_text_buffer_get_iter_at_child_anchor (buffer ,
                                            &iter,
                                            vc->anchor);

  layout->get_line_yrange (&iter,
                                   &buffer_y, NULL);

  buffer_y += vc->from_top_of_line;

  allocation->x = vc->from_left_of_buffer - text_view->xoffset;
  allocation->y = buffer_y - text_view->yoffset;

  gtk_widget_get_child_requisition (vc->widget, &req);
  allocation->width = req.width;
  allocation->height = req.height;
}*/

/*void
TextView::update_child_allocation (TextView      *text_view,
                                       TextViewChild *vc)
{
  GtkAllocation allocation;

  gtk_text_view_compute_child_allocation (text_view, vc, &allocation);
  
  gtk_widget_size_allocate (vc->widget, &allocation);

#if 0
  g_print ("allocation for %p allocated to %d,%d yoffset = %d\n",
           vc->widget,
           vc->widget->allocation.x,
           vc->widget->allocation.y,
           text_view->yoffset);
#endif
}*/

/*void
TextView::child_allocated (GtkTextLayout *layout,
                               GtkWidget     *child,
                               gint           x,
                               gint           y,
                               gpointer       data)
{
  TextViewChild *vc = NULL;
  TextView *text_view = data;
  
  * x,y is the position of the child from the top of the line, and
   * from the left of the buffer. We have to translate that into text
   * window coordinates, then size_allocate the child.
   *

  vc = g_object_get_data (G_OBJECT (child),
                          "gtk-text-view-child");

  g_assert (vc != NULL);

  DV (g_print ("child allocated at %d,%d\n", x, y));
  
  vc->from_left_of_buffer = x;
  vc->from_top_of_line = y;

  gtk_text_view_update_child_allocation (text_view, vc);
}*/

/*void
TextView::allocate_children (TextView *text_view)
{
  GSList *tmp_list;

  DV(g_print(G_STRLOC"\n"));
  
  tmp_list = text_view->children;
  while (tmp_list != NULL)
    {
      TextViewChild *child = tmp_list->data;

      g_assert (child != NULL);
          
      if (child->anchor)
        {
          * We need to force-validate the regions containing
           * children.
           *
          TextIter child_loc;
          gtk_text_buffer_get_iter_at_child_anchor (buffer,
                                                    &child_loc,
                                                    child->anchor);

	  * Since anchored children are only ever allocated from
           * gtk_text_layout_get_line_display() we have to make sure
	   * that the display line caching in the layout doesn't 
           * get in the way. Invalidating the layout around the anchor
           * achieves this.
	   *
	  if (GTK_WIDGET_ALLOC_NEEDED (child->widget))
	    {
	      TextIter end = child_loc;
	      gtk_text_iter_forward_char (&end);
	      gtk_text_layout_invalidate (text_view->layout, &child_loc, &end);
	    }

          gtk_text_layout_validate_yrange (text_view->layout,
                                           &child_loc,
                                           0, 1);
        }
      else
        {
          GtkAllocation allocation;          
          Rect child_req;
             
          allocation.x = child->x;
          allocation.y = child->y;

          gtk_widget_get_child_requisition (child->widget, &child_req);
          
          allocation.width = child_req.width;
          allocation.height = child_req.height;
          
          gtk_widget_size_allocate (child->widget, &allocation);          
        }

      tmp_list = g_slist_next (tmp_list);
    }
}*/

/*void
TextView::size_allocate (GtkWidget *widget,
                             GtkAllocation *allocation)
{
  TextView *text_view;
  TextIter first_para;
  gint y;
  gint width, height;
  GdkRect text_rect;
  GdkRect left_rect;
  GdkRect right_rect;
  GdkRect top_rect;
  GdkRect bottom_rect;
  gint focus_edge_width;
  gint focus_width;
  bool interior_focus;
  bool size_changed;
  
  text_view = GTK_TEXT_VIEW (widget);

  DV(g_print(G_STRLOC"\n"));

  size_changed =
    widget->allocation.width != allocation->width ||
    widget->allocation.height != allocation->height;
  
  widget->allocation = *allocation;

  if (GTK_WIDGET_REALIZED (widget))
    {
      gdk_window_move_resize (widget->window,
                              allocation->x, allocation->y,
                              allocation->width, allocation->height);
    }

  * distribute width/height among child windows. Ensure all
   * windows get at least a 1x1 allocation.
   *

  gtk_widget_style_get (widget,
			"interior-focus", &interior_focus,
			"focus-line-width", &focus_width,
			NULL);

  if (interior_focus)
    focus_edge_width = 0;
  else
    focus_edge_width = focus_width;
  
  width = allocation->width - focus_edge_width * 2 - GTK_CONTAINER (text_view)->border_width * 2;

  if (text_view->left_window)
    left_rect.width = text_view->left_window->requisition.width;
  else
    left_rect.width = 0;

  width -= left_rect.width;

  if (text_view->right_window)
    right_rect.width = text_view->right_window->requisition.width;
  else
    right_rect.width = 0;

  width -= right_rect.width;

  text_rect.width = MAX (1, width);

  top_rect.width = text_rect.width;
  bottom_rect.width = text_rect.width;


  height = allocation->height - focus_edge_width * 2 - GTK_CONTAINER (text_view)->border_width * 2;

  if (text_view->top_window)
    top_rect.height = text_view->top_window->requisition.height;
  else
    top_rect.height = 0;

  height -= top_rect.height;

  if (text_view->bottom_window)
    bottom_rect.height = text_view->bottom_window->requisition.height;
  else
    bottom_rect.height = 0;

  height -= bottom_rect.height;

  text_rect.height = MAX (1, height);

  left_rect.height = text_rect.height;
  right_rect.height = text_rect.height;

  * Origins *
  left_rect.x = focus_edge_width + GTK_CONTAINER (text_view)->border_width;
  top_rect.y = focus_edge_width + GTK_CONTAINER (text_view)->border_width;

  text_rect.x = left_rect.x + left_rect.width;
  text_rect.y = top_rect.y + top_rect.height;

  left_rect.y = text_rect.y;
  right_rect.y = text_rect.y;

  top_rect.x = text_rect.x;
  bottom_rect.x = text_rect.x;

  right_rect.x = text_rect.x + text_rect.width;
  bottom_rect.y = text_rect.y + text_rect.height;

  text_window_size_allocate (text_view->text_window,
                             &text_rect);

  if (text_view->left_window)
    text_window_size_allocate (text_view->left_window,
                               &left_rect);

  if (text_view->right_window)
    text_window_size_allocate (text_view->right_window,
                               &right_rect);

  if (text_view->top_window)
    text_window_size_allocate (text_view->top_window,
                               &top_rect);

  if (text_view->bottom_window)
    text_window_size_allocate (text_view->bottom_window,
                               &bottom_rect);

  gtk_text_view_update_layout_width (text_view);
  
  * Note that this will do some layout validation *
  gtk_text_view_allocate_children (text_view);

  * Ensure h/v adj exist *
  get_hadjustment (text_view);
  get_vadjustment (text_view);

  text_view->hadjustment->page_size = Width();
  text_view->hadjustment->page_increment = Width() * 0.9;
  text_view->hadjustment->step_increment = Width() * 0.1;
  text_view->hadjustment->lower = 0;
  text_view->hadjustment->upper = MAX (Width(),
                                       text_view->width);

  if (text_view->hadjustment->value > text_view->hadjustment->upper - text_view->hadjustment->page_size)
    gtk_adjustment_set_value (text_view->hadjustment, MAX (0, text_view->hadjustment->upper - text_view->hadjustment->page_size));

  hadjustment->adjustment_changed ();

  text_view->vadjustment->page_size = Height();
  text_view->vadjustment->page_increment = Height() * 0.9;
  text_view->vadjustment->step_increment = Height() * 0.1;
  text_view->vadjustment->lower = 0;
  text_view->vadjustment->upper = MAX (Height(),
                                       text_view->height);

  * Now adjust the value of the adjustment to keep the cursor at the
   * same place in the buffer
   *
  get_first_para_iter (&first_para);
  layout->get_line_yrange (&first_para, &y, NULL);

  y += text_view->first_para_pixels;

  if (y > text_view->vadjustment->upper - text_view->vadjustment->page_size)
    y = MAX (0, text_view->vadjustment->upper - text_view->vadjustment->page_size);

  if (y != text_view->yoffset)
    gtk_adjustment_set_value (text_view->vadjustment, y);

  vadjustment->adjustment_changed ();

  * The GTK resize loop processes all the pending exposes right
   * after doing the resize stuff, so the idle sizer won't have a
   * chance to run. So we do the work here. 
   *
  gtk_text_view_flush_first_validate (text_view);

  * widget->window doesn't get auto-redrawn as the layout is computed, so has to
   * be invalidated
   *
  if (size_changed && GTK_WIDGET_REALIZED (widget))
    gdk_window_invalidate_rect (widget->window, NULL, false);
}*/

void
TextView::get_first_para_iter ( TextIter *iter)
{
  buffer->get_iter_at_mark (iter,
                                    first_para_mark);
}

void
TextView::validate_onscreen (void)
{
  //GtkWidget *widget = GTK_WIDGET (text_view);
  
  //DV(g_print(">Validating onscreen ("G_STRLOC")\n"));
  
  if (Height() > 0)
    {
      TextIter first_para;

      /* Be sure we've validated the stuff onscreen; if we
       * scrolled, these calls won't have any effect, because
       * they were called in the recursive validate_onscreen
       */
      get_first_para_iter (&first_para);

      layout->validate_yrange (
                                       &first_para,
                                       0,
                                       first_para_pixels +
                                       Height());
    }

  onscreen_validated = true;

  //DV(g_print(">Done validating onscreen, onscreen_validated = true ("G_STRLOC")\n"));
  
  /* This can have the odd side effect of triggering a scroll, which should
   * flip "onscreen_validated" back to false, but should also get us
   * back into this function to turn it on again.
   */
  update_adjustments ();

  g_assert (onscreen_validated);
}

void
TextView::flush_first_validate (void)
{
  if (first_validate_idle == 0)
    return;

  /* Do this first, which means that if an "invalidate"
   * occurs during any of this process, a new first_validate_callback
   * will be installed, and we'll start again.
   */
  DV (g_print ("removing first validate in %s\n", G_STRLOC));
  g_source_remove (first_validate_idle);
  first_validate_idle = 0;
  
  /* be sure we have up-to-date screen size set on the
   * layout.
   */
  update_layout_width ();

  /* Bail out if we invalidated stuff; scrolling right away will just
   * confuse the issue.
   */
  if (first_validate_idle != 0)
    {
      DV(g_print(">Width change forced requeue ("G_STRLOC")\n"));
    }
  else
    {
      /* scroll to any marks, if that's pending. This can jump us to
       * the validation codepath used for scrolling onscreen, if so we
       * bail out.  It won't jump if already in that codepath since
       * value_changed is not recursive, so also validate if
       * necessary.
       */
      if (!flush_scroll () ||
          !onscreen_validated)
	validate_onscreen ();
      
      //DV(g_print(">Leaving first validate idle ("G_STRLOC")\n"));
      
      g_assert (onscreen_validated);
    }
}

//TODO find out where this function is used
bool
TextView::first_validate_callback (gpointer data)
{
  TextView *text_view = (TextView*)data;

  /* Note that some of this code is duplicated at the end of size_allocate,
   * keep in sync with that.
   */
  
  DV(g_print(G_STRLOC"\n"));

  flush_first_validate ();
  
  return false;
}

//TODO find out where this function is used
bool
TextView::incremental_validate_callback (gpointer data)
{
  TextView *text_view = (TextView*)data;
  bool result = true;

  //DV(g_print(G_STRLOC"\n"));
  
  layout->validate (2000); //TODO why 2000? in cim probably less is ok

  update_adjustments ();
  
  if (layout->is_valid ())
    {
      incremental_validate_idle = 0;
      result = false;
    }

  return result;
}


void
TextView::invalidate (void)
{  
  //DV (g_print (">Invalidate, onscreen_validated = %d now false ("G_STRLOC")\n",
   //            text_view->onscreen_validated));

  onscreen_validated = false;

  /* We'll invalidate when the layout is created */
  if (layout == NULL)
    return;
  
  if (!first_validate_idle)
    {
      //TODO first_validate_idle = gdk_threads_add_idle_full (GTK_PRIORITY_RESIZE - 2, first_validate_callback, text_view, NULL);
      //DV (g_print (G_STRLOC": adding first validate idle %d\n",
      //             text_view->first_validate_idle));
    }
      
  if (!incremental_validate_idle)
    {
      //TODO incremental_validate_idle = gdk_threads_add_idle_full (GTK_TEXT_VIEW_PRIORITY_VALIDATE, incremental_validate_callback, text_view, NULL);
      //DV (g_print (G_STRLOC": adding incremental validate idle %d\n",
      //             text_view->incremental_validate_idle));
    }
}

//TODO check where this is used
/*
void
TextView::invalidated_handler (GtkTextLayout *layout,
                     gpointer       data)
{
  TextView *text_view;

  text_view = (TextView*)data;

  //DV (g_print ("Invalidating due to layout invalidate signal\n"));
  invalidate ();
}*/

/*
void
TextView::changed_handler (TextLayout     *layout,
                 gint               start_y,
                 gint               old_height,
                 gint               new_height,
                 gpointer           data)
{
  TextView *text_view;
  GtkWidget *widget;
  GdkRect visible_rect;
  GdkRect redraw_rect;
  
  text_view = GTK_TEXT_VIEW (data);
  widget = GTK_WIDGET (data);
  
  DV(g_print(">Lines Validated ("G_STRLOC")\n"));

  if (GTK_WIDGET_REALIZED (text_view))
    {      
      gtk_text_view_get_visible_rect (text_view, &visible_rect);

      redraw_rect.x = visible_rect.x;
      redraw_rect.width = visible_rect.width;
      redraw_rect.y = start_y;

      if (old_height == new_height)
        redraw_rect.height = old_height;
      else if (start_y + old_height > visible_rect.y)
        redraw_rect.height = MAX (0, visible_rect.y + visible_rect.height - start_y);
      else
        redraw_rect.height = 0;
	
      if (gdk_rectangle_intersect (&redraw_rect, &visible_rect, &redraw_rect))
        {
          * text_window_invalidate_rect() takes buffer coordinates *
          text_window_invalidate_rect (text_view->text_window,
                                       &redraw_rect);

          DV(g_print(" invalidated rect: %d,%d %d x %d\n",
                     redraw_rect.x,
                     redraw_rect.y,
                     redraw_rect.width,
                     redraw_rect.height));
          
          if (text_view->left_window)
            text_window_invalidate_rect (text_view->left_window,
                                         &redraw_rect);
          if (text_view->right_window)
            text_window_invalidate_rect (text_view->right_window,
                                         &redraw_rect);
          if (text_view->top_window)
            text_window_invalidate_rect (text_view->top_window,
                                         &redraw_rect);
          if (text_view->bottom_window)
            text_window_invalidate_rect (text_view->bottom_window,
                                         &redraw_rect);

          queue_update_im_spot_location (text_view);
        }
    }
  
  if (old_height != new_height)
    {
      bool yoffset_changed = false;
      GSList *tmp_list;
      int new_first_para_top;
      int old_first_para_top;
      TextIter first;
      
      * If the bottom of the old area was above the top of the
       * screen, we need to scroll to keep the current top of the
       * screen in place.  Remember that first_para_pixels is the
       * position of the top of the screen in coordinates relative to
       * the first paragraph onscreen.
       *
       * In short we are adding the height delta of the portion of the
       * changed region above first_para_mark to text_view->yoffset.
       *
      buffer->get_iter_at_mark ( &first,
                                        text_view->first_para_mark);

      gtk_text_layout_get_line_yrange (layout, &first, &new_first_para_top, NULL);

      old_first_para_top = text_view->yoffset - text_view->first_para_pixels;

      if (new_first_para_top != old_first_para_top)
        {
          text_view->yoffset += new_first_para_top - old_first_para_top;
          
          get_vadjustment (text_view)->value = text_view->yoffset;
          yoffset_changed = true;
        }

      if (yoffset_changed)
        {
          DV(g_print ("Changing scroll position (%s)\n", G_STRLOC));
          gtk_adjustment_value_changed (get_vadjustment (text_view));
        }

      * FIXME be smarter about which anchored widgets we update *

      tmp_list = text_view->children;
      while (tmp_list != NULL)
        {
          TextViewChild *child = tmp_list->data;

          if (child->anchor)
            gtk_text_view_update_child_allocation (text_view, child);

          tmp_list = g_slist_next (tmp_list);
        }
    }

  {
    Rect old_req;
    Rect new_req;

    old_req = widget->requisition;

    * Use this instead of gtk_widget_size_request wrapper
     * to avoid the optimization which just returns widget->requisition
     * if a resize hasn't been queued.
     *
    GTK_WIDGET_GET_CLASS (widget)->size_request (widget, &new_req);

    if (old_req.width != new_req.width ||
        old_req.height != new_req.height)
      {
	gtk_widget_queue_resize_no_redraw (widget);
      }
  }
}*/

/*
void
TextView::realize (GtkWidget *widget)
{
  TextView *text_view;
  GdkWindowAttr attributes;
  gint attributes_mask;
  GSList *tmp_list;
  
  text_view = GTK_TEXT_VIEW (widget);
  GTK_WIDGET_SET_FLAGS (text_view, GTK_REALIZED);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);
  attributes.event_mask = GDK_VISIBILITY_NOTIFY_MASK | GDK_EXPOSURE_MASK;

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

  widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
                                   &attributes, attributes_mask);
  gdk_window_set_user_data (widget->window, widget);

  * must come before text_window_realize calls *
  widget->style = gtk_style_attach (widget->style, widget->window);

  gdk_window_set_background (widget->window,
                             &widget->style->bg[GTK_WIDGET_STATE (widget)]);

  text_window_realize (text_view->text_window, widget);

  if (text_view->left_window)
    text_window_realize (text_view->left_window, widget);

  if (text_view->top_window)
    text_window_realize (text_view->top_window, widget);

  if (text_view->right_window)
    text_window_realize (text_view->right_window, widget);

  if (text_view->bottom_window)
    text_window_realize (text_view->bottom_window, widget);

  ensure_layout();

  if (text_view->buffer)
    {
      GtkClipboard *clipboard = gtk_widget_get_clipboard (GTK_WIDGET (text_view),
							  GDK_SELECTION_PRIMARY);
      gtk_text_buffer_add_selection_clipboard (text_view->buffer, clipboard);
    }

  tmp_list = text_view->children;
  while (tmp_list != NULL)
    {
      TextViewChild *vc = tmp_list->data;
      
      text_view_child_set_parent_window (text_view, vc);
      
      tmp_list = tmp_list->next;
    }
}

void
TextView::unrealize (GtkWidget *widget)
{
  TextView *text_view;
  
  text_view = GTK_TEXT_VIEW (widget);

  if (text_view->buffer)
    {
      GtkClipboard *clipboard = gtk_widget_get_clipboard (GTK_WIDGET (text_view),
							  GDK_SELECTION_PRIMARY);
      gtk_text_buffer_remove_selection_clipboard (text_view->buffer, clipboard);
    }

  gtk_text_view_remove_validate_idles (text_view);

  if (text_view->popup_menu)
    {
      gtk_widget_destroy (text_view->popup_menu);
      text_view->popup_menu = NULL;
    }

  text_window_unrealize (text_view->text_window);

  if (text_view->left_window)
    text_window_unrealize (text_view->left_window);

  if (text_view->top_window)
    text_window_unrealize (text_view->top_window);

  if (text_view->right_window)
    text_window_unrealize (text_view->right_window);

  if (text_view->bottom_window)
    text_window_unrealize (text_view->bottom_window);

  gtk_text_view_destroy_layout (text_view);

  GTK_WIDGET_CLASS (gtk_text_view_parent_class)->unrealize (widget);
}*/

/*
void
TextView::set_background (TextView *text_view)
{
  GtkWidget *widget = GTK_WIDGET (text_view);

  gdk_window_set_background (widget->window,
			     &widget->style->bg[GTK_WIDGET_STATE (widget)]);
  
  gdk_window_set_background (text_view->text_window->bin_window,
			     &widget->style->base[GTK_WIDGET_STATE (widget)]);
  
  if (text_view->left_window)
    gdk_window_set_background (text_view->left_window->bin_window,
			       &widget->style->bg[GTK_WIDGET_STATE (widget)]);
  if (text_view->right_window)
    gdk_window_set_background (text_view->right_window->bin_window,
			       &widget->style->bg[GTK_WIDGET_STATE (widget)]);
  
  if (text_view->top_window)
    gdk_window_set_background (text_view->top_window->bin_window,
			       &widget->style->bg[GTK_WIDGET_STATE (widget)]);
  
  if (text_view->bottom_window)
    gdk_window_set_background (text_view->bottom_window->bin_window,
			       &widget->style->bg[GTK_WIDGET_STATE (widget)]);
}*/

/*
void
TextView::style_set (Style *new_style,
                         Style  *previous_style)
{
  //TextView *text_view = GTK_TEXT_VIEW (widget);
  //PangoContext *ltr_context, *rtl_context;

  //if (GTK_WIDGET_REALIZED (widget))
  //  {
  //    gtk_text_view_set_background (text_view);
  //  }

  if (layout && previous_style)
    {
      set_attributes_from_style (
                                               layout->default_style,
                                               new_style);
      
      
      //ltr_context = gtk_widget_create_pango_context (widget);
      //pango_context_set_base_dir (ltr_context, PANGO_DIRECTION_LTR);
      //rtl_context = gtk_widget_create_pango_context (widget);
      //pango_context_set_base_dir (rtl_context, PANGO_DIRECTION_RTL);

      //gtk_text_layout_set_contexts (text_view->layout, ltr_context, rtl_context);

      //g_object_unref (ltr_context);
      //g_object_unref (rtl_context);
    }
}*/

/*
void
TextView::direction_changed (GtkWidget        *widget,
                                 GtkTextDirection  previous_direction)
{
  TextView *text_view = GTK_TEXT_VIEW (widget);

  if (text_view->layout)
    {
      text_view->layout->default_style->direction = gtk_widget_get_direction (widget);

      gtk_text_layout_default_style_changed (text_view->layout);
    }
}*/

/*void
TextView::state_changed (GtkWidget      *widget,
		 	     GtkStateType    previous_state)
{
  TextView *text_view = GTK_TEXT_VIEW (widget);
  GdkCursor *cursor;

  if (GTK_WIDGET_REALIZED (widget))
    {
      gtk_text_view_set_background (text_view);

      if (GTK_WIDGET_IS_SENSITIVE (widget))
        cursor = gdk_cursor_new_for_display (gtk_widget_get_display (widget), GDK_XTERM);
      else
        cursor = NULL;

      gdk_window_set_cursor (text_view->text_window->bin_window, cursor);

      if (cursor)
      gdk_cursor_unref (cursor);

      text_view->mouse_cursor_obscured = false;
    }

  if (!GTK_WIDGET_IS_SENSITIVE (widget))
    {
      * Clear any selection *
      gtk_text_view_unselect (text_view);
    }
  
  gtk_widget_queue_draw (widget);
}*/

/*void
set_invisible_cursor (GdkWindow *window)
{
  GdkBitmap *empty_bitmap;
  GdkCursor *cursor;
  GdkColor useless;
  char invisible_cursor_bits[] = { 0x0 };	
	
  useless.red = useless.green = useless.blue = 0;
  useless.pixel = 0;
  
  empty_bitmap = gdk_bitmap_create_from_data (window,
					      invisible_cursor_bits,
					      1, 1);
  
  cursor = gdk_cursor_new_from_pixmap (empty_bitmap,
				       empty_bitmap,
				       &useless,
				       &useless, 0, 0);
  
  gdk_window_set_cursor (window, cursor);
  
  gdk_cursor_unref (cursor);
  
  g_object_unref (empty_bitmap);
}*/

/*void
TextView::obscure_mouse_cursor (TextView *text_view)
{
  if (text_view->mouse_cursor_obscured)
    return;

  set_invisible_cursor (text_view->text_window->bin_window);
  
  text_view->mouse_cursor_obscured = true;  
}*/

/*void
TextView::unobscure_mouse_cursor (TextView *text_view)
{
  if (text_view->mouse_cursor_obscured)
    {
      GdkCursor *cursor;
      
      cursor = gdk_cursor_new_for_display (gtk_widget_get_display (GTK_WIDGET (text_view)),
					   GDK_XTERM);
      gdk_window_set_cursor (text_view->text_window->bin_window, cursor);
      gdk_cursor_unref (cursor);
      text_view->mouse_cursor_obscured = false;
    }
}*/

/*void
TextView::grab_notify (GtkWidget *widget,
		 	   bool   was_grabbed)
{
  if (!was_grabbed)
    {
      gtk_text_view_end_selection_drag (GTK_TEXT_VIEW (widget));
      gtk_text_view_unobscure_mouse_cursor (GTK_TEXT_VIEW (widget));
    }
}*/


/*
 * Events
 */

/*
bool
get_event_coordinates (GdkEvent *event, gint *x, gint *y)
{
  if (event)
    switch (event->type)
      {
      case GDK_MOTION_NOTIFY:
        *x = event->motion.x;
        *y = event->motion.y;
        return true;
        break;

      case GDK_BUTTON_PRESS:
      case GDK_2BUTTON_PRESS:
      case GDK_3BUTTON_PRESS:
      case GDK_BUTTON_RELEASE:
        *x = event->button.x;
        *y = event->button.y;
        return true;
        break;

      case GDK_KEY_PRESS:
      case GDK_KEY_RELEASE:
      case GDK_ENTER_NOTIFY:
      case GDK_LEAVE_NOTIFY:
      case GDK_PROPERTY_NOTIFY:
      case GDK_SELECTION_CLEAR:
      case GDK_SELECTION_REQUEST:
      case GDK_SELECTION_NOTIFY:
      case GDK_PROXIMITY_IN:
      case GDK_PROXIMITY_OUT:
      case GDK_DRAG_ENTER:
      case GDK_DRAG_LEAVE:
      case GDK_DRAG_MOTION:
      case GDK_DRAG_STATUS:
      case GDK_DROP_START:
      case GDK_DROP_FINISHED:
      default:
        return false;
        break;
      }

  return false;
}

static gint
emit_event_on_tags (GtkWidget   *widget,
                    GdkEvent    *event,
                    TextIter *iter)
{
  GSList *tags;
  GSList *tmp;
  bool retval = false;

  tags = gtk_text_iter_get_tags (iter);

  tmp = tags;
  while (tmp != NULL)
    {
      GtkTextTag *tag = tmp->data;

      if (gtk_text_tag_event (tag, G_OBJECT (widget), event, iter))
        {
          retval = true;
          break;
        }

      tmp = g_slist_next (tmp);
    }

  g_slist_free (tags);

  return retval;
}*/

/*
static gint
TextView::event (GtkWidget *widget, GdkEvent *event)
{
  TextView *text_view;
  gint x = 0, y = 0;

  text_view = GTK_TEXT_VIEW (widget);

  if (text_view->layout == NULL ||
      buffer == NULL)
    return false;

  if (event->any.window != text_view->text_window->bin_window)
    return false;

  if (get_event_coordinates (event, &x, &y))
    {
      TextIter iter;

      x += text_view->xoffset;
      y += text_view->yoffset;

      * FIXME this is slow and we do it twice per event.
       * My favorite solution is to have GtkTextLayout cache
       * the last couple lookups.
       *
      layout->get_iter_at_pixel (
                                         &iter,
                                         x, y);

      return emit_event_on_tags (widget, event, &iter);
    }
  else if (event->type == GDK_KEY_PRESS ||
           event->type == GDK_KEY_RELEASE)
    {
      TextIter iter;

      buffer->get_iter_at_mark ( &iter,
                                        buffer->get_insert ());

      return emit_event_on_tags (widget, event, &iter);
    }
  else
    return false;
}*/

/*TODO this function should be ported as bindings
static gint
TextView::key_press_event (GtkWidget *widget, GdkEventKey *event)
{
  bool retval = false;
  bool obscure = false;
  TextView *text_view = GTK_TEXT_VIEW (widget);
  TextMark *insert;
  TextIter iter;
  bool can_insert;
  
  if (text_view->layout == NULL ||
      buffer == NULL)
    return false;

  * Make sure input method knows where it is *
  flush_update_im_spot_location (text_view);

  insert = buffer->get_insert ();
  buffer->get_iter_at_mark ( &iter, insert);
  can_insert = gtk_text_iter_can_insert (&iter, text_view->editable);
  if (gtk_im_context_filter_keypress (text_view->im_context, event))
    {
      text_view->need_im_reset = true;
      if (!can_insert)
        gtk_text_view_reset_im_context (text_view);
      obscure = can_insert;
      retval = true;
    }
  * Binding set *
  else if (GTK_WIDGET_CLASS (gtk_text_view_parent_class)->key_press_event (widget, event))
    {
      retval = true;
    }
  * use overall editability not can_insert, more predictable for users *
  else if (text_view->editable &&
           (event->keyval == GDK_Return ||
            event->keyval == GDK_ISO_Enter ||
            event->keyval == GDK_KP_Enter))
    {
      * this won't actually insert the newline if the cursor isn't
       * editable
       *
      gtk_text_view_reset_im_context (text_view);
      gtk_text_view_commit_text (text_view, "\n");

      obscure = true;
      retval = true;
    }
  * Pass through Tab as literal tab, unless Control is held down *
  else if ((event->keyval == GDK_Tab ||
            event->keyval == GDK_KP_Tab ||
            event->keyval == GDK_ISO_Left_Tab) &&
           !(event->state & GDK_CONTROL_MASK))
    {
      * If the text widget isn't editable overall, or if the application
       * has turned off "accepts_tab", move the focus instead
       *
      if (text_view->accepts_tab && text_view->editable)
	{
	  gtk_text_view_reset_im_context (text_view);
	  gtk_text_view_commit_text (text_view, "\t");
	  obscure = true;
	}
      else
	g_signal_emit_by_name (text_view, "move-focus",
                               (event->state & GDK_SHIFT_MASK) ?
                               GTK_DIR_TAB_BACKWARD : GTK_DIR_TAB_FORWARD);

      retval = true;
    }
  else
    retval = false;

  if (obscure)
    gtk_text_view_obscure_mouse_cursor (text_view);
  
  gtk_text_view_reset_blink_time (text_view);
  gtk_text_view_pend_cursor_blink (text_view);

  return retval;
}*/

/*
static gint
TextView::key_release_event (GtkWidget *widget, GdkEventKey *event)
{
  TextView *text_view = GTK_TEXT_VIEW (widget);
  TextMark *insert;
  TextIter iter;

  if (text_view->layout == NULL || buffer == NULL)
    return false;
      
  insert = buffer->get_insert ();
  buffer->get_iter_at_mark ( &iter, insert);
  if (gtk_text_iter_can_insert (&iter, text_view->editable) &&
      gtk_im_context_filter_keypress (text_view->im_context, event))
    {
      text_view->need_im_reset = true;
      return true;
    }
  else
    return GTK_WIDGET_CLASS (gtk_text_view_parent_class)->key_release_event (widget, event);
}

static gint
TextView::button_press_event (GtkWidget *widget, GdkEventButton *event)
{
  TextView *text_view;

  text_view = GTK_TEXT_VIEW (widget);

  gtk_widget_grab_focus (widget);

  if (event->window != text_view->text_window->bin_window)
    {
      * Remove selection if any. *
      gtk_text_view_unselect (text_view);
      return false;
    }

  gtk_text_view_reset_blink_time (text_view);

#if 0
  * debug hack *
  if (event->button == 3 && (event->state & GDK_CONTROL_MASK) != 0)
    _gtk_text_buffer_spew (GTK_TEXT_VIEW (widget)->buffer);
  else if (event->button == 3)
    gtk_text_layout_spew (GTK_TEXT_VIEW (widget)->layout);
#endif

  if (event->type == GDK_BUTTON_PRESS)
    {
      gtk_text_view_reset_im_context (text_view);

      if (event->button == 1)
        {
          * If we're in the selection, start a drag copy/move of the
           * selection; otherwise, start creating a new selection.
           *
          TextIter iter;
          TextIter start, end;

          layout->get_iter_at_pixel (
                                             &iter,
                                             event->x + text_view->xoffset,
                                             event->y + text_view->yoffset);

          if (gtk_text_buffer_get_selection_bounds (buffer,
                                                    &start, &end) &&
              gtk_text_iter_in_range (&iter, &start, &end))
            {
              text_view->drag_start_x = event->x;
              text_view->drag_start_y = event->y;
              text_view->pending_place_cursor_button = event->button;
            }
          else
            {
              gtk_text_view_start_selection_drag (text_view, &iter, event);
            }

          return true;
        }
      else if (event->button == 2)
        {
          TextIter iter;

          layout->get_iter_at_pixel (
                                             &iter,
                                             event->x + text_view->xoffset,
                                             event->y + text_view->yoffset);

          gtk_text_buffer_paste_clipboard (buffer,
					   gtk_widget_get_clipboard (widget, GDK_SELECTION_PRIMARY),
					   &iter,
					   text_view->editable);
          return true;
        }
      else if (event->button == 3)
        {
	  gtk_text_view_do_popup (text_view, event);
	  return true;
        }
    }
  else if ((event->type == GDK_2BUTTON_PRESS ||
	    event->type == GDK_3BUTTON_PRESS) &&
	   event->button == 1) 
    {
      TextIter iter;

      gtk_text_view_end_selection_drag (text_view);

      layout->get_iter_at_pixel (
					 &iter,
					 event->x + text_view->xoffset,
					 event->y + text_view->yoffset);
      
      gtk_text_view_start_selection_drag (text_view, &iter, event);
      return true;
    }
  
  return false;
}

static gint
TextView::button_release_event (GtkWidget *widget, GdkEventButton *event)
{
  TextView *text_view;

  text_view = GTK_TEXT_VIEW (widget);

  if (event->window != text_view->text_window->bin_window)
    return false;

  if (event->button == 1)
    {
      if (text_view->drag_start_x >= 0)
        {
          text_view->drag_start_x = -1;
          text_view->drag_start_y = -1;
        }

      if (gtk_text_view_end_selection_drag (GTK_TEXT_VIEW (widget)))
        return true;
      else if (text_view->pending_place_cursor_button == event->button)
        {
	  TextIter iter;

          * Unselect everything; we clicked inside selection, but
           * didn't move by the drag threshold, so just clear selection
           * and place cursor.
           *
	  layout->get_iter_at_pixel (
					     &iter,
					     event->x + text_view->xoffset,
					     event->y + text_view->yoffset);

	  gtk_text_buffer_place_cursor (buffer, &iter);
	  gtk_text_view_check_cursor_blink (text_view);
	  
          text_view->pending_place_cursor_button = 0;
          
          return false;
        }
    }

  return false;
}*/

/*static void
keymap_direction_changed (GdkKeymap   *keymap,
			  TextView *text_view)
{
  gtk_text_view_check_keymap_direction (text_view);
}*/

/*TODO this should overload some on_focus handler for the widget 
static gint
TextView::focus_in_event (GtkWidget *widget, GdkEventFocus *event)
{
  TextView *text_view = GTK_TEXT_VIEW (widget);

  gtk_widget_queue_draw (widget);

  DV(g_print (G_STRLOC": focus_in_event\n"));

  gtk_text_view_reset_blink_time (text_view);

  if (text_view->cursor_visible && text_view->layout)
    {
      gtk_text_layout_set_cursor_visible (text_view->layout, true);
      gtk_text_view_check_cursor_blink (text_view);
    }

  g_signal_connect (gdk_keymap_get_for_display (gtk_widget_get_display (widget)),
		    "direction-changed",
		    G_CALLBACK (keymap_direction_changed), text_view);
  gtk_text_view_check_keymap_direction (text_view);

  if (text_view->editable)
    {
      text_view->need_im_reset = true;
      gtk_im_context_focus_in (GTK_TEXT_VIEW (widget)->im_context);
    }

  return false;
}

static gint
TextView::focus_out_event (GtkWidget *widget, GdkEventFocus *event)
{
  TextView *text_view = GTK_TEXT_VIEW (widget);

  gtk_text_view_end_selection_drag (text_view);

  gtk_widget_queue_draw (widget);

  DV(g_print (G_STRLOC": focus_out_event\n"));
  
  if (text_view->cursor_visible && text_view->layout)
    {
      gtk_text_view_check_cursor_blink (text_view);
      gtk_text_layout_set_cursor_visible (text_view->layout, false);
    }

  g_signal_handlers_disconnect_by_func (gdk_keymap_get_for_display (gtk_widget_get_display (widget)),
					keymap_direction_changed,
					text_view);

  if (text_view->editable)
    {
      text_view->need_im_reset = true;
      gtk_im_context_focus_out (GTK_TEXT_VIEW (widget)->im_context);
    }

  return false;
}*/

/*static gint
TextView::motion_event (GtkWidget *widget, GdkEventMotion *event)
{
  TextView *text_view = GTK_TEXT_VIEW (widget);

  gtk_text_view_unobscure_mouse_cursor (text_view);

  if (event->window == text_view->text_window->bin_window &&
      text_view->drag_start_x >= 0)
    {
      gint x = event->x;
      gint y = event->y;

      gdk_event_request_motions (event);

      if (gtk_drag_check_threshold (widget,
				    text_view->drag_start_x, 
				    text_view->drag_start_y,
				    x, y))
        {
          TextIter iter;
          gint buffer_x, buffer_y;

          gtk_text_view_window_to_buffer_coords (text_view,
                                                 GTK_TEXT_WINDOW_TEXT,
                                                 text_view->drag_start_x,
                                                 text_view->drag_start_y,
                                                 &buffer_x,
                                                 &buffer_y);

          layout->get_iter_at_pixel (
                                             &iter,
                                             buffer_x, buffer_y);

          gtk_text_view_start_selection_dnd (text_view, &iter, event);
          return true;
        }
    }

  return false;
}*/

/*TODO this should be the handler for Redraw()
static void
TextView::paint (GtkWidget      *widget,
                     GdkRect   *area,
                     GdkEventExpose *event)
{
  TextView *text_view;
  GList *child_exposes;
  GList *tmp_list;
  GdkRegion *updates;
  
  //text_view = GTK_TEXT_VIEW (widget);

  g_return_if_fail (layout != NULL);
  g_return_if_fail (xoffset >= 0);
  g_return_if_fail (yoffset >= 0);

  while (first_validate_idle != 0)
    {
      DV (g_print (G_STRLOC": first_validate_idle: %d\n",
                   text_view->first_validate_idle));
      gtk_text_view_flush_first_validate (text_view);
    }

  * More regions could have become invalid in the above loop *
  updates = gdk_window_get_update_area (text_view->text_window->bin_window);
  if (updates)
    {
      GdkRect rect;
      
      gdk_region_get_clipbox (updates, &rect);

      gdk_rectangle_union (area, &rect, area);
      
      gdk_region_destroy (updates);
    }
  
  if (!onscreen_validated)
    {
      g_warning (G_STRLOC ": somehow some text lines were modified or scrolling occurred since the last validation of lines on the screen - may be a text widget bug.");
      g_assert_not_reached ();
    }
  
#if 0
  printf ("painting %d,%d  %d x %d\n",
          area->x, area->y,
          area->width, area->height);
#endif

  child_exposes = NULL;
  gtk_text_layout_draw (text_view->layout,
                        widget,
                        text_view->text_window->bin_window,
			NULL,
                        text_view->xoffset,
                        text_view->yoffset,
                        area->x, area->y,
                        area->width, area->height,
                        &child_exposes);

  tmp_list = child_exposes;
  while (tmp_list != NULL)
    {
      GtkWidget *child = tmp_list->data;
  
      gtk_container_propagate_expose (GTK_CONTAINER (text_view),
                                      child,
                                      event);

      g_object_unref (child);
      
      tmp_list = tmp_list->next;
    }

  g_list_free (child_exposes);
}*/

/*
gint
TextView::expose_event (GtkWidget *widget, GdkEventExpose *event)
{
  GSList *tmp_list;
  
  if (event->window == gtk_text_view_get_window (GTK_TEXT_VIEW (widget),
                                                 GTK_TEXT_WINDOW_TEXT))
    {
      DV(g_print (">Exposed ("G_STRLOC")\n"));
      gtk_text_view_paint (widget, &event->area, event);
    }

  if (event->window == widget->window)
    gtk_text_view_draw_focus (widget);

  * Propagate exposes to all unanchored children. 
   * Anchored children are handled in gtk_text_view_paint(). 
   *
  tmp_list = GTK_TEXT_VIEW (widget)->children;
  while (tmp_list != NULL)
    {
      TextViewChild *vc = tmp_list->data;

      * propagate_expose checks that event->window matches
       * child->window
       *
      if (!vc->anchor)
        gtk_container_propagate_expose (GTK_CONTAINER (widget),
                                        vc->widget,
                                        event);
      
      tmp_list = tmp_list->next;
    }
  
  return false;
}*/

/*
static void
TextView::draw_focus (GtkWidget *widget)
{
  bool interior_focus;

  * We clear the focus if we are in interior focus mode. *
  gtk_widget_style_get (widget,
			"interior-focus", &interior_focus,
			NULL);
  
  if (GTK_WIDGET_DRAWABLE (widget))
    {
      if (GTK_WIDGET_HAS_FOCUS (widget) && !interior_focus)
        {          
          gtk_paint_focus (widget->style, widget->window, GTK_WIDGET_STATE (widget), 
                           NULL, widget, "textview",
                           0, 0,
                           widget->allocation.width,
                           widget->allocation.height);
        }
      else
        {
          gdk_window_clear (widget->window);
        }
    }
}*/

/*TODO handler for grabbing focus?
bool
TextView::focus (GtkWidget        *widget,
                     GtkDirectionType  direction)
{
  GtkContainer *container;
  bool result;
  
  container = GTK_CONTAINER (widget);  

  if (!gtk_widget_is_focus (widget) &&
      container->focus_child == NULL)
    {
      gtk_widget_grab_focus (widget);
      return true;
    }
  else
    {
      *
       * Unset CAN_FOCUS flag so that gtk_container_focus() allows
       * children to get the focus
       *
      GTK_WIDGET_UNSET_FLAGS (widget, GTK_CAN_FOCUS); 
      result = GTK_WIDGET_CLASS (gtk_text_view_parent_class)->focus (widget, direction);
      GTK_WIDGET_SET_FLAGS (widget, GTK_CAN_FOCUS); 

      return result;
    }
}*/

/*
static void
TextView::move_focus (GtkWidget        *widget,
                          GtkDirectionType  direction_type)
{
  TextView *text_view = GTK_TEXT_VIEW (widget);

  if (GTK_TEXT_VIEW_GET_CLASS (text_view)->move_focus)
    GTK_TEXT_VIEW_GET_CLASS (text_view)->move_focus (text_view,
                                                     direction_type);
}*/

/*
 * Container
 */

/*static void
TextView::add (GtkContainer *container,
                   GtkWidget    *child)
{
  * This is pretty random. *
  gtk_text_view_add_child_in_window (GTK_TEXT_VIEW (container),
                                     child,
                                     GTK_TEXT_WINDOW_WIDGET,
                                     0, 0);
}

static void
TextView::remove (GtkContainer *container,
                      GtkWidget    *child)
{
  GSList *iter;
  TextView *text_view;
  TextViewChild *vc;

  text_view = GTK_TEXT_VIEW (container);

  vc = NULL;
  iter = text_view->children;

  while (iter != NULL)
    {
      vc = iter->data;

      if (vc->widget == child)
        break;

      iter = g_slist_next (iter);
    }

  g_assert (iter != NULL); * be sure we had the child in the list *

  text_view->children = g_slist_remove (text_view->children, vc);

  gtk_widget_unparent (vc->widget);

  text_view_child_free (vc);
}*/

/*static void
TextView::forall (GtkContainer *container,
                      bool      include_internals,
                      GtkCallback   callback,
                      gpointer      callback_data)
{
  GSList *iter;
  TextView *text_view;
  GSList *copy;

  g_return_if_fail (GTK_IS_TEXT_VIEW (container));
  g_return_if_fail (callback != NULL);

  text_view = GTK_TEXT_VIEW (container);

  copy = g_slist_copy (text_view->children);
  iter = copy;

  while (iter != NULL)
    {
      TextViewChild *vc = iter->data;

      (* callback) (vc->widget, callback_data);

      iter = g_slist_next (iter);
    }

  g_slist_free (copy);
}*/

#define CURSOR_ON_MULTIPLIER 2
#define CURSOR_OFF_MULTIPLIER 1
#define CURSOR_PEND_MULTIPLIER 3
#define CURSOR_DIVIDER 3

/*
static bool
cursor_blinks (TextView *text_view)
{
  GtkSettings *settings = gtk_widget_get_settings (GTK_WIDGET (text_view));
  bool blink;

#ifdef DEBUG_VALIDATION_AND_SCROLLING
  return false;
#endif
  if (gtk_debug_flags & GTK_DEBUG_UPDATES)
    return false;

  g_object_get (settings, "gtk-cursor-blink", &blink, NULL);

  if (!blink)
    return false;

  if (text_view->editable)
    {
      TextMark *insert;
      TextIter iter;
      
      insert = buffer->get_insert ();
      buffer->get_iter_at_mark ( &iter, insert);
      
      if (gtk_text_iter_editable (&iter, text_view->editable))
	return blink;
    }

  return false;
}*/

/*static gint
get_cursor_time (TextView *text_view)
{
  GtkSettings *settings = gtk_widget_get_settings (GTK_WIDGET (text_view));
  gint time;

  g_object_get (settings, "gtk-cursor-blink-time", &time, NULL);

  return time;
}*/

/*static gint
get_cursor_blink_timeout (TextView *text_view)
{
  GtkSettings *settings = gtk_widget_get_settings (GTK_WIDGET (text_view));
  gint time;

  g_object_get (settings, "gtk-cursor-blink-timeout", &time, NULL);

  return time;
}*/


/*
 * Blink!
 */

/*static gint
blink_cb (gpointer data)
{
  TextView *text_view;
  TextViewPrivate *priv;
  bool visible;
  gint blink_timeout;

  text_view = GTK_TEXT_VIEW (data);
  priv = GTK_TEXT_VIEW_GET_PRIVATE (text_view);

  if (!GTK_WIDGET_HAS_FOCUS (text_view))
    {
      g_warning ("TextView - did not receive focus-out-event. If you\n"
                 "connect a handler to this signal, it must return\n"
                 "false so the text view gets the event as well");

      gtk_text_view_check_cursor_blink (text_view);

      return false;
    }

  g_assert (text_view->layout);
  g_assert (text_view->cursor_visible);

  visible = gtk_text_layout_get_cursor_visible (text_view->layout);

  blink_timeout = get_cursor_blink_timeout (text_view);
  if (priv->blink_time > 1000 * blink_timeout &&
      blink_timeout < G_MAXINT/1000) 
    {
      * we've blinked enough without the user doing anything, stop blinking *
      visible = 0;
      text_view->blink_timeout = 0;
    } 
  else if (visible)
    text_view->blink_timeout = gdk_threads_add_timeout (get_cursor_time (text_view) * CURSOR_OFF_MULTIPLIER / CURSOR_DIVIDER,
					      blink_cb,
					      text_view);
  else 
    {
      text_view->blink_timeout = gdk_threads_add_timeout (get_cursor_time (text_view) * CURSOR_ON_MULTIPLIER / CURSOR_DIVIDER,
						blink_cb,
						text_view);
      priv->blink_time += get_cursor_time (text_view);
    }

  * Block changed_handler while changing the layout's cursor visibility
   * because it would expose the whole paragraph. Instead, we expose
   * the cursor's area(s) manually below.
   *
  g_signal_handlers_block_by_func (text_view->layout,
                                   changed_handler,
                                   text_view);
  gtk_text_layout_set_cursor_visible (text_view->layout, !visible);
  g_signal_handlers_unblock_by_func (text_view->layout,
                                     changed_handler,
                                     text_view);

  text_window_invalidate_cursors (text_view->text_window);

  * Remove ourselves *
  return false;
}*/


/*static void
TextView::stop_cursor_blink (TextView *text_view)
{
  if (text_view->blink_timeout)  
    { 
      g_source_remove (text_view->blink_timeout);
      text_view->blink_timeout = 0;
    }
}

static void
TextView::check_cursor_blink (TextView *text_view)
{
  if (text_view->layout != NULL &&
      text_view->cursor_visible &&
      GTK_WIDGET_HAS_FOCUS (text_view))
    {
      if (cursor_blinks (text_view))
	{
	  if (text_view->blink_timeout == 0)
	    {
	      gtk_text_layout_set_cursor_visible (text_view->layout, true);
	      
	      text_view->blink_timeout = gdk_threads_add_timeout (get_cursor_time (text_view) * CURSOR_OFF_MULTIPLIER / CURSOR_DIVIDER,
							blink_cb,
							text_view);
	    }
	}
      else
	{
	  gtk_text_view_stop_cursor_blink (text_view);
	  gtk_text_layout_set_cursor_visible (text_view->layout, true);
	}
    }
  else
    {
      gtk_text_view_stop_cursor_blink (text_view);
      gtk_text_layout_set_cursor_visible (text_view->layout, false);
    }
}

static void
TextView::pend_cursor_blink (TextView *text_view)
{
  if (text_view->layout != NULL &&
      text_view->cursor_visible &&
      GTK_WIDGET_HAS_FOCUS (text_view) &&
      cursor_blinks (text_view))
    {
      gtk_text_view_stop_cursor_blink (text_view);
      gtk_text_layout_set_cursor_visible (text_view->layout, true);
      
      text_view->blink_timeout = gdk_threads_add_timeout (get_cursor_time (text_view) * CURSOR_PEND_MULTIPLIER / CURSOR_DIVIDER,
						blink_cb,
						text_view);
    }
}

static void
TextView::reset_blink_time (TextView *text_view)
{
  TextViewPrivate *priv;

  priv = GTK_TEXT_VIEW_GET_PRIVATE (text_view);

  priv->blink_time = 0;
}*/


/*
 * Key binding handlers
 */

bool TextView::move_iter_by_lines ( TextIter *newplace, gint         count)
{
  bool ret = true;

  while (count < 0)
    {
      ret = layout->move_iter_to_previous_line (newplace);
      count++;
    }

  while (count > 0)
    {
      ret = layout->move_iter_to_next_line (newplace);
      count--;
    }

  return ret;
}

void TextView::some_move_cursor ( TextIter *new_location, bool           extend_selection)
{
  if (extend_selection)
    buffer->move_mark_by_name ( "insert", new_location);
  else
      buffer->place_cursor ( new_location);
  //check_cursor_blink (text_view);
}

void
TextView::move_cursor_internal (
                                    CursorMovement  step,
                                    gint             count,
                                    bool         extend_selection)
{
  TextIter insert;
  TextIter newplace;
  gint cursor_x_pos = 0;
  DirectionType leave_direction = DIR_NONE;

  if (!cursor_visible) 
    {
      ScrollStep scroll_step;

      switch (step) 
	{
        case MOVE_VISUAL_POSITIONS:
          leave_direction = count > 0 ? DIR_RIGHT : DIR_LEFT;
          /* fall through */
        case MOVE_LOGICAL_POSITIONS:
        case MOVE_WORDS:
	  scroll_step = SCROLL_HORIZONTAL_STEPS;
	  break;
        case MOVE_DISPLAY_LINE_ENDS:
	  scroll_step = SCROLL_HORIZONTAL_ENDS;
	  break;	  
        case MOVE_DISPLAY_LINES:
          leave_direction = count > 0 ? DIR_DOWN : DIR_UP;
          /* fall through */
        case MOVE_PARAGRAPHS:
        case MOVE_PARAGRAPH_ENDS:
	  scroll_step = SCROLL_STEPS;
	  break;
	case MOVE_PAGES:
	  scroll_step = SCROLL_PAGES;
	  break;
	case MOVE_HORIZONTAL_PAGES:
	  scroll_step = SCROLL_HORIZONTAL_PAGES;
	  break;
	case MOVE_BUFFER_ENDS:
	  scroll_step = SCROLL_ENDS;
	  break;
	default:
          scroll_step = SCROLL_PAGES;
          break;
	}

      if (!move_viewport (scroll_step, count))
        {
          if (leave_direction != DIR_NONE /*&&
              !gtk_widget_keynav_failed (GTK_WIDGET (text_view),
                                         leave_direction)*/)
            {
	      //TODO move focus to some other widget
              //g_signal_emit_by_name (text_view, "move-focus", leave_direction);
            }
        }

      return;
    }

  //gtk_text_view_reset_im_context (text_view);

  if (step == MOVE_PAGES)
    {
      if (!scroll_pages (count, extend_selection))
	     ;//TODO emit beep? also do this for all the error_bell calls
        //gtk_widget_error_bell (GTK_WIDGET (text_view));

      //check_cursor_blink ();
      //pend_cursor_blink ();
      return;
    }
  else if (step == MOVE_HORIZONTAL_PAGES)
    {
      if (!scroll_hpages (count, extend_selection))
        ;//gtk_widget_error_bell (GTK_WIDGET (text_view));

      //gtk_text_view_check_cursor_blink (text_view);
      //gtk_text_view_pend_cursor_blink (text_view);
      return;
    }

  buffer->get_iter_at_mark (&insert,
                                    buffer->get_insert ());
  newplace = insert;

  if (step == MOVE_DISPLAY_LINES)
    get_virtual_cursor_pos (&cursor_x_pos, NULL);

  switch (step)
    {
    case MOVE_LOGICAL_POSITIONS:
      newplace.forward_visible_cursor_positions (count);
      break;

    case MOVE_VISUAL_POSITIONS:
      layout->move_iter_visually ( &newplace, count);
      break;

    case MOVE_WORDS:
      if (count < 0)
        newplace.backward_visible_word_starts (-count);
      else if (count > 0) 
	{
	  if (!newplace.forward_visible_word_ends (count))
	    newplace.forward_to_line_end ();
	}
      break;

    case MOVE_DISPLAY_LINES:
      if (count < 0)
        {
          leave_direction = DIR_UP;

          if (move_iter_by_lines (&newplace, count))
            layout->move_iter_to_x (&newplace, cursor_x_pos);
          else
            newplace.set_line_offset (0);
        }
      if (count > 0)
        {
          leave_direction = DIR_DOWN;

          if (move_iter_by_lines ( &newplace, count))
            layout->move_iter_to_x (&newplace, cursor_x_pos);
          else
            newplace.forward_to_line_end ();
        }
      break;

    case MOVE_DISPLAY_LINE_ENDS:
      if (count > 1)
        move_iter_by_lines ( &newplace, --count);
      else if (count < -1)
        move_iter_by_lines ( &newplace, ++count);

      if (count != 0)
        layout->move_iter_to_line_end (&newplace, count);
      break;

    case MOVE_PARAGRAPHS:
      if (count > 0)
        {
          if (!newplace.ends_line ())
            {
              newplace.forward_to_line_end ();
              --count;
            }
          newplace.forward_visible_lines (count);
          newplace.forward_to_line_end ();
        }
      else if (count < 0)
        {
          if (newplace.get_line_offset () > 0)
	    newplace.set_line_offset (0);
          newplace.forward_visible_lines (count);
          newplace.set_line_offset (0);
        }
      break;

    case MOVE_PARAGRAPH_ENDS:
      if (count > 0)
        {
          if (!newplace.ends_line ())
            newplace.forward_to_line_end ();
        }
      else if (count < 0)
        {
          newplace.set_line_offset (0);
        }
      break;

    case MOVE_BUFFER_ENDS:
      if (count > 0)
        buffer->get_end_iter (&newplace);
      else if (count < 0)
        buffer->get_iter_at_offset (&newplace, 0);
     break;
      
    default:
      break;
    }

  /* call move_cursor() even if the cursor hasn't moved, since it 
     cancels the selection
  */
  some_move_cursor (&newplace, extend_selection);

  if (!TextIter::equal (&insert, &newplace))
    {
      //DV(g_print (G_STRLOC": scrolling onscreen\n"));
      scroll_mark_onscreen (
                                          buffer->get_insert ());

      if (step == MOVE_DISPLAY_LINES)
        set_virtual_cursor_pos (cursor_x_pos, -1);
    }
  else if (leave_direction != -1)
    {
      /*TODO ?
      if (!gtk_widget_keynav_failed (GTK_WIDGET (text_view),
                                     leave_direction))
        {
          g_signal_emit_by_name (text_view, "move-focus", leave_direction);
        }*/
    }
  else
    {
	     //TODO emit beep! (or not)
      //gtk_widget_error_bell (GTK_WIDGET (text_view));
    }

  //gtk_text_view_check_cursor_blink (text_view);
  //gtk_text_view_pend_cursor_blink (text_view);
}

void
TextView::move_cursor (
                           CursorMovement  step,
                           gint             count,
                           bool         extend_selection)
{
  move_cursor_internal (step, count, extend_selection);
}

void
TextView::page_horizontally (
                                 gint             count,
                                 bool         extend_selection)
{
  move_cursor_internal (MOVE_HORIZONTAL_PAGES,
                                      count, extend_selection);
}


bool
TextView::move_viewport ( ScrollStep    step, gint             count)
{
  Adjustment *adjustment;
  gdouble increment;
  
  switch (step) 
    {
    case SCROLL_STEPS:
    case SCROLL_PAGES:
    case SCROLL_ENDS:
      adjustment = get_vadjustment ();
      break;
    case SCROLL_HORIZONTAL_STEPS:
    case SCROLL_HORIZONTAL_PAGES:
    case SCROLL_HORIZONTAL_ENDS:
      adjustment = get_hadjustment ();
      break;
    default:
      adjustment = get_vadjustment ();
      break;
    }

  switch (step) 
    {
    case SCROLL_STEPS:
    case SCROLL_HORIZONTAL_STEPS:
      increment = adjustment->step_increment;
      break;
    case SCROLL_PAGES:
    case SCROLL_HORIZONTAL_PAGES:
      increment = adjustment->page_increment;
      break;
    case SCROLL_ENDS:
    case SCROLL_HORIZONTAL_ENDS:
      increment = adjustment->upper - adjustment->lower;
      break;
    default:
      increment = 0.0;
      break;
    }

  return set_adjustment_clamped (adjustment, adjustment->value + count * increment);
}

void TextView::set_anchor (void)
{
  TextIter insert;

  buffer->get_iter_at_mark ( &insert,
                                    buffer->get_insert ());

  buffer->create_mark ("anchor", &insert, true);
}

bool
TextView::scroll_pages (
                            gint         count,
                            bool     extend_selection)
{
  gdouble newval;
  gdouble oldval;
  Adjustment *adj;
  gint cursor_x_pos, cursor_y_pos;
  TextMark *insert_mark;
  TextIter old_insert;
  TextIter new_insert;
  TextIter anchor;
  gint y0, y1;

  g_return_val_if_fail (vadjustment != NULL, false);
  
  adj = vadjustment;

  insert_mark = buffer->get_insert ();

  /* Make sure we start from the current cursor position, even
   * if it was offscreen, but don't queue more scrolls if we're
   * already behind.
   */
  if (pending_scroll)
    cancel_pending_scroll ();
  else
    scroll_mark_onscreen (insert_mark);

  /* Validate the region that will be brought into view by the cursor motion
   */
  buffer->get_iter_at_mark (
                                    &old_insert, insert_mark);

  if (count < 0)
    {
      get_first_para_iter (&anchor);
      y0 = adj->page_size;
      y1 = adj->page_size + count * adj->page_increment;
    }
  else
    {
      get_first_para_iter (&anchor);
      y0 = count * adj->page_increment + adj->page_size;
      y1 = 0;
    }

  layout->validate_yrange (&anchor, y0, y1);
  /* FIXME do we need to update the adjustment ranges here? */

  new_insert = old_insert;

  if (count < 0 && adj->value <= (adj->lower + 1e-12))
    {
      /* already at top, just be sure we are at offset 0 */
      buffer->get_start_iter (&new_insert);
      some_move_cursor (&new_insert, extend_selection);
    }
  else if (count > 0 && adj->value >= (adj->upper - adj->page_size - 1e-12))
    {
      /* already at bottom, just be sure we are at the end */
      buffer->get_end_iter (&new_insert);
      some_move_cursor (&new_insert, extend_selection);
    }
  else
    {
      get_virtual_cursor_pos (&cursor_x_pos, &cursor_y_pos);

      oldval = adj->value;
      newval = adj->value;

      newval += count * adj->page_increment;

      set_adjustment_clamped (adj, newval);
      cursor_y_pos += adj->value - oldval;

      layout->get_iter_at_pixel ( &new_insert, cursor_x_pos, cursor_y_pos);
      clamp_iter_onscreen (&new_insert);
      some_move_cursor (&new_insert, extend_selection);

      set_virtual_cursor_pos (cursor_x_pos, cursor_y_pos);
    }
  
  /* Adjust to have the cursor _entirely_ onscreen, move_mark_onscreen
   * only guarantees 1 pixel onscreen.
   */
  DV(g_print (G_STRLOC": scrolling onscreen\n"));
  scroll_mark_onscreen (insert_mark);

  return !TextIter::equal (&old_insert, &new_insert);
}

bool
TextView::scroll_hpages (
                             gint         count,
                             bool     extend_selection)
{
  gdouble newval;
  gdouble oldval;
  Adjustment *adj;
  gint cursor_x_pos, cursor_y_pos;
  TextMark *insert_mark;
  TextIter old_insert;
  TextIter new_insert;
  gint y, height;
  
  g_return_val_if_fail (hadjustment != NULL, false);

  adj = hadjustment;

  insert_mark = buffer->get_insert ();

  /* Make sure we start from the current cursor position, even
   * if it was offscreen, but don't queue more scrolls if we're
   * already behind.
   */
  if (pending_scroll)
    cancel_pending_scroll ();
  else
    scroll_mark_onscreen (insert_mark);

  /* Validate the line that we're moving within.
   */
  buffer->get_iter_at_mark (
                                    &old_insert, insert_mark);

  layout->get_line_yrange (&old_insert, &y, &height);
  layout->validate_yrange (&old_insert, y, y + height);
  /* FIXME do we need to update the adjustment ranges here? */

  new_insert = old_insert;

  if (count < 0 && adj->value <= (adj->lower + 1e-12))
    {
      /* already at far left, just be sure we are at offset 0 */
      new_insert.set_line_offset (0);
      some_move_cursor (&new_insert, extend_selection);
    }
  else if (count > 0 && adj->value >= (adj->upper - adj->page_size - 1e-12))
    {
      /* already at far right, just be sure we are at the end */
      if (!new_insert.ends_line ())
	  new_insert.forward_to_line_end ();
      some_move_cursor (&new_insert, extend_selection);
    }
  else
    {
      get_virtual_cursor_pos (&cursor_x_pos, &cursor_y_pos);

      oldval = adj->value;
      newval = adj->value;

      newval += count * adj->page_increment;

      set_adjustment_clamped (adj, newval);
      cursor_x_pos += adj->value - oldval;

      layout->get_iter_at_pixel ( &new_insert, cursor_x_pos, cursor_y_pos);
      clamp_iter_onscreen (&new_insert);
      some_move_cursor (&new_insert, extend_selection);

      set_virtual_cursor_pos (cursor_x_pos, cursor_y_pos);
    }

  /*  FIXME for lines shorter than the overall widget width, this results in a
   *  "bounce" effect as we scroll to the right of the widget, then scroll
   *  back to get the end of the line onscreen.
   *      http://bugzilla.gnome.org/show_bug.cgi?id=68963
   */
  
  /* Adjust to have the cursor _entirely_ onscreen, move_mark_onscreen
   * only guarantees 1 pixel onscreen.
   */
  DV(g_print (G_STRLOC": scrolling onscreen\n"));
  scroll_mark_onscreen (insert_mark);

  return !TextIter::equal (&old_insert, &new_insert);
}

bool TextView::whitespace (gunichar ch, gpointer user_data)
{
  return (ch == ' ' || ch == '\t');
}

bool TextView::not_whitespace (gunichar ch, gpointer user_data)
{
  return !whitespace (ch, user_data);
}

bool TextView::find_whitepace_region (const TextIter *center,
                       TextIter *start, TextIter *end)
{
  *start = *center;
  *end = *center;

  if (start->backward_find_char (TextView::not_whitespace, NULL, NULL))
    start->forward_char (); /* we want the first whitespace... */
  if (whitespace (end->get_char (), NULL))
    end->forward_find_char (TextView::not_whitespace, NULL, NULL);

  return !TextIter::equal (start, end);
}

void
TextView::insert_at_cursor (
                                const gchar *str)
{
  if (!buffer->insert_interactive_at_cursor ( str, -1,
                                                     editable))
    {
      //TODO beep! gtk_widget_error_bell (GTK_WIDGET (text_view));
    }
}

void
TextView::delete_from_cursor (
                                  DeleteType  type,
                                  gint           count)
{
  TextIter insert;
  TextIter start;
  TextIter end;
  bool leave_one = false;

  //gtk_text_view_reset_im_context (text_view);

  if (type == DELETE_CHARS)
    {
      /* Char delete deletes the selection, if one exists */
      if (buffer->delete_selection (true, editable))
        return;
    }

  buffer->get_iter_at_mark ( &insert, buffer->get_insert ());

  start = insert;
  end = insert;

  switch (type)
    {
    case DELETE_CHARS:
      end.forward_cursor_positions (count);
      break;

    case DELETE_WORD_ENDS:
      if (count > 0)
        end.forward_word_ends (count);
      else if (count < 0)
        start.backward_word_starts (0 - count);
      break;

    case DELETE_WORDS:
      break;

    case DELETE_DISPLAY_LINE_ENDS:
      break;

    case DELETE_DISPLAY_LINES:
      break;

    case DELETE_PARAGRAPH_ENDS:
      if (count > 0)
        {
          /* If we're already at a newline, we need to
           * simply delete that newline, instead of
           * moving to the next one.
           */
          if (end.ends_line ())
            {
              end.forward_line ();
              --count;
            }

          while (count > 0)
            {
              if (!end.forward_to_line_end ())
                break;

              --count;
            }
        }
      else if (count < 0)
        {
          if (start.starts_line ())
            {
              start.backward_line ();
              if (!end.ends_line ())
                start.forward_to_line_end ();
            }
          else
            {
              start.set_line_offset (0);
            }
          ++count;

          start.backward_lines ( -count);
        }
      break;

    case DELETE_PARAGRAPHS:
      if (count > 0)
        {
          start.set_line_offset (0);
          end.forward_to_line_end ();

          /* Do the lines beyond the first. */
          while (count > 1)
            {
              end.forward_to_line_end ();

              --count;
            }
        }

      /* FIXME negative count? */

      break;

    case DELETE_WHITESPACE:
      {
        find_whitepace_region (&insert, &start, &end);
      }
      break;

    default:
      break;
    }

  if (!TextIter::equal (&start, &end))
    {
      buffer->begin_user_action ();

      if (buffer->delete_text_interactive (&start, &end,
                                              editable))
        {
          if (leave_one)
            buffer->insert_interactive_at_cursor (
                                                          " ", 1,
                                                          editable);
        }
      else
        {
          //TODO beep! gtk_widget_error_bell (GTK_WIDGET (text_view));
        }

      buffer->end_user_action ();
      set_virtual_cursor_pos (-1, -1);

      DV(g_print (G_STRLOC": scrolling onscreen\n"));
      scroll_mark_onscreen (
                                          buffer->get_insert ());
    }
  else
    {
      //TODO beep! gtk_widget_error_bell (GTK_WIDGET (text_view));
    }
}

void TextView::backspace (void)
{
  TextIter insert;

  //gtk_text_view_reset_im_context (text_view);

  /* Backspace deletes the selection, if one exists */
  if (buffer->delete_selection (true,
                                        editable))
    return;

  buffer->get_iter_at_mark (
                                    &insert,
                                    buffer->get_insert ());

  if (buffer->backspace (&insert,
				 true, editable))
    {
      set_virtual_cursor_pos (-1, -1);
      DV(g_print (G_STRLOC": scrolling onscreen\n"));
      scroll_mark_onscreen (
                                          buffer->get_insert ());
    }
  else
    {
      //TODO beep! gtk_widget_error_bell (GTK_WIDGET (text_view));
    }
}

/*
static void
TextView::cut_clipboard (TextView *text_view)
{
  GtkClipboard *clipboard = gtk_widget_get_clipboard (GTK_WIDGET (text_view),
						      GDK_SELECTION_CLIPBOARD);
  
  gtk_text_buffer_cut_clipboard (buffer,
				 clipboard,
				 text_view->editable);
  DV(g_print (G_STRLOC": scrolling onscreen\n"));
  scroll_mark_onscreen (
                                      buffer->get_insert ());
}

static void
TextView::copy_clipboard (TextView *text_view)
{
  GtkClipboard *clipboard = gtk_widget_get_clipboard (GTK_WIDGET (text_view),
						      GDK_SELECTION_CLIPBOARD);
  
  gtk_text_buffer_copy_clipboard (buffer,
				  clipboard);

  * on copy do not scroll, we are already onscreen *
}

static void
TextView::paste_clipboard (TextView *text_view)
{
  GtkClipboard *clipboard = gtk_widget_get_clipboard (GTK_WIDGET (text_view),
						      GDK_SELECTION_CLIPBOARD);
  
  gtk_text_buffer_paste_clipboard (buffer,
				   clipboard,
				   NULL,
				   text_view->editable);
  DV(g_print (G_STRLOC": scrolling onscreen\n"));
  scroll_mark_onscreen (
                                      buffer->get_insert ());
}*/

void TextView::toggle_overwrite (void)
{
  //if (text_view->text_window)
  //  text_window_invalidate_cursors (text_view->text_window);

  overwrite_mode = !overwrite_mode;

  if (layout)
    layout->set_overwrite_mode ( overwrite_mode && editable);

  //if (text_view->text_window)
  //  text_window_invalidate_cursors (text_view->text_window);

  //gtk_text_view_pend_cursor_blink (text_view);

  //TODO g_object_notify (G_OBJECT (text_view), "overwrite");
}

/**
 * gtk_text_view_get_overwrite:
 * @text_view: a #TextView
 *
 * Returns whether the #TextView is in overwrite mode or not.
 *
 * Return value: whether @text_view is in overwrite mode or not.
 * 
 * Since: 2.4
 **/
bool TextView::get_overwrite (void)
{
  //g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), false);

  return overwrite_mode;
}

/**
 * gtk_text_view_set_overwrite:
 * @text_view: a #TextView
 * @overwrite: %true to turn on overwrite mode, %false to turn it off
 *
 * Changes the #TextView overwrite mode.
 *
 * Since: 2.4
 **/
void TextView::set_overwrite ( bool     overwrite)
{
  //g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))
  overwrite = overwrite != false;//double-you tee ef!

  if (overwrite_mode != overwrite)
    toggle_overwrite ();
}

/**
 * gtk_text_view_set_accepts_tab:
 * @text_view: A #TextView
 * @accepts_tab: %true if pressing the Tab key should insert a tab 
 *    character, %false, if pressing the Tab key should move the 
 *    keyboard focus.
 * 
 * Sets the behavior of the text widget when the Tab key is pressed. 
 * If @accepts_tab is %true, a tab character is inserted. If @accepts_tab 
 * is %false the keyboard focus is moved to the next widget in the focus 
 * chain.
 * 
 * Since: 2.4
 **/
void
TextView::set_accepts_tab (
			       bool     accepts_tab)
{
  //g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))

  accepts_tab = accepts_tab != false;

  if (this->accepts_tab != accepts_tab)
    {
      this->accepts_tab = accepts_tab;

      //TODO g_object_notify (G_OBJECT (text_view), "accepts-tab");
    }
}

/**
 * gtk_text_view_get_accepts_tab:
 * @text_view: A #TextView
 * 
 * Returns whether pressing the Tab key inserts a tab characters.
 * gtk_text_view_set_accepts_tab().
 * 
 * Return value: %true if pressing the Tab key inserts a tab character, 
 *   %false if pressing the Tab key moves the keyboard focus.
 * 
 * Since: 2.4
 **/
bool TextView::get_accepts_tab (void)
{
 // g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), false);

  return accepts_tab;
}

/*
void TextView::compat_move_focus ( DirectionType direction_type)
{
  GSignalInvocationHint *hint = g_signal_get_invocation_hint (text_view);

  *  as of GTK+ 2.12, the "move-focus" signal has been moved to GtkWidget,
   *  the evil code below makes sure that both emitting the signal and
   *  calling the virtual function directly continue to work as expetcted
   *

  if (hint->signal_id == g_signal_lookup ("move-focus", GTK_TYPE_WIDGET))
    {
      *  if this is a signal emission, chain up  *

      bool retval;

      g_signal_chain_from_overridden_handler (text_view,
                                              direction_type, &retval);
    }
  else
    {
      *  otherwise emit the signal, since somebody called the virtual
       *  function directly
       *

      g_signal_emit_by_name (text_view, "move-focus", direction_type);
    }
}*/

/*
 * Selections
 */

void TextView::unselect (void)
{
  TextIter insert;

  buffer->get_iter_at_mark ( &insert,
                                    buffer->get_insert ());

  buffer->move_mark ( buffer->get_selection_bound (), &insert);
}

/*TODO who uses this?
static void
get_iter_at_pointer (
                     TextIter *iter,
		     gint        *x,
		     gint        *y)
{
  gint xcoord, ycoord;
  ModifierType state;

  //gdk_window_get_pointer (text_view->text_window->bin_window,
  //                        &xcoord, &ycoord, &state);
  
  layout->get_iter_at_pixel (
                                     iter,
                                     xcoord + text_view->xoffset,
                                     ycoord + text_view->yoffset);
  if (x)
    *x = xcoord;

  if (y)
    *y = ycoord;
}*/

/*
void TextView::move_mark_to_pointer_and_scroll ( const gchar *mark_name)
{
  TextIter newplace;
  TextMark *mark;

  get_iter_at_pointer (&newplace, NULL, NULL);
  
  mark = buffer->get_mark (mark_name);
  
  * This may invalidate the layout *
  DV(g_print (G_STRLOC": move mark\n"));
  
  buffer->move_mark (
			     mark,
			     &newplace);
  
  DV(g_print (G_STRLOC": scrolling onscreen\n"));
  scroll_mark_onscreen (mark);

  DV (g_print ("first validate idle leaving %s is %d\n",
               G_STRLOC, text_view->first_validate_idle));
}*/

bool TextView::selection_scan_timeout (gpointer data)
{
  TextView *text_view;

  text_view = (TextView*)data;

  //DV(g_print (G_STRLOC": calling move_mark_to_pointer_and_scroll\n"));
  scroll_mark_onscreen ( buffer->get_insert ());

  return true; /* remain installed. */
}

#define UPPER_OFFSET_ANCHOR 0.8
#define LOWER_OFFSET_ANCHOR 0.2

bool TextView::check_scroll (gdouble offset, Adjustment *adj)
{
  if ((offset > UPPER_OFFSET_ANCHOR &&
       adj->value + adj->page_size < adj->upper) ||
      (offset < LOWER_OFFSET_ANCHOR &&
       adj->value > adj->lower))
    return true;

  return false;
}

/*
static gint
drag_scan_timeout (gpointer data)
{
  TextView *text_view;
  TextIter newplace;
  gint x, y, width, height;
  gdouble pointer_xoffset, pointer_yoffset;

  text_view = GTK_TEXT_VIEW (data);

  get_iter_at_pointer (text_view, &newplace, &x, &y);
  gdk_drawable_get_size (text_view->text_window->bin_window, &width, &height);

  buffer->move_mark (
                             text_view->dnd_mark,
                             &newplace);

  pointer_xoffset = (gdouble) x / width;
  pointer_yoffset = (gdouble) y / height;

  if (check_scroll (pointer_xoffset, text_view->hadjustment) ||
      check_scroll (pointer_yoffset, text_view->vadjustment))
    {
      * do not make offsets surpass lower nor upper anchors, this makes
       * scrolling speed relative to the distance of the pointer to the
       * anchors when it moves beyond them.
       *
      pointer_xoffset = CLAMP (pointer_xoffset, LOWER_OFFSET_ANCHOR, UPPER_OFFSET_ANCHOR);
      pointer_yoffset = CLAMP (pointer_yoffset, LOWER_OFFSET_ANCHOR, UPPER_OFFSET_ANCHOR);

      gtk_text_view_scroll_to_mark (text_view,
                                    text_view->dnd_mark,
                                    0., true, pointer_xoffset, pointer_yoffset);
    }

  return true;
}*/

/*
 * Move @start and @end to the boundaries of the selection unit (indicated by 
 * @granularity) which contained @start initially.
 * If the selction unit is SELECT_WORDS and @start is not contained in a word
 * the selection is extended to all the white spaces between the end of the 
 * word preceding @start and the start of the one following.
 */
void TextView::extend_selection (
		  SelectionGranularity granularity, 
		  TextIter *start, 
		  TextIter *end)
{
  *end = *start;

  if (granularity == SELECT_WORDS) 
    {
      if (start->inside_word ())
	{
	  if (!start->starts_word ())
	    start->backward_visible_word_start ();
	  
	  if (!end->ends_word ())
	    {
	      if (!end->forward_visible_word_end ())
		end->forward_to_end ();
	    }
	}
      else
	{
	  TextIter tmp;

	  tmp = *start;
	  if (tmp.backward_visible_word_start ())
	    tmp.forward_visible_word_end ();

	  if (tmp.get_line () == start->get_line ())
	    *start = tmp;
	  else
	    start->set_line_offset (0);

	  tmp = *end;
	  if (!tmp.forward_visible_word_end ())
	    tmp.forward_to_end ();

	  if (tmp.ends_word ())
	    tmp.backward_visible_word_start ();

	  if (tmp.get_line () == end->get_line ())
	    *end = tmp;
	  else
	    end->forward_to_line_end ();
	}
    }
  else if (granularity == SELECT_LINES) 
    {
      if (starts_display_line (start))
	{
	  /* If on a display line boundary, we assume the user
	   * clicked off the end of a line and we therefore select
	   * the line before the boundary.
	   */
	  backward_display_line_start (start);
	}
      else
	{
	  /* start isn't on the start of a line, so we move it to the
	   * start, and move end to the end unless it's already there.
	   */
	  backward_display_line_start (start);
	  
	  if (!starts_display_line (end))
	    forward_display_line_end (end);
	}
    }
}
 

void TextView::selection_data_free (SelectionData *data)
{
  if (data->orig_start != NULL)
    data->orig_start->get_buffer()->delete_mark ( data->orig_start);
  if (data->orig_end != NULL)
    data->orig_end->get_buffer()->delete_mark ( data->orig_end);
  g_free (data);
}

/*
gint TextView::selection_motion_event_handler (
				EventMotion *event, 
				SelectionData  *data)
{
  gdk_event_request_motions (event);

  if (data->granularity == SELECT_CHARACTERS) 
    {
      move_mark_to_pointer_and_scroll (text_view, "insert");
    }
  else 
    {
      TextIter cursor, start, end;
      TextIter orig_start, orig_end;
      GtkTextBuffer *buffer;
      
      buffer = buffer;

      buffer->get_iter_at_mark ( &orig_start, data->orig_start);
      buffer->get_iter_at_mark ( &orig_end, data->orig_end);

      get_iter_at_pointer (text_view, &cursor, NULL, NULL);
      
      start = cursor;
      extend_selection (text_view, data->granularity, &start, &end);

      * either the selection extends to the front, or end (or not) *
      if (gtk_text_iter_compare (&cursor, &orig_start) < 0)
        gtk_text_buffer_select_range (buffer, &start, &orig_end);
      else
        gtk_text_buffer_select_range (buffer, &end, &orig_start);

      scroll_mark_onscreen (
					  buffer->get_insert ());
    }

  * If we had to scroll offscreen, insert a timeout to do so
   * again. Note that in the timeout, even if the mouse doesn't
   * move, due to this scroll xoffset/yoffset will have changed
   * and we'll need to scroll again.
   *
  if (text_view->scroll_timeout != 0) * reset on every motion event *
    g_source_remove (text_view->scroll_timeout);
  
  text_view->scroll_timeout =
    gdk_threads_add_timeout (50, selection_scan_timeout, text_view);

  return true;
}*/

/*
static void
TextView::start_selection_drag (TextView       *text_view,
                                    const TextIter *iter,
                                    GdkEventButton    *button)
{
  TextIter cursor, ins, bound;
  TextIter orig_start, orig_end;
  GtkTextBuffer *buffer;
  SelectionData *data;

  if (text_view->selection_drag_handler != 0)
    return;
  
  data = g_new0 (SelectionData, 1);

  if (button->type == GDK_2BUTTON_PRESS)
    data->granularity = SELECT_WORDS;
  else if (button->type == GDK_3BUTTON_PRESS)
    data->granularity = SELECT_LINES;
  else 
    data->granularity = SELECT_CHARACTERS;

  gtk_grab_add (GTK_WIDGET (text_view));

  buffer = buffer;
  
  cursor = *iter;
  ins = cursor;
  
  extend_selection (text_view, data->granularity, &ins, &bound);
  orig_start = ins;
  orig_end = bound;

  if (button->state & GDK_SHIFT_MASK)
    {
      * Extend selection *
      TextIter old_ins, old_bound;
      TextIter old_start, old_end;

      buffer->get_iter_at_mark ( &old_ins, buffer->get_insert ());
      buffer->get_iter_at_mark ( &old_bound, gtk_text_buffer_get_selection_bound (buffer));
      old_start = old_ins;
      old_end = old_bound;
      gtk_text_iter_order (&old_start, &old_end);
      
      * move the front cursor, if the mouse is in front of the selection. Should the
       * cursor however be inside the selection (this happens on tripple click) then we
       * move the side which was last moved (current insert mark) *
      if (gtk_text_iter_compare (&cursor, &old_start) <= 0 ||
          (gtk_text_iter_compare (&cursor, &old_end) < 0 && 
           gtk_text_iter_compare (&old_ins, &old_bound) <= 0))
        {
          bound = old_end;
          orig_start = old_end;
          orig_end = old_end;
        }
      else
        {
          ins = bound;
          bound = old_start;
          orig_end = bound;
          orig_start = bound;
        }
    }

  gtk_text_buffer_select_range (buffer, &ins, &bound);

  gtk_text_iter_order (&orig_start, &orig_end);
  data->orig_start = buffer->create_mark (NULL,
                                                  &orig_start, true);
  data->orig_end = buffer->create_mark (NULL,
                                                &orig_end, true);

  gtk_text_view_check_cursor_blink (text_view);

  text_view->selection_drag_handler = g_signal_connect_data (text_view,
                                                             "motion-notify-event",
                                                             G_CALLBACK (selection_motion_event_handler),
                                                             data,
                                                             (GClosureNotify) selection_data_free, 0);  
}*/

/* returns whether we were really dragging */
/*
static bool
TextView::end_selection_drag (TextView    *text_view) 
{
  if (text_view->selection_drag_handler == 0)
    return false;

  g_signal_handler_disconnect (text_view, text_view->selection_drag_handler);
  text_view->selection_drag_handler = 0;

  if (text_view->scroll_timeout != 0)
    {
      g_source_remove (text_view->scroll_timeout);
      text_view->scroll_timeout = 0;
    }

  gtk_grab_remove (GTK_WIDGET (text_view));

  return true;
}*/

/*
 * Layout utils
 */

/*
void TextView::set_attributes_from_style (TextView        *text_view,
                                         TextAttributes  *values,
                                         Style           *style)
{
  values->appearance.bg_color = style->base[GTK_STATE_NORMAL];
  values->appearance.fg_color = style->text[GTK_STATE_NORMAL];

  if (values->font)
    pango_font_description_free (values->font);

  values->font = pango_font_description_copy (style->font_desc);
}
*/

void TextView::check_keymap_direction (void)
{
  /*TODO? if (text_view->layout)
    {
      GtkSettings *settings = gtk_widget_get_settings (GTK_WIDGET (text_view));
      GdkKeymap *keymap = gdk_keymap_get_for_display (gtk_widget_get_display (GTK_WIDGET (text_view)));
      GtkTextDirection new_cursor_dir;
      GtkTextDirection new_keyboard_dir;
      bool split_cursor;

      g_object_get (settings,
		    "gtk-split-cursor", &split_cursor,
		    NULL);
      
      if (gdk_keymap_get_direction (keymap) == PANGO_DIRECTION_RTL)
	new_keyboard_dir = GTK_TEXT_DIR_RTL;
      else
	new_keyboard_dir  = GTK_TEXT_DIR_LTR;
  
      if (split_cursor)
	new_cursor_dir = GTK_TEXT_DIR_NONE;
      else
	new_cursor_dir = new_keyboard_dir;
      
      layout->set_cursor_direction (new_cursor_dir);
      layout->set_keyboard_direction (new_keyboard_dir);
    }*/
}

void TextView::ensure_layout (void)
{
  //GtkWidget *widget;

  //widget = GTK_WIDGET (text_view);

  if (layout == NULL)
    {
      TextAttributes *style;
      //PangoContext *ltr_context, *rtl_context;
      GSList *tmp_list;

      //DV(g_print(G_STRLOC"\n"));
      
      layout = new TextLayout ();

      /*TODO
      g_signal_connect (text_view->layout,
			"invalidated",
			G_CALLBACK (invalidated_handler),
			text_view);

      g_signal_connect (text_view->layout,
			"changed",
			G_CALLBACK (changed_handler),
			text_view);

      g_signal_connect (text_view->layout,
			"allocate-child",
			G_CALLBACK (gtk_text_view_child_allocated),
			text_view);*/
      
      if (buffer)
        layout->set_buffer (buffer);

      if (HasFocus() && cursor_visible)
        ;//gtk_text_view_pend_cursor_blink (text_view);
      else
        layout->set_cursor_visible (false);

      layout->set_overwrite_mode (overwrite_mode && editable);

      //ltr_context = gtk_widget_create_pango_context (GTK_WIDGET (text_view));
      //pango_context_set_base_dir (ltr_context, PANGO_DIRECTION_LTR);
      //rtl_context = gtk_widget_create_pango_context (GTK_WIDGET (text_view));
      //pango_context_set_base_dir (rtl_context, PANGO_DIRECTION_RTL);

      //layout_set->contexts (ltr_context, rtl_context);

      //TODOg_object_unref (ltr_context);
      //g_object_unref (rtl_context);

      check_keymap_direction ();

      style = new TextAttributes ();

      //gtk_widget_ensure_style (widget);
      //set_attributes_from_style ( style, widget->style);

      style->pixels_above_lines = pixels_above_lines;
      style->pixels_below_lines = pixels_below_lines;
      style->pixels_inside_wrap = pixels_inside_wrap;
      style->left_margin = left_margin;
      style->right_margin = right_margin;
      style->indent = indent;
      //style->tabs = tabs ? pango_tab_array_copy (text_view->tabs) : NULL;

      style->wrap_mode = wrap_mode;
      //style->justification = justify;
      //style->direction = gtk_widget_get_direction (GTK_WIDGET (text_view));

      layout->set_default_style (style);

      //TODOgtk_text_attributes_unref (style);

      /* Set layout for all anchored children */

      /*
      tmp_list = text_view->children;
      while (tmp_list != NULL)
        {
          TextViewChild *vc = tmp_list->data;

          if (vc->anchor)
            {
              gtk_text_anchored_child_set_layout (vc->widget,
                                                  text_view->layout);
              * vc may now be invalid! *
            }

          tmp_list = g_slist_next (tmp_list);
        }*/

      invalidate ();
    }
}

/**
 * gtk_text_view_get_default_attributes:
 * @text_view: a #TextView
 * 
 * Obtains a copy of the default text attributes. These are the
 * attributes used for text unless a tag overrides them.
 * You'd typically pass the default attributes in to
 * gtk_text_iter_get_attributes() in order to get the
 * attributes in effect at a given text position.
 *
 * The return value is a copy owned by the caller of this function,
 * and should be freed.
 * 
 * Return value: a new #GtkTextAttributes
 **/
TextAttributes* TextView::get_default_attributes (void)
{
 // g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), NULL);
  
  ensure_layout();

 TextAttributes* attr = new TextAttributes(*(layout->default_style));

  //return layout->default_style->copy ();
  return attr;
}

void TextView::destroy_layout (void)
{
  if (layout)
    {
      GSList *tmp_list;

      remove_validate_idles ();

      /*TODO
      g_signal_handlers_disconnect_by_func (layout,
					    TextView::invalidated_handler,
					    this);
      g_signal_handlers_disconnect_by_func (layout,
					    TextView::changed_handler, 
					    this);
					    */
      
      /* Remove layout from all anchored children */
      /*
      tmp_list = text_view->children;
      while (tmp_list != NULL)
        {
          TextViewChild *vc = tmp_list->data;

          if (vc->anchor)
            {
              gtk_text_anchored_child_set_layout (vc->widget, NULL);
              * vc may now be invalid! *
            }

          tmp_list = g_slist_next (tmp_list);
        }*/
      
      //gtk_text_view_stop_cursor_blink (text_view);
      //gtk_text_view_end_selection_drag (text_view);

      //TODOg_object_unref (text_view->layout);
      layout = NULL;
    }
}

/*
void TextView::reset_im_context (TextView *text_view)
{
  if (text_view->need_im_reset)
    {
      text_view->need_im_reset = false;
      gtk_im_context_reset (text_view->im_context);
    }
}*/

/*
 * DND feature
 */

/*
static void
drag_begin_cb (GtkWidget      *widget,
               GdkDragContext *context,
               gpointer        data)
{
  TextView   *text_view = GTK_TEXT_VIEW (widget);
  GtkTextBuffer *buffer = gtk_text_view_buffer;
  TextIter    start;
  TextIter    end;
  GdkPixmap     *pixmap = NULL;

  g_signal_handlers_disconnect_by_func (widget, drag_begin_cb, NULL);

  if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
    pixmap = _gtk_text_util_create_rich_drag_icon (widget, buffer, &start, &end);

  if (pixmap)
    {
      gtk_drag_set_icon_pixmap (context,
                                gdk_drawable_get_colormap (pixmap),
                                pixmap,
                                NULL,
                                -2, -2);
      g_object_unref (pixmap);
    }
  else
    {
      gtk_drag_set_icon_default (context);
    }
}

static void
TextView::start_selection_dnd (TextView       *text_view,
                                   const TextIter *iter,
                                   GdkEventMotion    *event)
{
  GtkTargetList *target_list;

  text_view->drag_start_x = -1;
  text_view->drag_start_y = -1;
  text_view->pending_place_cursor_button = 0;

  target_list = gtk_text_buffer_get_copy_target_list (buffer);

  g_signal_connect (text_view, "drag-begin",
                    G_CALLBACK (drag_begin_cb), NULL);
  gtk_drag_begin (GTK_WIDGET (text_view), target_list,
		  GDK_ACTION_COPY | GDK_ACTION_MOVE,
		  1, (GdkEvent*)event);
}

static void
TextView::drag_begin (GtkWidget        *widget,
                          GdkDragContext   *context)
{
  * do nothing *
}

static void
TextView::drag_end (GtkWidget        *widget,
                        GdkDragContext   *context)
{
}

static void
TextView::drag_data_get (GtkWidget        *widget,
                             GdkDragContext   *context,
                             GtkSelectionData *selection_data,
                             guint             info,
                             guint             time)
{
  TextView *text_view = GTK_TEXT_VIEW (widget);
  GtkTextBuffer *buffer = gtk_text_view_buffer;

  if (info == GTK_TEXT_BUFFER_TARGET_INFO_BUFFER_CONTENTS)
    {
      gtk_selection_data_set (selection_data,
                              gdk_atom_intern_static_string ("GTK_TEXT_BUFFER_CONTENTS"),
                              8, * bytes *
                              (void*)&buffer,
                              sizeof (buffer));
    }
  else if (info == GTK_TEXT_BUFFER_TARGET_INFO_RICH_TEXT)
    {
      TextIter start;
      TextIter end;
      guint8 *str = NULL;
      gsize len;

      if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
        {
          * Extract the selected text *
          str = gtk_text_buffer_serialize (buffer, buffer,
                                           selection_data->target,
                                           &start, &end,
                                           &len);
        }

      if (str)
        {
          gtk_selection_data_set (selection_data,
                                  selection_data->target,
                                  8, * bytes *
                                  (guchar *) str, len);
          g_free (str);
        }
    }
  else
    {
      TextIter start;
      TextIter end;
      gchar *str = NULL;

      if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
        {
          * Extract the selected text *
          str = gtk_text_iter_get_visible_text (&start, &end);
        }

      if (str)
        {
          gtk_selection_data_set_text (selection_data, str, -1);
          g_free (str);
        }
    }
}

static void
TextView::drag_data_delete (GtkWidget        *widget,
                                GdkDragContext   *context)
{
  gtk_text_buffer_delete_selection (GTK_TEXT_VIEW (widget)->buffer,
                                    true, GTK_TEXT_VIEW (widget)->editable);
}

static void
TextView::drag_leave (GtkWidget        *widget,
                          GdkDragContext   *context,
                          guint             time)
{
  TextView *text_view;

  text_view = GTK_TEXT_VIEW (widget);

  gtk_text_mark_set_visible (text_view->dnd_mark, false);
  
  if (text_view->scroll_timeout != 0)
    g_source_remove (text_view->scroll_timeout);

  text_view->scroll_timeout = 0;
}

static bool
TextView::drag_motion (GtkWidget        *widget,
                           GdkDragContext   *context,
                           gint              x,
                           gint              y,
                           guint             time)
{
  TextIter newplace;
  TextView *text_view;
  TextIter start;
  TextIter end;
  GdkRect target_rect;
  gint bx, by;
  GdkAtom target;
  GdkDragAction suggested_action = 0;
  
  text_view = GTK_TEXT_VIEW (widget);

  target_rect = text_view->text_window->allocation;
  
  if (x < target_rect.x ||
      y < target_rect.y ||
      x > (target_rect.x + target_rect.width) ||
      y > (target_rect.y + target_rect.height))
    return false; * outside the text window, allow parent widgets to handle event *

  gtk_text_view_window_to_buffer_coords (text_view,
                                         GTK_TEXT_WINDOW_WIDGET,
                                         x, y,
                                         &bx, &by);

  layout->get_iter_at_pixel (
                                     &newplace,
                                     bx, by);  

  target = gtk_drag_dest_find_target (widget, context,
                                      gtk_drag_dest_get_target_list (widget));

  if (target == GDK_NONE)
    {
      * can't accept any of the offered targets *
    }                                 
  else if (gtk_text_buffer_get_selection_bounds (buffer,
                                                 &start, &end) &&
           gtk_text_iter_compare (&newplace, &start) >= 0 &&
           gtk_text_iter_compare (&newplace, &end) <= 0)
    {
      * We're inside the selection. *
    }
  else
    {      
      if (gtk_text_iter_can_insert (&newplace, text_view->editable))
        {
          GtkWidget *source_widget;
          
          suggested_action = context->suggested_action;
          
          source_widget = gtk_drag_get_source_widget (context);
          
          if (source_widget == widget)
            {
              * Default to MOVE, unless the user has
               * pressed ctrl or alt to affect available actions
               *
              if ((context->actions & GDK_ACTION_MOVE) != 0)
                suggested_action = GDK_ACTION_MOVE;
            }
        }
      else
        {
          * Can't drop here. *
        }
    }

  if (suggested_action != 0)
    {
      gtk_text_mark_set_visible (text_view->dnd_mark,
                                 text_view->cursor_visible);
      
      gdk_drag_status (context, suggested_action, time);
    }
  else
    {
      gdk_drag_status (context, 0, time);
      gtk_text_mark_set_visible (text_view->dnd_mark, false);
    }
      
  if (!text_view->scroll_timeout)
    text_view->scroll_timeout =
      gdk_threads_add_timeout (100, drag_scan_timeout, text_view);

  * true return means don't propagate the drag motion to parent
   * widgets that may also be drop sites.
   *
  return true;
}

static bool
TextView::drag_drop (GtkWidget        *widget,
                         GdkDragContext   *context,
                         gint              x,
                         gint              y,
                         guint             time)
{
  TextView *text_view;
  TextIter drop_point;
  GdkAtom target = GDK_NONE;
  
  text_view = GTK_TEXT_VIEW (widget);
  
  if (text_view->scroll_timeout != 0)
    g_source_remove (text_view->scroll_timeout);

  text_view->scroll_timeout = 0;

  gtk_text_mark_set_visible (text_view->dnd_mark, false);

  buffer->get_iter_at_mark (
                                    &drop_point,
                                    text_view->dnd_mark);

  if (gtk_text_iter_can_insert (&drop_point, text_view->editable))
    target = gtk_drag_dest_find_target (widget, context, NULL);

  if (target != GDK_NONE)
    gtk_drag_get_data (widget, context, target, time);
  else
    gtk_drag_finish (context, false, false, time);

  return true;
}*/

/*
void TextView::insert_text_data (
                  TextIter      *drop_point,
                  GtkSelectionData *selection_data)
{
  guchar *str;

  str = gtk_selection_data_get_text (selection_data);

  if (str)
    {
      if (!gtk_text_buffer_insert_interactive (buffer,
                                               drop_point, (gchar *) str, -1,
                                               text_view->editable))
        {
          gtk_widget_error_bell (GTK_WIDGET (text_view));
        }

      g_free (str);
    }
}*/

/*static void
TextView::drag_data_received (GtkWidget        *widget,
                                  GdkDragContext   *context,
                                  gint              x,
                                  gint              y,
                                  GtkSelectionData *selection_data,
                                  guint             info,
                                  guint             time)
{
  TextIter drop_point;
  TextView *text_view;
  bool success = false;
  GtkTextBuffer *buffer = NULL;

  text_view = GTK_TEXT_VIEW (widget);

  if (!text_view->dnd_mark)
    goto done;

  buffer = buffer;

  buffer->get_iter_at_mark (
                                    &drop_point,
                                    text_view->dnd_mark);
  
  if (!gtk_text_iter_can_insert (&drop_point, text_view->editable))
    goto done;

  success = true;

  gtk_text_buffer_begin_user_action (buffer);

  if (info == GTK_TEXT_BUFFER_TARGET_INFO_BUFFER_CONTENTS)
    {
      GtkTextBuffer *src_buffer = NULL;
      TextIter start, end;
      bool copy_tags = true;

      if (selection_data->length != sizeof (src_buffer))
        return;

      memcpy (&src_buffer, selection_data->data, sizeof (src_buffer));

      if (src_buffer == NULL)
        return;

      g_return_if_fail (GTK_IS_TEXT_BUFFER (src_buffer));

      if (gtk_text_buffer_get_tag_table (src_buffer) !=
          gtk_text_buffer_get_tag_table (buffer))
        {
          *  try to find a suitable rich text target instead  *
          GdkAtom *atoms;
          gint     n_atoms;
          GList   *list;
          GdkAtom  target = GDK_NONE;

          copy_tags = false;

          atoms = gtk_text_buffer_get_deserialize_formats (buffer, &n_atoms);

          for (list = context->targets; list; list = g_list_next (list))
            {
              gint i;

              for (i = 0; i < n_atoms; i++)
                if (GUINT_TO_POINTER (atoms[i]) == list->data)
                  {
                    target = atoms[i];
                    break;
                  }
            }

          g_free (atoms);

          if (target != GDK_NONE)
            {
              gtk_drag_get_data (widget, context, target, time);
              buffer->end_user_action ();
              return;
            }
        }

      if (gtk_text_buffer_get_selection_bounds (src_buffer,
                                                &start,
                                                &end))
        {
          if (copy_tags)
            gtk_text_buffer_insert_range_interactive (buffer,
                                                      &drop_point,
                                                      &start,
                                                      &end,
                                                      text_view->editable);
          else
            {
              gchar *str;

              str = gtk_text_iter_get_visible_text (&start, &end);
              gtk_text_buffer_insert_interactive (buffer,
                                                  &drop_point, str, -1,
                                                  text_view->editable);
              g_free (str);
            }
        }
    }
  else if (selection_data->length > 0 &&
           info == GTK_TEXT_BUFFER_TARGET_INFO_RICH_TEXT)
    {
      bool retval;
      GError *error = NULL;

      retval = gtk_text_buffer_deserialize (buffer, buffer,
                                            selection_data->target,
                                            &drop_point,
                                            (guint8 *) selection_data->data,
                                            selection_data->length,
                                            &error);

      if (!retval)
        {
          g_warning ("error pasting: %s\n", error->message);
          g_clear_error (&error);
        }
    }
  else
    insert_text_data (text_view, &drop_point, selection_data);

 done:
  gtk_drag_finish (context, success,
		   success && context->action == GDK_ACTION_MOVE,
		   time);

  if (success)
    {
      buffer->get_iter_at_mark (
                                        &drop_point,
                                        text_view->dnd_mark);
      gtk_text_buffer_place_cursor (buffer, &drop_point);

      buffer->end_user_action ();
    }
}*/

Adjustment* TextView::get_hadjustment (void)
{
  if (hadjustment == NULL)
    set_scroll_adjustments ( NULL, /* forces creation */
                                          vadjustment);

  return hadjustment;
}

Adjustment* TextView::get_vadjustment (void)
{
  if (vadjustment == NULL)
    set_scroll_adjustments ( hadjustment,
                                          NULL); /* forces creation */
  return vadjustment;
}


void TextView::set_scroll_adjustments (
                                      Adjustment *hadj,
                                      Adjustment *vadj)
{
  bool need_adjust = false;

  if (hadj)
    g_return_if_fail (hadj);
  else
    hadj = new Adjustment (0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  if (vadj)
    g_return_if_fail (vadj);
  else
    vadj = new Adjustment(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

  if (hadjustment && (hadjustment != hadj))
    {
	    /*TODO
      g_signal_handlers_disconnect_by_func (text_view->hadjustment,
					    gtk_text_view_value_changed,
					    text_view);
      g_object_unref (text_view->hadjustment);*/
    }

  if (vadjustment && (vadjustment != vadj))
    {
	    /*TODO
      g_signal_handlers_disconnect_by_func (text_view->vadjustment,
					    gtk_text_view_value_changed,
					    text_view);
      g_object_unref (text_view->vadjustment);*/
    }

  if (hadjustment != hadj)
    {
      hadjustment = hadj;
      /*TODO g_object_ref_sink (text_view->hadjustment);
      
      g_signal_connect (text_view->hadjustment, "value-changed",
                        G_CALLBACK (gtk_text_view_value_changed),
			text_view);*/
      need_adjust = true;
    }

  if (vadjustment != vadj)
    {
      vadjustment = vadj;
      /*TODO g_object_ref_sink (text_view->vadjustment);
      
      g_signal_connect (text_view->vadjustment, "value-changed",
                        G_CALLBACK (gtk_text_view_value_changed),
			text_view);*/
      need_adjust = true;
    }

  if (need_adjust)
    value_changed (NULL);
}

/* FIXME this adjust_allocation is a big cut-and-paste from
 * GtkCList, needs to be some "official" way to do this
 * factored out.
 */
/*
typedef struct
{
  GdkWindow *window;
  int dx;
  int dy;
} ScrollData;*/

/* The window to which widget->window is relative */
/*
#define ALLOCATION_WINDOW(widget)		\
   (GTK_WIDGET_NO_WINDOW (widget) ?		\
    (widget)->window :                          \
     gdk_window_get_parent ((widget)->window))

static void
adjust_allocation_recurse (GtkWidget *widget,
			   gpointer   data)
{
  ScrollData *scroll_data = data;

  * Need to really size allocate instead of just poking
   * into widget->allocation if the widget is not realized.
   * FIXME someone figure out why this was.
   *
  if (!GTK_WIDGET_REALIZED (widget))
    {
      if (GTK_WIDGET_VISIBLE (widget))
	{
	  GdkRect tmp_rectangle = widget->allocation;
	  tmp_rectangle.x += scroll_data->dx;
          tmp_rectangle.y += scroll_data->dy;
          
	  gtk_widget_size_allocate (widget, &tmp_rectangle);
	}
    }
  else
    {
      if (ALLOCATION_WINDOW (widget) == scroll_data->window)
	{
	  widget->allocation.x += scroll_data->dx;
          widget->allocation.y += scroll_data->dy;
          
	  if (GTK_IS_CONTAINER (widget))
	    gtk_container_forall (GTK_CONTAINER (widget),
				  adjust_allocation_recurse,
				  data);
	}
    }
}

static void
adjust_allocation (GtkWidget *widget,
		   int        dx,
                   int        dy)
{
  ScrollData scroll_data;

  if (GTK_WIDGET_REALIZED (widget))
    scroll_data.window = ALLOCATION_WINDOW (widget);
  else
    scroll_data.window = NULL;
    
  scroll_data.dx = dx;
  scroll_data.dy = dy;
  
  adjust_allocation_recurse (widget, &scroll_data);
}*/
            
void TextView::value_changed (Adjustment *adj)
{
  TextIter iter;
  gint line_top;
  gint dx = 0;
  gint dy = 0;
  
  /* Note that we oddly call this function with adj == NULL
   * sometimes
   */
  
  onscreen_validated = false;

  //DV(g_print(">Scroll offset changed %s/%g, onscreen_validated = false ("G_STRLOC")\n",
  //           adj == text_view->hadjustment ? "hadj" : adj == text_view->vadjustment ? "vadj" : "none",
  //           adj ? adj->value : 0.0));
  
  if (adj == hadjustment)
    {
      dx = xoffset - (gint)adj->value;
      xoffset = adj->value;

      /* If the change is due to a size change we need 
       * to invalidate the entire text window because there might be
       * right-aligned or centered text 
       */
      if (width_changed)
	{
	  //if (GTK_WIDGET_REALIZED (text_view))
	  //TODO?  gdk_window_invalidate_rect (text_view->text_window->bin_window, NULL, false);
	  
	  width_changed = false;
	}
    }
  else if (adj == vadjustment)
    {
      dy = yoffset - (gint)adj->value;
      yoffset = adj->value;

      if (layout)
        {
          layout->get_line_at_y (&iter, adj->value, &line_top);

          buffer->move_mark ( first_para_mark, &iter);

          first_para_pixels = adj->value - line_top;
        }
    }
  
  if (dx != 0 || dy != 0)
    {
      GSList *tmp_list;

      //if (GTK_WIDGET_REALIZED (text_view))
        {
          if (dy != 0)
            {
              //if (text_view->left_window)
              //  text_window_scroll (text_view->left_window, 0, dy);
              //if (text_view->right_window)
              //  text_window_scroll (text_view->right_window, 0, dy);
            }
      
          if (dx != 0)
            {
              //if (text_view->top_window)
              //  text_window_scroll (text_view->top_window, dx, 0);
              //if (text_view->bottom_window)
              //  text_window_scroll (text_view->bottom_window, dx, 0);
            }
      
          /* It looks nicer to scroll the main area last, because
           * it takes a while, and making the side areas update
           * afterward emphasizes the slowness of scrolling the
           * main area.
           */
          //text_window_scroll (text_view->text_window, dx, dy);
        }
      
      /* Children are now "moved" in the text window, poke
       * into widget->allocation for each child
       */
	/*
      tmp_list = text_view->children;
      while (tmp_list != NULL)
        {
          TextViewChild *child = tmp_list->data;
          
          if (child->anchor)
            adjust_allocation (child->widget, dx, dy);
          
          tmp_list = g_slist_next (tmp_list);
        }*/
    }

  /* This could result in invalidation, which would install the
   * first_validate_idle, which would validate onscreen;
   * but we're going to go ahead and validate here, so
   * first_validate_idle shouldn't have anything to do.
   */
  update_layout_width ();
  
  /* note that validation of onscreen could invoke this function
   * recursively, by scrolling to maintain first_para, or in response
   * to updating the layout width, however there is no problem with
   * that, or shouldn't be.
   */
  validate_onscreen ();
  
  /* process exposes */
  //if (GTK_WIDGET_REALIZED (text_view))
    {
      //DV (g_print ("Processing updates (%s)\n", G_STRLOC));
      /*
      if (text_view->left_window)
        gdk_window_process_updates (text_view->left_window->bin_window, true);

      if (text_view->right_window)
        gdk_window_process_updates (text_view->right_window->bin_window, true);

      if (text_view->top_window)
        gdk_window_process_updates (text_view->top_window->bin_window, true);
      
      if (text_view->bottom_window)
        gdk_window_process_updates (text_view->bottom_window->bin_window, true);
  
      gdk_window_process_updates (text_view->text_window->bin_window, true);*/
    }

  /* If this got installed, get rid of it, it's just a waste of time. */
  if (first_validate_idle != 0)
    {
      //g_source_remove (text_view->first_validate_idle);
      first_validate_idle = 0;
    }

  //update_im_spot_location (text_view);
  
  //DV(g_print(">End scroll offset changed handler ("G_STRLOC")\n"));
}

/*
static void
TextView::commit_handler (GtkIMContext  *context,
                              const gchar   *str,
                              TextView   *text_view)
{
  gtk_text_view_commit_text (text_view, str);
}*/

//TODO inputprocessor function?
void TextView::commit_text ( const gchar   *str)
{
  bool had_selection;
  
  buffer->begin_user_action ();

  had_selection = buffer->get_selection_bounds ( NULL, NULL);
  
  buffer->delete_selection (true, editable);

  if (!strcmp (str, "\n"))
    {
      if (!buffer->insert_interactive_at_cursor ( "\n", 1,
                                                         editable))
        {
          //TODO beep! gtk_widget_error_bell (GTK_WIDGET (text_view));
        }
    }
  else
    {
      if (!had_selection && overwrite_mode)
	{
	  TextIter insert;
	  
	  buffer->get_iter_at_mark ( &insert,
					    buffer->get_insert ());
	  if (!insert.ends_line ())
	    delete_from_cursor (DELETE_CHARS, 1);
	}

      if (!buffer->insert_interactive_at_cursor ( str, -1,
                                                         editable))
        {
          //TODO beep! gtk_widget_error_bell (GTK_WIDGET (text_view));
        }
    }

  buffer->end_user_action ();

  set_virtual_cursor_pos (-1, -1);
  DV(g_print (G_STRLOC": scrolling onscreen\n"));
  scroll_mark_onscreen (
                                      buffer->get_insert ());
}

/*
static void
TextView::preedit_changed_handler (GtkIMContext *context,
				       TextView  *text_view)
{
  gchar *str;
  PangoAttrList *attrs;
  gint cursor_pos;
  TextIter iter;

  gtk_text_buffer_get_iter_at_mark (text_view->buffer, &iter, 
				    gtk_text_buffer_get_insert (text_view->buffer));

  * Keypress events are passed to input method even if cursor position is not editable;
   * so beep here if it's multi-key input sequence, input method will be reset in 
   * key-press-event handler. *
  if (!gtk_text_iter_can_insert (&iter, text_view->editable))
    {
      gtk_widget_error_bell (GTK_WIDGET (text_view));
      return;
    }

  gtk_im_context_get_preedit_string (context, &str, &attrs, &cursor_pos);
  gtk_text_layout_set_preedit_string (text_view->layout, str, attrs, cursor_pos);
  pango_attr_list_unref (attrs);
  g_free (str);

  if (GTK_WIDGET_HAS_FOCUS (text_view))
    scroll_mark_onscreen (
					buffer->get_insert ());
}*/

/*
static bool
TextView::retrieve_surrounding_handler (GtkIMContext  *context,
					    TextView   *text_view)
{
  TextIter start;
  TextIter end;
  gint pos;
  gchar *text;

  gtk_text_buffer_get_iter_at_mark (text_view->buffer, &start,  
				    gtk_text_buffer_get_insert (text_view->buffer));
  end = start;

  pos = gtk_text_iter_get_line_index (&start);
  start.set_line_offset (0);
  end->forward_to_line_end ();

  text = gtk_text_iter_get_slice (&start, &end);
  gtk_im_context_set_surrounding (context, text, -1, pos);
  g_free (text);

  return true;
}

static bool
TextView::delete_surrounding_handler (GtkIMContext  *context,
					  gint           offset,
					  gint           n_chars,
					  TextView   *text_view)
{
  TextIter start;
  TextIter end;

  gtk_text_buffer_get_iter_at_mark (text_view->buffer, &start,  
				    gtk_text_buffer_get_insert (text_view->buffer));
  end = start;

  gtk_text_iter_forward_chars (&start, offset);
  gtk_text_iter_forward_chars (&end, offset + n_chars);

  buffer->delete_interactive (&start, &end,
				      editable);

  return true;
}*/

void TextView::mark_set_handler (TextBuffer *buffer,
                                const TextIter *location,
                                TextMark       *mark,
                                gpointer           data)
{
 // TextView *text_view = GTK_TEXT_VIEW (data);
  bool need_reset = false;

  if (mark == buffer->get_insert ())
    {
      virtual_cursor_x = -1;
      virtual_cursor_y = -1;
      //gtk_text_view_update_im_spot_location (text_view);
      need_reset = true;
    }
  else if (mark == buffer->get_selection_bound ())
    {
      need_reset = true;
    }

  if (need_reset)
    ;//gtk_text_view_reset_im_context (text_view);
}


void TextView::target_list_notify ( void )
 
{
	/*TODO? (see .h)
  GtkWidget     *widget = GTK_WIDGET (data);
  GtkTargetList *view_list;
  GtkTargetList *buffer_list;
  GList         *list;

  view_list = gtk_drag_dest_get_target_list (widget);
  buffer_list = gtk_text_buffer_get_paste_target_list (buffer);

  if (view_list)
    gtk_target_list_ref (view_list);
  else
    view_list = gtk_target_list_new (NULL, 0);

  list = view_list->list;
  while (list)
    {
      GtkTargetPair *pair = list->data;

      list = g_list_next (list); * get next element before removing *

      if (pair->info >= GTK_TEXT_BUFFER_TARGET_INFO_TEXT &&
          pair->info <= GTK_TEXT_BUFFER_TARGET_INFO_BUFFER_CONTENTS)
        {
          gtk_target_list_remove (view_list, pair->target);
        }
    }

  for (list = buffer_list->list; list; list = g_list_next (list))
    {
      GtkTargetPair *pair = list->data;

      gtk_target_list_add (view_list, pair->target, pair->flags, pair->info);
    }

  gtk_drag_dest_set_target_list (widget, view_list);
  gtk_target_list_unref (view_list);
  */
}

void
TextView::get_cursor_location  (
				    Rect  *pos)
{
  TextIter insert;
  
  buffer->get_iter_at_mark (&insert,
                            buffer->get_insert ());

  layout->get_cursor_locations (&insert, pos, NULL);
}

void
TextView::get_virtual_cursor_pos (
                                      gint        *x,
                                      gint        *y)
{
  Rect pos;

  if ((x && virtual_cursor_x == -1) ||
      (y && virtual_cursor_y == -1))
    get_cursor_location (&pos);

  if (x)
    {
      if (virtual_cursor_x != -1)
        *x = virtual_cursor_x;
      else
        *x = pos.x;
    }

  if (y)
    {
      if (virtual_cursor_x != -1)
        *y = virtual_cursor_y;
      else
        *y = pos.y + pos.height / 2;
    }
}

void
TextView::set_virtual_cursor_pos (
                                      gint         x,
                                      gint         y)
{
  Rect pos;

  if (!layout)
    return;

  if (x == -1 || y == -1)
    get_cursor_location (&pos);

  virtual_cursor_x = (x == -1) ? pos.x : x;
  virtual_cursor_y = (y == -1) ? pos.y + pos.height / 2 : y;
}

/* Quick hack of a popup menu
 */
/*
static void
activate_cb (GtkWidget   *menuitem,
	     TextView *text_view)
{
  const gchar *signal = g_object_get_data (G_OBJECT (menuitem), "gtk-signal");
  g_signal_emit_by_name (text_view, signal);
}

static void
append_action_signal (TextView  *text_view,
		      GtkWidget    *menu,
		      const gchar  *stock_id,
		      const gchar  *signal,
                      bool      sensitive)
{
  GtkWidget *menuitem = gtk_image_menu_item_new_from_stock (stock_id, NULL);

  g_object_set_data (G_OBJECT (menuitem), I_("gtk-signal"), (char *)signal);
  g_signal_connect (menuitem, "activate",
		    G_CALLBACK (activate_cb), text_view);

  gtk_widget_set_sensitive (menuitem, sensitive);
  
  gtk_widget_show (menuitem);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
}*/

/*
void TextView::select_all (GtkWidget *widget,
			  bool select)
{
 // TextView *text_view = GTK_TEXT_VIEW (widget);
//  TextBuffer *buffer;
  TextIter start_iter, end_iter, insert;

  //buffer = text_view->buffer;
  if (select) 
    {
      buffer->get_bounds (&start_iter, &end_iter);
      buffer->select_range (&start_iter, &end_iter);
    }
  else 
    {
      buffer->get_iter_at_mark (buffer, &insert,
					buffer->get_insert ());
      buffer->move_mark_by_name (buffer, "selection_bound", &insert);
    }
}*/

/*
static void
select_all_cb (GtkWidget   *menuitem,
	       TextView *text_view)
{
  gtk_text_view_select_all (GTK_WIDGET (text_view), true);
}

static void
delete_cb (TextView *text_view)
{
  buffer->delete_selection (true,
				    text_view->editable);
}

static void
popup_menu_detach (GtkWidget *attach_widget,
		   GtkMenu   *menu)
{
  GTK_TEXT_VIEW (attach_widget)->popup_menu = NULL;
}

static void
popup_position_func (GtkMenu   *menu,
                     gint      *x,
                     gint      *y,
                     bool  *push_in,
                     gpointer	user_data)
{
  TextView *text_view;
  GtkWidget *widget;
  GdkRect cursor_rect;
  GdkRect onscreen_rect;
  gint root_x, root_y;
  TextIter iter;
  Rect req;      
  GdkScreen *screen;
  gint monitor_num;
  GdkRect monitor;
      
  text_view = GTK_TEXT_VIEW (user_data);
  widget = GTK_WIDGET (text_view);
  
  g_return_if_fail (GTK_WIDGET_REALIZED (text_view));
  
  screen = gtk_widget_get_screen (widget);

  gdk_window_get_origin (widget->window, &root_x, &root_y);

  buffer->get_iter_at_mark (
                                    &iter,
                                    buffer->get_insert ());

  gtk_text_view_get_iter_location (text_view,
                                   &iter,
                                   &cursor_rect);

  gtk_text_view_get_visible_rect (text_view, &onscreen_rect);
  
  gtk_widget_size_request (text_view->popup_menu, &req);

  * can't use rectangle_intersect since cursor rect can have 0 width *
  if (cursor_rect.x >= onscreen_rect.x &&
      cursor_rect.x < onscreen_rect.x + onscreen_rect.width &&
      cursor_rect.y >= onscreen_rect.y &&
      cursor_rect.y < onscreen_rect.y + onscreen_rect.height)
    {    
      gtk_text_view_buffer_to_window_coords (text_view,
                                             GTK_TEXT_WINDOW_WIDGET,
                                             cursor_rect.x, cursor_rect.y,
                                             &cursor_rect.x, &cursor_rect.y);

      *x = root_x + cursor_rect.x + cursor_rect.width;
      *y = root_y + cursor_rect.y + cursor_rect.height;
    }
  else
    {
      * Just center the menu, since cursor is offscreen. *      
      *x = root_x + (widget->allocation.width / 2 - req.width / 2);
      *y = root_y + (widget->allocation.height / 2 - req.height / 2);      
    }
  
  * Ensure sanity *
  *x = CLAMP (*x, root_x, (root_x + widget->allocation.width));
  *y = CLAMP (*y, root_y, (root_y + widget->allocation.height));

  monitor_num = gdk_screen_get_monitor_at_point (screen, *x, *y);
  gtk_menu_set_monitor (menu, monitor_num);
  gdk_screen_get_monitor_geometry (screen, monitor_num, &monitor);

  *x = CLAMP (*x, monitor.x, monitor.x + MAX (0, monitor.width - req.width));
  *y = CLAMP (*y, monitor.y, monitor.y + MAX (0, monitor.height - req.height));

  *push_in = false;
}
*/
/*
typedef struct
{
  TextView *text_view;
  gint button;
  guint time;
} PopupInfo;
*/

bool
TextView::range_contains_editable_text (const TextIter *start,
                              TextIter *end,
                              bool default_editability)
{
  TextIter iter = *start;

  while (TextIter::compare (&iter, end) < 0)
    {
      if (iter.editable (default_editability))
        return true;
      
      iter.forward_to_tag_toggle (NULL);
    }

  return false;
}                             

/*
static void
unichar_chosen_func (const char *text,
                     gpointer    data)
{
  //TextView *text_view = GTK_TEXT_VIEW (data);

  commit_text (text);
}*/

/* TODO implement popup menu for textview?
static void
popup_targets_received (GtkClipboard     *clipboard,
			GtkSelectionData *data,
			gpointer          user_data)
{
  PopupInfo *info = user_data;
  TextView *text_view = info->text_view;
  
  if (GTK_WIDGET_REALIZED (text_view))
    {
      * We implicitely rely here on the fact that if we are pasting ourself, we'll
       * have text targets as well as the private GTK_TEXT_BUFFER_CONTENTS target.
       *
      bool clipboard_contains_text;
      GtkWidget *menuitem;
      GtkWidget *submenu;
      bool have_selection;
      bool can_insert;
      TextIter iter;
      TextIter sel_start, sel_end;
      bool show_input_method_menu;
      bool show_unicode_menu;
      
      clipboard_contains_text = gtk_selection_data_targets_include_text (data);

      if (text_view->popup_menu)
	gtk_widget_destroy (text_view->popup_menu);

      text_view->popup_menu = gtk_menu_new ();
      
      gtk_menu_attach_to_widget (GTK_MENU (text_view->popup_menu),
				 GTK_WIDGET (text_view),
				 popup_menu_detach);
      
      have_selection = gtk_text_buffer_get_selection_bounds (buffer,
                                                             &sel_start, &sel_end);
      
      buffer->get_iter_at_mark (
					&iter,
					buffer->get_insert ());
      
      can_insert = gtk_text_iter_can_insert (&iter, text_view->editable);
      
      append_action_signal (text_view, text_view->popup_menu, GTK_STOCK_CUT, "cut-clipboard",
			    have_selection &&
                            range_contains_editable_text (&sel_start, &sel_end,
                                                          text_view->editable));
      append_action_signal (text_view, text_view->popup_menu, GTK_STOCK_COPY, "copy-clipboard",
			    have_selection);
      append_action_signal (text_view, text_view->popup_menu, GTK_STOCK_PASTE, "paste-clipboard",
			    can_insert && clipboard_contains_text);
      
      menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_DELETE, NULL);
      gtk_widget_set_sensitive (menuitem, 
				have_selection &&
				range_contains_editable_text (&sel_start, &sel_end,
							      text_view->editable));
      g_signal_connect_swapped (menuitem, "activate",
			        G_CALLBACK (delete_cb), text_view);
      gtk_widget_show (menuitem);
      gtk_menu_shell_append (GTK_MENU_SHELL (text_view->popup_menu), menuitem);

      menuitem = gtk_separator_menu_item_new ();
      gtk_widget_show (menuitem);
      gtk_menu_shell_append (GTK_MENU_SHELL (text_view->popup_menu), menuitem);

      menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_SELECT_ALL, NULL);
      g_signal_connect (menuitem, "activate",
			G_CALLBACK (select_all_cb), text_view);
      gtk_widget_show (menuitem);
      gtk_menu_shell_append (GTK_MENU_SHELL (text_view->popup_menu), menuitem);

      g_object_get (gtk_widget_get_settings (GTK_WIDGET (text_view)),
                    "gtk-show-input-method-menu", &show_input_method_menu,
                    "gtk-show-unicode-menu", &show_unicode_menu,
                    NULL);
      
      if (show_input_method_menu || show_unicode_menu)
        {
	  menuitem = gtk_separator_menu_item_new ();
	  gtk_widget_show (menuitem);
	  gtk_menu_shell_append (GTK_MENU_SHELL (text_view->popup_menu), menuitem);
	}

      if (show_input_method_menu)
        {
	  menuitem = gtk_menu_item_new_with_mnemonic (_("Input _Methods"));
	  gtk_widget_show (menuitem);
	  gtk_widget_set_sensitive (menuitem, can_insert);

	  submenu = gtk_menu_new ();
	  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), submenu);
	  gtk_menu_shell_append (GTK_MENU_SHELL (text_view->popup_menu), menuitem);
	  
	  gtk_im_multicontext_append_menuitems (GTK_IM_MULTICONTEXT (text_view->im_context),
						GTK_MENU_SHELL (submenu));
	}

      if (show_unicode_menu)
        {
	  menuitem = gtk_menu_item_new_with_mnemonic (_("_Insert Unicode Control Character"));
	  gtk_widget_show (menuitem);
	  gtk_widget_set_sensitive (menuitem, can_insert);
      
	  submenu = gtk_menu_new ();
	  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), submenu);
	  gtk_menu_shell_append (GTK_MENU_SHELL (text_view->popup_menu), menuitem);      
	  
	  _gtk_text_util_append_special_char_menuitems (GTK_MENU_SHELL (submenu),
							unichar_chosen_func,
							text_view);
	}
	  
      g_signal_emit (text_view,
		     signals[POPULATE_POPUP],
		     0,
		     text_view->popup_menu);
      
      if (info->button)
	gtk_menu_popup (GTK_MENU (text_view->popup_menu), NULL, NULL,
			NULL, NULL,
			info->button, info->time);
      else
	{
	  gtk_menu_popup (GTK_MENU (text_view->popup_menu), NULL, NULL,
			  popup_position_func, text_view,
			  0, gtk_get_current_event_time ());
	  gtk_menu_shell_select_first (GTK_MENU_SHELL (text_view->popup_menu), false);
	}
    }

  g_object_unref (text_view);
  g_free (info);
}

static void
TextView::do_popup (
                        GdkEventButton *event)
{
  PopupInfo *info = g_new (PopupInfo, 1);

  * In order to know what entries we should make sensitive, we
   * ask for the current targets of the clipboard, and when
   * we get them, then we actually pop up the menu.
   *
  info->text_view = g_object_ref (text_view);
  
  if (event)
    {
      info->button = event->button;
      info->time = event->time;
    }
  else
    {
      info->button = 0;
      info->time = gtk_get_current_event_time ();
    }

  gtk_clipboard_request_contents (gtk_widget_get_clipboard (GTK_WIDGET (text_view),
							    GDK_SELECTION_CLIPBOARD),
				  gdk_atom_intern_static_string ("TARGETS"),
				  popup_targets_received,
				  info);
}

bool
TextView::popup_menu ()
{
  do_popup (NULL);  
  return true;
}*/

/* Child GdkWindows */

/*
static GtkTextWindow*
text_window_new (GtkTextWindowType  type,
                 GtkWidget         *widget,
                 gint               width_request,
                 gint               height_request)
{
  GtkTextWindow *win;

  win = g_new (GtkTextWindow, 1);

  win->type = type;
  win->widget = widget;
  win->window = NULL;
  win->bin_window = NULL;
  win->requisition.width = width_request;
  win->requisition.height = height_request;
  win->allocation.width = width_request;
  win->allocation.height = height_request;
  win->allocation.x = 0;
  win->allocation.y = 0;

  return win;
}

static void
text_window_free (GtkTextWindow *win)
{
  if (win->window)
    text_window_unrealize (win);

  g_free (win);
}

static void
text_window_realize (GtkTextWindow *win,
                     GtkWidget     *widget)
{
  GdkWindowAttr attributes;
  gint attributes_mask;
  GdkCursor *cursor;

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = win->allocation.x;
  attributes.y = win->allocation.y;
  attributes.width = win->allocation.width;
  attributes.height = win->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (win->widget);
  attributes.colormap = gtk_widget_get_colormap (win->widget);
  attributes.event_mask = GDK_VISIBILITY_NOTIFY_MASK;

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

  win->window = gdk_window_new (widget->window,
                                &attributes,
                                attributes_mask);

  gdk_window_set_back_pixmap (win->window, NULL, false);
  
  gdk_window_show (win->window);
  gdk_window_set_user_data (win->window, win->widget);
  gdk_window_lower (win->window);

  attributes.x = 0;
  attributes.y = 0;
  attributes.width = win->allocation.width;
  attributes.height = win->allocation.height;
  attributes.event_mask = (GDK_EXPOSURE_MASK            |
                           GDK_SCROLL_MASK              |
                           GDK_KEY_PRESS_MASK           |
                           GDK_BUTTON_PRESS_MASK        |
                           GDK_BUTTON_RELEASE_MASK      |
                           GDK_POINTER_MOTION_MASK      |
                           GDK_POINTER_MOTION_HINT_MASK |
                           gtk_widget_get_events (win->widget));

  win->bin_window = gdk_window_new (win->window,
                                    &attributes,
                                    attributes_mask);

  gdk_window_show (win->bin_window);
  gdk_window_set_user_data (win->bin_window, win->widget);

  if (win->type == GTK_TEXT_WINDOW_TEXT)
    {
      if (GTK_WIDGET_IS_SENSITIVE (widget))
        {
          * I-beam cursor *
          cursor = gdk_cursor_new_for_display (gdk_drawable_get_display (widget->window),
					       GDK_XTERM);
          gdk_window_set_cursor (win->bin_window, cursor);
          gdk_cursor_unref (cursor);
        } 

      gtk_im_context_set_client_window (GTK_TEXT_VIEW (widget)->im_context,
                                        win->window);


      gdk_window_set_background (win->bin_window,
                                 &widget->style->base[GTK_WIDGET_STATE (widget)]);
    }
  else
    {
      gdk_window_set_background (win->bin_window,
                                 &widget->style->bg[GTK_WIDGET_STATE (widget)]);
    }

  g_object_set_qdata (G_OBJECT (win->window),
                      g_quark_from_static_string ("gtk-text-view-text-window"),
                      win);

  g_object_set_qdata (G_OBJECT (win->bin_window),
                      g_quark_from_static_string ("gtk-text-view-text-window"),
                      win);
}

static void
text_window_unrealize (GtkTextWindow *win)
{
  if (win->type == GTK_TEXT_WINDOW_TEXT)
    {
      gtk_im_context_set_client_window (GTK_TEXT_VIEW (win->widget)->im_context,
                                        NULL);
    }

  gdk_window_set_user_data (win->window, NULL);
  gdk_window_set_user_data (win->bin_window, NULL);
  gdk_window_destroy (win->bin_window);
  gdk_window_destroy (win->window);
  win->window = NULL;
  win->bin_window = NULL;
}

static void
text_window_size_allocate (GtkTextWindow *win,
                           GdkRect  *rect)
{
  win->allocation = *rect;

  if (win->window)
    {
      gdk_window_move_resize (win->window,
                              rect->x, rect->y,
                              rect->width, rect->height);

      gdk_window_resize (win->bin_window,
                         rect->width, rect->height);
    }
}

static void
text_window_scroll        (GtkTextWindow *win,
                           gint           dx,
                           gint           dy)
{
  if (dx != 0 || dy != 0)
    {
      gdk_window_scroll (win->bin_window, dx, dy);
    }
}

static void
text_window_invalidate_rect (GtkTextWindow *win,
                             GdkRect  *rect)
{
  GdkRect window_rect;

  gtk_text_view_buffer_to_window_coords (GTK_TEXT_VIEW (win->widget),
                                         win->type,
                                         rect->x,
                                         rect->y,
                                         &window_rect.x,
                                         &window_rect.y);

  window_rect.width = rect->width;
  window_rect.height = rect->height;
  
  * Adjust the rect as appropriate *
  
  switch (win->type)
    {
    case GTK_TEXT_WINDOW_TEXT:
      break;

    case GTK_TEXT_WINDOW_LEFT:
    case GTK_TEXT_WINDOW_RIGHT:
      window_rect.x = 0;
      window_rect.width = win->allocation.width;
      break;

    case GTK_TEXT_WINDOW_TOP:
    case GTK_TEXT_WINDOW_BOTTOM:
      window_rect.y = 0;
      window_rect.height = win->allocation.height;
      break;

    default:
      g_warning ("%s: bug!", G_STRFUNC);
      return;
      break;
    }
          
  gdk_window_invalidate_rect (win->bin_window, &window_rect, false);

#if 0
  {
    cairo_t *cr = gdk_cairo_create (win->bin_window);
    gdk_cairo_rectangle (cr, &window_rect);
    cairo_set_source_rgb  (cr, 1.0, 0.0, 0.0);	* red *
    cairo_fill (cr);
    cairo_destroy (cr);
  }
#endif
}*/

/*
static void
text_window_invalidate_cursors (GtkTextWindow *win)
{
  TextView *text_view = GTK_TEXT_VIEW (win->widget);
  TextIter  iter;
  GdkRect strong;
  GdkRect weak;
  bool     draw_arrow;
  gfloat       cursor_aspect_ratio;
  gint         stem_width;
  gint         arrow_width;

  gtk_text_buffer_get_iter_at_mark (text_view->buffer, &iter,
                                    gtk_text_buffer_get_insert (text_view->buffer));

  if (_gtk_text_layout_get_block_cursor (text_view->layout, &strong))
    {
      text_window_invalidate_rect (win, &strong);
      return;
    }

  gtk_text_layout_get_cursor_locations (text_view->layout, &iter,
                                        &strong, &weak);

  * cursor width calculation as in gtkstyle.c:draw_insertion_cursor(),
   * ignoring the text direction be exposing both sides of the cursor
   *

  draw_arrow = (strong.x != weak.x || strong.y != weak.y);

  gtk_widget_style_get (win->widget,
                        "cursor-aspect-ratio", &cursor_aspect_ratio,
                        NULL);
  
  stem_width = strong.height * cursor_aspect_ratio + 1;
  arrow_width = stem_width + 1;

  strong.width = stem_width;

  * round up to the next even number *
  if (stem_width & 1)
    stem_width++;

  strong.x     -= stem_width / 2;
  strong.width += stem_width;

  if (draw_arrow)
    {
      strong.x     -= arrow_width;
      strong.width += arrow_width * 2;
    }

  text_window_invalidate_rect (win, &strong);

  if (draw_arrow) * == have weak *
    {
      stem_width = weak.height * cursor_aspect_ratio + 1;
      arrow_width = stem_width + 1;

      weak.width = stem_width;

      * round up to the next even number *
      if (stem_width & 1)
        stem_width++;

      weak.x     -= stem_width / 2;
      weak.width += stem_width;

      weak.x     -= arrow_width;
      weak.width += arrow_width * 2;

      text_window_invalidate_rect (win, &weak);
    }
}

static gint
text_window_get_width (GtkTextWindow *win)
{
  return win->allocation.width;
}

static gint
text_window_get_height (GtkTextWindow *win)
{
  return win->allocation.height;
}*/

/* Windows */


/**
 * gtk_text_view_get_window:
 * @text_view: a #TextView
 * @win: window to get
 *
 * Retrieves the #GdkWindow corresponding to an area of the text view;
 * possible windows include the overall widget window, child windows
 * on the left, right, top, bottom, and the window that displays the
 * text buffer. Windows are %NULL and nonexistent if their width or
 * height is 0, and are nonexistent before the widget has been
 * realized.
 *
 * Return value: a #GdkWindow, or %NULL
 **/
/*
GdkWindow*
TextView::get_window (TextView *text_view,
                          GtkTextWindowType win)
{
  g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), NULL);

  switch (win)
    {
    case GTK_TEXT_WINDOW_WIDGET:
      return GTK_WIDGET (text_view)->window;
      break;

    case GTK_TEXT_WINDOW_TEXT:
      return text_view->text_window->bin_window;
      break;

    case GTK_TEXT_WINDOW_LEFT:
      if (text_view->left_window)
        return text_view->left_window->bin_window;
      else
        return NULL;
      break;

    case GTK_TEXT_WINDOW_RIGHT:
      if (text_view->right_window)
        return text_view->right_window->bin_window;
      else
        return NULL;
      break;

    case GTK_TEXT_WINDOW_TOP:
      if (text_view->top_window)
        return text_view->top_window->bin_window;
      else
        return NULL;
      break;

    case GTK_TEXT_WINDOW_BOTTOM:
      if (text_view->bottom_window)
        return text_view->bottom_window->bin_window;
      else
        return NULL;
      break;

    case GTK_TEXT_WINDOW_PRIVATE:
      g_warning ("%s: You can't get GTK_TEXT_WINDOW_PRIVATE, it has \"PRIVATE\" in the name because it is private.", G_STRFUNC);
      return NULL;
      break;
    }

  g_warning ("%s: Unknown GtkTextWindowType", G_STRFUNC);
  return NULL;
}*/

/**
 * gtk_text_view_get_window_type:
 * @text_view: a #TextView
 * @window: a window type
 *
 * Usually used to find out which window an event corresponds to.
 * If you connect to an event signal on @text_view, this function
 * should be called on <literal>event-&gt;window</literal> to
 * see which window it was.
 *
 * Return value: the window type.
 **/
/*
GtkTextWindowType
TextView::get_window_type (TextView *text_view,
                               GdkWindow   *window)
{
  GtkTextWindow *win;

  g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), 0);
  g_return_val_if_fail (GDK_IS_WINDOW (window), 0);

  if (window == GTK_WIDGET (text_view)->window)
    return GTK_TEXT_WINDOW_WIDGET;

  win = g_object_get_qdata (G_OBJECT (window),
                            g_quark_try_string ("gtk-text-view-text-window"));

  if (win)
    return win->type;
  else
    {
      return GTK_TEXT_WINDOW_PRIVATE;
    }
}

static void
buffer_to_widget (TextView      *text_view,
                  gint              buffer_x,
                  gint              buffer_y,
                  gint             *window_x,
                  gint             *window_y)
{  
  if (window_x)
    {
      *window_x = buffer_x - text_view->xoffset;
      *window_x += text_view->text_window->allocation.x;
    }

  if (window_y)
    {
      *window_y = buffer_y - text_view->yoffset;
      *window_y += text_view->text_window->allocation.y;
    }
}

static void
widget_to_text_window (GtkTextWindow *win,
                       gint           widget_x,
                       gint           widget_y,
                       gint          *window_x,
                       gint          *window_y)
{
  if (window_x)
    *window_x = widget_x - win->allocation.x;

  if (window_y)
    *window_y = widget_y - win->allocation.y;
}

static void
buffer_to_text_window (TextView   *text_view,
                       GtkTextWindow *win,
                       gint           buffer_x,
                       gint           buffer_y,
                       gint          *window_x,
                       gint          *window_y)
{
  if (win == NULL)
    {
      g_warning ("Attempt to convert text buffer coordinates to coordinates "
                 "for a nonexistent or private child window of TextView");
      return;
    }

  buffer_to_widget (text_view,
                    buffer_x, buffer_y,
                    window_x, window_y);

  widget_to_text_window (win,
                         window_x ? *window_x : 0,
                         window_y ? *window_y : 0,
                         window_x,
                         window_y);
}*/

/**
 * gtk_text_view_buffer_to_window_coords:
 * @text_view: a #TextView
 * @win: a #GtkTextWindowType except #GTK_TEXT_WINDOW_PRIVATE
 * @buffer_x: buffer x coordinate
 * @buffer_y: buffer y coordinate
 * @window_x: window x coordinate return location
 * @window_y: window y coordinate return location
 *
 * Converts coordinate (@buffer_x, @buffer_y) to coordinates for the window
 * @win, and stores the result in (@window_x, @window_y). 
 *
 * Note that you can't convert coordinates for a nonexisting window (see 
 * gtk_text_view_set_border_window_size()).
 **/
/*
void
TextView::buffer_to_window_coords (TextView      *text_view,
                                       GtkTextWindowType win,
                                       gint              buffer_x,
                                       gint              buffer_y,
                                       gint             *window_x,
                                       gint             *window_y)
{
  //g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))

  switch (win)
    {
    case GTK_TEXT_WINDOW_WIDGET:
      buffer_to_widget (text_view,
                        buffer_x, buffer_y,
                        window_x, window_y);
      break;

    case GTK_TEXT_WINDOW_TEXT:
      if (window_x)
        *window_x = buffer_x - text_view->xoffset;
      if (window_y)
        *window_y = buffer_y - text_view->yoffset;
      break;

    case GTK_TEXT_WINDOW_LEFT:
      buffer_to_text_window (text_view,
                             text_view->left_window,
                             buffer_x, buffer_y,
                             window_x, window_y);
      break;

    case GTK_TEXT_WINDOW_RIGHT:
      buffer_to_text_window (text_view,
                             text_view->right_window,
                             buffer_x, buffer_y,
                             window_x, window_y);
      break;

    case GTK_TEXT_WINDOW_TOP:
      buffer_to_text_window (text_view,
                             text_view->top_window,
                             buffer_x, buffer_y,
                             window_x, window_y);
      break;

    case GTK_TEXT_WINDOW_BOTTOM:
      buffer_to_text_window (text_view,
                             text_view->bottom_window,
                             buffer_x, buffer_y,
                             window_x, window_y);
      break;

    case GTK_TEXT_WINDOW_PRIVATE:
      g_warning ("%s: can't get coords for private windows", G_STRFUNC);
      break;

    default:
      g_warning ("%s: Unknown GtkTextWindowType", G_STRFUNC);
      break;
    }
}

static void
widget_to_buffer (TextView *text_view,
                  gint         widget_x,
                  gint         widget_y,
                  gint        *buffer_x,
                  gint        *buffer_y)
{  
  if (buffer_x)
    {
      *buffer_x = widget_x + text_view->xoffset;
      *buffer_x -= text_view->text_window->allocation.x;
    }

  if (buffer_y)
    {
      *buffer_y = widget_y + text_view->yoffset;
      *buffer_y -= text_view->text_window->allocation.y;
    }
}

static void
text_window_to_widget (GtkTextWindow *win,
                       gint           window_x,
                       gint           window_y,
                       gint          *widget_x,
                       gint          *widget_y)
{
  if (widget_x)
    *widget_x = window_x + win->allocation.x;

  if (widget_y)
    *widget_y = window_y + win->allocation.y;
}

static void
text_window_to_buffer (TextView   *text_view,
                       GtkTextWindow *win,
                       gint           window_x,
                       gint           window_y,
                       gint          *buffer_x,
                       gint          *buffer_y)
{
  if (win == NULL)
    {
      g_warning ("Attempt to convert TextView buffer coordinates into "
                 "coordinates for a nonexistent child window.");
      return;
    }

  text_window_to_widget (win,
                         window_x,
                         window_y,
                         buffer_x,
                         buffer_y);

  widget_to_buffer (text_view,
                    buffer_x ? *buffer_x : 0,
                    buffer_y ? *buffer_y : 0,
                    buffer_x,
                    buffer_y);
}*/

/**
 * gtk_text_view_window_to_buffer_coords:
 * @text_view: a #TextView
 * @win: a #GtkTextWindowType except #GTK_TEXT_WINDOW_PRIVATE
 * @window_x: window x coordinate
 * @window_y: window y coordinate
 * @buffer_x: buffer x coordinate return location
 * @buffer_y: buffer y coordinate return location
 *
 * Converts coordinates on the window identified by @win to buffer
 * coordinates, storing the result in (@buffer_x,@buffer_y).
 *
 * Note that you can't convert coordinates for a nonexisting window (see 
 * gtk_text_view_set_border_window_size()).
 **/
/*
void
TextView::window_to_buffer_coords (TextView      *text_view,
                                       GtkTextWindowType win,
                                       gint              window_x,
                                       gint              window_y,
                                       gint             *buffer_x,
                                       gint             *buffer_y)
{
  //g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))

  switch (win)
    {
    case GTK_TEXT_WINDOW_WIDGET:
      widget_to_buffer (text_view,
                        window_x, window_y,
                        buffer_x, buffer_y);
      break;

    case GTK_TEXT_WINDOW_TEXT:
      if (buffer_x)
        *buffer_x = window_x + text_view->xoffset;
      if (buffer_y)
        *buffer_y = window_y + text_view->yoffset;
      break;

    case GTK_TEXT_WINDOW_LEFT:
      text_window_to_buffer (text_view,
                             text_view->left_window,
                             window_x, window_y,
                             buffer_x, buffer_y);
      break;

    case GTK_TEXT_WINDOW_RIGHT:
      text_window_to_buffer (text_view,
                             text_view->right_window,
                             window_x, window_y,
                             buffer_x, buffer_y);
      break;

    case GTK_TEXT_WINDOW_TOP:
      text_window_to_buffer (text_view,
                             text_view->top_window,
                             window_x, window_y,
                             buffer_x, buffer_y);
      break;

    case GTK_TEXT_WINDOW_BOTTOM:
      text_window_to_buffer (text_view,
                             text_view->bottom_window,
                             window_x, window_y,
                             buffer_x, buffer_y);
      break;

    case GTK_TEXT_WINDOW_PRIVATE:
      g_warning ("%s: can't get coords for private windows", G_STRFUNC);
      break;

    default:
      g_warning ("%s: Unknown GtkTextWindowType", G_STRFUNC);
      break;
    }
}*/

/*
static void
set_window_width (TextView      *text_view,
                  gint              width,
                  GtkTextWindowType type,
                  GtkTextWindow   **winp)
{
  if (width == 0)
    {
      if (*winp)
        {
          text_window_free (*winp);
          *winp = NULL;
          gtk_widget_queue_resize (GTK_WIDGET (text_view));
        }
    }
  else
    {
      if (*winp == NULL)
        {
          *winp = text_window_new (type,
                                   GTK_WIDGET (text_view),
                                   width, 0);
          * if the widget is already realized we need to realize the child manually *
          if (GTK_WIDGET_REALIZED (text_view))
            text_window_realize (*winp, GTK_WIDGET (text_view));
        }
      else
        {
          if ((*winp)->requisition.width == width)
            return;

          (*winp)->requisition.width = width;
        }

      gtk_widget_queue_resize (GTK_WIDGET (text_view));
    }
}*/

/*
static void
set_window_height (TextView      *text_view,
                   gint              height,
                   GtkTextWindowType type,
                   GtkTextWindow   **winp)
{
  if (height == 0)
    {
      if (*winp)
        {
          text_window_free (*winp);
          *winp = NULL;
          gtk_widget_queue_resize (GTK_WIDGET (text_view));
        }
    }
  else
    {
      if (*winp == NULL)
        {
          *winp = text_window_new (type,
                                   GTK_WIDGET (text_view),
                                   0, height);

          * if the widget is already realized we need to realize the child manually *
          if (GTK_WIDGET_REALIZED (text_view))
            text_window_realize (*winp, GTK_WIDGET (text_view));
        }
      else
        {
          if ((*winp)->requisition.height == height)
            return;

          (*winp)->requisition.height = height;
        }

      gtk_widget_queue_resize (GTK_WIDGET (text_view));
    }
}*/

/**
 * gtk_text_view_set_border_window_size:
 * @text_view: a #TextView
 * @type: window to affect
 * @size: width or height of the window
 *
 * Sets the width of %GTK_TEXT_WINDOW_LEFT or %GTK_TEXT_WINDOW_RIGHT,
 * or the height of %GTK_TEXT_WINDOW_TOP or %GTK_TEXT_WINDOW_BOTTOM.
 * Automatically destroys the corresponding window if the size is set
 * to 0, and creates the window if the size is set to non-zero.  This
 * function can only be used for the "border windows," it doesn't work
 * with #GTK_TEXT_WINDOW_WIDGET, #GTK_TEXT_WINDOW_TEXT, or
 * #GTK_TEXT_WINDOW_PRIVATE.
 **/
/*
void
TextView::set_border_window_size (TextView      *text_view,
                                      GtkTextWindowType type,
                                      gint              size)

{
  //g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))
  g_return_if_fail (size >= 0);

  switch (type)
    {
    case GTK_TEXT_WINDOW_LEFT:
      set_window_width (text_view, size, GTK_TEXT_WINDOW_LEFT,
                        &text_view->left_window);
      break;

    case GTK_TEXT_WINDOW_RIGHT:
      set_window_width (text_view, size, GTK_TEXT_WINDOW_RIGHT,
                        &text_view->right_window);
      break;

    case GTK_TEXT_WINDOW_TOP:
      set_window_height (text_view, size, GTK_TEXT_WINDOW_TOP,
                         &text_view->top_window);
      break;

    case GTK_TEXT_WINDOW_BOTTOM:
      set_window_height (text_view, size, GTK_TEXT_WINDOW_BOTTOM,
                         &text_view->bottom_window);
      break;

    default:
      g_warning ("Can only set size of left/right/top/bottom border windows with gtk_text_view_set_border_window_size()");
      break;
    }
}*/

/**
 * gtk_text_view_get_border_window_size:
 * @text_view: a #TextView
 * @type: window to return size from
 *
 * Gets the width of the specified border window. See
 * gtk_text_view_set_border_window_size().
 *
 * Return value: width of window
 **/
/*
gint
TextView::get_border_window_size (TextView       *text_view,
				      GtkTextWindowType  type)
{
  g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), 0);
  
  switch (type)
    {
    case GTK_TEXT_WINDOW_LEFT:
      if (text_view->left_window)
        return text_view->left_window->requisition.width;
      break;
      
    case GTK_TEXT_WINDOW_RIGHT:
      if (text_view->right_window)
        return text_view->right_window->requisition.width;
      break;
      
    case GTK_TEXT_WINDOW_TOP:
      if (text_view->top_window)
        return text_view->top_window->requisition.height;
      break;

    case GTK_TEXT_WINDOW_BOTTOM:
      if (text_view->bottom_window)
        return text_view->bottom_window->requisition.height;
      break;
      
    default:
      g_warning ("Can only get size of left/right/top/bottom border windows with gtk_text_view_get_border_window_size()");
      break;
    }

  return 0;
}*/

/*
 * Child widgets
 */
/*
static TextViewChild*
text_view_child_new_anchored (GtkWidget          *child,
                              GtkTextChildAnchor *anchor,
                              GtkTextLayout      *layout)
{
  TextViewChild *vc;

  vc = g_new (TextViewChild, 1);

  vc->type = GTK_TEXT_WINDOW_PRIVATE;
  vc->widget = child;
  vc->anchor = anchor;

  vc->from_top_of_line = 0;
  vc->from_left_of_buffer = 0;
  
  g_object_ref (vc->widget);
  g_object_ref (vc->anchor);

  g_object_set_data (G_OBJECT (child),
                     I_("gtk-text-view-child"),
                     vc);

  gtk_text_child_anchor_register_child (anchor, child, layout);
  
  return vc;
}

static TextViewChild*
text_view_child_new_window (GtkWidget          *child,
                            GtkTextWindowType   type,
                            gint                x,
                            gint                y)
{
  TextViewChild *vc;

  vc = g_new (TextViewChild, 1);

  vc->widget = child;
  vc->anchor = NULL;

  vc->from_top_of_line = 0;
  vc->from_left_of_buffer = 0;
 
  g_object_ref (vc->widget);

  vc->type = type;
  vc->x = x;
  vc->y = y;

  g_object_set_data (G_OBJECT (child),
                     I_("gtk-text-view-child"),
                     vc);
  
  return vc;
}

static void
text_view_child_free (TextViewChild *child)
{
  g_object_set_data (G_OBJECT (child->widget),
                     I_("gtk-text-view-child"), NULL);

  if (child->anchor)
    {
      gtk_text_child_anchor_unregister_child (child->anchor,
                                              child->widget);
      g_object_unref (child->anchor);
    }

  g_object_unref (child->widget);

  g_free (child);
}

static void
text_view_child_set_parent_window (TextView      *text_view,
				   TextViewChild *vc)
{
  if (vc->anchor)
    gtk_widget_set_parent_window (vc->widget,
                                  text_view->text_window->bin_window);
  else
    {
      GdkWindow *window;
      window = gtk_text_view_get_window (text_view,
                                         vc->type);
      gtk_widget_set_parent_window (vc->widget, window);
    }
}

static void
add_child (TextView      *text_view,
           TextViewChild *vc)
{
  text_view->children = g_slist_prepend (text_view->children,
                                         vc);

  if (GTK_WIDGET_REALIZED (text_view))
    text_view_child_set_parent_window (text_view, vc);
  
  gtk_widget_set_parent (vc->widget, GTK_WIDGET (text_view));
}
*/

/**
 * gtk_text_view_add_child_at_anchor:
 * @text_view: a #TextView
 * @child: a #GtkWidget
 * @anchor: a #GtkTextChildAnchor in the #GtkTextBuffer for @text_view
 * 
 * Adds a child widget in the text buffer, at the given @anchor.
 **/
/*
void
TextView::add_child_at_anchor (TextView          *text_view,
                                   GtkWidget            *child,
                                   GtkTextChildAnchor   *anchor)
{
  TextViewChild *vc;

  //g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (GTK_IS_TEXT_CHILD_ANCHOR (anchor));
  g_return_if_fail (child->parent == NULL);

  ensure_layout();

  vc = text_view_child_new_anchored (child, anchor,
                                     text_view->layout);

  add_child (text_view, vc);

  g_assert (vc->widget == child);
  g_assert (gtk_widget_get_parent (child) == GTK_WIDGET (text_view));
}*/

/**
 * gtk_text_view_add_child_in_window:
 * @text_view: a #TextView
 * @child: a #GtkWidget
 * @which_window: which window the child should appear in
 * @xpos: X position of child in window coordinates
 * @ypos: Y position of child in window coordinates
 *
 * Adds a child at fixed coordinates in one of the text widget's
 * windows. The window must have nonzero size (see
 * gtk_text_view_set_border_window_size()). Note that the child
 * coordinates are given relative to the #GdkWindow in question, and
 * that these coordinates have no sane relationship to scrolling. When
 * placing a child in #GTK_TEXT_WINDOW_WIDGET, scrolling is
 * irrelevant, the child floats above all scrollable areas. But when
 * placing a child in one of the scrollable windows (border windows or
 * text window), you'll need to compute the child's correct position
 * in buffer coordinates any time scrolling occurs or buffer changes
 * occur, and then call gtk_text_view_move_child() to update the
 * child's position. Unfortunately there's no good way to detect that
 * scrolling has occurred, using the current API; a possible hack
 * would be to update all child positions when the scroll adjustments
 * change or the text buffer changes. See bug 64518 on
 * bugzilla.gnome.org for status of fixing this issue.
 **/
/*
void
TextView::add_child_in_window (TextView       *text_view,
                                   GtkWidget         *child,
                                   GtkTextWindowType  which_window,
                                   gint               xpos,
                                   gint               ypos)
{
  TextViewChild *vc;

  //g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (child->parent == NULL);

  vc = text_view_child_new_window (child, which_window,
                                   xpos, ypos);

  add_child (text_view, vc);

  g_assert (vc->widget == child);
  g_assert (gtk_widget_get_parent (child) == GTK_WIDGET (text_view));
}*/

/**
 * gtk_text_view_move_child:
 * @text_view: a #TextView
 * @child: child widget already added to the text view
 * @xpos: new X position in window coordinates
 * @ypos: new Y position in window coordinates
 *
 * Updates the position of a child, as for gtk_text_view_add_child_in_window().
 **/
/*
void
TextView::move_child (TextView *text_view,
                          GtkWidget   *child,
                          gint         xpos,
                          gint         ypos)
{
  TextViewChild *vc;

  //g_return_if_fail (GTK_IS_TEXT_VIEW (text_view))
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (child->parent == (GtkWidget*) text_view);

  vc = g_object_get_data (G_OBJECT (child),
                          "gtk-text-view-child");

  g_assert (vc != NULL);

  if (vc->x == xpos &&
      vc->y == ypos)
    return;
  
  vc->x = xpos;
  vc->y = ypos;

  if (GTK_WIDGET_VISIBLE (child) && GTK_WIDGET_VISIBLE (text_view))
    gtk_widget_queue_resize (child);
}*/


/* Iterator operations */

/**
 * gtk_text_view_forward_display_line:
 * @text_view: a #TextView
 * @iter: a #TextIter
 * 
 * Moves the given @iter forward by one display (wrapped) line.
 * A display line is different from a paragraph. Paragraphs are
 * separated by newlines or other paragraph separator characters.
 * Display lines are created by line-wrapping a paragraph. If
 * wrapping is turned off, display lines and paragraphs will be the
 * same. Display lines are divided differently for each view, since
 * they depend on the view's width; paragraphs are the same in all
 * views, since they depend on the contents of the #GtkTextBuffer.
 * 
 * Return value: %true if @iter was moved and is not on the end iterator
 **/
bool
TextView::forward_display_line (
                                    TextIter *iter)
{
 // g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), false);
  g_return_val_if_fail (iter != NULL, false);

  ensure_layout();

  return layout->move_iter_to_next_line (iter);
}

/**
 * gtk_text_view_backward_display_line:
 * @text_view: a #TextView
 * @iter: a #TextIter
 * 
 * Moves the given @iter backward by one display (wrapped) line.
 * A display line is different from a paragraph. Paragraphs are
 * separated by newlines or other paragraph separator characters.
 * Display lines are created by line-wrapping a paragraph. If
 * wrapping is turned off, display lines and paragraphs will be the
 * same. Display lines are divided differently for each view, since
 * they depend on the view's width; paragraphs are the same in all
 * views, since they depend on the contents of the #GtkTextBuffer.
 * 
 * Return value: %true if @iter was moved and is not on the end iterator
 **/
bool
TextView::backward_display_line (
                                     TextIter *iter)
{
 // g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), false);
  g_return_val_if_fail (iter != NULL, false);

  ensure_layout();

  return layout->move_iter_to_previous_line (iter);
}

/**
 * gtk_text_view_forward_display_line_end:
 * @text_view: a #TextView
 * @iter: a #TextIter
 * 
 * Moves the given @iter forward to the next display line end.
 * A display line is different from a paragraph. Paragraphs are
 * separated by newlines or other paragraph separator characters.
 * Display lines are created by line-wrapping a paragraph. If
 * wrapping is turned off, display lines and paragraphs will be the
 * same. Display lines are divided differently for each view, since
 * they depend on the view's width; paragraphs are the same in all
 * views, since they depend on the contents of the #GtkTextBuffer.
 * 
 * Return value: %true if @iter was moved and is not on the end iterator
 **/
bool
TextView::forward_display_line_end (
                                        TextIter *iter)
{
 // g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), false);
  g_return_val_if_fail (iter != NULL, false);

  ensure_layout();

  return layout->move_iter_to_line_end (iter, 1);
}

/**
 * gtk_text_view_backward_display_line_start:
 * @text_view: a #TextView
 * @iter: a #TextIter
 * 
 * Moves the given @iter backward to the next display line start.
 * A display line is different from a paragraph. Paragraphs are
 * separated by newlines or other paragraph separator characters.
 * Display lines are created by line-wrapping a paragraph. If
 * wrapping is turned off, display lines and paragraphs will be the
 * same. Display lines are divided differently for each view, since
 * they depend on the view's width; paragraphs are the same in all
 * views, since they depend on the contents of the #GtkTextBuffer.
 * 
 * Return value: %true if @iter was moved and is not on the end iterator
 **/
bool
TextView::backward_display_line_start (
                                           TextIter *iter)
{
 // g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), false);
  g_return_val_if_fail (iter != NULL, false);

  ensure_layout();

  return layout->move_iter_to_line_end (iter, -1);
}

/**
 * gtk_text_view_starts_display_line:
 * @text_view: a #TextView
 * @iter: a #TextIter
 * 
 * Determines whether @iter is at the start of a display line.
 * See gtk_text_view_forward_display_line() for an explanation of
 * display lines vs. paragraphs.
 * 
 * Return value: %true if @iter begins a wrapped line
 **/
bool TextView::starts_display_line ( TextIter *iter)
{
 // g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), false);
  g_return_val_if_fail (iter != NULL, false);

  ensure_layout();

  return layout->iter_starts_line (iter);
}

/**
 * gtk_text_view_move_visually:
 * @text_view: a #TextView
 * @iter: a #TextIter
 * @count: number of characters to move (negative moves left, 
 *    positive moves right)
 *
 * Move the iterator a given number of characters visually, treating
 * it as the strong cursor position. If @count is positive, then the
 * new strong cursor position will be @count positions to the right of
 * the old cursor position. If @count is negative then the new strong
 * cursor position will be @count positions to the left of the old
 * cursor position.
 *
 * In the presence of bi-directional text, the correspondence
 * between logical and visual order will depend on the direction
 * of the current run, and there may be jumps when the cursor
 * is moved off of the end of a run.
 * 
 * Return value: %true if @iter moved and is not on the end iterator
 **/
bool
TextView::move_visually (
                             TextIter *iter,
                             gint         count)
{
 // g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view), false);
  g_return_val_if_fail (iter != NULL, false);

  ensure_layout();

  return layout->move_iter_visually (iter, count);
}

void TextView::ActionMoveCursor(CursorMovement step, int count, bool extend_selection)
{
	move_cursor(step, count, extend_selection);
	Redraw();
}

void TextView::ActionSelectAll(bool select_all)
{
	if (select_all) {
		ActionMoveCursor(MOVE_BUFFER_ENDS, -1, false);
		ActionMoveCursor(MOVE_BUFFER_ENDS, 1, true);
	} else {
		ActionMoveCursor(MOVE_VISUAL_POSITIONS, 0, false);
	}
	Redraw();
}

void TextView::ActionDelete(DeleteType type, gint count)
{
	delete_from_cursor(type, count);
	Redraw();
}
   
void TextView::ActionBackspace(void)
{
	backspace();
	Redraw();
}

void TextView::ActionToggleOverwrite(void)
{
	toggle_overwrite();
}


void TextView::DeclareBindables(void)
{
	const gchar *context = "textentry";
	//TODO implement what is possible for textentry as well.

	/* cursor movement */

	/* character */
	DeclareBindable(context, "cursor-right",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionMoveCursor), MOVE_VISUAL_POSITIONS, 1, false),
		_("Move the cursor to the right."), InputProcessor::Bindable_Override);

	DeclareBindable(context, "cursor-left",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionMoveCursor), MOVE_VISUAL_POSITIONS, -1, false),
		_("Move the cursor to the left."), InputProcessor::Bindable_Override);

	/* word */
	DeclareBindable(context, "cursor-right-word",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionMoveCursor), MOVE_WORDS, 1, false),
		_("Move the cursor to the right by one word."), InputProcessor::Bindable_Override);

	DeclareBindable(context, "cursor-left-word",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionMoveCursor), MOVE_WORDS, -1, false),
		_("Move the cursor to the left by one word."), InputProcessor::Bindable_Override);

	/* line */
	DeclareBindable(context, "cursor-end-line",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionMoveCursor), MOVE_DISPLAY_LINE_ENDS, 1, false),
		_("Move the cursor to the end of the line."), InputProcessor::Bindable_Override);

	DeclareBindable(context, "cursor-begin-line",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionMoveCursor), MOVE_DISPLAY_LINE_ENDS, -1, false),
		_("Move the cursor to the beginning of the line."), InputProcessor::Bindable_Override);

	/* page */
	DeclareBindable(context, "cursor-down-page",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionMoveCursor), MOVE_HORIZONTAL_PAGES, 1, false),
		_("Move the cursor one page downwards."), InputProcessor::Bindable_Override);

	DeclareBindable(context, "cursor-up-page",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionMoveCursor), MOVE_HORIZONTAL_PAGES, -1, false),
		_("Move the cursor one page upwards."), InputProcessor::Bindable_Override);

	/* buffer */
	DeclareBindable(context, "cursor-end",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionMoveCursor), MOVE_BUFFER_ENDS, 1, false),
		_("Move the cursor to the end of the text."), InputProcessor::Bindable_Override);

	DeclareBindable(context, "cursor-begin",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionMoveCursor), MOVE_BUFFER_ENDS, -1, false),
		_("Move the cursor to the beginning of the text."), InputProcessor::Bindable_Override);

	/* viewport movement */
	//TODO all cursor movement bindables for viewport as well

	/* cursor movement selection extension variants */

	/* character */
	DeclareBindable(context, "selection-right",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionMoveCursor), MOVE_VISUAL_POSITIONS, 1, true),
		_("Extend the selection to the right."), InputProcessor::Bindable_Override);

	DeclareBindable(context, "selection-left",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionMoveCursor), MOVE_VISUAL_POSITIONS, -1, true),
		_("Extend the selection to the left."), InputProcessor::Bindable_Override);

	/* word */
	DeclareBindable(context, "selection-right-word",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionMoveCursor), MOVE_WORDS, 1, true),
		_("Extend the selection to the right by one word."), InputProcessor::Bindable_Override);

	DeclareBindable(context, "selection-left-word",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionMoveCursor), MOVE_WORDS, -1, true),
		_("Extend the selection to the left by one word."), InputProcessor::Bindable_Override);

	/* line */
	DeclareBindable(context, "selection-end-line",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionMoveCursor), MOVE_DISPLAY_LINE_ENDS, 1, true),
		_("Extend the selection to the end of the line"), InputProcessor::Bindable_Override);

	DeclareBindable(context, "selection-begin-line",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionMoveCursor), MOVE_DISPLAY_LINE_ENDS, -1, true),
		_("Extend the selection to the beginning of the line."), InputProcessor::Bindable_Override);

	/* page */
	DeclareBindable(context, "selection-down-page",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionMoveCursor), MOVE_HORIZONTAL_PAGES, 1, true),
		_("Extend the selection by one page downwards."), InputProcessor::Bindable_Override);

	DeclareBindable(context, "selection-up-page",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionMoveCursor), MOVE_HORIZONTAL_PAGES, -1, true),
		_("Extend the selection one page upwards."), InputProcessor::Bindable_Override);

	/* buffer */
	DeclareBindable(context, "selection-end",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionMoveCursor), MOVE_BUFFER_ENDS, 1, true),
		_("Extend the selection to the end of the text."), InputProcessor::Bindable_Override);

	DeclareBindable(context, "selection-begin",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionMoveCursor), MOVE_BUFFER_ENDS, -1, true),
		_("Extend the selection to the beginning of the text."), InputProcessor::Bindable_Override);

	/* select all */
	DeclareBindable(context, "select-all",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionSelectAll), true),
		_("Select all text."), InputProcessor::Bindable_Override);

	/* unselect all */
	DeclareBindable(context, "unselect-all",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionSelectAll), false),
		_("Unselect all text."), InputProcessor::Bindable_Override);

	/* deleting text */

	/* character */
	DeclareBindable(context, "delete-char",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionDelete), DELETE_CHARS, 1),
		_("Delete character under cursor."), InputProcessor::Bindable_Override);

	/* selection */
	DeclareBindable(context, "backspace",
		sigc::mem_fun(this, &TextView::ActionBackspace),
		_("Delete character before cursor."), InputProcessor::Bindable_Override);

	/* word */
	DeclareBindable(context, "delete-word-end",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionDelete), DELETE_WORD_ENDS, 1),
		_("Delete text until the end of the word at the cursor."), InputProcessor::Bindable_Override);

	DeclareBindable(context, "delete-word-begin",
		sigc::bind(sigc::mem_fun(this, &TextView::ActionDelete), DELETE_WORD_ENDS, -1),
		_("Delete text until the beginning of the word at the cursor."), InputProcessor::Bindable_Override);

	/* line */
	//TODO implement (also paragraph, page, buffer?)
	
	/* misc */

	/* overwrite */
	DeclareBindable(context, "toggle-overwrite",
		sigc::mem_fun(this, &TextView::ActionToggleOverwrite),
		_("Enable/Disable overwrite mode."), InputProcessor::Bindable_Override);

	/* non text editing bindables */
	DeclareBindable(context, "activate", sigc::mem_fun(this, &TextView::OnActivate),
		_("Accept input and move focus."), InputProcessor::Bindable_Override);
}

void TextView::BindActions(void)
{
	//TODO move all this to an editable abstract class?
	const gchar *context = "textview";

	//TODO implement more of these
	BindAction(context, "cursor-right", KEYS->Key_right(), false);
	BindAction(context, "cursor-left", KEYS->Key_left(), false);
	BindAction(context, "cursor-right-word", KEYS->Key_ctrl_right(), false);
	BindAction(context, "cursor-left-word", KEYS->Key_ctrl_left(), false);
	BindAction(context, "cursor-end-line", KEYS->Key_end(), false);
	BindAction(context, "cursor-begin-line", KEYS->Key_home(), false);
	/*BindAction(context, "cursor-down-page", KEYS->(), false);
	BindAction(context, "cursor-up-page", KEYS->(), false);
	BindAction(context, "cursor-end", KEYS->(), false);
	BindAction(context, "cursor-begin", KEYS->(), false);
	BindAction(context, "cursor-end", KEYS->Key_ctrl_end(), false);
	BindAction(context, "cursor-begin", KEYS->Key_ctrl_home(), false);
	BindAction(context, "selection-right", KEYS->Key_(), false);
	BindAction(context, "selection-left", KEYS->Key_(), false);
	BindAction(context, "selection-right-word", KEYS->Key_(), false);
	BindAction(context, "selection-left-word", KEYS->Key_(), false);
	BindAction(context, "selection-end-line", KEYS->Key_(), false);
	BindAction(context, "selection-begin-line", KEYS->Key_(), false);
	BindAction(context, "selection-down-page", KEYS->Key_(), false);
	BindAction(context, "selection-up-page", KEYS->Key_(), false);
	BindAction(context, "selection-end", KEYS->Key_(), false);
	BindAction(context, "selection-begin", KEYS->Key_(), false);*/
	BindAction(context, "select-all", KEYS->Key_ctrl_a(), false);
	/*BindAction(context, "unselect-all", KEYS->Key_ctrl_shift_a(), false);*/
	BindAction(context, "delete-char", KEYS->Key_del(), false);
	BindAction(context, "backspace", KEYS->Key_backspace(), false);
	/*BindAction(context, "delete-word-end", KEYS->Key_(), false);
	BindAction(context, "delete-word-begin", KEYS->Key_(), false);*/
	BindAction(context, "toggle-overwrite", KEYS->Key_ins(), false);

	//dont bind enter, since it should add a newline
	//BindAction(context, "activate", KEYS->Key_enter(), false);

  
  /* TODO these are not yet bindable
  add_move_binding (binding_set, GDK_Up, GDK_CONTROL_MASK,
                    MOVE_PARAGRAPHS, -1);

  add_move_binding (binding_set, GDK_Down, GDK_CONTROL_MASK,
                    MOVE_PARAGRAPHS, 1);
  
  gtk_binding_entry_add_signal (binding_set, GDK_Delete, GDK_SHIFT_MASK | GDK_CONTROL_MASK,
				"delete_from_cursor", 2,
				G_TYPE_ENUM, DELETE_PARAGRAPH_ENDS,
				G_TYPE_INT, 1);

  gtk_binding_entry_add_signal (binding_set, GDK_BackSpace, GDK_SHIFT_MASK | GDK_CONTROL_MASK,
				"delete_from_cursor", 2,
				G_TYPE_ENUM, DELETE_PARAGRAPH_ENDS,
				G_TYPE_INT, -1);
  */

  /* Cut/copy/paste */
  /*
  gtk_binding_entry_add_signal (binding_set, GDK_x, GDK_CONTROL_MASK,
				"cut-clipboard", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_c, GDK_CONTROL_MASK,
				"copy-clipboard", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_v, GDK_CONTROL_MASK,
				"paste-clipboard", 0);

  gtk_binding_entry_add_signal (binding_set, GDK_Delete, GDK_SHIFT_MASK,
				"cut-clipboard", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_Insert, GDK_CONTROL_MASK,
				"copy-clipboard", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_Insert, GDK_SHIFT_MASK,
				"paste-clipboard", 0);

  * Caret mode *
  gtk_binding_entry_add_signal (binding_set, GDK_F7, 0,
				"toggle-cursor-visible", 0);
  */

  /* Control-tab focus motion */
	/*TODO we might need these?
  gtk_binding_entry_add_signal (binding_set, GDK_Tab, GDK_CONTROL_MASK,
				"move-focus", 1,
				GTK_TYPE_DIRECTION_TYPE, GTK_DIR_TAB_FORWARD);
  gtk_binding_entry_add_signal (binding_set, GDK_KP_Tab, GDK_CONTROL_MASK,
				"move-focus", 1,
				GTK_TYPE_DIRECTION_TYPE, GTK_DIR_TAB_FORWARD);
  gtk_binding_entry_add_signal (binding_set, GDK_Tab, GDK_SHIFT_MASK | GDK_CONTROL_MASK,
				"move-focus", 1,
				GTK_TYPE_DIRECTION_TYPE, GTK_DIR_TAB_BACKWARD);
  gtk_binding_entry_add_signal (binding_set, GDK_KP_Tab, GDK_SHIFT_MASK | GDK_CONTROL_MASK,
				"move-focus", 1,
				GTK_TYPE_DIRECTION_TYPE, GTK_DIR_TAB_BACKWARD);
		*/

}

//TODO custom handlers?
void TextView::OnActivate(void)
{
	if (parent) {
		parent->MoveFocus(FocusNext);
	}
}

//#define __GTK_TEXT_VIEW_C__
//#include "gtkaliasdef.c"
