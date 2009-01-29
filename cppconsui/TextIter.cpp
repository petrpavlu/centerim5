/* GTK - The GIMP Toolkit
 * gtktextiter.c Copyright (C) 2000 Red Hat, Inc.
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

//#define GTK_TEXT_USE_INTERNAL_UNSUPPORTED_API
//#include "config.h"
#include "TextIter.h"
#include "TextBTree.h"
#include "TextSegment.h"
#include "TextTypes.h"
//#include "gtktextiterprivate.h"
//#include "gtkintl.h"
//#include "gtkdebug.h"
//#include "gtkalias.h"
#include <string.h>

#define FIX_OVERFLOWS(varname) if ((varname) == G_MININT) (varname) = G_MININT + 1

//typedef struct _TextRealIter TextRealIter;

//TODO figure out what this should really do
TextIter::TextIter()
: tree(NULL)
, line(NULL)
, line_byte_offset(0)
, line_char_offset(0)
, cached_char_index(0)
, cached_line_number(0)
, chars_changed_stamp(0)
, segments_changed_stamp(0)
, segment(NULL)
, any_segment(NULL)
, segment_byte_offset(0)
, segment_char_offset(0)
{
}

TextIter::TextIter(const TextIter& iter)
{
	*this = iter;
}

TextIter& TextIter::operator=(const TextIter& iter)
{
	if (this != &iter) {
  		tree = iter.tree;
		line = iter.line;
		line_byte_offset = iter.line_byte_offset;
		line_char_offset = iter.line_char_offset;
		cached_char_index = iter.cached_char_index;
		cached_line_number = iter.cached_line_number;
		chars_changed_stamp = iter.chars_changed_stamp;
		segments_changed_stamp = iter.segments_changed_stamp;
		segment = iter.segment;
		any_segment = iter.any_segment;
		segment_byte_offset = iter.segment_byte_offset;
		segment_char_offset = iter.segment_char_offset;
	}

	return *this;
}

/* These "set" functions should not assume any fields
   other than the char stamp and the tree are valid.
*/
void TextIter::set_common (TextLine *line)
{
  /* Update segments stamp */
  segments_changed_stamp =
    tree->get_segments_changed_stamp ();

  this->line = line;

  line_byte_offset = -1;
  line_char_offset = -1;
  segment_byte_offset = -1;
  segment_char_offset = -1;
  cached_char_index = -1;
  cached_line_number = -1;
}

void TextIter::set_from_byte_offset (
                           TextLine *line,
                           gint byte_offset)
{
  set_common (line);

  if (!line->byte_locate (
                                   byte_offset,
                                   &segment,
                                   &any_segment,
                                   &segment_byte_offset,
                                   &line_byte_offset))
    g_error ("Byte index %d is off the end of the line",
             byte_offset);
}

void TextIter::set_from_char_offset (
                           TextLine *line,
                           gint char_offset)
{
  set_common (line);

  if (!line->char_locate (
                                   char_offset,
                                   &segment,
                                   &any_segment,
                                   &segment_char_offset,
                                   &line_char_offset))
    g_error ("Char offset %d is off the end of the line",
             char_offset);
}

void TextIter::set_from_segment (
                       TextLine *line,
                       TextLineSegment *segment)
{
  TextLineSegment *seg;
  gint byte_offset;

  /* This could theoretically be optimized by computing all the iter
     fields in this same loop, but I'm skipping it for now. */
  byte_offset = 0;
  seg = line->segments;
  while (seg != segment)
    {
      byte_offset += seg->byte_count;
      seg = seg->next;
    }

  set_from_byte_offset (line, byte_offset);
}

/* This function ensures that the segment-dependent information is
   truly computed lazily; often we don't need to do the full make_real
   work. This ensures the btree and line are valid, but doesn't
   update the segments. */
bool TextIter::make_surreal (void)
{
 // TextRealIter *iter = (TextRealIter*)_iter;

  if (chars_changed_stamp !=
      tree->get_chars_changed_stamp ())
    {
      g_warning ("Invalid text buffer iterator: either the iterator "
                 "is uninitialized, or the characters/pixbufs/widgets "
                 "in the buffer have been modified since the iterator "
                 "was created.\nYou must use marks, character numbers, "
                 "or line numbers to preserve a position across buffer "
                 "modifications.\nYou can apply tags and insert marks "
                 "without invalidating your iterators,\n"
                 "but any mutation that affects 'indexable' buffer contents "
                 "(contents that can be referred to by character offset)\n"
                 "will invalidate all outstanding iterators");
      //return NULL;
      return false;
    }

  /* We don't update the segments information since we are becoming
     only surreal. However we do invalidate the segments information
     if appropriate, to be sure we segfault if we try to use it and we
     should have used make_real. */

  if (segments_changed_stamp !=
      tree->get_segments_changed_stamp ())
    {
      segment = NULL;
      any_segment = NULL;
      /* set to segfault-causing values. */
      segment_byte_offset = -10000;
      segment_char_offset = -10000;
    }

  return true;
}

bool TextIter::make_real (void)
{
//  TextRealIter *iter;

  //iter = text_iter_make_surreal (_iter);
  make_surreal ();

  if (segments_changed_stamp !=
      tree->get_segments_changed_stamp ())
    {
      if (line_byte_offset >= 0)
        {
          set_from_byte_offset (
                                     line,
                                     line_byte_offset);
        }
      else
        {
          g_assert (line_char_offset >= 0);

          set_from_char_offset (
                                     line,
                                     line_char_offset);
        }
    }

  g_assert (segment != NULL);
  g_assert (any_segment != NULL);
  g_assert (segment->char_count > 0);

//  return iter;
  return true;
}

void TextIter::init_common ( TextBTree *tree)
{
//  TextRealIter *iter = (TextRealIter*)_iter;

//  g_return_val_if_fail (iter != NULL, NULL);
  //g_return_val_if_fail (tree != NULL, NULL);
  g_return_if_fail (tree != NULL);

  this->tree = tree;

  chars_changed_stamp =
    tree->get_chars_changed_stamp ();

//  return iter;
}

void TextIter::init_from_segment (
                        TextBTree *tree,
                        TextLine *line,
                        TextLineSegment *segment)
{
//  TextRealIter *real;

  g_return_if_fail (line != NULL);

  //real = init_common (tree);
  init_common (tree);

  set_from_segment (line, segment);

//  return real;
}

void TextIter::init_from_byte_offset (
                            TextBTree *tree,
                            TextLine *line,
                            gint line_byte_offset)
{
//  TextRealIter *real;

  g_return_if_fail (line != NULL);

  //real = init_common (tree);
  init_common (tree);

  set_from_byte_offset (line, line_byte_offset);

  if (segment->type == text_segment_char &&
      (segment->body.chars[segment_byte_offset] & 0xc0) == 0x80)
    g_warning ("Incorrect line byte index %d falls in the middle of a UTF-8 "
               "character; this will crash the text buffer. "
               "Byte indexes must refer to the start of a character.",
               line_byte_offset);
  
//  return real;
}

void TextIter::init_from_char_offset (
                            TextBTree *tree,
                            TextLine *line,
                            gint line_char_offset)
{
//  TextRealIter *real;

  g_return_if_fail (line != NULL);

  //real = init_common (tree);
  init_common (tree);

  set_from_char_offset (line, line_char_offset);

 // return real;
}

/*inline*/ void TextIter::invalidate_char_index (void)
{
  cached_char_index = -1;
}

/*inline*/ void TextIter::adjust_char_index (gint count)
{
  if (cached_char_index >= 0)
    cached_char_index += count;
}

/*inline*/ void TextIter::adjust_line_number (gint count)
{
  if (cached_line_number >= 0)
    cached_line_number += count;
}

/*inline*/ void TextIter::ensure_char_offsets (void)
{
  if (line_char_offset < 0)
    {
      g_assert (line_byte_offset >= 0);

     line->byte_to_char_offsets (
                                          line_byte_offset,
                                          &line_char_offset,
                                          &segment_char_offset);
    }
}

/*inline*/ void TextIter::ensure_byte_offsets (void)
{
  if (line_byte_offset < 0)
    {
      g_assert (line_char_offset >= 0);

      line->char_to_byte_offsets (
                                          line_char_offset,
                                          &line_byte_offset,
                                          &segment_byte_offset);
    }
}

/*inline*/ bool TextIter::is_segment_start (void)
{
  return segment_byte_offset == 0 || segment_char_offset == 0;
}

void TextIter::check_invariants (void)
{
  if (1)//TODO(gtk_debug_flags & GTK_DEBUG_TEXT)
    check ();
}

/**
 * gtk_text_iter_get_buffer:
 * @iter: an iterator
 *
 * Returns the #TextBuffer this iterator is associated with.
 *
 * Return value: the buffer
 **/
TextBuffer* TextIter::get_buffer (void) //TODO function is const?
{
//  TextRealIter *real;

//  g_return_val_if_fail (iter != NULL, NULL);

  make_surreal ();

//  if (real == NULL)
//    return NULL;

  check_invariants ();

  return tree->get_buffer ();
}

/**
 * gtk_text_iter_copy:
 * @iter: an iterator
 *
 * Creates a dynamically-allocated copy of an iterator. This function
 * is not useful in applications, because iterators can be copied with a
 * simple assignment (<literal>TextIter i = j;</literal>). The
 * function is used by language bindings.
 *
 * Return value: a copy of the @iter, free with gtk_text_iter_free ()
 **/
//TODO copy constructor
TextIter*
gtk_text_iter_copy (const TextIter *iter)
{
  TextIter *new_iter;

  g_return_val_if_fail (iter != NULL, NULL);

  new_iter = g_slice_new (TextIter);

  *new_iter = *iter;

  return new_iter;
}

/**
 * gtk_text_iter_free:
 * @iter: a dynamically-allocated iterator
 *
 * Free an iterator allocated on the heap. This function
 * is intended for use in language bindings, and is not
 * especially useful for applications, because iterators can
 * simply be allocated on the stack.
 **/
void
gtk_text_iter_free (TextIter *iter)
{
  g_return_if_fail (iter != NULL);

  g_slice_free (TextIter, iter);
}

/*
GType
gtk_text_iter_get_type (void)
{
  static GType our_type = 0;
  
  if (our_type == 0)
    our_type = g_boxed_type_register_static (I_("TextIter"),
					     (GBoxedCopyFunc) gtk_text_iter_copy,
					     (GBoxedFreeFunc) gtk_text_iter_free);

  return our_type;
}*/

TextLineSegment* TextIter::get_indexable_segment (void)
{
//  TextRealIter *real;

//  g_return_val_if_fail (iter != NULL, NULL);

  if (!make_real ())
	  return NULL;

//  if (real == NULL)
//    return NULL;

  check_invariants ();

  g_assert (segment != NULL);

  return segment;
}

TextLineSegment* TextIter::get_any_segment (void)
{
//  TextRealIter *real;

//  g_return_val_if_fail (iter != NULL, NULL);

  if (!make_real ())
	  return NULL;

//  if (real == NULL)
//    return NULL;

  check_invariants ();

  g_assert (any_segment != NULL);

  return any_segment;
}

gint TextIter::get_segment_byte (void)
{
//  TextRealIter *real;

//  g_return_val_if_fail (iter != NULL, 0);

  if (!make_real ())
	  return 0;

//  if (real == NULL)
//    return 0;

  ensure_byte_offsets ();

  check_invariants ();

  return segment_byte_offset;
}

gint TextIter::get_segment_char (void)//TODO function const?
{
//  TextRealIter *real;

//  g_return_val_if_fail (iter != NULL, 0);

  if (!make_real ())
	  return 0;

//  if (real == NULL)
//    return 0;

  ensure_char_offsets ();

  check_invariants ();

  return segment_char_offset;
}

/* This function does not require a still-valid
   iterator */
TextLine* TextIter::get_text_line (void) //TODO function const?
{
//  const TextRealIter *real;

//  g_return_val_if_fail (iter != NULL, NULL);

  //real = (const TextRealIter*)iter;

  return line;
}

/* This function does not require a still-valid
   iterator */
TextBTree* TextIter::get_btree (void)//TODO function const?
{
//  const TextRealIter *real;

//  g_return_val_if_fail (iter != NULL, NULL);

//  real = (const TextRealIter*)iter;

  return tree;
}

/*
 * Conversions
 */

/**
 * gtk_text_iter_get_offset:
 * @iter: an iterator
 *
 * Returns the character offset of an iterator.
 * Each character in a #TextBuffer has an offset,
 * starting with 0 for the first character in the buffer.
 * Use gtk_text_buffer_get_iter_at_offset () to convert an
 * offset back into an iterator.
 *
 * Return value: a character offset
 **/
gint TextIter::get_offset (void)//TODO const func?
{
//  TextRealIter *real;

//  g_return_val_if_fail (iter != NULL, 0);

  if (! make_surreal ())
	  return 0;

//  if (real == NULL)
//    return 0;

  check_invariants ();
  
  if (cached_char_index < 0)
    {
      ensure_char_offsets ();
      
      cached_char_index =
        line->char_index ();
      cached_char_index += line_char_offset;
    }

  check_invariants ();

  return cached_char_index;
}

/**
 * gtk_text_iter_get_line:
 * @iter: an iterator
 *
 * Returns the line number containing the iterator. Lines in
 * a #TextBuffer are numbered beginning with 0 for the first
 * line in the buffer.
 *
 * Return value: a line number
 **/
gint TextIter::get_line (void)//TODO function const
{
//  TextRealIter *real;

//  g_return_val_if_fail (iter != NULL, 0);

  if (!make_surreal ())
	  return 0;

//  if (real == NULL)
//    return 0;

  if (cached_line_number < 0)
    cached_line_number =
      line->get_number ();

  check_invariants ();

  return cached_line_number;
}

/**
 * gtk_text_iter_get_line_offset:
 * @iter: an iterator
 *
 * Returns the character offset of the iterator,
 * counting from the start of a newline-terminated line.
 * The first character on the line has offset 0.
 *
 * Return value: offset from start of line
 **/
gint TextIter::get_line_offset (void)//TODO const func?
{
//  TextRealIter *real;

//  g_return_val_if_fail (iter != NULL, 0);

  if (!make_surreal())
	  return 0;

//  if (real == NULL)
//    return 0;

  ensure_char_offsets ();

  check_invariants ();

  return line_char_offset;
}

/**
 * gtk_text_iter_get_line_index:
 * @iter: an iterator
 *
 * Returns the byte index of the iterator, counting
 * from the start of a newline-terminated line.
 * Remember that #TextBuffer encodes text in
 * UTF-8, and that characters can require a variable
 * number of bytes to represent.
 *
 * Return value: distance from start of line, in bytes
 **/
gint TextIter::get_line_index (void)//TODO const func?
{
//  TextRealIter *real;
  
//  g_return_val_if_fail (iter != NULL, 0);
//
  if (!make_surreal())
	  return 0;

//  if (real == NULL)
//    return 0;

  ensure_byte_offsets ();

  check_invariants ();

  return line_byte_offset;
}

/**
 * gtk_text_iter_get_visible_line_offset:
 * @iter: a #TextIter
 * 
 * Returns the offset in characters from the start of the
 * line to the given @iter, not counting characters that
 * are invisible due to tags with the "invisible" flag
 * toggled on.
 * 
 * Return value: offset in visible characters from the start of the line 
 **/
gint TextIter::get_visible_line_offset (void)//TODO const func?
{
//  TextRealIter *real;
  gint vis_offset;
  TextLineSegment *seg;
  TextIter pos;
  
//  g_return_val_if_fail (iter != NULL, 0);

  if (!make_real ())
	  return 0;

//  if (real == NULL)
//    return 0;

  ensure_char_offsets ();

  check_invariants ();
  
  vis_offset = line_char_offset;

  g_assert (vis_offset >= 0);
  
  tree->get_iter_at_line ( &pos, line, 0);

  seg = (&pos)->get_indexable_segment ();

  while (seg != segment)
    {
      /* This is a pretty expensive call, making the
       * whole function pretty lame; we could keep track
       * of current invisibility state by looking at toggle
       * segments as we loop, and then call this function
       * only once per line, in order to speed up the loop
       * quite a lot.
       */
      if (TextBTree::char_is_invisible (&pos))
        vis_offset -= seg->char_count;

      (&pos)->forward_indexable_segment ();

      seg = (&pos)->get_indexable_segment ();
    }

  if (TextBTree::char_is_invisible (&pos))
    vis_offset -= segment_char_offset;
  
  return vis_offset;
}


/**
 * gtk_text_iter_get_visible_line_index:
 * @iter: a #TextIter
 * 
 * Returns the number of bytes from the start of the
 * line to the given @iter, not counting bytes that
 * are invisible due to tags with the "invisible" flag
 * toggled on.
 * 
 * Return value: byte index of @iter with respect to the start of the line
 **/
gint TextIter::get_visible_line_index (void)//TODO const func?
{
//  TextRealIter *real;
  gint vis_offset;
  TextLineSegment *seg;
  TextIter pos;
  
//  g_return_val_if_fail (iter != NULL, 0);

  if (!make_real ())
	  return 0;

//  if (real == NULL)
//    return 0;

  ensure_byte_offsets ();

  check_invariants ();

  vis_offset = line_byte_offset;

  g_assert (vis_offset >= 0);
  
  tree->get_iter_at_line ( &pos, line, 0);

  seg = (&pos)->get_indexable_segment ();

  while (seg != segment)
    {
      /* This is a pretty expensive call, making the
       * whole function pretty lame; we could keep track
       * of current invisibility state by looking at toggle
       * segments as we loop, and then call this function
       * only once per line, in order to speed up the loop
       * quite a lot.
       */
      if (TextBTree::char_is_invisible (&pos))
        vis_offset -= seg->byte_count;

      (&pos)->forward_indexable_segment ();

      seg = (&pos)->get_indexable_segment ();
    }

  if (TextBTree::char_is_invisible (&pos))
    vis_offset -= segment_byte_offset;
  
  return vis_offset;
}

/*
 * Dereferencing
 */

/**
 * gtk_text_iter_get_char:
 * @iter: an iterator
 *
 * Returns the Unicode character at this iterator.  (Equivalent to
 * operator* on a C++ iterator.)  If the element at this iterator is a
 * non-character element, such as an image embedded in the buffer, the
 * Unicode "unknown" character 0xFFFC is returned. If invoked on
 * the end iterator, zero is returned; zero is not a valid Unicode character.
 * So you can write a loop which ends when gtk_text_iter_get_char ()
 * returns 0.
 *
 * Return value: a Unicode character, or 0 if @iter is not dereferenceable
 **/
gunichar TextIter::get_char (void)
{
//  TextRealIter *real;

//  g_return_val_if_fail (iter != NULL, 0);

  if (!make_real ())
	  return 0;

//  if (real == NULL)
//    return 0;

  check_invariants ();

  if (is_end ())
    return 0;
  else if (segment->type == text_segment_char)
    {
      ensure_byte_offsets ();
      
      return g_utf8_get_char (segment->body.chars +
                              segment_byte_offset);
    }
  else
    {
      /* Unicode "unknown character" 0xFFFC */
      return TEXT_UNKNOWN_CHAR;
    }
}

/**
 * TextIter::get_slice:
 * @start: iterator at start of a range
 * @end: iterator at end of a range
 *
 * Returns the text in the given range. A "slice" is an array of
 * characters encoded in UTF-8 format, including the Unicode "unknown"
 * character 0xFFFC for iterable non-character elements in the buffer,
 * such as images.  Because images are encoded in the slice, byte and
 * character offsets in the returned array will correspond to byte
 * offsets in the text buffer. Note that 0xFFFC can occur in normal
 * text as well, so it is not a reliable indicator that a pixbuf or
 * widget is in the buffer.
 *
 * Return value: slice of text from the buffer
 **/
gchar* TextIter::get_slice       (TextIter *start,
                               TextIter *end)
{
  g_return_val_if_fail (start != NULL, NULL);
  g_return_val_if_fail (end != NULL, NULL);

  start->check_invariants ();
  end->check_invariants ();

  return TextBTree::get_text (start, end, true, true);
}

/**
 * TextIter::get_text:
 * @start: iterator at start of a range
 * @end: iterator at end of a range
 *
 * Returns <emphasis>text</emphasis> in the given range.  If the range
 * contains non-text elements such as images, the character and byte
 * offsets in the returned string will not correspond to character and
 * byte offsets in the buffer. If you want offsets to correspond, see
 * TextIter::get_slice ().
 *
 * Return value: array of characters from the buffer
 **/
gchar* TextIter::get_text       (TextIter *start,
                              TextIter *end)
{
  g_return_val_if_fail (start != NULL, NULL);
  g_return_val_if_fail (end != NULL, NULL);

  start->check_invariants();
  end->check_invariants();

  return TextBTree::get_text (start, end, true, false);
}

/**
 * TextIter::get_visible_slice:
 * @start: iterator at start of range
 * @end: iterator at end of range
 *
 * Like TextIter::get_slice (), but invisible text is not included.
 * Invisible text is usually invisible because a #TextTag with the
 * "invisible" attribute turned on has been applied to it.
 *
 * Return value: slice of text from the buffer
 **/
gchar* TextIter::get_visible_slice (TextIter  *start,
                                 TextIter  *end)
{
  g_return_val_if_fail (start != NULL, NULL);
  g_return_val_if_fail (end != NULL, NULL);

  start->check_invariants ();
  end->check_invariants ();

  return TextBTree::get_text (start, end, false, true);
}

/**
 * TextIter::get_visible_text:
 * @start: iterator at start of range
 * @end: iterator at end of range
 *
 * Like TextIter::get_text (), but invisible text is not included.
 * Invisible text is usually invisible because a #TextTag with the
 * "invisible" attribute turned on has been applied to it.
 *
 * Return value: string containing visible text in the range
 **/
gchar* TextIter::get_visible_text (TextIter  *start,
                                TextIter  *end)
{
  g_return_val_if_fail (start != NULL, NULL);
  g_return_val_if_fail (end != NULL, NULL);

  start->check_invariants ();
  end->check_invariants ();

  return TextBTree::get_text (start, end, false, false);
}

/**
 * gtk_text_iter_get_pixbuf:
 * @iter: an iterator
 *
 * If the element at @iter is a pixbuf, the pixbuf is returned
 * (with no new reference count added). Otherwise,
 * %NULL is returned.
 *
 * Return value: the pixbuf at @iter
 **/
/*
GdkPixbuf*
gtk_text_iter_get_pixbuf (const TextIter *iter)
{
  TextRealIter *real;

  g_return_val_if_fail (iter != NULL, NULL);

  if (!make_real ())
	  return 0;

//  if (real == NULL)
//    return NULL;

  check_invariants (iter);

  if (real->segment->type != &gtk_text_pixbuf_type)
    return NULL;
  else
    return real->segment->body.pixbuf.pixbuf;
}*/

/**
 * gtk_text_iter_get_child_anchor:
 * @iter: an iterator
 *
 * If the location at @iter contains a child anchor, the
 * anchor is returned (with no new reference count added). Otherwise,
 * %NULL is returned.
 *
 * Return value: the anchor at @iter
 **/
/* TODO cim5 doesnt support child widgets for now
TextChildAnchor* TextIter::get_child_anchor (void)
{
//  TextRealIter *real;

//  g_return_val_if_fail (iter != NULL, NULL);

  if (!make_real ())
	  return 0;

//  if (real == NULL)
//    return NULL;

  check_invariants ();

  if (segment->type != text_segment_child)
    return NULL;
//  else
//    return segment->body.child.obj;
}*/

/**
 * gtk_text_iter_get_marks:
 * @iter: an iterator
 *
 * Returns a list of all #TextMark at this location. Because marks
 * are not iterable (they don't take up any "space" in the buffer,
 * they are just marks in between iterable locations), multiple marks
 * can exist in the same place. The returned list is not in any
 * meaningful order.
 *
 * Return value: list of #TextMark
 **/
GSList* TextIter::get_marks (void)
{
//  TextRealIter *real;
  TextLineSegment *seg;
  GSList *retval;

//  g_return_val_if_fail (iter != NULL, NULL);

  if (!make_real ())
	  return 0;

//  if (real == NULL)
//    return NULL;

  check_invariants ();

  retval = NULL;
  seg = any_segment;
  while (seg != segment)
    {
      if (seg->type == text_segment_left_mark ||
          seg->type == text_segment_right_mark)
        retval = g_slist_prepend (retval, seg->body.mark.obj);

      seg = seg->next;
    }

  /* The returned list isn't guaranteed to be in any special order,
     and it isn't. */
  return retval;
}

/**
 * gtk_text_iter_get_toggled_tags:
 * @iter: an iterator
 * @toggled_on: %true to get toggled-on tags
 *
 * Returns a list of #TextTag that are toggled on or off at this
 * point.  (If @toggled_on is %true, the list contains tags that are
 * toggled on.) If a tag is toggled on at @iter, then some non-empty
 * range of characters following @iter has that tag applied to it.  If
 * a tag is toggled off, then some non-empty range following @iter
 * does <emphasis>not</emphasis> have the tag applied to it.
 *
 * Return value: tags toggled at this point
 **/
GSList* TextIter::get_toggled_tags  ( bool toggled_on)
{
//  TextRealIter *real;
  TextLineSegment *seg;
  GSList *retval;

//  g_return_val_if_fail (iter != NULL, NULL);

  if (!make_real ())
	  return 0;

//  if (real == NULL)
//    return NULL;

  check_invariants ();

  retval = NULL;
  seg = any_segment;
  while (seg != segment)
    {
      if (toggled_on)
        {
          if (seg->type == text_segment_toggle_on)
            {
              retval = g_slist_prepend (retval, seg->body.toggle.info->tag);
            }
        }
      else
        {
          if (seg->type == text_segment_toggle_off)
            {
              retval = g_slist_prepend (retval, seg->body.toggle.info->tag);
            }
        }

      seg = seg->next;
    }

  /* The returned list isn't guaranteed to be in any special order,
     and it isn't. */
  return retval;
}

/**
 * gtk_text_iter_begins_tag:
 * @iter: an iterator
 * @tag: a #TextTag, or %NULL
 *
 * Returns %true if @tag is toggled on at exactly this point. If @tag
 * is %NULL, returns %true if any tag is toggled on at this point. Note
 * that the gtk_text_iter_begins_tag () returns %true if @iter is the
 * <emphasis>start</emphasis> of the tagged range;
 * gtk_text_iter_has_tag () tells you whether an iterator is
 * <emphasis>within</emphasis> a tagged range.
 *
 * Return value: whether @iter is the start of a range tagged with @tag
 **/
bool TextIter::begins_tag    ( TextTag         *tag)
{
//  TextRealIter *real;
  TextLineSegment *seg;

//  g_return_val_if_fail (iter != NULL, false);

  if (!make_real ())
	  return false;

//  if (real == NULL)
//    return false;

  check_invariants ();

  seg = any_segment;
  while (seg != segment)
    {
      if (seg->type == text_segment_toggle_on)
        {
          if (tag == NULL ||
              seg->body.toggle.info->tag == tag)
            return true;
        }

      seg = seg->next;
    }

  return false;
}

/**
 * gtk_text_iter_ends_tag:
 * @iter: an iterator
 * @tag: a #TextTag, or %NULL
 *
 * Returns %true if @tag is toggled off at exactly this point. If @tag
 * is %NULL, returns %true if any tag is toggled off at this point. Note
 * that the gtk_text_iter_ends_tag () returns %true if @iter is the
 * <emphasis>end</emphasis> of the tagged range;
 * gtk_text_iter_has_tag () tells you whether an iterator is
 * <emphasis>within</emphasis> a tagged range.
 *
 * Return value: whether @iter is the end of a range tagged with @tag
 *
 **/
bool TextIter::ends_tag   ( TextTag         *tag)
{
//  TextRealIter *real;
  TextLineSegment *seg;

//  g_return_val_if_fail (iter != NULL, false);

  if (!make_real ())
	  return false;

//  if (real == NULL)
//    return false;

  check_invariants ();

  seg = any_segment;
  while (seg != segment)
    {
      if (seg->type == text_segment_toggle_off)
        {
          if (tag == NULL ||
              seg->body.toggle.info->tag == tag)
            return true;
        }

      seg = seg->next;
    }

  return false;
}

/**
 * gtk_text_iter_toggles_tag:
 * @iter: an iterator
 * @tag: a #TextTag, or %NULL
 *
 * This is equivalent to (gtk_text_iter_begins_tag () ||
 * gtk_text_iter_ends_tag ()), i.e. it tells you whether a range with
 * @tag applied to it begins <emphasis>or</emphasis> ends at @iter.
 *
 * Return value: whether @tag is toggled on or off at @iter
 **/
bool TextIter::toggles_tag (TextTag *tag)
{
//  TextRealIter *real;
  TextLineSegment *seg;

//  g_return_val_if_fail (iter != NULL, false);

  if (!make_real ())
	  return false;

//  if (real == NULL)
//    return false;

  check_invariants ();

  seg = any_segment;
  while (seg != segment)
    {
      if ( (seg->type == text_segment_toggle_off ||
            seg->type == text_segment_toggle_on) &&
           (tag == NULL ||
            seg->body.toggle.info->tag == tag) )
        return true;

      seg = seg->next;
    }

  return false;
}

/**
 * gtk_text_iter_has_tag:
 * @iter: an iterator
 * @tag: a #TextTag
 *
 * Returns %true if @iter is within a range tagged with @tag.
 *
 * Return value: whether @iter is tagged with @tag
 **/
bool TextIter::has_tag (
                       TextTag          *tag)
{
//  TextRealIter *real;

//  g_return_val_if_fail (iter != NULL, false);
//TODO  g_return_val_if_fail (GTK_IS_TEXT_TAG (tag), false);

  if (!make_surreal())
	  return false;

//  if (real == NULL)
//    return false;

  check_invariants ();

  if (line_byte_offset >= 0)
    {
      return line->byte_has_tag (tree, line_byte_offset, tag);
    }
  else
    {
      g_assert (line_char_offset >= 0);
      return line->char_has_tag (tree, line_char_offset, tag);
    }
}

/**
 * gtk_text_iter_get_tags:
 * @iter: a #TextIter
 * 
 * Returns a list of tags that apply to @iter, in ascending order of
 * priority (highest-priority tags are last). The #TextTag in the
 * list don't have a reference added, but you have to free the list
 * itself.
 * 
 * Return value: list of #TextTag
 **/
GSList* TextIter::get_tags (void)
{
  TextTag** tags;
  gint tag_count = 0;
  gint i;
  GSList *retval;
  
//  g_return_val_if_fail (iter != NULL, NULL);
  
  /* Get the tags at this spot */
  tags = TextBTree::get_tags (this, &tag_count);

  /* No tags, use default style */
  if (tags == NULL || tag_count == 0)
    {
      g_free (tags);

      return NULL;
    }

  retval = NULL;
  i = 0;
  while (i < tag_count)
    {
      retval = g_slist_prepend (retval, tags[i]);
      ++i;
    }
  
  g_free (tags);

  /* Return tags in ascending order of priority */
  return g_slist_reverse (retval);
}

/**
 * gtk_text_iter_editable:
 * @iter: an iterator
 * @default_setting: %true if text is editable by default
 *
 * Returns whether the character at @iter is within an editable region
 * of text.  Non-editable text is "locked" and can't be changed by the
 * user via #TextView. This function is simply a convenience
 * wrapper around gtk_text_iter_get_attributes (). If no tags applied
 * to this text affect editability, @default_setting will be returned.
 *
 * You don't want to use this function to decide whether text can be
 * inserted at @iter, because for insertion you don't want to know
 * whether the char at @iter is inside an editable range, you want to
 * know whether a new character inserted at @iter would be inside an
 * editable range. Use gtk_text_iter_can_insert() to handle this
 * case.
 * 
 * Return value: whether @iter is inside an editable range
 **/
bool TextIter::editable (
                        bool           default_setting)
{
  TextAttributes *values;
  bool retval;

//  g_return_val_if_fail (iter != NULL, false);
  
  values = new TextAttributes();//gtk_text_attributes_new ();

  values->editable = default_setting;

  get_attributes (values);

  retval = values->editable;

  values->unref ();

  return retval;
}

/**
 * gtk_text_iter_can_insert:
 * @iter: an iterator
 * @default_editability: %true if text is editable by default
 * 
 * Considering the default editability of the buffer, and tags that
 * affect editability, determines whether text inserted at @iter would
 * be editable. If text inserted at @iter would be editable then the
 * user should be allowed to insert text at @iter.
 * gtk_text_buffer_insert_interactive() uses this function to decide
 * whether insertions are allowed at a given position.
 * 
 * Return value: whether text inserted at @iter would be editable
 **/
bool TextIter::can_insert ( bool           default_editability)
{
//  g_return_val_if_fail (iter != NULL, false);
  
  if (editable (default_editability))
    return true;
  /* If at start/end of buffer, default editability is used */
  else if ((is_start () ||
            is_end ()) &&
           default_editability)
    return true;
  else
    {
      /* if iter isn't editable, and the char before iter is,
       * then iter is the first char in an editable region
       * and thus insertion at iter results in editable text.
       */
      TextIter prev = *this;
      (&prev)->backward_char ();
      return (&prev)->editable (default_editability);
    }
}


/**
 * gtk_text_iter_get_language:
 * @iter: an iterator
 *
 * A convenience wrapper around gtk_text_iter_get_attributes (),
 * which returns the language in effect at @iter. If no tags affecting
 * language apply to @iter, the return value is identical to that of
 * gtk_get_default_language ().
 *
 * Return value: language in effect at @iter
 **/
/*
PangoLanguage *
gtk_text_iter_get_language (const TextIter *iter)
{
  TextAttributes *values;
  PangoLanguage *retval;
  
  values = gtk_text_attributes_new ();

  gtk_text_iter_get_attributes (iter, values);

  retval = values->language;

  gtk_text_attributes_unref (values);

  return retval;
}*/

/**
 * gtk_text_iter_starts_line:
 * @iter: an iterator
 *
 * Returns %true if @iter begins a paragraph,
 * i.e. if gtk_text_iter_get_line_offset () would return 0.
 * However this function is potentially more efficient than
 * gtk_text_iter_get_line_offset () because it doesn't have to compute
 * the offset, it just has to see whether it's 0.
 *
 * Return value: whether @iter begins a line
 **/
bool TextIter::starts_line (void)
{
//  TextRealIter *real;

//  g_return_val_if_fail (iter != NULL, false);

  if(!make_surreal())
	  return false;

//  if (real == NULL)
//    return false;

  check_invariants ();

  if (line_byte_offset >= 0)
    {
      return (line_byte_offset == 0);
    }
  else
    {
      g_assert (line_char_offset >= 0);
      return (line_char_offset == 0);
    }
}

/**
 * gtk_text_iter_ends_line:
 * @iter: an iterator
 *
 * Returns %true if @iter points to the start of the paragraph
 * delimiter characters for a line (delimiters will be either a
 * newline, a carriage return, a carriage return followed by a
 * newline, or a Unicode paragraph separator character). Note that an
 * iterator pointing to the \n of a \r\n pair will not be counted as
 * the end of a line, the line ends before the \r. The end iterator is
 * considered to be at the end of a line, even though there are no
 * paragraph delimiter chars there.
 *
 * Return value: whether @iter is at the end of a line
 **/
bool TextIter::ends_line (void)
{
  gunichar wc;
  
//  g_return_val_if_fail (iter != NULL, false);

  check_invariants ();

  /* Only one character has type G_UNICODE_PARAGRAPH_SEPARATOR in
   * Unicode 3.0; update this if that changes.
   */
#define PARAGRAPH_SEPARATOR 0x2029

  wc = get_char ();
  
  if (wc == '\r' || wc == PARAGRAPH_SEPARATOR || wc == 0) /* wc == 0 is end iterator */
    return true;
  else if (wc == '\n')
    {
      TextIter tmp = *this;

      /* need to determine if a \r precedes the \n, in which case
       * we aren't the end of the line.
       * Note however that if \r and \n are on different lines, they
       * both are terminators. This for instance may happen after
       * deleting some text:

          1 some text\r    delete 'a'    1 some text\r
          2 a\n            --------->    2 \n
          3 ...                          3 ...

       */

      if ((&tmp)->get_line_offset () == 0)
        return true;

      if (!(&tmp)->backward_char ())
        return true;

      return (&tmp)->get_char () != '\r';
    }
  else
    return false;
}

/**
 * gtk_text_iter_is_end:
 * @iter: an iterator
 *
 * Returns %true if @iter is the end iterator, i.e. one past the last
 * dereferenceable iterator in the buffer. gtk_text_iter_is_end () is
 * the most efficient way to check whether an iterator is the end
 * iterator.
 *
 * Return value: whether @iter is the end iterator
 **/
bool TextIter::is_end (void)
{
//  TextRealIter *real;

//  g_return_val_if_fail (iter != NULL, false);

  if (!make_surreal())
	  return false;

//  if (real == NULL)
//    return false;

  check_invariants ();

  if (!line->contains_end_iter (tree))
    return false;

  /* Now we need the segments validated */
  if (!make_real ())
	  return false;
//  real = gtk_text_iter_make_real (iter);

//  if (real == NULL)
//    return false;
  
  return tree->is_end (line,
                                 segment,
                                 segment_byte_offset,
                                 segment_char_offset);
}

/**
 * gtk_text_iter_is_start:
 * @iter: an iterator
 *
 * Returns %true if @iter is the first iterator in the buffer, that is
 * if @iter has a character offset of 0.
 *
 * Return value: whether @iter is the first in the buffer
 **/
bool TextIter::is_start (void)
{
  return get_offset () == 0;
}

/**
 * gtk_text_iter_get_chars_in_line:
 * @iter: an iterator
 *
 * Returns the number of characters in the line containing @iter,
 * including the paragraph delimiters.
 *
 * Return value: number of characters in the line
 **/
gint TextIter::get_chars_in_line (void)
{
//  TextRealIter *real;
  gint count;
  TextLineSegment *seg;

//  g_return_val_if_fail (iter != NULL, 0);

  if (!make_surreal())
	  return 0;

//  if (real == NULL)
//    return 0;

  check_invariants ();

  if (line_char_offset >= 0)
    {
      /* We can start at the segments we've already found. */
      count = line_char_offset - segment_char_offset;
      seg = get_indexable_segment ();
    }
  else
    {
      /* count whole line. */
      seg = line->segments;
      count = 0;
    }


  while (seg != NULL)
    {
      count += seg->char_count;

      seg = seg->next;
    }

  if (line->contains_end_iter (tree))
    count -= 1; /* Dump the newline that was in the last segment of the end iter line */
  
  return count;
}

/**
 * gtk_text_iter_get_bytes_in_line:
 * @iter: an iterator
 *
 * Returns the number of bytes in the line containing @iter,
 * including the paragraph delimiters.
 *
 * Return value: number of bytes in the line
 **/
gint TextIter::get_bytes_in_line (void)
{
//  TextRealIter *real;
  gint count;
  TextLineSegment *seg;

//  g_return_val_if_fail (iter != NULL, 0);

  if (!make_surreal())
	  return 0;

//  if (real == NULL)
//    return 0;

  check_invariants ();

  if (line_byte_offset >= 0)
    {
      /* We can start at the segments we've already found. */
      count = line_byte_offset - segment_byte_offset;
      seg = get_indexable_segment ();
    }
  else
    {
      /* count whole line. */
      seg = line->segments;
      count = 0;
    }

  while (seg != NULL)
    {
      count += seg->byte_count;

      seg = seg->next;
    }

  if (line->contains_end_iter (tree))
    count -= 1; /* Dump the newline that was in the last segment of the end iter line */
  
  return count;
}

/**
 * gtk_text_iter_get_attributes:
 * @iter: an iterator
 * @values: a #TextAttributes to be filled in
 *
 * Computes the effect of any tags applied to this spot in the
 * text. The @values parameter should be initialized to the default
 * settings you wish to use if no tags are in effect. You'd typically
 * obtain the defaults from gtk_text_view_get_default_attributes().
 *
 * gtk_text_iter_get_attributes () will modify @values, applying the
 * effects of any tags present at @iter. If any tags affected @values,
 * the function returns %true.
 *
 * Return value: %true if @values was modified
 **/
bool TextIter::get_attributes (
                              TextAttributes  *values)
{
  TextTag** tags;
  gint tag_count = 0;

  /* Get the tags at this spot */
  tags = TextBTree::get_tags (this, &tag_count);

  /* No tags, use default style */
  if (tags == NULL || tag_count == 0)
    {
      g_free (tags);

      return false;
    }

  values->fill_from_tags ( tags, tag_count);

  g_free (tags);

  return true;
}

/*
 * Increments/decrements
 */

/* The return value of this indicates WHETHER WE MOVED.
 * The return value of public functions indicates
 * (MOVEMENT OCCURRED && NEW ITER IS DEREFERENCEABLE)
 *
 * This function will not change the iterator if
 * it's already on the last (end iter) line, i.e. it
 * won't move to the end of the last line.
 */
bool TextIter::forward_line_leaving_caches_unmodified (void)
{
  if (!line->contains_end_iter (tree))
    {
      TextLine *new_line;
      
      //new_line = _gtk_text_line_next (real->line);
      new_line = line->next_line ();

      g_assert (new_line);
      g_assert (new_line != line);
      g_assert (!tree->line_is_last(new_line));
      
      line = new_line;

      line_byte_offset = 0;
      line_char_offset = 0;

      segment_byte_offset = 0;
      segment_char_offset = 0;

      /* Find first segments in new line */
      any_segment = line->segments;
      segment = any_segment;
      while (segment->char_count == 0)
        segment = segment->next;

      return true;
    }
  else
    {
      /* There is no way to move forward a line; we were already at
       * the line containing the end iterator.
       * However we may not be at the end iterator itself.
       */
      
      return false;
    }
}

#if 0
/* The return value of this indicates WHETHER WE MOVED.
 * The return value of public functions indicates
 * (MOVEMENT OCCURRED && NEW ITER IS DEREFERENCEABLE)
 *
 * This function is currently unused, thus it is #if-0-ed. It is
 * left here, since it's non-trivial code that might be useful in
 * the future.
 */
static bool
backward_line_leaving_caches_unmodified (TextRealIter *real)
{
  TextLine *new_line;

  new_line = _gtk_text_line_previous (real->line);

  g_assert (new_line != real->line);

  if (new_line != NULL)
    {
      real->line = new_line;

      real->line_byte_offset = 0;
      real->line_char_offset = 0;

      real->segment_byte_offset = 0;
      real->segment_char_offset = 0;

      /* Find first segments in new line */
      real->any_segment = real->line->segments;
      real->segment = real->any_segment;
      while (real->segment->char_count == 0)
        real->segment = real->segment->next;

      return true;
    }
  else
    {
      /* There is no way to move backward; we were already
         at the first line. */

      /* We leave real->line as-is */

      /* Note that we didn't clamp to the start of the first line. */

      return false;
    }
}
#endif 

/* The return value indicates (MOVEMENT OCCURRED && NEW ITER IS
 * DEREFERENCEABLE)
 */
//TODO better name? forward_char is taken probably merge with textiter::forward_char()
bool TextIter::__forward_char (void)
{
//  TextIter *iter = (TextIter*)real;

  //check_invariants ((TextIter*)real);
  check_invariants ();

  ensure_char_offsets ();

  if ( (segment_char_offset + 1) == segment->char_count)
    {
      /* Need to move to the next segment; if no next segment,
         need to move to next line. */
      return forward_indexable_segment ();
    }
  else
    {
      /* Just moving within a segment. Keep byte count
         up-to-date, if it was already up-to-date. */

      g_assert (segment->type == text_segment_char);

      if (line_byte_offset >= 0)
        {
          gint bytes;
          const char * start =
            segment->body.chars + segment_byte_offset;

          bytes = g_utf8_next_char (start) - start;

          line_byte_offset += bytes;
          segment_byte_offset += bytes;

          g_assert (segment_byte_offset < segment->byte_count);
        }

      line_char_offset += 1;
      segment_char_offset += 1;

      adjust_char_index (1);

      g_assert (segment_char_offset < segment->char_count);

      /* We moved into the middle of a segment, so the any_segment
         must now be the segment we're in the middle of. */
      any_segment = segment;

      check_invariants ();

      if (is_end ())
        return false;
      else
        return true;
    }
}

bool TextIter::forward_indexable_segment (void)
{
  /* Need to move to the next segment; if no next segment,
     need to move to next line. */
  TextLineSegment *seg;
  TextLineSegment *any_seg;
//  TextRealIter *real;
  gint chars_skipped;
  gint bytes_skipped;

//  g_return_val_if_fail (iter != NULL, false);

  if (!make_real ())
	  return false;

//  if (real == NULL)
//    return false;

  check_invariants ();

  if (line_char_offset >= 0)
    {
      chars_skipped = segment->char_count - segment_char_offset;
      g_assert (chars_skipped > 0);
    }
  else
    chars_skipped = 0;

  if (line_byte_offset >= 0)
    {
      bytes_skipped = segment->byte_count - segment_byte_offset;
      g_assert (bytes_skipped > 0);
    }
  else
    bytes_skipped = 0;

  /* Get first segment of any kind */
  any_seg = segment->next;
  /* skip non-indexable segments, if any */
  seg = any_seg;
  while (seg != NULL && seg->char_count == 0)
    seg = seg->next;

  if (seg != NULL)
    {
      any_segment = any_seg;
      segment = seg;

      if (line_byte_offset >= 0)
        {
          g_assert (bytes_skipped > 0);
          segment_byte_offset = 0;
          line_byte_offset += bytes_skipped;
        }

      if (line_char_offset >= 0)
        {
          g_assert (chars_skipped > 0);
          segment_char_offset = 0;
          line_char_offset += chars_skipped;
          adjust_char_index (chars_skipped);
        }

      check_invariants ();

      return !is_end ();
    }
  else
    {
      /* End of the line */
      if (forward_line_leaving_caches_unmodified ())
        {
          adjust_line_number (1);
          if (line_char_offset >= 0)
            adjust_char_index (chars_skipped);

          g_assert (line_byte_offset == 0);
          g_assert (line_char_offset == 0);
          g_assert (segment_byte_offset == 0);
          g_assert (segment_char_offset == 0);
          g_assert (starts_line ());

          check_invariants ();

          return !is_end ();
        }
      else
        {
          /* End of buffer, but iter is still at start of last segment,
           * not at the end iterator. We put it on the end iterator.
           */
          
          check_invariants ();

          g_assert (!tree->line_is_last (line));
          g_assert (line->contains_end_iter (tree));

          forward_to_line_end ();

          g_assert (is_end ());
          
          return false;
        }
    }
}

bool TextIter::at_last_indexable_segment (void)
{
  TextLineSegment *seg;

  /* Return true if there are no indexable segments after
   * this iterator.
   */

  seg = segment->next;
  while (seg)
    {
      if (seg->char_count > 0)
        return false;
      seg = seg->next;
    }
  return true;
}

/* Goes back to the start of the next segment, even if
 * we're not at the start of the current segment (always
 * ends up on a different segment if it returns true)
 */
bool TextIter::backward_indexable_segment (void)
{
  /* Move to the start of the previous segment; if no previous
   * segment, to the last segment in the previous line. This is
   * inherently a bit inefficient due to the singly-linked list and
   * tree nodes, but we can't afford the RAM for doubly-linked.
   */
//  TextRealIter *real;
  TextLineSegment *seg;
  TextLineSegment *any_seg;
  TextLineSegment *prev_seg;
  TextLineSegment *prev_any_seg;
  gint bytes_skipped;
  gint chars_skipped;

//  g_return_val_if_fail (iter != NULL, false);

  if (!make_real ())
	  return false;

//  if (real == NULL)
 //   return false;

  check_invariants ();

  /* Find first segments in line */
  any_seg = line->segments;
  seg = any_seg;
  while (seg->char_count == 0)
    seg = seg->next;

  if (seg == segment)
    {
      /* Could probably do this case faster by hand-coding the
       * iteration.
       */

      /* We were already at the start of a line;
       * go back to the previous line.
       */
      if (backward_line ())
        {
          /* Go forward to last indexable segment in line. */
          while (!at_last_indexable_segment ())
            forward_indexable_segment ();

          check_invariants ();

          return true;
        }
      else
        return false; /* We were at the start of the first line. */
    }

  /* We must be in the middle of a line; so find the indexable
   * segment just before our current segment.
   */
  g_assert (seg != segment);
  do
    {
      prev_seg = seg;
      prev_any_seg = any_seg;

      any_seg = seg->next;
      seg = any_seg;
      while (seg->char_count == 0)
        seg = seg->next;
    }
  while (seg != segment);

  g_assert (prev_seg != NULL);
  g_assert (prev_any_seg != NULL);
  g_assert (prev_seg->char_count > 0);

  /* We skipped the entire previous segment, plus any
   * chars we were into the current segment.
   */
  if (segment_byte_offset >= 0)
    bytes_skipped = prev_seg->byte_count + segment_byte_offset;
  else
    bytes_skipped = -1;

  if (segment_char_offset >= 0)
    chars_skipped = prev_seg->char_count + segment_char_offset;
  else
    chars_skipped = -1;

  segment = prev_seg;
  any_segment = prev_any_seg;
  segment_byte_offset = 0;
  segment_char_offset = 0;

  if (bytes_skipped >= 0)
    {
      if (line_byte_offset >= 0)
        {
          line_byte_offset -= bytes_skipped;
          g_assert (line_byte_offset >= 0);
        }
    }
  else
    line_byte_offset = -1;

  if (chars_skipped >= 0)
    {
      if (line_char_offset >= 0)
        {
          line_char_offset -= chars_skipped;
          g_assert (line_char_offset >= 0);
        }

      if (cached_char_index >= 0)
        {
          cached_char_index -= chars_skipped;
          g_assert (cached_char_index >= 0);
        }
    }
  else
    {
      line_char_offset = -1;
      cached_char_index = -1;
    }

  /* line number is unchanged. */

  check_invariants ();

  return true;
}

/**
 * gtk_text_iter_forward_char:
 * @iter: an iterator
 *
 * Moves @iter forward by one character offset. Note that images
 * embedded in the buffer occupy 1 character slot, so
 * gtk_text_iter_forward_char () may actually move onto an image instead
 * of a character, if you have images in your buffer.  If @iter is the
 * end iterator or one character before it, @iter will now point at
 * the end iterator, and gtk_text_iter_forward_char () returns %false for
 * convenience when writing loops.
 *
 * Return value: whether @iter moved and is dereferenceable
 **/
bool TextIter::forward_char (void)
{
//  TextRealIter *real;

//  g_return_val_if_fail (iter != NULL, false);

  if (!make_real ())
	  return false;

//  if (real == NULL)
//    return false;
 // else
   // {
      check_invariants ();
      return __forward_char ();
//    }
}

/**
 * gtk_text_iter_backward_char:
 * @iter: an iterator
 *
 * Moves backward by one character offset. Returns %true if movement
 * was possible; if @iter was the first in the buffer (character
 * offset 0), gtk_text_iter_backward_char () returns %false for convenience when
 * writing loops.
 *
 * Return value: whether movement was possible
 **/
bool TextIter::backward_char (void)
{
//  g_return_val_if_fail (iter != NULL, false);

  check_invariants ();

  return backward_chars (1);
}

/*
  Definitely we should try to linear scan as often as possible for
  movement within a single line, because we can't use the BTree to
  speed within-line searches up; for movement between lines, we would
  like to avoid the linear scan probably.

  Instead of using this constant, it might be nice to cache the line
  length in the iterator and linear scan if motion is within a single
  line.

  I guess you'd have to profile the various approaches.
*/
#define MAX_LINEAR_SCAN 150


/**
 * gtk_text_iter_forward_chars:
 * @iter: an iterator
 * @count: number of characters to move, may be negative
 *
 * Moves @count characters if possible (if @count would move past the
 * start or end of the buffer, moves to the start or end of the
 * buffer). The return value indicates whether the new position of
 * @iter is different from its original position, and dereferenceable
 * (the last iterator in the buffer is not dereferenceable). If @count
 * is 0, the function does nothing and returns %false.
 *
 * Return value: whether @iter moved and is dereferenceable
 **/
bool TextIter::forward_chars (gint count)
{
//  TextRealIter *real;

//  g_return_val_if_fail (iter != NULL, false);

  FIX_OVERFLOWS (count);
  
  if (!make_real ())
	  return false;
//  real = gtk_text_iter_make_real (iter);

//  if (real == NULL)
//    return false;
  else if (count == 0)
    return false;
  else if (count < 0)
    return backward_chars (0 - count);
  else if (count < MAX_LINEAR_SCAN)
    {
      check_invariants ();

      while (count > 1)
        {
          if (!__forward_char ())
            return false;
          --count;
        }

      return __forward_char ();
    }
  else
    {
      gint current_char_index;
      gint new_char_index;

      check_invariants ();

      current_char_index = get_offset ();

      if (current_char_index == tree->get_char_count ())
        return false; /* can't move forward */

      new_char_index = current_char_index + count;
      set_offset (new_char_index);

      check_invariants ();

      /* Return false if we're on the non-dereferenceable end
       * iterator.
       */
      if (is_end ())
        return false;
      else
        return true;
    }
}

/**
 * gtk_text_iter_backward_chars:
 * @iter: an iterator
 * @count: number of characters to move
 *
 * Moves @count characters backward, if possible (if @count would move
 * past the start or end of the buffer, moves to the start or end of
 * the buffer).  The return value indicates whether the iterator moved
 * onto a dereferenceable position; if the iterator didn't move, or
 * moved onto the end iterator, then %false is returned. If @count is 0,
 * the function does nothing and returns %false.
 *
 * Return value: whether @iter moved and is dereferenceable
 *
 **/
bool TextIter::backward_chars (gint count)
{
//  TextRealIter *real;

//  g_return_val_if_fail (iter != NULL, false);

  FIX_OVERFLOWS (count);
  
  if (!make_real ())
	  return false;

//  if (real == NULL)
//    return false;
  else if (count == 0)
    return false;
  else if (count < 0)
    return forward_chars (0 - count);

  ensure_char_offsets ();
  check_invariants ();

  /* <, not <=, because if count == segment_char_offset
   * we're going to the front of the segment and the any_segment
   * might change
   */
  if (count < segment_char_offset)
    {
      /* Optimize the within-segment case */
      g_assert (segment->char_count > 0);
      g_assert (segment->type == text_segment_char);

      if (line_byte_offset >= 0)
        {
          const char *p;
          gint new_byte_offset;

          /* if in the last fourth of the segment walk backwards */
          if (count < segment_char_offset / 4)
            p = g_utf8_offset_to_pointer (segment->body.chars + segment_byte_offset, 
                                          -count);
          else
            p = g_utf8_offset_to_pointer (segment->body.chars,
                                          segment_char_offset - count);

          new_byte_offset = p - segment->body.chars;
          line_byte_offset -= (segment_byte_offset - new_byte_offset);
          segment_byte_offset = new_byte_offset;
        }

      segment_char_offset -= count;
      line_char_offset -= count;

      adjust_char_index (0 - count);

      check_invariants ();

      return true;
    }
  else
    {
      /* We need to go back into previous segments. For now,
       * just keep this really simple. FIXME
       * use backward_indexable_segment.
       */
      if (true || count > MAX_LINEAR_SCAN)
        {
          gint current_char_index;
          gint new_char_index;

          current_char_index = get_offset ();

          if (current_char_index == 0)
            return false; /* can't move backward */

          new_char_index = current_char_index - count;
          if (new_char_index < 0)
            new_char_index = 0;

          set_offset (new_char_index);

          check_invariants ();

          return true;
        }
      else
        {
          /* FIXME backward_indexable_segment here */

          return false;
        }
    }
}

#if 0

/* These two can't be implemented efficiently (always have to use
 * a linear scan, since that's the only way to find all the non-text
 * segments)
 */

/**
 * gtk_text_iter_forward_text_chars:
 * @iter: a #TextIter
 * @count: number of chars to move
 *
 * Moves forward by @count text characters (pixbufs, widgets,
 * etc. do not count as characters for this). Equivalent to moving
 * through the results of TextIter::get_text (), rather than
 * TextIter::get_slice ().
 *
 * Return value: whether @iter moved and is dereferenceable
 **/
bool
gtk_text_iter_forward_text_chars  (TextIter *iter,
                                   gint         count)
{



}

/**
 * gtk_text_iter_forward_text_chars:
 * @iter: a #TextIter
 * @count: number of chars to move
 *
 * Moves backward by @count text characters (pixbufs, widgets,
 * etc. do not count as characters for this). Equivalent to moving
 * through the results of TextIter::get_text (), rather than
 * TextIter::get_slice ().
 *
 * Return value: whether @iter moved and is dereferenceable
 **/
bool
gtk_text_iter_backward_text_chars (TextIter *iter,
                                   gint         count)
{


}
#endif

/**
 * gtk_text_iter_forward_line:
 * @iter: an iterator
 *
 * Moves @iter to the start of the next line. If the iter is already on the
 * last line of the buffer, moves the iter to the end of the current line.
 * If after the operation, the iter is at the end of the buffer and not
 * dereferencable, returns %false. Otherwise, returns %true.
 *
 * Return value: whether @iter can be dereferenced
 **/
bool TextIter::forward_line (void)
{
//  TextRealIter *real;

//  g_return_val_if_fail (iter != NULL, false);
  
  if (!make_real ())
	  return false;

//  if (real == NULL)
//    return false;

  check_invariants ();

  if (forward_line_leaving_caches_unmodified ())
    {
      invalidate_char_index ();
      adjust_line_number (1);

      check_invariants ();

      if (is_end ())
        return false;
      else
        return true;
    }
  else
    {
      /* On the last line, move to end of it */
      
      if (!is_end ())
        forward_to_end ();
      
      check_invariants ();
      return false;
    }
}

/**
 * gtk_text_iter_backward_line:
 * @iter: an iterator
 *
 * Moves @iter to the start of the previous line. Returns %true if
 * @iter could be moved; i.e. if @iter was at character offset 0, this
 * function returns %false. Therefore if @iter was already on line 0,
 * but not at the start of the line, @iter is snapped to the start of
 * the line and the function returns %true. (Note that this implies that
 * in a loop calling this function, the line number may not change on
 * every iteration, if your first iteration is on line 0.)
 *
 * Return value: whether @iter moved
 **/
bool TextIter::backward_line (void)
{
  TextLine *new_line;
//  TextRealIter *real;
  bool offset_will_change;
  gint offset;

//  g_return_val_if_fail (iter != NULL, false);

  if (!make_real ())
	  return false;

//  if (real == NULL)
  //  return false;

  check_invariants ();

  new_line = line->previous_line ();

  offset_will_change = false;
  if (line_char_offset > 0)
    offset_will_change = true;

  if (new_line != NULL)
    {
      line = new_line;

      adjust_line_number (-1);
    }
  else
    {
      if (!offset_will_change)
        return false;
    }

  invalidate_char_index ();

  line_byte_offset = 0;
  line_char_offset = 0;

  segment_byte_offset = 0;
  segment_char_offset = 0;

  /* Find first segment in line */
  any_segment = line->segments;
  segment = line->byte_to_segment ( 0, &offset);

  g_assert (offset == 0);

  /* Note that if we are on the first line, we snap to the start of
   * the first line and return true, so true means the iterator
   * changed, not that the line changed; this is maybe a bit
   * weird. I'm not sure there's an obvious right thing to do though.
   */

  check_invariants ();

  return true;
}


/**
 * gtk_text_iter_forward_lines:
 * @iter: a #TextIter
 * @count: number of lines to move forward
 *
 * Moves @count lines forward, if possible (if @count would move
 * past the start or end of the buffer, moves to the start or end of
 * the buffer).  The return value indicates whether the iterator moved
 * onto a dereferenceable position; if the iterator didn't move, or
 * moved onto the end iterator, then %false is returned. If @count is 0,
 * the function does nothing and returns %false. If @count is negative,
 * moves backward by 0 - @count lines.
 *
 * Return value: whether @iter moved and is dereferenceable
 **/
bool TextIter::forward_lines (gint count)
{
  FIX_OVERFLOWS (count);
  
  if (count < 0)
    return backward_lines (0 - count);
  else if (count == 0)
    return false;
  else if (count == 1)
    {
      check_invariants ();
      return forward_line ();
    }
  else
    {
      gint old_line;

      if (is_end ())
        return false;
      
      old_line = get_line ();

      set_line (old_line + count);

      if ((get_line () - old_line) < count)
        {
          /* count went past the last line, so move to end of last line */
          if (!is_end ())
            forward_to_end ();
        }
      
      return !is_end ();
    }
}

/**
 * gtk_text_iter_backward_lines:
 * @iter: a #TextIter
 * @count: number of lines to move backward
 *
 * Moves @count lines backward, if possible (if @count would move
 * past the start or end of the buffer, moves to the start or end of
 * the buffer).  The return value indicates whether the iterator moved
 * onto a dereferenceable position; if the iterator didn't move, or
 * moved onto the end iterator, then %false is returned. If @count is 0,
 * the function does nothing and returns %false. If @count is negative,
 * moves forward by 0 - @count lines.
 *
 * Return value: whether @iter moved and is dereferenceable
 **/
bool TextIter::backward_lines (gint count)
{
  FIX_OVERFLOWS (count);
  
  if (count < 0)
    return forward_lines (0 - count);
  else if (count == 0)
    return false;
  else if (count == 1)
    {
      return backward_line ();
    }
  else
    {
      gint old_line;

      old_line = get_line ();

      set_line (MAX (old_line - count, 0));

      return (get_line () != old_line);
    }
}

/**
 * gtk_text_iter_forward_visible_line:
 * @iter: an iterator
 *
 * Moves @iter to the start of the next visible line. Returns %true if there
 * was a next line to move to, and %false if @iter was simply moved to
 * the end of the buffer and is now not dereferenceable, or if @iter was
 * already at the end of the buffer.
 *
 * Return value: whether @iter can be dereferenced
 * 
 * Since: 2.8
 **/
bool TextIter::forward_visible_line (void)
{
  while (forward_line ())
    {
      if (!TextBTree::char_is_invisible (this))
        return true;
      else
        {
          do
            {
              if (!forward_char ())
                return false;
          
              if (!TextBTree::char_is_invisible (this))
                return true;
            }
          while (!ends_line ());
        }
    }
    
  return false;
}

/**
 * gtk_text_iter_backward_visible_line:
 * @iter: an iterator
 *
 * Moves @iter to the start of the previous visible line. Returns %true if
 * @iter could be moved; i.e. if @iter was at character offset 0, this
 * function returns %false. Therefore if @iter was already on line 0,
 * but not at the start of the line, @iter is snapped to the start of
 * the line and the function returns %true. (Note that this implies that
 * in a loop calling this function, the line number may not change on
 * every iteration, if your first iteration is on line 0.)
 *
 * Return value: whether @iter moved
 *
 * Since: 2.8
 **/
bool TextIter::backward_visible_line (void)
{
  while (backward_line ())
    {
      if (!TextBTree::char_is_invisible (this))
        return true;
      else
        {
          do
            {
              if (!TextIter::backward_char ())
                return false;
          
              if (!TextBTree::char_is_invisible (this))
                return true;
            }
          while (!starts_line ());
        }
    }
    
  return false;
}

/**
 * gtk_text_iter_forward_visible_lines:
 * @iter: a #TextIter
 * @count: number of lines to move forward
 *
 * Moves @count visible lines forward, if possible (if @count would move
 * past the start or end of the buffer, moves to the start or end of
 * the buffer).  The return value indicates whether the iterator moved
 * onto a dereferenceable position; if the iterator didn't move, or
 * moved onto the end iterator, then %false is returned. If @count is 0,
 * the function does nothing and returns %false. If @count is negative,
 * moves backward by 0 - @count lines.
 *
 * Return value: whether @iter moved and is dereferenceable
 * 
 * Since: 2.8
 **/
bool TextIter::forward_visible_lines ( gint         count)
{
  FIX_OVERFLOWS (count);
  
  if (count < 0)
    return backward_visible_lines (0 - count);
  else if (count == 0)
    return false;
  else if (count == 1)
    {
      check_invariants ();
      return forward_visible_line ();
    }
  else
    {
      while (forward_visible_line () && count > 0)
        count--;
      return count == 0;
    }    
}

/**
 * gtk_text_iter_backward_visible_lines:
 * @iter: a #TextIter
 * @count: number of lines to move backward
 *
 * Moves @count visible lines backward, if possible (if @count would move
 * past the start or end of the buffer, moves to the start or end of
 * the buffer).  The return value indicates whether the iterator moved
 * onto a dereferenceable position; if the iterator didn't move, or
 * moved onto the end iterator, then %false is returned. If @count is 0,
 * the function does nothing and returns %false. If @count is negative,
 * moves forward by 0 - @count lines.
 *
 * Return value: whether @iter moved and is dereferenceable
 *
 * Since: 2.8
 **/
bool TextIter::backward_visible_lines (
                                      gint         count)
{
  FIX_OVERFLOWS (count);
  
  if (count < 0)
    return forward_visible_lines (0 - count);
  else if (count == 0)
    return false;
  else if (count == 1)
    {
      return backward_visible_line ();
    }
  else
    {
      while (backward_visible_line () && count > 0)
        count--;
      return count == 0;
    }
}

/*
typedef bool (* FindLogAttrFunc) (const PangoLogAttr *attrs,
                                      gint                offset,
                                      gint                min_offset,
                                      gint                len,
                                      gint               *found_offset,
                                      bool            already_moved_initially);

typedef bool (* TestLogAttrFunc) (const PangoLogAttr *attrs,
                                      gint                offset,
                                      gint                min_offset,
                                      gint                len);
*/

/* Word funcs */

/*
bool TextIter::find_word_end_func (const PangoLogAttr *attrs,
                    gint          offset,
                    gint          min_offset,
                    gint          len,
                    gint         *found_offset,
                    bool      already_moved_initially)
{
  if (!already_moved_initially)
    ++offset;

  * Find end of next word *
  while (offset < min_offset + len &&
         !attrs[offset].is_word_end)
    ++offset;

  *found_offset = offset;

  return offset < min_offset + len;
}*/

/*
static bool
is_word_end_func (const PangoLogAttr *attrs,
                  gint          offset,
                  gint          min_offset,
                  gint          len)
{
  return attrs[offset].is_word_end;
}

static bool
find_word_start_func (const PangoLogAttr *attrs,
                      gint          offset,
                      gint          min_offset,
                      gint          len,
                      gint         *found_offset,
                      bool      already_moved_initially)
{
  if (!already_moved_initially)
    --offset;

  * Find start of prev word *
  while (offset >= min_offset &&
         !attrs[offset].is_word_start)
    --offset;

  *found_offset = offset;

  return offset >= min_offset;
}*/

/*static bool
is_word_start_func (const PangoLogAttr *attrs,
                    gint          offset,
                    gint          min_offset,
                    gint          len)
{
  return attrs[offset].is_word_start;
}

static bool
inside_word_func (const PangoLogAttr *attrs,
                  gint          offset,
                  gint          min_offset,
                  gint          len)
{
  * Find next word start or end *
  while (offset >= min_offset &&
         !(attrs[offset].is_word_start || attrs[offset].is_word_end))
    --offset;

  if (offset >= 0)
    return attrs[offset].is_word_start;
  else
    return false;
}*/

/* Sentence funcs */

/*static bool
find_sentence_end_func (const PangoLogAttr *attrs,
                        gint          offset,
                        gint          min_offset,
                        gint          len,
                        gint         *found_offset,
                        bool      already_moved_initially)
{
  if (!already_moved_initially)
    ++offset;

  * Find end of next sentence *
  while (offset < min_offset + len &&
         !attrs[offset].is_sentence_end)
    ++offset;

  *found_offset = offset;

  return offset < min_offset + len;
}

static bool
is_sentence_end_func (const PangoLogAttr *attrs,
                      gint          offset,
                      gint          min_offset,
                      gint          len)
{
  return attrs[offset].is_sentence_end;
}

static bool
find_sentence_start_func (const PangoLogAttr *attrs,
                          gint          offset,
                          gint          min_offset,
                          gint          len,
                          gint         *found_offset,
                          bool      already_moved_initially)
{
  if (!already_moved_initially)
    --offset;

  * Find start of prev sentence *
  while (offset >= min_offset &&
         !attrs[offset].is_sentence_start)
    --offset;

  *found_offset = offset;

  return offset >= min_offset;
}

static bool
is_sentence_start_func (const PangoLogAttr *attrs,
                        gint          offset,
                        gint          min_offset,
                        gint          len)
{
  return attrs[offset].is_sentence_start;
}

static bool
inside_sentence_func (const PangoLogAttr *attrs,
                      gint          offset,
                      gint          min_offset,
                      gint          len)
{
  * Find next sentence start or end *
  while (offset >= min_offset &&
         !(attrs[offset].is_sentence_start || attrs[offset].is_sentence_end))
    --offset;

  return attrs[offset].is_sentence_start;
}*/

/*
static bool
test_log_attrs (const TextIter *iter,
                TextLogAttrFunc    func)
{
  gint char_len;
  //const PangoLogAttr *attrs;
  int offset;
  bool result = false;

  g_return_val_if_fail (iter != NULL, false);

  //attrs = _gtk_text_buffer_get_line_log_attrs (gtk_text_iter_get_buffer (iter),
  //                                             iter, &char_len);

  offset = gtk_text_iter_get_line_offset (iter);

  * char_len may be 0 and attrs will be NULL if so, if
   * iter is the end iter and the last line is empty.
   * 
   * offset may be equal to char_len, since attrs contains an entry
   * for one past the end
   *
  
  if (attrs && offset <= char_len)
    result = (* func) (attrs, offset, 0, char_len);

  return result;
}*/

/*
static bool
find_line_log_attrs (const TextIter *iter,
                     FindLogAttrFunc    func,
                     gint              *found_offset,
                     bool           already_moved_initially)
{
  gint char_len;
  const PangoLogAttr *attrs;
  int offset;
  bool result = false;

  g_return_val_if_fail (iter != NULL, false);
  
  attrs = _gtk_text_buffer_get_line_log_attrs (gtk_text_iter_get_buffer (iter),
                                               iter, &char_len);      

  offset = gtk_text_iter_get_line_offset (iter);
  
  * char_len may be 0 and attrs will be NULL if so, if
   * iter is the end iter and the last line is empty
   *
  
  if (attrs)
    result = (* func) (attrs, offset, 0, char_len, found_offset,
                       already_moved_initially);

  return result;
}*/

/* FIXME this function is very, very gratuitously slow */
/*static bool
find_by_log_attrs (TextIter    *iter,
                   FindLogAttrFunc func,
                   bool        forward,
                   bool        already_moved_initially)
{
  TextIter orig;
  gint offset = 0;
  bool found = false;

  g_return_val_if_fail (iter != NULL, false);

  orig = *iter;
  
  found = find_line_log_attrs (iter, func, &offset, already_moved_initially);
  
  if (!found)
    {
      if (forward)
        {
          if (gtk_text_iter_forward_line (iter))
            return find_by_log_attrs (iter, func, forward,
                                      true);
          else
            return false;
        }
      else
        {                    
          * go to end of previous line. need to check that
           * line is > 0 because backward_line snaps to start of
           * line 0 if it's on line 0
           *
          if (gtk_text_iter_get_line (iter) > 0 && 
              gtk_text_iter_backward_line (iter))
            {
              if (!gtk_text_iter_ends_line (iter))
                gtk_text_iter_forward_to_line_end (iter);
              
              return find_by_log_attrs (iter, func, forward,
                                        true);
            }
          else
            return false;
        }
    }
  else
    {      
      gtk_text_iter_set_line_offset (iter, offset);

      return
        (already_moved_initially || !TextIter::equal (iter, &orig)) &&
        !gtk_text_iter_is_end (iter);
    }
}

static bool 
find_visible_by_log_attrs (TextIter    *iter,
			   FindLogAttrFunc func,
			   bool        forward,
			   bool        already_moved_initially)
{
  TextIter pos;

  g_return_val_if_fail (iter != NULL, false);
  
  pos = *iter;
  
  while (find_by_log_attrs (&pos, func, forward, already_moved_initially)) 
    {
      if (!_gtk_text_btree_char_is_invisible (&pos)) 
	{
	  *iter = pos;
	  return true;
	}
  }

  return false;
}*/

/*
typedef bool (* OneStepFunc) (TextIter *iter);
typedef bool (* MultipleStepFunc) (TextIter *iter, gint count);
				  
bool TextIter::move_multiple_steps (
		     gint count,
		     OneStepFunc step_forward,
		     MultipleStepFunc n_steps_backward)
{
//  g_return_val_if_fail (iter != NULL, false);

  FIX_OVERFLOWS (count);
  
  if (count == 0)
    return false;
  
  if (count < 0)
    return n_steps_backward (iter, -count);
  
  if (!step_forward (iter))
    return false;
  --count;

  while (count > 0)
    {
      if (!step_forward (iter))
        break;
      --count;
    }
  
  return !is_end ();  
}*/
	       

/**
 * gtk_text_iter_forward_word_end:
 * @iter: a #TextIter
 * 
 * Moves forward to the next word end. (If @iter is currently on a
 * word end, moves forward to the next one after that.) Word breaks
 * are determined by Pango and should be correct for nearly any
 * language (if not, the correct fix would be to the Pango word break
 * algorithms).
 * 
 * Return value: %true if @iter moved and is not the end iterator 
 **/
bool TextIter::forward_word_end (void)
{
  //TODO implement!
  //return find_by_log_attrs (iter, find_word_end_func, true, false);
}

/**
 * gtk_text_iter_backward_word_start:
 * @iter: a #TextIter
 * 
 * Moves backward to the previous word start. (If @iter is currently on a
 * word start, moves backward to the next one after that.) Word breaks
 * are determined by Pango and should be correct for nearly any
 * language (if not, the correct fix would be to the Pango word break
 * algorithms).
 * 
 * Return value: %true if @iter moved and is not the end iterator 
 **/
bool TextIter::backward_word_start (void)
{
	//TODO implement
//  return find_by_log_attrs (iter, find_word_start_func, false, false);
}

/* FIXME a loop around a truly slow function means
 * a truly spectacularly slow function.
 */

/**
 * gtk_text_iter_forward_word_ends:
 * @iter: a #TextIter
 * @count: number of times to move
 * 
 * Calls gtk_text_iter_forward_word_end() up to @count times.
 *
 * Return value: %true if @iter moved and is not the end iterator 
 **/
bool TextIter::forward_word_ends ( gint              count)
{
	//TODO implement
//  return move_multiple_steps (iter, count, 
//			      gtk_text_iter_forward_word_end,
//			      gtk_text_iter_backward_word_starts);
}

/**
 * gtk_text_iter_backward_word_starts
 * @iter: a #TextIter
 * @count: number of times to move
 * 
 * Calls gtk_text_iter_backward_word_start() up to @count times.
 *
 * Return value: %true if @iter moved and is not the end iterator 
 **/
bool TextIter::backward_word_starts ( gint               count)
{
	//TODO implement!
//  return move_multiple_steps (iter, count, 
//			      gtk_text_iter_backward_word_start,
//			      gtk_text_iter_forward_word_ends);
}

/**
 * gtk_text_iter_forward_visible_word_end:
 * @iter: a #TextIter
 * 
 * Moves forward to the next visible word end. (If @iter is currently on a
 * word end, moves forward to the next one after that.) Word breaks
 * are determined by Pango and should be correct for nearly any
 * language (if not, the correct fix would be to the Pango word break
 * algorithms).
 * 
 * Return value: %true if @iter moved and is not the end iterator 
 *
 * Since: 2.4
 **/
bool TextIter::forward_visible_word_end (void)
{
	//TODO implement!
//  return find_visible_by_log_attrs (iter, find_word_end_func, true, false);
}

/**
 * gtk_text_iter_backward_visible_word_start:
 * @iter: a #TextIter
 * 
 * Moves backward to the previous visible word start. (If @iter is currently 
 * on a word start, moves backward to the next one after that.) Word breaks
 * are determined by Pango and should be correct for nearly any
 * language (if not, the correct fix would be to the Pango word break
 * algorithms).
 * 
 * Return value: %true if @iter moved and is not the end iterator 
 * 
 * Since: 2.4
 **/
bool TextIter::backward_visible_word_start (void)
{
	//TODO implement
  //return find_visible_by_log_attrs (iter, find_word_start_func, false, false);
}

/**
 * gtk_text_iter_forward_visible_word_ends:
 * @iter: a #TextIter
 * @count: number of times to move
 * 
 * Calls gtk_text_iter_forward_visible_word_end() up to @count times.
 *
 * Return value: %true if @iter moved and is not the end iterator 
 *
 * Since: 2.4
 **/
bool TextIter::forward_visible_word_ends ( gint         count)
{
	//TODO implement
//  return move_multiple_steps (iter, count, 
//			      gtk_text_iter_forward_visible_word_end,
//			      gtk_text_iter_backward_visible_word_starts);
}

/**
 * gtk_text_iter_backward_visible_word_starts
 * @iter: a #TextIter
 * @count: number of times to move
 * 
 * Calls gtk_text_iter_backward_visible_word_start() up to @count times.
 *
 * Return value: %true if @iter moved and is not the end iterator 
 * 
 * Since: 2.4
 **/
bool TextIter::backward_visible_word_starts ( gint         count)
{
	//TODO implement
//  return move_multiple_steps (iter, count, 
//			      gtk_text_iter_backward_visible_word_start,
//			      gtk_text_iter_forward_visible_word_ends);
}

/**
 * gtk_text_iter_starts_word:
 * @iter: a #TextIter
 * 
 * Determines whether @iter begins a natural-language word.  Word
 * breaks are determined by Pango and should be correct for nearly any
 * language (if not, the correct fix would be to the Pango word break
 * algorithms).
 *
 * Return value: %true if @iter is at the start of a word
 **/
bool TextIter::starts_word (void)
{
	//TODO implement
//  return test_log_attrs (iter, is_word_start_func);
}

/**
 * gtk_text_iter_ends_word:
 * @iter: a #TextIter
 * 
 * Determines whether @iter ends a natural-language word.  Word breaks
 * are determined by Pango and should be correct for nearly any
 * language (if not, the correct fix would be to the Pango word break
 * algorithms).
 *
 * Return value: %true if @iter is at the end of a word
 **/
bool TextIter::ends_word (void)
{
	//TODO implement
//  return test_log_attrs (iter, is_word_end_func);
}

/**
 * gtk_text_iter_inside_word:
 * @iter: a #TextIter
 * 
 * Determines whether @iter is inside a natural-language word (as
 * opposed to say inside some whitespace).  Word breaks are determined
 * by Pango and should be correct for nearly any language (if not, the
 * correct fix would be to the Pango word break algorithms).
 * 
 * Return value: %true if @iter is inside a word
 **/
bool TextIter::inside_word (void)
{
	//TODO implement
//  return test_log_attrs (iter, inside_word_func);
}

/**
 * gtk_text_iter_starts_sentence:
 * @iter: a #TextIter
 * 
 * Determines whether @iter begins a sentence.  Sentence boundaries are
 * determined by Pango and should be correct for nearly any language
 * (if not, the correct fix would be to the Pango text boundary
 * algorithms).
 * 
 * Return value: %true if @iter is at the start of a sentence.
 **/
bool TextIter::starts_sentence (void)
{
	//TODO implement
//  return test_log_attrs (iter, is_sentence_start_func);
}

/**
 * gtk_text_iter_ends_sentence:
 * @iter: a #TextIter
 * 
 * Determines whether @iter ends a sentence.  Sentence boundaries are
 * determined by Pango and should be correct for nearly any language
 * (if not, the correct fix would be to the Pango text boundary
 * algorithms).
 * 
 * Return value: %true if @iter is at the end of a sentence.
 **/
bool TextIter::ends_sentence (void)
{
	//TODO implement
//  return test_log_attrs (iter, is_sentence_end_func);
}

/**
 * gtk_text_iter_inside_sentence:
 * @iter: a #TextIter
 * 
 * Determines whether @iter is inside a sentence (as opposed to in
 * between two sentences, e.g. after a period and before the first
 * letter of the next sentence).  Sentence boundaries are determined
 * by Pango and should be correct for nearly any language (if not, the
 * correct fix would be to the Pango text boundary algorithms).
 * 
 * Return value: %true if @iter is inside a sentence.
 **/
bool TextIter::inside_sentence (void)
{
	//TODO implement
//  return test_log_attrs (iter, inside_sentence_func);
}

/**
 * gtk_text_iter_forward_sentence_end:
 * @iter: a #TextIter
 * 
 * Moves forward to the next sentence end. (If @iter is at the end of
 * a sentence, moves to the next end of sentence.)  Sentence
 * boundaries are determined by Pango and should be correct for nearly
 * any language (if not, the correct fix would be to the Pango text
 * boundary algorithms).
 * 
 * Return value: %true if @iter moved and is not the end iterator
 **/
bool TextIter::forward_sentence_end (void)
{
	//TODO implement
//  return find_by_log_attrs (iter, find_sentence_end_func, true, false);
}

/**
 * gtk_text_iter_backward_sentence_start:
 * @iter: a #TextIter
 * 
 * Moves backward to the previous sentence start; if @iter is already at
 * the start of a sentence, moves backward to the next one.  Sentence
 * boundaries are determined by Pango and should be correct for nearly
 * any language (if not, the correct fix would be to the Pango text
 * boundary algorithms).
 * 
 * Return value: %true if @iter moved and is not the end iterator
 **/
bool TextIter::backward_sentence_start (void)
{
	//TODO implement
//  return find_by_log_attrs (iter, find_sentence_start_func, false, false);
}

/* FIXME a loop around a truly slow function means
 * a truly spectacularly slow function.
 */
/**
 * gtk_text_iter_forward_sentence_ends:
 * @iter: a #TextIter
 * @count: number of sentences to move
 * 
 * Calls gtk_text_iter_forward_sentence_end() @count times (or until
 * gtk_text_iter_forward_sentence_end() returns %false). If @count is
 * negative, moves backward instead of forward.
 * 
 * Return value: %true if @iter moved and is not the end iterator
 **/
bool TextIter::forward_sentence_ends ( gint              count)
{
	//TODO implement
//  return move_multiple_steps (iter, count, 
//			      gtk_text_iter_forward_sentence_end,
//			      gtk_text_iter_backward_sentence_starts);
}

/**
 * gtk_text_iter_backward_sentence_starts:
 * @iter: a #TextIter
 * @count: number of sentences to move
 * 
 * Calls gtk_text_iter_backward_sentence_start() up to @count times,
 * or until it returns %false. If @count is negative, moves forward
 * instead of backward.
 * 
 * Return value: %true if @iter moved and is not the end iterator
 **/
bool TextIter::backward_sentence_starts ( gint               count)
{
	//TODO implement
//  return move_multiple_steps (iter, count, 
//			      gtk_text_iter_backward_sentence_start,
//			      gtk_text_iter_forward_sentence_ends);
}

/*
bool TextIter::find_forward_cursor_pos_func (const PangoLogAttr *attrs,
                              gint          offset,
                              gint          min_offset,
                              gint          len,
                              gint         *found_offset,
                              bool      already_moved_initially)
{
  if (!already_moved_initially)
    ++offset;

  while (offset < (min_offset + len) &&
         !attrs[offset].is_cursor_position)
    ++offset;

  *found_offset = offset;

  return offset < (min_offset + len);
}

static bool
find_backward_cursor_pos_func (const PangoLogAttr *attrs,
                               gint          offset,
                               gint          min_offset,
                               gint          len,
                               gint         *found_offset,
                               bool      already_moved_initially)
{  
  if (!already_moved_initially)
    --offset;

  while (offset > min_offset &&
         !attrs[offset].is_cursor_position)
    --offset;

  *found_offset = offset;
  
  return offset >= min_offset;
}

static bool
is_cursor_pos_func (const PangoLogAttr *attrs,
                    gint          offset,
                    gint          min_offset,
                    gint          len)
{
  return attrs[offset].is_cursor_position;
}

**
 * gtk_text_iter_forward_cursor_position:
 * @iter: a #TextIter
 * 
 * Moves @iter forward by a single cursor position. Cursor positions
 * are (unsurprisingly) positions where the cursor can appear. Perhaps
 * surprisingly, there may not be a cursor position between all
 * characters. The most common example for European languages would be
 * a carriage return/newline sequence. For some Unicode characters,
 * the equivalent of say the letter "a" with an accent mark will be
 * represented as two characters, first the letter then a "combining
 * mark" that causes the accent to be rendered; so the cursor can't go
 * between those two characters. See also the #PangoLogAttr structure and
 * pango_break() function.
 * 
 * Return value: %true if we moved and the new position is dereferenceable
 **

*/
bool TextIter::forward_cursor_position (void)
{
	//TODO implement
//  return find_by_log_attrs (iter, find_forward_cursor_pos_func, true, false);
}

/**
 * gtk_text_iter_backward_cursor_position:
 * @iter: a #TextIter
 * 
 * Like gtk_text_iter_forward_cursor_position(), but moves backward.
 * 
 * Return value: %true if we moved
 **/
bool TextIter::backward_cursor_position (void)
{
	//TODO implement
//  return find_by_log_attrs (iter, find_backward_cursor_pos_func, false, false);
}

/**
 * gtk_text_iter_forward_cursor_positions:
 * @iter: a #TextIter
 * @count: number of positions to move
 * 
 * Moves up to @count cursor positions. See
 * gtk_text_iter_forward_cursor_position() for details.
 * 
 * Return value: %true if we moved and the new position is dereferenceable
 **/
bool TextIter::forward_cursor_positions ( gint         count)
{
	//TODO implement
//  return move_multiple_steps (iter, count, 
//			      gtk_text_iter_forward_cursor_position,
//			      gtk_text_iter_backward_cursor_positions);
}

/**
 * gtk_text_iter_backward_cursor_positions:
 * @iter: a #TextIter
 * @count: number of positions to move
 *
 * Moves up to @count cursor positions. See
 * gtk_text_iter_forward_cursor_position() for details.
 * 
 * Return value: %true if we moved and the new position is dereferenceable
 **/
bool TextIter::backward_cursor_positions ( gint         count)
{
	//TODO implement
//  return move_multiple_steps (iter, count, 
//			      gtk_text_iter_backward_cursor_position,
//			      gtk_text_iter_forward_cursor_positions);
}

/**
 * gtk_text_iter_forward_visible_cursor_position:
 * @iter: a #TextIter
 * 
 * Moves @iter forward to the next visible cursor position. See 
 * gtk_text_iter_forward_cursor_position() for details.
 * 
 * Return value: %true if we moved and the new position is dereferenceable
 * 
 * Since: 2.4
 **/
bool TextIter::forward_visible_cursor_position (void)
{
	//TODO implement
//  return find_visible_by_log_attrs (iter, find_forward_cursor_pos_func, true, false);
}

/**
 * gtk_text_iter_backward_visible_cursor_position:
 * @iter: a #TextIter
 * 
 * Moves @iter forward to the previous visible cursor position. See 
 * gtk_text_iter_backward_cursor_position() for details.
 * 
 * Return value: %true if we moved and the new position is dereferenceable
 * 
 * Since: 2.4
 **/
bool TextIter::backward_visible_cursor_position (void)
{
	//TODO implement
//  return find_visible_by_log_attrs (iter, find_backward_cursor_pos_func, false, false);
}

/**
 * gtk_text_iter_forward_visible_cursor_positions:
 * @iter: a #TextIter
 * @count: number of positions to move
 * 
 * Moves up to @count visible cursor positions. See
 * gtk_text_iter_forward_cursor_position() for details.
 * 
 * Return value: %true if we moved and the new position is dereferenceable
 * 
 * Since: 2.4
 **/
bool TextIter::forward_visible_cursor_positions ( gint         count)
{
	//TODO implement
//  return move_multiple_steps (iter, count, 
//			      gtk_text_iter_forward_visible_cursor_position,
//			      gtk_text_iter_backward_visible_cursor_positions);
}

/**
 * gtk_text_iter_backward_visible_cursor_positions:
 * @iter: a #TextIter
 * @count: number of positions to move
 *
 * Moves up to @count visible cursor positions. See
 * gtk_text_iter_backward_cursor_position() for details.
 * 
 * Return value: %true if we moved and the new position is dereferenceable
 * 
 * Since: 2.4
 **/
bool TextIter::backward_visible_cursor_positions ( gint         count)
{
	//TODO implement
//  return move_multiple_steps (iter, count, 
//			      gtk_text_iter_backward_visible_cursor_position,
//			      gtk_text_iter_forward_visible_cursor_positions);
}

/**
 * gtk_text_iter_is_cursor_position:
 * @iter: a #TextIter
 * 
 * See gtk_text_iter_forward_cursor_position() or #PangoLogAttr or
 * pango_break() for details on what a cursor position is.
 * 
 * Return value: %true if the cursor can be placed at @iter
 **/
bool TextIter::is_cursor_position (void)
{
	//TODO implement
//  return test_log_attrs (iter, is_cursor_pos_func);
}

/**
 * gtk_text_iter_set_line_offset:
 * @iter: a #TextIter 
 * @char_on_line: a character offset relative to the start of @iter's current line
 * 
 * Moves @iter within a line, to a new <emphasis>character</emphasis>
 * (not byte) offset. The given character offset must be less than or
 * equal to the number of characters in the line; if equal, @iter
 * moves to the start of the next line. See
 * gtk_text_iter_set_line_index() if you have a byte index rather than
 * a character offset.
 *
 **/
void TextIter::set_line_offset ( gint         char_on_line)
{
//  TextRealIter *real;
  gint chars_in_line;
  
//  g_return_if_fail (iter != NULL);

  if (!make_surreal())
	  return;

//  if (real == NULL)
//    return;
  
  check_invariants ();

  chars_in_line = get_chars_in_line ();

  g_return_if_fail (char_on_line <= chars_in_line);

  if (char_on_line < chars_in_line)
    set_from_char_offset (line, char_on_line);
  else
    forward_line (); /* set to start of next line */
  
  check_invariants ();
}

/**
 * gtk_text_iter_set_line_index:
 * @iter: a #TextIter
 * @byte_on_line: a byte index relative to the start of @iter's current line
 *
 * Same as gtk_text_iter_set_line_offset(), but works with a
 * <emphasis>byte</emphasis> index. The given byte index must be at
 * the start of a character, it can't be in the middle of a UTF-8
 * encoded character.
 * 
 **/
void TextIter::set_line_index ( gint         byte_on_line)
{
//  TextRealIter *real;
  gint bytes_in_line;
  
//  g_return_if_fail (iter != NULL);

  if (!make_surreal())
	  return;

//  if (real == NULL)
//    return;

  check_invariants ();

  bytes_in_line = get_bytes_in_line ();

  g_return_if_fail (byte_on_line <= bytes_in_line);
  
  if (byte_on_line < bytes_in_line)
    set_from_byte_offset (line, byte_on_line);
  else
    forward_line ();

  if (segment->type == text_segment_char &&
      (segment->body.chars[segment_byte_offset] & 0xc0) == 0x80)
    g_warning ("%s: Incorrect byte offset %d falls in the middle of a UTF-8 "
               "character; this will crash the text buffer. "
               "Byte indexes must refer to the start of a character.",
               G_STRLOC, byte_on_line);

  check_invariants ();
}


/**
 * gtk_text_iter_set_visible_line_offset:
 * @iter: a #TextIter
 * @char_on_line: a character offset
 * 
 * Like gtk_text_iter_set_line_offset(), but the offset is in visible
 * characters, i.e. text with a tag making it invisible is not
 * counted in the offset.
 **/
void TextIter::set_visible_line_offset ( gint         char_on_line)
{
  gint chars_seen = 0;
  TextIter pos;

//  g_return_if_fail (iter != NULL);
  
  set_line_offset (0);

  pos = *this;

  /* For now we use a ludicrously slow implementation */
  while (chars_seen < char_on_line)
    {
      if (!TextBTree::char_is_invisible (&pos))
        ++chars_seen;

      if (!(&pos)->forward_char ())
        break;

      if (chars_seen == char_on_line)
        break;
    }
  
  if ((&pos)->get_text_line () == get_text_line ())
    *this = pos;
  else
    forward_line ();
}

/**
 * gtk_text_iter_set_visible_line_index:
 * @iter: a #TextIter
 * @byte_on_line: a byte index
 * 
 * Like gtk_text_iter_set_line_index(), but the index is in visible
 * bytes, i.e. text with a tag making it invisible is not counted
 * in the index.
 **/
void TextIter::set_visible_line_index ( gint         byte_on_line)
{
//  TextRealIter *real;
  gint offset = 0;
  TextIter pos;
  TextLineSegment *seg;
  
//  g_return_if_fail (iter != NULL);

  set_line_offset (0);

  pos = *this;

  if (!pos.make_real ())
	  return;

//  if (real == NULL)
  //  return;

  pos.ensure_byte_offsets ();

  pos.check_invariants ();

  seg = pos.get_indexable_segment ();

  while (seg != NULL && byte_on_line > 0)
    {
      if (!TextBTree::char_is_invisible (&pos))
        {
          if (byte_on_line < seg->byte_count)
            {
              pos.set_from_byte_offset (line, offset + byte_on_line);
              byte_on_line = 0;
              break;
            }
          else
            byte_on_line -= seg->byte_count;
        }

      offset += seg->byte_count;
      pos.forward_indexable_segment ();
      seg = pos.get_indexable_segment ();
    }

  if (byte_on_line == 0)
    *this = pos;
  else
    forward_line ();
}

/**
 * gtk_text_iter_set_line:
 * @iter: a #TextIter
 * @line_number: line number (counted from 0)
 *
 * Moves iterator @iter to the start of the line @line_number.  If
 * @line_number is negative or larger than the number of lines in the
 * buffer, moves @iter to the start of the last line in the buffer.
 * 
 **/
void TextIter::set_line ( gint         line_number)
{
  TextLine *line;
  gint real_line;
//  TextRealIter *real;

//  g_return_if_fail (iter != NULL);

  if (!make_surreal())
	  return;

//  if (real == NULL)
//    return;

  check_invariants ();

  line = tree->get_line_no_last (line_number, &real_line);

  set_from_char_offset (line, 0);

  /* We might as well cache this, since we know it. */
  cached_line_number = real_line;

  check_invariants ();
}

/**
 * gtk_text_iter_set_offset:
 * @iter: a #TextIter
 * @char_offset: a character number
 *
 * Sets @iter to point to @char_offset. @char_offset counts from the start
 * of the entire text buffer, starting with 0.
 **/
void TextIter::set_offset (gint         char_offset)
{
  TextLine *line;
//  TextRealIter *real;
  gint line_start;
  gint real_char_index;

//  g_return_if_fail (iter != NULL);

  if (!make_surreal())
	  return;

//  if (real == NULL)
//    return;

  check_invariants ();

  if (cached_char_index >= 0 &&
      cached_char_index == char_offset)
    return;

  line = tree->get_line_at_char (
                                           char_offset,
                                           &line_start,
                                           &real_char_index);

  set_from_char_offset (line, real_char_index - line_start);

  /* Go ahead and cache this since we have it. */
  cached_char_index = real_char_index;

  check_invariants ();
}

/**
 * gtk_text_iter_forward_to_end:
 * @iter: a #TextIter
 *
 * Moves @iter forward to the "end iterator," which points one past the last
 * valid character in the buffer. gtk_text_iter_get_char() called on the
 * end iterator returns 0, which is convenient for writing loops.
 **/
void TextIter::forward_to_end  (void)
{
  TextBuffer *buffer;
//  TextRealIter *real;

//  g_return_if_fail (iter != NULL);

  if (!make_surreal())
	  return;

//  if (real == NULL)
//    return;

  buffer = tree->get_buffer ();

  buffer->get_end_iter (this);
}

/* FIXME this and gtk_text_iter_forward_to_line_end() could be cleaned up
 * and made faster. Look at iter_ends_line() for inspiration, perhaps.
 * If all else fails we could cache the para delimiter pos in the iter.
 * I think forward_to_line_end() actually gets called fairly often.
 */
int TextIter::find_paragraph_delimiter_for_line (void)
{
  TextIter end;
  end = *this;

  if ((&end)->get_text_line ()->contains_end_iter(
                                        (&end)->get_btree ()))
    {
      (&end)->forward_to_end ();
    }
  else
    {
      /* if we aren't on the last line, go forward to start of next line, then scan
       * back for the delimiters on the previous line
       */
      (&end)->forward_line ();
      (&end)->backward_char ();
      while (!(&end)->ends_line ())
        (&end)->backward_char ();
    }

  return (&end)->get_line_offset ();
}

/**
 * gtk_text_iter_forward_to_line_end:
 * @iter: a #TextIter
 * 
 * Moves the iterator to point to the paragraph delimiter characters,
 * which will be either a newline, a carriage return, a carriage
 * return/newline in sequence, or the Unicode paragraph separator
 * character. If the iterator is already at the paragraph delimiter
 * characters, moves to the paragraph delimiter characters for the
 * next line. If @iter is on the last line in the buffer, which does
 * not end in paragraph delimiters, moves to the end iterator (end of
 * the last line), and returns %false.
 * 
 * Return value: %true if we moved and the new location is not the end iterator
 **/
bool TextIter::forward_to_line_end (void)
{
  gint current_offset;
  gint new_offset;

//  g_return_val_if_fail (iter != NULL, false);

  current_offset = get_line_offset ();
  new_offset = find_paragraph_delimiter_for_line ();
  
  if (current_offset < new_offset)
    {
      /* Move to end of this line. */
      set_line_offset (new_offset);
      return !is_end ();
    }
  else
    {
      /* Move to end of next line. */
      if (forward_line ())
        {
          /* We don't want to move past all
           * empty lines.
           */
          if (!ends_line ())
            forward_to_line_end ();
          return !is_end ();
        }
      else
        return false;
    }
}

/**
 * gtk_text_iter_forward_to_tag_toggle:
 * @iter: a #TextIter
 * @tag: a #TextTag, or %NULL
 *
 * Moves forward to the next toggle (on or off) of the
 * #TextTag @tag, or to the next toggle of any tag if
 * @tag is %NULL. If no matching tag toggles are found,
 * returns %false, otherwise %true. Does not return toggles
 * located at @iter, only toggles after @iter. Sets @iter to
 * the location of the toggle, or to the end of the buffer
 * if no toggle is found.
 *
 * Return value: whether we found a tag toggle after @iter
 **/
bool TextIter::forward_to_tag_toggle ( TextTag  *tag)
{
  TextLine *next_line;
  TextLine *current_line;
//  TextRealIter *real;

//  g_return_val_if_fail (iter != NULL, false);

  if (!make_real ())
	  return false;

//  if (real == NULL)
//    return false;

  check_invariants ();

  current_line = line;
  next_line = current_line->next_could_contain_tag ( tree, tag);

  while (forward_indexable_segment ())
    {
      /* If we went forward to a line that couldn't contain a toggle
         for the tag, then skip forward to a line that could contain
         it. This potentially skips huge hunks of the tree, so we
         aren't a purely linear search. */
      if (line != current_line)
        {
          if (next_line == NULL)
            {
              /* End of search. Set to end of buffer. */
              tree->get_end_iter (this);
              return false;
            }

          if (line != next_line)
            set_from_byte_offset (next_line, 0);

          current_line = line;
          next_line = line->next_could_contain_tag ( tree, tag);
        }

      if (toggles_tag (tag))
        {
          /* If there's a toggle here, it isn't indexable so
             any_segment can't be the indexable segment. */
          g_assert (any_segment != segment);
          return true;
        }
    }

  /* Check end iterator for tags */
  if (toggles_tag (tag))
    {
      /* If there's a toggle here, it isn't indexable so
         any_segment can't be the indexable segment. */
      g_assert (any_segment != segment);
      return true;
    }

  /* Reached end of buffer */
  return false;
}

/**
 * gtk_text_iter_backward_to_tag_toggle:
 * @iter: a #TextIter
 * @tag: a #TextTag, or %NULL
 *
 * Moves backward to the next toggle (on or off) of the
 * #TextTag @tag, or to the next toggle of any tag if
 * @tag is %NULL. If no matching tag toggles are found,
 * returns %false, otherwise %true. Does not return toggles
 * located at @iter, only toggles before @iter. Sets @iter
 * to the location of the toggle, or the start of the buffer
 * if no toggle is found.
 *
 * Return value: whether we found a tag toggle before @iter
 **/
bool TextIter::backward_to_tag_toggle ( TextTag  *tag)
{
  TextLine *prev_line;
  TextLine *current_line;
//  TextRealIter *real;

//  g_return_val_if_fail (iter != NULL, false);

  if (!make_real ())
	  return false;

//  if (real == NULL)
  //  return false;

  check_invariants ();

  current_line = line;
  prev_line = current_line->previous_could_contain_tag ( tree, tag);


  /* If we're at segment start, go to the previous segment;
   * if mid-segment, snap to start of current segment.
   */
  if (is_segment_start ())
    {
      if (!backward_indexable_segment ())
        return false;
    }
  else
    {
      ensure_char_offsets ();

      if (!backward_chars (segment_char_offset))
        return false;
    }

  do
    {
      /* If we went backward to a line that couldn't contain a toggle
       * for the tag, then skip backward further to a line that
       * could contain it. This potentially skips huge hunks of the
       * tree, so we aren't a purely linear search.
       */
      if (line != current_line)
        {
          if (prev_line == NULL)
            {
              /* End of search. Set to start of buffer. */
              tree->get_iter_at_char (this, 0);
              return false;
            }

          if (line != prev_line)
            {
              /* Set to last segment in prev_line (could do this
               * more quickly)
               */
              set_from_byte_offset (prev_line, 0);

              while (!at_last_indexable_segment ())
                forward_indexable_segment ();
            }

          current_line = line;
          prev_line = current_line->previous_could_contain_tag ( tree, tag);
        }

      if (toggles_tag (tag))
        {
          /* If there's a toggle here, it isn't indexable so
           * any_segment can't be the indexable segment.
           */
          g_assert (any_segment != segment);
          return true;
        }
    }
  while (backward_indexable_segment ());

  /* Reached front of buffer */
  return false;
}

bool TextIter::matches_pred (
              TextCharPredicate pred,
              gpointer user_data)
{
  gint ch;

  ch = get_char ();

  return pred (ch, user_data);
}

/**
 * gtk_text_iter_forward_find_char:
 * @iter: a #TextIter
 * @pred: a function to be called on each character
 * @user_data: user data for @pred
 * @limit: search limit, or %NULL for none 
 * 
 * Advances @iter, calling @pred on each character. If
 * @pred returns %true, returns %true and stops scanning.
 * If @pred never returns %true, @iter is set to @limit if
 * @limit is non-%NULL, otherwise to the end iterator.
 * 
 * Return value: whether a match was found
 **/
bool TextIter::forward_find_char (
                                 TextCharPredicate pred,
                                 gpointer             user_data,
                                 TextIter   *limit)
{
//  g_return_val_if_fail (iter != NULL, false);
  g_return_val_if_fail (pred != NULL, false);

  if (limit &&
      TextIter::compare (this, limit) >= 0)
    return false;
  
  while ((limit == NULL ||
          !TextIter::equal (limit, this)) &&
         forward_char ())
    {      
      if (matches_pred (pred, user_data))
        return true;
    }

  return false;
}

/**
 * gtk_text_iter_backward_find_char:
 * @iter: a #TextIter
 * @pred: function to be called on each character
 * @user_data: user data for @pred
 * @limit: search limit, or %NULL for none
 * 
 * Same as gtk_text_iter_forward_find_char(), but goes backward from @iter.
 * 
 * Return value: whether a match was found
 **/
bool TextIter::backward_find_char (
                                  TextCharPredicate pred,
                                  gpointer             user_data,
                                  TextIter   *limit)
{
//  g_return_val_if_fail (iter != NULL, false);
  g_return_val_if_fail (pred != NULL, false);

  if (limit &&
      TextIter::compare (this, limit) <= 0)
    return false;
  
  while ((limit == NULL ||
          !TextIter::equal (limit, this)) &&
         backward_char ())
    {
      if (matches_pred (pred, user_data))
        return true;
    }

  return false;
}

void TextIter::forward_chars_with_skipping (
                             gint         count,
                             bool     skip_invisible,
                             bool     skip_nontext)
{

  gint i;

  g_return_if_fail (count >= 0);

  i = count;

  while (i > 0)
    {
      bool ignored = false;

      if (skip_nontext &&
          get_char () == TEXT_UNKNOWN_CHAR)
        ignored = true;

      if (!ignored &&
          skip_invisible &&
          TextBTree::char_is_invisible (this))
        ignored = true;

      forward_char ();

      if (!ignored)
        --i;
    }
}

bool TextIter::lines_match (
             const gchar **lines,
             bool visible_only,
             bool slice,
             TextIter *match_start,
             TextIter *match_end)
{
  TextIter next;
  gchar *line_text;
  const gchar *found;
  gint offset;

  if (*lines == NULL || **lines == '\0')
    {
      if (match_start)
        *match_start = *this;

      if (match_end)
        *match_end = *this;
      return true;
    }

  next = *this;
  (&next)->forward_line ();

  /* No more text in buffer, but *lines is nonempty */
  if (TextIter::equal (this, &next))
    {
      return false;
    }

  if (slice)
    {
      if (visible_only)
        line_text = get_visible_slice (this, &next);
      else
        line_text = get_slice (this, &next);
    }
  else
    {
      if (visible_only)
        line_text = get_visible_text (this, &next);
      else
        line_text = get_text (this, &next);
    }

  if (match_start) /* if this is the first line we're matching */
    found = strstr (line_text, *lines);
  else
    {
      /* If it's not the first line, we have to match from the
       * start of the line.
       */
      if (strncmp (line_text, *lines, strlen (*lines)) == 0)
        found = line_text;
      else
        found = NULL;
    }

  if (found == NULL)
    {
      g_free (line_text);
      return false;
    }

  /* Get offset to start of search string */
  offset = g_utf8_strlen (line_text, found - line_text);

  next = *this;

  /* If match start needs to be returned, set it to the
   * start of the search string.
   */
  if (match_start)
    {
      *match_start = next;

      match_start->forward_chars_with_skipping (offset,
                                   visible_only, !slice);
    }

  /* Go to end of search string */
  offset += g_utf8_strlen (*lines, -1);

  next.forward_chars_with_skipping (offset,
                               visible_only, !slice);

  g_free (line_text);

  ++lines;

  if (match_end)
    *match_end = next;

  /* pass NULL for match_start, since we don't need to find the
   * start again.
   */
  return next.lines_match (lines, visible_only, slice, NULL, match_end);
}

/* strsplit () that retains the delimiter as part of the string. */
static gchar **
strbreakup (const char *string,
            const char *delimiter,
            gint        max_tokens)
{
  GSList *string_list = NULL, *slist;
  gchar **str_array, *s;
  guint i, n = 1;

  g_return_val_if_fail (string != NULL, NULL);
  g_return_val_if_fail (delimiter != NULL, NULL);

  if (max_tokens < 1)
    max_tokens = G_MAXINT;

  s = strstr (string, delimiter);
  if (s)
    {
      guint delimiter_len = strlen (delimiter);

      do
        {
          guint len;
          gchar *new_string;

          len = s - string + delimiter_len;
          new_string = g_new (gchar, len + 1);
          strncpy (new_string, string, len);
          new_string[len] = 0;
          string_list = g_slist_prepend (string_list, new_string);
          n++;
          string = s + delimiter_len;
          s = strstr (string, delimiter);
        }
      while (--max_tokens && s);
    }
  if (*string)
    {
      n++;
      string_list = g_slist_prepend (string_list, g_strdup (string));
    }

  str_array = g_new (gchar*, n);

  i = n - 1;

  str_array[i--] = NULL;
  for (slist = string_list; slist; slist = slist->next)
    str_array[i--] = (gchar*)slist->data; //TODO change datatype of slist->data

  g_slist_free (string_list);

  return str_array;
}

/**
 * gtk_text_iter_forward_search:
 * @iter: start of search
 * @str: a search string
 * @flags: flags affecting how the search is done
 * @match_start: return location for start of match, or %NULL
 * @match_end: return location for end of match, or %NULL
 * @limit: bound for the search, or %NULL for the end of the buffer
 * 
 * Searches forward for @str. Any match is returned by setting 
 * @match_start to the first character of the match and @match_end to the 
 * first character after the match. The search will not continue past
 * @limit. Note that a search is a linear or O(n) operation, so you
 * may wish to use @limit to avoid locking up your UI on large
 * buffers.
 * 
 * If the #GTK_TEXT_SEARCH_VISIBLE_ONLY flag is present, the match may
 * have invisible text interspersed in @str. i.e. @str will be a
 * possibly-noncontiguous subsequence of the matched range. similarly,
 * if you specify #GTK_TEXT_SEARCH_TEXT_ONLY, the match may have
 * pixbufs or child widgets mixed inside the matched range. If these
 * flags are not given, the match must be exact; the special 0xFFFC
 * character in @str will match embedded pixbufs or child widgets.
 *
 * Return value: whether a match was found
 **/
bool TextIter::forward_search (
                              const gchar       *str,
                              TextSearchFlags flags,
                              TextIter       *match_start,
                              TextIter       *match_end,
                              TextIter *limit)
{
  gchar **lines = NULL;
  TextIter match;
  bool retval = false;
  TextIter search;
  bool visible_only;
  bool slice;
  
//  g_return_val_if_fail (iter != NULL, false);
  g_return_val_if_fail (str != NULL, false);

  if (limit &&
      TextIter::compare (this, limit) >= 0)
    return false;
  
  if (*str == '\0')
    {
      /* If we can move one char, return the empty string there */
      match = *this;
      
      if (match.forward_char ())
        {
          if (limit &&
              TextIter::equal (&match, limit))
            return false;
          
          if (match_start)
            *match_start = match;
          if (match_end)
            *match_end = match;
          return true;
        }
      else
        return false;
    }

  visible_only = (flags & GTK_TEXT_SEARCH_VISIBLE_ONLY) != 0;
  slice = (flags & GTK_TEXT_SEARCH_TEXT_ONLY) == 0;
  
  /* locate all lines */

  lines = strbreakup (str, "\n", -1);

  search = *this;

  do
    {
      /* This loop has an inefficient worst-case, where
       * TextIter::get_text () is called repeatedly on
       * a single line.
       */
      TextIter end;

      if (limit &&
          TextIter::compare (&search, limit) >= 0)
        break;
      
      if (search.lines_match ((const gchar**)lines,
                       visible_only, slice, &match, &end))
        {
          if (limit == NULL ||
              (limit &&
               TextIter::compare (&end, limit) <= 0))
            {
              retval = true;
              
              if (match_start)
                *match_start = match;
              
              if (match_end)
                *match_end = end;
            }
          
          break;
        }
    }
  while (search.forward_line ());

  g_strfreev ((gchar**)lines);

  return retval;
}

bool TextIter::vectors_equal_ignoring_trailing (gchar **vec1, gchar **vec2)
{
  /* Ignores trailing chars in vec2's last line */

  gchar **i1, **i2;

  i1 = vec1;
  i2 = vec2;

  while (*i1 && *i2)
    {
      if (strcmp (*i1, *i2) != 0)
        {
          if (*(i2 + 1) == NULL) /* if this is the last line */
            {
              gint len1 = strlen (*i1);
              gint len2 = strlen (*i2);

              if (len2 >= len1 &&
                  strncmp (*i1, *i2, len1) == 0)
                {
                  /* We matched ignoring the trailing stuff in vec2 */
                  return true;
                }
              else
                {
                  return false;
                }
            }
          else
            {
              return false;
            }
        }
      ++i1;
      ++i2;
    }

  if (*i1 || *i2)
    {
      return false;
    }
  else
    return true;
}

class LinesWindow
{
	public:
	void init ( TextIter *start);
	bool back (void);
	void free (void);

  gint n_lines;
  gchar **lines;
  TextIter first_line_start;
  TextIter first_line_end;
  bool slice;
  bool visible_only;

	protected:
	private:
};

void LinesWindow::init ( TextIter *start)
{
  gint i;
  TextIter line_start;
  TextIter line_end;

  /* If we start on line 1, there are 2 lines to search (0 and 1), so
   * n_lines can be 2.
   */
  if (start->is_start () ||
      start->get_line () + 1 < n_lines)
    {
      /* Already at the end, or not enough lines to match */
      lines = g_new0 (gchar*, 1);
      *lines = NULL;
      return;
    }

  line_start = *start;
  line_end = *start;

  /* Move to start iter to start of line */
  line_start.set_line_offset (0);

  if (TextIter::equal (&line_start, &line_end))
    {
      /* we were already at the start; so go back one line */
      line_start.backward_line ();
    }

  first_line_start = line_start;
  first_line_end = line_end;

  lines = g_new0 (gchar*, n_lines + 1);

  i = n_lines - 1;
  while (i >= 0)
    {
      gchar *line_text;

      if (slice)
        {
          if (visible_only)
            line_text = TextIter::get_visible_slice (&line_start, &line_end);
          else
            line_text = TextIter::get_slice (&line_start, &line_end);
        }
      else
        {
          if (visible_only)
            line_text = TextIter::get_visible_text (&line_start, &line_end);
          else
            line_text = TextIter::get_text (&line_start, &line_end);
        }

      lines[i] = line_text;

      line_end = line_start;
      line_start.backward_line ();

      --i;
    }
}

bool LinesWindow::back (void)
{
  TextIter new_start;
  gchar *line_text;

  new_start = first_line_start;

  if (!new_start.backward_line ())
    return false;
  else
    {
      first_line_start = new_start;
      first_line_end = new_start;

      first_line_end.forward_line ();
    }

  if (slice)
    {
      if (visible_only)
        line_text = TextIter::get_visible_slice (&first_line_start,
                                                     &first_line_end);
      else
        line_text = TextIter::get_slice (&first_line_start,
                                             &first_line_end);
    }
  else
    {
      if (visible_only)
        line_text = TextIter::get_visible_text (&first_line_start,
                                                    &first_line_end);
      else
        line_text = TextIter::get_text (&first_line_start,
                                            &first_line_end);
    }

  /* Move lines to make room for first line. */
  g_memmove (lines + 1, lines, n_lines * sizeof (gchar*));

  *lines = line_text;

  /* Free old last line and NULL-terminate */
  g_free (lines[n_lines]);
  lines[n_lines] = NULL;

  return true;
}

void LinesWindow::free (void)
{
  g_strfreev (lines);
}

/**
 * gtk_text_iter_backward_search:
 * @iter: a #TextIter where the search begins
 * @str: search string
 * @flags: bitmask of flags affecting the search
 * @match_start: return location for start of match, or %NULL
 * @match_end: return location for end of match, or %NULL
 * @limit: location of last possible @match_start, or %NULL for start of buffer
 * 
 * Same as gtk_text_iter_forward_search(), but moves backward.
 * 
 * Return value: whether a match was found
 **/
bool TextIter::backward_search (
                               const gchar       *str,
                               TextSearchFlags flags,
                               TextIter       *match_start,
                               TextIter       *match_end,
                               TextIter *limit)
{
  gchar **lines = NULL;
  gchar **l;
  gint n_lines;
  LinesWindow win;
  bool retval = false;
  bool visible_only;
  bool slice;
  
//  g_return_val_if_fail (iter != NULL, false);
  g_return_val_if_fail (str != NULL, false);

  if (limit &&
      TextIter::compare (limit, this) > 0)
    return false;
  
  if (*str == '\0')
    {
      /* If we can move one char, return the empty string there */
      TextIter match = *this;

      if (limit && TextIter::equal (limit, &match))
        return false;
      
      if (match.backward_char ())
        {
          if (match_start)
            *match_start = match;
          if (match_end)
            *match_end = match;
          return true;
        }
      else
        return false;
    }

  visible_only = (flags & GTK_TEXT_SEARCH_VISIBLE_ONLY) != 0;
  slice = (flags & GTK_TEXT_SEARCH_TEXT_ONLY) == 0;
  
  /* locate all lines */

  lines = strbreakup (str, "\n", -1);

  l = lines;
  n_lines = 0;
  while (*l)
    {
      ++n_lines;
      ++l;
    }

  win.n_lines = n_lines;
  win.slice = slice;
  win.visible_only = visible_only;

  win.init (this);

  if (*win.lines == NULL)
    goto out;

  do
    {
      gchar *first_line_match;

      if (limit &&
          TextIter::compare (limit, &win.first_line_end) > 0)
        {
          /* We're now before the search limit, abort. */
          goto out;
        }
      
      /* If there are multiple lines, the first line will
       * end in '\n', so this will only match at the
       * end of the first line, which is correct.
       */
      first_line_match = g_strrstr (*win.lines, *lines);

      if (first_line_match &&
          vectors_equal_ignoring_trailing (lines + 1, win.lines + 1))
        {
          /* Match! */
          gint offset;
          TextIter next;
          TextIter start_tmp;
          
          /* Offset to start of search string */
          offset = g_utf8_strlen (*win.lines, first_line_match - *win.lines);

          next = win.first_line_start;
          start_tmp = next;
          start_tmp.forward_chars_with_skipping (offset,
                                       visible_only, !slice);

          if (limit &&
              TextIter::compare (limit, &start_tmp) > 0)
            goto out; /* match was bogus */
          
          if (match_start)
            *match_start = start_tmp;

          /* Go to end of search string */
          l = lines;
          while (*l)
            {
              offset += g_utf8_strlen (*l, -1);
              ++l;
            }

          next.forward_chars_with_skipping (offset,
                                       visible_only, !slice);

          if (match_end)
            *match_end = next;

          retval = true;
          goto out;
        }
    }
  while (win.back ());

 out:
  win.free ();
  g_strfreev (lines);
  
  return retval;
}

/*
 * Comparisons
 */

/**
 * TextIter::equal:
 * @lhs: a #TextIter
 * @rhs: another #TextIter
 * 
 * Tests whether two iterators are equal, using the fastest possible
 * mechanism. This function is very fast; you can expect it to perform
 * better than e.g. getting the character offset for each iterator and
 * comparing the offsets yourself. Also, it's a bit faster than
 * TextIter::compare().
 * 
 * Return value: %true if the iterators point to the same place in the buffer
 **/
bool
TextIter::equal (TextIter *lhs,
                     TextIter *rhs)
{
//  TextRealIter *real_lhs;
//  TextRealIter *real_rhs;

//  real_lhs = (TextRealIter*)lhs;
//  real_rhs = (TextRealIter*)rhs;

  lhs->check_invariants ();
  rhs->check_invariants ();

  if (lhs->line != rhs->line)
    return false;
  else if (lhs->line_byte_offset >= 0 &&
           rhs->line_byte_offset >= 0)
    return lhs->line_byte_offset == rhs->line_byte_offset;
  else
    {
      /* the ensure_char_offsets () calls do nothing if the char offsets
         are already up-to-date. */
      lhs->ensure_char_offsets ();
      rhs->ensure_char_offsets ();
      return lhs->line_char_offset == rhs->line_char_offset;
    }
}

/**
 * TextIter::compare:
 * @lhs: a #TextIter
 * @rhs: another #TextIter
 * 
 * A qsort()-style function that returns negative if @lhs is less than
 * @rhs, positive if @lhs is greater than @rhs, and 0 if they're equal.
 * Ordering is in character offset order, i.e. the first character in the buffer
 * is less than the second character in the buffer.
 * 
 * Return value: -1 if @lhs is less than @rhs, 1 if @lhs is greater, 0 if they are equal
 **/
gint TextIter::compare (TextIter *lhs,
                       TextIter *rhs)
{
//  TextRealIter *real_lhs;
//  TextRealIter *real_rhs;

//  real_lhs = gtk_text_iter_make_surreal (lhs);
//  real_rhs = gtk_text_iter_make_surreal (rhs);
    if (!lhs->make_surreal() || !rhs->make_surreal())
	    return -1;

//  if (real_lhs == NULL ||
//      real_rhs == NULL)
//    return -1; /* why not */

  lhs->check_invariants ();
  rhs->check_invariants ();
  
  if (lhs->line == rhs->line)
    {
      gint left_index, right_index;

      if (lhs->line_byte_offset >= 0 &&
          rhs->line_byte_offset >= 0)
        {
          left_index = lhs->line_byte_offset;
          right_index = rhs->line_byte_offset;
        }
      else
        {
          /* the ensure_char_offsets () calls do nothing if
             the offsets are already up-to-date. */
          lhs->ensure_char_offsets ();
          rhs->ensure_char_offsets ();
          left_index = lhs->line_char_offset;
          right_index = rhs->line_char_offset;
        }

      if (left_index < right_index)
        return -1;
      else if (left_index > right_index)
        return 1;
      else
        return 0;
    }
  else
    {
      gint line1, line2;

      line1 = lhs->get_line ();
      line2 = rhs->get_line ();
      if (line1 < line2)
        return -1;
      else if (line1 > line2)
        return 1;
      else
        return 0;
    }
}

/**
 * gtk_text_iter_in_range:
 * @iter: a #TextIter
 * @start: start of range
 * @end: end of range
 * 
 * Checks whether @iter falls in the range [@start, @end).
 * @start and @end must be in ascending order.
 * 
 * Return value: %true if @iter is in the range
 **/
bool TextIter::in_range (
                        TextIter *start,
                        TextIter *end)
{
//  g_return_val_if_fail (iter != NULL, false);
  g_return_val_if_fail (start != NULL, false);
  g_return_val_if_fail (end != NULL, false);
  g_return_val_if_fail (TextIter::compare (start, end) <= 0, false);
  
  return TextIter::compare (this, start) >= 0 &&
    TextIter::compare (this, end) < 0;
}

/**
 * gtk_text_iter_order:
 * @first: a #TextIter
 * @second: another #TextIter
 *
 * Swaps the value of @first and @second if @second comes before
 * @first in the buffer. That is, ensures that @first and @second are
 * in sequence. Most text buffer functions that take a range call this
 * automatically on your behalf, so there's no real reason to call it yourself
 * in those cases. There are some exceptions, such as gtk_text_iter_in_range(),
 * that expect a pre-sorted range.
 * 
 **/
void TextIter::order (TextIter *first,
                     TextIter *second)
{
  g_return_if_fail (first != NULL);
  g_return_if_fail (second != NULL);

  if (TextIter::compare (first, second) > 0)
    {
      TextIter tmp;

      tmp = *first;
      *first = *second;
      *second = tmp;
    }
}



void TextIter::check (void)
{
//  const TextRealIter *real = (const TextRealIter*)iter;
  //gint line_char_offset, line_byte_offset;
  gint  seg_byte_offset, seg_char_offset;;
  TextLineSegment *byte_segment = NULL;
  TextLineSegment *byte_any_segment = NULL;
  TextLineSegment *char_segment = NULL;
  TextLineSegment *char_any_segment = NULL;
  bool segments_updated;

  /* This function checks our class invariants for the Iter class. */

//  g_assert (sizeof (TextIter) == sizeof (TextRealIter));

  if (chars_changed_stamp !=
      tree->get_chars_changed_stamp ())
    g_error ("iterator check failed: invalid iterator");

  if (line_char_offset < 0 && line_byte_offset < 0)
    g_error ("iterator check failed: both char and byte offsets are invalid");

  segments_updated = (segments_changed_stamp ==
                      tree->get_segments_changed_stamp ());

#if 0
  printf ("checking iter, segments %s updated, byte %d char %d\n",
          segments_updated ? "are" : "aren't",
          line_byte_offset,
          line_char_offset);
#endif

  if (segments_updated)
    {
      if (segment_char_offset < 0 && segment_byte_offset < 0)
        g_error ("iterator check failed: both char and byte segment offsets are invalid");

      if (segment->char_count == 0)
        g_error ("iterator check failed: segment is not indexable.");

      if (line_char_offset >= 0 && segment_char_offset < 0)
        g_error ("segment char offset is not properly up-to-date");

      if (line_byte_offset >= 0 && segment_byte_offset < 0)
        g_error ("segment byte offset is not properly up-to-date");

      if (segment_byte_offset >= 0 &&
          segment_byte_offset >= segment->byte_count)
        g_error ("segment byte offset is too large.");

      if (segment_char_offset >= 0 &&
          segment_char_offset >= segment->char_count)
        g_error ("segment char offset is too large.");
    }

  if (line_byte_offset >= 0)
    {
      line->byte_locate (line_byte_offset,
                                  &byte_segment, &byte_any_segment,
                                  &seg_byte_offset, &line_byte_offset);

      if (line_byte_offset != line_byte_offset)
        g_error ("wrong byte offset was stored in iterator");

      if (segments_updated)
        {
          if (segment != byte_segment)
            g_error ("wrong segment was stored in iterator");

          if (any_segment != byte_any_segment)
            g_error ("wrong any_segment was stored in iterator");

          if (seg_byte_offset != segment_byte_offset)
            g_error ("wrong segment byte offset was stored in iterator");

          if (byte_segment->type == text_segment_char)
            {
              const gchar *p;
              p = byte_segment->body.chars + seg_byte_offset;
              
              if (!text_byte_begins_utf8_char (p))
                g_error ("broken iterator byte index pointed into the middle of a character");
            }
        }
    }

  if (line_char_offset >= 0)
    {
      line->char_locate (line_char_offset,
                                  &char_segment, &char_any_segment,
                                  &seg_char_offset, &line_char_offset);

      if (line_char_offset != line_char_offset)
        g_error ("wrong char offset was stored in iterator");

      if (segments_updated)
        {          
          if (segment != char_segment)
            g_error ("wrong segment was stored in iterator");

          if (any_segment != char_any_segment)
            g_error ("wrong any_segment was stored in iterator");

          if (seg_char_offset != segment_char_offset)
            g_error ("wrong segment char offset was stored in iterator");

          if (char_segment->type == text_segment_char)
            {
              const gchar *p;
              p = g_utf8_offset_to_pointer (char_segment->body.chars,
                                            seg_char_offset);

              /* hmm, not likely to happen eh */
              if (!text_byte_begins_utf8_char (p))
                g_error ("broken iterator char offset pointed into the middle of a character");
            }
        }
    }

  if (line_char_offset >= 0 && line_byte_offset >= 0)
    {
      if (byte_segment != char_segment)
        g_error ("char and byte offsets did not point to the same segment");

      if (byte_any_segment != char_any_segment)
        g_error ("char and byte offsets did not point to the same any segment");

      /* Make sure the segment offsets are equivalent, if it's a char
         segment. */
      if (char_segment->type == text_segment_char)
        {
          gint byte_offset = 0;
          gint char_offset = 0;
          while (char_offset < seg_char_offset)
            {
              const char * start = char_segment->body.chars + byte_offset;
              byte_offset += g_utf8_next_char (start) - start;
              char_offset += 1;
            }

          if (byte_offset != seg_byte_offset)
            g_error ("byte offset did not correspond to char offset");

          char_offset =
            g_utf8_strlen (char_segment->body.chars, seg_byte_offset);

          if (char_offset != seg_char_offset)
            g_error ("char offset did not correspond to byte offset");

          if (!text_byte_begins_utf8_char (char_segment->body.chars + seg_byte_offset))
            g_error ("byte index for iterator does not index the start of a character");
        }
    }

  if (cached_line_number >= 0)
    {
      gint should_be;

      should_be = line->get_number ();
      if (cached_line_number != should_be)
        g_error ("wrong line number was cached");
    }

  if (cached_char_index >= 0)
    {
      if (line_char_offset >= 0) /* only way we can check it
                                          efficiently, not a real
                                          invariant. */
        {
          gint char_index;

          char_index = line->char_index ();
          char_index += line_char_offset;

          if (cached_char_index != char_index)
            g_error ("wrong char index was cached");
        }
    }

  if (tree->line_is_last (line))
    g_error ("Iterator was on last line (past the end iterator)");
}
