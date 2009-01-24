/* GTK - The GIMP Toolkit
 * gtktextlayout.h
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

#ifndef __GTK_TEXT_LAYOUT_H__
#define __GTK_TEXT_LAYOUT_H__

//#include <gtk/gtk.h>

#include "CppConsUI.h"
#include "TextBTree.h"
#include "TextBuffer.h"
#include "TextIter.h"
#include "TextChild.h"

//G_BEGIN_DECLS

/* forward declarations that have to be here to avoid including
 * gtktextbtree.h
 */
//typedef _TextLine     TextLine;
//typedef _TextLineData TextLineData;

//#define PIXEL_BOUND(d) (((d) + PANGO_SCALE - 1) / PANGO_SCALE)
#define PIXEL_BOUND(d) (((d) + 1024 - 1) / 1024 )
//TODO get rid of this
#define PANGO_PIXELS(d) (((int)(d) + 512) >> 10)


//#define GTK_TYPE_TEXT_LAYOUT             (gtk_text_layout_get_type ())
//#define GTK_TEXT_LAYOUT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_TEXT_LAYOUT, TextLayout))
//#define GTK_TEXT_LAYOUT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_TEXT_LAYOUT, TextLayoutClass))
//#define GTK_IS_TEXT_LAYOUT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_TEXT_LAYOUT))
//#define GTK_IS_TEXT_LAYOUT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_TEXT_LAYOUT))
//#define GTK_TEXT_LAYOUT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_TEXT_LAYOUT, TextLayoutClass))

//typedef struct _TextLayout         TextLayout;
//typedef struct _TextLayoutClass    TextLayoutClass;
//typedef struct _TextLineDisplay    TextLineDisplay;
//typedef struct _TextCursorDisplay  TextCursorDisplay;
//typedef struct _TextAttrAppearance TextAttrAppearance;

typedef enum //TODO move somewhere else?
{
  GTK_TEXT_DIR_NONE,
  GTK_TEXT_DIR_LTR,
  GTK_TEXT_DIR_RTL
} TextDirection;

typedef struct
{
  //PangoAttribute attr;
  TextAppearance appearance;
} TextAttrAppearance;

typedef struct
{
  gint x;
  gint y;
  gint height;
  guint is_strong : 1;
  guint is_weak : 1;
} TextCursorDisplay;

/*enum {
  INVALIDATED,
  CHANGED,
  ALLOCATE_CHILD,
  LAST_SIGNAL
};*/

typedef struct
{
  //PangoLayout *layout;
  GSList *cursors;
  GSList *shaped_objects;	/* Only for backwards compatibility */
  
  TextDirection direction;

  gint width;                   /* Width of layout */
  gint total_width;             /* width - margins, if no width set on layout, if width set on layout, -1 */
  gint height;
  /* Amount layout is shifted from left edge - this is the left margin
   * plus any other factors, such as alignment or indentation.
   */
  gint x_offset;
  gint left_margin;
  gint right_margin;
  gint top_margin;
  gint bottom_margin;
  gint insert_index;		/* Byte index of insert cursor within para or -1 */

  bool size_only;
  TextLine *line;
  
  int *pg_bg_color; //TODO figure out a datatype for colors

  Rect block_cursor;
  guint cursors_invalid : 1;
  guint has_block_cursor : 1;
  guint cursor_at_line_end : 1;
} TextLineDisplay;



class TextLineData;
class TextBuffer;
class TextLine;
class TextIter;

class TextLayout
{
	public:
		TextLayout ();
		~TextLayout();

		  TextBuffer *buffer;

		void               set_buffer            ( TextBuffer     *buffer);
		TextBuffer     *get_buffer            (void);

		void invalidate        ( const TextIter *start, const TextIter *end);
		void invalidate_cursors( const TextIter *start, const TextIter *end);

		void     changed              (
                                               gint               y,
                                               gint               old_height,
                                               gint               new_height);
		void     cursors_changed      (
                                               gint               y,
                                               gint               old_height,
                                               gint               new_height);

		void free_line_data    (
                                        TextLine       *line,
                                        TextLineData   *line_data);

		/* This function should return the passed-in line data,
		 * OR remove the existing line data from the line, and
		 * return a NEW line data after adding it to the line.
		 * That is, invariant after calling the callback is that
		 * there should be exactly one line data for this view
		 * stored on the btree line.
		 */
		TextLineData* wrap  (
							TextLine     *line,
							TextLineData *line_data); /* may be NULL */

void get_iter_at_pixel (
                                        TextIter       *iter,
                                        gint               x,
                                        gint               y);

void get_iter_at_position (
					   TextIter       *iter,
					   gint              *trailing,
					   gint               x,
					   gint               y);

	void get_iter_location ( TextIter *iter, Rect      *rect);

void     get_line_yrange      (
                                               TextIter *iter,
                                               gint              *y,
                                               gint              *height);
void     get_line_xrange     (
                                               TextIter *iter,
                                               gint              *x,
                                               gint              *width);

	void get_line_at_y     (
                                        TextIter       *target_iter,
                                        gint               y,
                                        gint              *line_top);

bool is_valid        (void);
void validate_yrange (
                                          TextIter   *anchor_line,
                                          gint           y0_,
                                          gint           y1_);

/* Getting the size or the lines potentially results in a call to
 * recompute, which is pretty massively expensive. Thus it should
 * basically only be done in an idle handler.
 *
 * Long-term, we would really like to be able to do these without
 * a full recompute so they may get cheaper over time.
 */
void    get_size  (
                                   gint           *width,
                                   gint           *height);
GSList* get_lines (
                                   /* [top_y, bottom_y) */
                                   gint            top_y,
                                   gint            bottom_y,
                                   gint           *first_line_y);

void set_screen_width       ( gint               width);

void               default_style_changed (void);
//TODO move to private
  /* Default style used if no tags override it */

  TextAttributes *default_style;

void     validate        ( gint           max_pixels);
  //
bool clamp_iter_to_vrange (
                                               TextIter       *iter,
                                               gint               top,
                                               gint               bottom);
void		   set_overwrite_mode	 ( bool           overwrite);
void     set_cursor_visible ( bool           cursor_visible);


bool move_iter_to_previous_line ( TextIter   *iter);
bool move_iter_to_next_line     ( TextIter   *iter);
void     move_iter_to_x             ( TextIter   *iter, gint           x);
bool move_iter_visually         ( TextIter   *iter, gint           count);

bool iter_starts_line           ( TextIter   *iter);

void     get_iter_at_line (
			     TextIter    *iter,
			     TextLine    *line,
			     gint            byte_offset);

	bool move_iter_to_line_end ( TextIter   *iter, gint           direction);
void               set_default_style     ( TextAttributes *values);

	void get_cursor_locations ( TextIter    *iter, Rect   *strong_pos, Rect   *weak_pos);;

	protected:
	private:
  /* width of the display area on-screen,
   * i.e. pixels we should wrap to fit inside. */
  gint screen_width;

  /* width/height of the total logical area being layed out */
  gint width;
  gint height;

  /* Pixel offsets from the left and from the top to be used when we
   * draw; these allow us to create left/top margins. We don't need
   * anything special for bottom/right margins, because those don't
   * affect drawing.
   */
  /* gint left_edge; */
  /* gint top_edge; */


  /* Pango contexts used for creating layouts */
  //PangoContext *ltr_context;
  //PangoContext *rtl_context;

  /* A cache of one style; this is used to ensure
   * we don't constantly regenerate the style
   * over long runs with the same style. */
  TextAttributes *one_style_cache;

  /* A cache of one line display. Getting the same line
   * many times in a row is the most common case.
   */
  TextLineDisplay *one_display_cache;

  /* Whether we are allowed to wrap right now */
  gint wrap_loop_count;
  
  /* Whether to show the insertion cursor */
  guint cursor_visible : 1;

  /* For what TextDirection to draw cursor GTK_TEXT_DIR_NONE -
   * means draw both cursors.
   */
  guint cursor_direction : 2;

  /* The keyboard direction is used to default the alignment when
     there are no strong characters.
  */
  guint keyboard_direction : 2;

  /* The preedit string and attributes, if any */

  gchar *preedit_string;
  //PangoAttrList *preedit_attrs;
  gint preedit_len;
  gint preedit_cursor;

  guint overwrite_mode : 1;


//struct _TextLayoutClass
//{
  //GObjectClass parent_class;

  /* Some portion of the layout was invalidated
   *
  void  (*invalidated)  (TextLayout *layout);

  * A range of the layout changed appearance and possibly height
   *
  void  (*changed)              (TextLayout     *layout,
                                 gint               y,
                                 gint               old_height,
                                 gint               new_height);
  TextLineData* (*wrap)      (TextLayout     *layout,
                                 TextLine       *line,
                                 TextLineData   *line_data); * may be NULL *
*  void  (*get_log_attrs)        (TextLayout     *layout,
                                 TextLine       *line,
                                 PangoLogAttr     **attrs,
                                 gint              *n_attrs);*
  void  (*invalidate)           (TextLayout     *layout,
                                 const TextIter *start,
                                 const TextIter *end);
  void  (*free_line_data)       (TextLayout     *layout,
                                 TextLine       *line,
                                 TextLineData   *line_data);

*  void (*allocate_child)        (TextLayout     *layout,
                                 GtkWidget         *child,
                                 gint               x,
                                 gint               y);*

  void (*invalidate_cursors)    (TextLayout     *layout,
                                 const TextIter *start,
                                 const TextIter *end);

  * Padding for future expansion *
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  */
//};
//
//
//extern PangoAttrType gtk_text_attr_appearance_type;

//GType         gtk_text_layout_get_type    (void) G_GNUC_CONST;

/*void               gtk_text_layout_set_contexts          (TextLayout     *layout,
							  PangoContext      *ltr_context,
							  PangoContext      *rtl_context);*/
void               set_cursor_direction  (
                                                          TextDirection   direction);
void               set_keyboard_direction (
							   TextDirection keyboard_dir);

/*void gtk_text_layout_set_preedit_string     (TextLayout     *layout,
 					     const gchar       *preedit_string,
 					     PangoAttrList     *preedit_attrs,
 					     gint               cursor_pos);*/

bool get_cursor_visible (void);

void wrap_loop_start (void);
void wrap_loop_end   (void);

TextLineDisplay* get_line_display  (
                                                       TextLine        *line,
                                                       bool            size_only);
void                free_line_display (
                                                       TextLineDisplay *display);


/*void     gtk_text_layout_get_iter_location    (TextLayout     *layout,
                                               const TextIter *iter,
                                               Rect      *rect);*/
/*void     gtk_text_layout_get_cursor_locations (TextLayout     *layout,
                                               TextIter       *iter,
                                               Rect      *strong_pos,
                                               Rect      *weak_pos);*/
/*bool _gtk_text_layout_get_block_cursor    (TextLayout     *layout,
					       Rect      *pos);*/

bool iter_to_line_end      (
                                                     TextIter   *iter,
                                                     gint           direction);
/* Don't use these. Use gtk_text_view_add_child_at_anchor().
 * These functions are defined in gtktextchild.c, but here
 * since they are semi-public and require TextLayout to
 * be declared.
 */
/*void gtk_text_child_anchor_register_child   (TextChildAnchor *anchor,
                                             GtkWidget          *child,
                                             TextLayout      *layout);
void gtk_text_child_anchor_unregister_child (TextChildAnchor *anchor,
                                             GtkWidget          *child);

void gtk_text_child_anchor_queue_resize     (TextChildAnchor *anchor,
                                             TextLayout      *layout);*/

/*void gtk_text_anchored_child_set_layout     (GtkWidget          *child,
                                             TextLayout      *layout);*/

void spew (void);

TextLineData *real_wrap (
                                                   TextLine *line,
                                                   /* may be NULL */
                                                   TextLineData *line_data);

void invalidated     (void);

void real_invalidate        ( TextIter *start, TextIter *end);
void real_invalidate_cursors( TextIter *start, TextIter *end);
void invalidate_cache       (
						    TextLine       *line,
						    bool           cursors_only);
void invalidate_cursor_line (
						    bool           cursors_only);
void real_free_line_data    (
						    TextLine       *line,
						    TextLineData   *line_data);
void emit_changed           (
						    gint               y,
						    gint               old_height,
						    gint               new_height);

void invalidate_all (void);

//PangoAttribute *gtk_text_attr_appearance_new (const TextAppearance *appearance);

//TODO move to textbuffer class?
static void mark_set_handler    (TextBuffer     *buffer,
						 const TextIter *location,
						 TextMark       *mark,
						 gpointer           data);
static void buffer_insert_text  (TextBuffer     *textbuffer,
						 TextIter       *iter,
						 gchar             *str,
						 gint               len,
						 gpointer           data);
static void buffer_delete_range (TextBuffer     *textbuffer,
						 TextIter       *start,
						 TextIter       *end,
						 gpointer           data);

void update_cursor_line (void);


//void gtk_text_layout_finalize (GObject *object);

//guint signals[LAST_SIGNAL] = { 0 };

//PangoAttrType gtk_text_attr_appearance_type = 0;

//G_DEFINE_TYPE (TextLayout, gtk_text_layout, G_TYPE_OBJECT)


void free_style_cache (void);

	void __changed (
                     gint           y,
                     gint           old_height,
                     gint           new_height,
                     bool       cursors_only);

	void invalidate_cached_style (void);

	TextLine *cursor_line;
	void update_layout_size (void);

	TextAttributes* get_style ( GPtrArray     *tags);
	bool totally_invisible_line ( TextLine   *line, TextIter   *iter);


bool __get_block_cursor (
		  TextLineDisplay *display,
		  TextIter  *insert_iter,
		  gint                insert_index,
		  Rect       *pos,
		  bool           *cursor_at_line_end);

void update_text_display_cursors (
			     TextLine        *line,
			     TextLineDisplay *display);



	GPtrArray * get_tags_array_at_iter (TextIter *iter);
	void release_style ( TextAttributes *style);

	gint line_display_iter_to_index (
			    TextLineDisplay *display,
			    TextIter  *iter);

	void line_display_index_to_iter (
			    TextLineDisplay *display,
			    TextIter        *iter,
			    gint                index,
			    gint                trailing);

	void __get_line_at_y (
               gint           y,
               TextLine  **line,
               gint          *line_top);


	bool get_block_cursor ( Rect  *pos);

	void add_cursor (
		    TextLineDisplay *display,
		    TextLineSegment *seg,
		    gint                start);

	void find_display_line_below ( TextIter   *iter, gint           y);
	void find_display_line_above ( TextIter   *iter, gint           y);

};


#endif  /* __GTK_TEXT_LAYOUT_H__ */
