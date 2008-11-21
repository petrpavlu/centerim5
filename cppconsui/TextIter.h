/* GTK - The GIMP Toolkit
 * gtktextiter.h Copyright (C) 2000 Red Hat, Inc.
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

#if defined(GTK_DISABLE_SINGLE_INCLUDES) && !defined (__GTK_H_INSIDE__) && !defined (GTK_COMPILATION)
#error "Only <gtk/gtk.h> can be included directly."
#endif

#ifndef __GTK_TEXT_ITER_H__
#define __GTK_TEXT_ITER_H__

#include <glib.h>
#include "TextTag.h"
//#include "TextBuffer.h"
//#include "TextBTree.h"
#include "TextChild.h"
//#include "TextSegment.h"

class TextLineSegment;
class TextLine;
class TextBTree;
class TextChildAnchor;

typedef enum {
  GTK_TEXT_SEARCH_VISIBLE_ONLY = 1 << 0,
  GTK_TEXT_SEARCH_TEXT_ONLY    = 1 << 1
  /* Possible future plans: SEARCH_CASE_INSENSITIVE, SEARCH_REGEXP */
} TextSearchFlags;

/*
 * Iter: represents a location in the text. Becomes invalid if the
 * characters/pixmaps/widgets (indexable objects) in the text buffer
 * are changed.
 */

class TextBuffer;

//#define GTK_TYPE_TEXT_ITER     (get_type ())


class TextIter
{
	public:
		TextIter();

		TextIter(const TextIter&);
		TextIter& operator=(const TextIter&);

		/* This is primarily intended for language bindings that want to avoid
		 * a "buffer" argument to text insertions, deletions, etc. */
		TextBuffer* get_buffer(void) ;

/*
 * Life cycle
 */

		//TODO make this a copy operator
const TextIter *copy (void);
void free(TextIter *iter);

//GType        get_type (void) G_GNUC_CONST;

/*
 * Convert to different kinds of index
 */

gint get_offset      (void);
gint get_line        (void);
gint get_line_offset (void);
gint get_line_index  (void);

gint get_visible_line_offset (void);
gint get_visible_line_index (void);


/*
 * "Dereference" operators
 */
gunichar get_char          (void);

/* includes the 0xFFFC char for pixmaps/widgets, so char offsets
 * into the returned string map properly into buffer char offsets
 */
static gchar   *get_slice         (TextIter  *start,
                                          TextIter  *end);

/* includes only text, no 0xFFFC */
static gchar   *get_text          (TextIter  *start,
                                          TextIter  *end);
/* exclude invisible chars */
static gchar   *get_visible_slice (TextIter  *start,
                                          TextIter  *end);
static gchar   *get_visible_text  (TextIter  *start,
                                          TextIter  *end);

//GdkPixbuf* get_pixbuf (const (void));
GSList  *  get_marks  (void);

//TODO not supported by cim5 right now TextChildAnchor* get_child_anchor (void);

/* Return list of tags toggled at this point (toggled_on determines
 * whether the list is of on-toggles or off-toggles)
 */
GSList  *get_toggled_tags  ( bool            toggled_on);

bool begins_tag        (TextTag *tag);

bool ends_tag          (TextTag *tag);

bool toggles_tag (TextTag *tag);

bool has_tag           (TextTag *tag);
GSList  *get_tags          (void);

bool editable          ( bool default_setting);
bool can_insert        ( bool default_editability);

bool starts_word        (void); //TODO are these all const functions?
bool ends_word          (void);
bool inside_word        (void);
bool starts_sentence    (void);
bool ends_sentence      (void);
bool inside_sentence    (void);
bool starts_line        (void);
bool ends_line          (void);
bool is_cursor_position (void);

gint     get_chars_in_line (void);
gint     get_bytes_in_line (void);

bool       get_attributes ( TextAttributes *values);
//PangoLanguage* get_language   (const (void));
bool       is_end         (void);
bool       is_start       (void);

/*
 * Moving around the buffer
 */

bool forward_char         (void);
bool backward_char        (void);
bool forward_chars        (
                                             gint         count);
bool backward_chars       (
                                             gint         count);
bool forward_line         (void);
bool backward_line        (void);
bool forward_lines        (
                                             gint         count);
bool backward_lines       (
                                             gint         count);
bool forward_word_end     (void);
bool backward_word_start  (void);
bool forward_word_ends    (
                                             gint         count);
bool backward_word_starts (
                                             gint         count);
                                             
bool forward_visible_line   (void);
bool backward_visible_line  (void);
bool forward_visible_lines  (
                                               gint         count);
bool backward_visible_lines (
                                               gint         count);

bool forward_visible_word_end     (void);
bool backward_visible_word_start  (void);
bool forward_visible_word_ends    (
                                             gint         count);
bool backward_visible_word_starts (
                                             gint         count);

bool forward_sentence_end     (void);
bool backward_sentence_start  (void);
bool forward_sentence_ends    (
                                                 gint         count);
bool backward_sentence_starts (
                                                 gint         count);
/* cursor positions are almost equivalent to chars, but not quite;
 * in some languages, you can't put the cursor between certain
 * chars. Also, you can't put the cursor between \r\n at the end
 * of a line.
 */
bool forward_cursor_position   (void);
bool backward_cursor_position  (void);
bool forward_cursor_positions  (
                                                  gint         count);
bool backward_cursor_positions (
                                                  gint         count);

bool forward_visible_cursor_position   (void);
bool backward_visible_cursor_position  (void);
bool forward_visible_cursor_positions  (
                                                          gint         count);
bool backward_visible_cursor_positions (
                                                          gint         count);


void     set_offset         ( gint         char_offset);
void     set_line           ( gint         line_number);
void     set_line_offset    ( gint         char_on_line);
void     set_line_index     ( gint         byte_on_line);
void     forward_to_end     (void);
bool forward_to_line_end (void);

void     set_visible_line_offset ( gint         char_on_line);
void     set_visible_line_index  ( gint         byte_on_line);

/* returns TRUE if a toggle was found; NULL for the tag pointer
 * means "any tag toggle", otherwise the next toggle of the
 * specified tag is located.
 */
bool forward_to_tag_toggle ( TextTag  *tag);

bool backward_to_tag_toggle ( TextTag  *tag);

typedef bool (* TextCharPredicate) (gunichar ch, gpointer user_data);

bool forward_find_char  (
                                           TextCharPredicate  pred,
                                           gpointer              user_data,
                                           TextIter    *limit);
bool backward_find_char (
                                           TextCharPredicate  pred,
                                           gpointer              user_data,
                                           TextIter    *limit);

bool forward_search  (
                                        const gchar       *str,
                                        TextSearchFlags flags,
                                        TextIter       *match_start,
                                        TextIter       *match_end,
                                        TextIter *limit);

bool backward_search (
                                        const gchar       *str,
                                        TextSearchFlags flags,
                                        TextIter       *match_start,
                                        TextIter       *match_end,
                                        TextIter *limit);


/*
 * Comparisons
 */
//TODO change into operators?

static bool equal           (TextIter *lhs, TextIter *rhs);
static gint     compare         (TextIter *lhs, TextIter *rhs);

bool in_range        ( TextIter *start, TextIter *end);

/* Put these two in ascending order */
static void     order           (TextIter *first,
                                        TextIter *second);
	
//TODO from textiterprivate.h
TextLineSegment *get_indexable_segment      (void) ;
TextLineSegment *get_any_segment            (void) ;
TextLine*       get_text_line              (void) ;
TextBTree *      get_btree                  (void) ;
bool            forward_indexable_segment  (void);
bool            backward_indexable_segment (void);
gint                get_segment_byte           (void);
gint                get_segment_char           (void);


/* debug */
void check (void);

		void init_common ( TextBTree *tree);
		void init_from_char_offset (
                            TextBTree *tree,
                            TextLine *line,
                            gint line_char_offset);
		void init_from_byte_offset (
                            TextBTree *tree,
                            TextLine *line,
                            gint line_byte_offset);
		void init_from_segment (
                        TextBTree *tree,
                        TextLine *line,
                        TextLineSegment *segment);
		void check_invariants (void);

  /* TextIter is an opaque datatype; ignore all these fields.
   * Initialize the iter with gtk_text_buffer_get_iter_*
   * functions
   */
  /* Always-valid information */

  TextBTree *tree;
  TextLine *line;
  /* At least one of these is always valid;
     if invalid, they are -1.

     If the line byte offset is valid, so is the segment byte offset;
     and ditto for char offsets. */
  gint line_byte_offset;
  gint line_char_offset;
  /* These two are valid if >= 0 */
  gint cached_char_index;
  gint cached_line_number;
  /* Stamps to detect the buffer changing under us */
  gint chars_changed_stamp;
  gint segments_changed_stamp;
  /* Valid if the segments_changed_stamp is up-to-date */
  TextLineSegment *segment;     /* indexable segment we index */
  TextLineSegment *any_segment; /* first segment in our location,
                                      maybe same as "segment" */
  /* One of these will always be valid if segments_changed_stamp is
     up-to-date. If invalid, they are -1.

     If the line byte offset is valid, so is the segment byte offset;
     and ditto for char offsets. */
  gint segment_byte_offset;
  gint segment_char_offset;

		void invalidate_char_index (void);
		void adjust_char_index (gint count);
		void adjust_line_number (gint count);
		void ensure_char_offsets (void);
		void ensure_byte_offsets (void);
		bool is_segment_start (void);

		bool lines_match (
		     const gchar **lines,
		     bool visible_only,
		     bool slice,
		     TextIter *match_start,
		     TextIter *match_end);

	protected:
	private:


	void set_common (TextLine *line);
	void set_from_byte_offset ( TextLine *line, gint byte_offset);
	void set_from_char_offset ( TextLine *line, gint char_offset);
	void set_from_segment ( TextLine *line, TextLineSegment *segment);
	bool make_surreal (void);
	bool make_real (void);

	bool forward_line_leaving_caches_unmodified (void);
	bool at_last_indexable_segment (void);
	bool __forward_char (void);
	int find_paragraph_delimiter_for_line (void);

	bool matches_pred (
              TextCharPredicate pred,
              gpointer user_data);

	void forward_chars_with_skipping (
                             gint         count,
                             bool     skip_invisible,
                             bool     skip_nontext);

	bool vectors_equal_ignoring_trailing (gchar **vec1, gchar **vec2);
};

#endif
