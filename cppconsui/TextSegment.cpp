/*
 * gtktextsegment.c --
 *
 * Code for segments in general, and toggle/char segments in particular.
 *
 * Copyright (c) 1992-1994 The Regents of the University of California.
 * Copyright (c) 1994-1995 Sun Microsystems, Inc.
 * Copyright (c) 2000      Red Hat, Inc.
 * Tk -> Gtk port by Havoc Pennington <hp@redhat.com>
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

//#define GTK_TEXT_USE_INTERNAL_UNSUPPORTED_API
//#include "config.h"
#include "TextSegment.h"
#include "TextBTree.h"
#include <string.h>
//#include <stdlib.h>
//#include <stdio.h>
#include "TextTag.h"
#include "TextTagTable.h"
#include "TextLayout.h"
#include "TextTypes.h"
//#include "gtktextiterprivate.h"
//#include "gtkdebug.h"
//#include "gtkalias.h"

/* TextLineSegment functions */

/* Constructors */


TextLineSegment::TextLineSegment(void)
: type(text_segment_none)
, next(NULL)
, char_count(0)
, byte_count(0)
, name(NULL)
, leftGravity(false)
{
}
/*
TextLineSegment::TextLineSegment(const gchar *text, guint len)
: type(text_segment_none), next(NULL), char_count(0), byte_count(0), name(NULL), leftGravity(false)
{
}

TextLineSegment::TextLineSegment(const gchar *text1, guint len1, guint chars1,
			const gchar *text2, guint len2, guint chars2)
: type(text_segment_none), next(NULL), char_count(0), byte_count(0), name(NULL), leftGravity(false)
{
}

TextLineSegment::TextLineSegment(TextTagInfo *info, bool on)
: type(text_segment_none), next(NULL), char_count(0), byte_count(0), name(NULL), leftGravity(false)
{
}
*/

void TextLineSegment::set_tree ( TextBTree *tree)
{
  g_assert (body.mark.tree == NULL);
  g_assert (body.mark.obj != NULL);

  byte_count = 0;
  char_count = 0;

  body.mark.tree = tree;
  body.mark.line = NULL;
  next = NULL;

  body.mark.not_deleteable = false;
}

TextLineSegment* TextLineSegment::split_segment (TextIter *iter)
{
  TextLineSegment *prev, *seg;
  TextBTree *tree;
  TextLine *line;
  int count;

  line = iter->get_text_line ();
  tree = iter->get_btree ();

  count = iter->get_line_index ();

  if (true)//TODO(gtk_debug_flags & GTK_DEBUG_TEXT)
    iter->check();
  
  prev = NULL;
  seg = line->segments;

  while (seg != NULL)
    {
      if (seg->byte_count > count)
        {
          if (count == 0)
            {
              return prev;
            }
          else
            {
              g_assert (count != seg->byte_count);
              g_assert (seg->byte_count > 0);

              tree->segments_changed ();

              seg = seg->split (count);

              if (prev == NULL)
                line->segments = seg;
              else
                prev->next = seg;

              return seg;
            }
        }
      else if ((seg->byte_count == 0) && (count == 0)
               && !seg->leftGravity)
        {
          return prev;
        }

      count -= seg->byte_count;
      prev = seg;
      seg = seg->next;
    }
  g_error ("split_segment reached end of line!");
  return NULL;
}

TextLineSegment* TextLineSegment::split (int index)
{
	//TODO nothing to do here? perhaps make the class abstract?
	return NULL;
}


bool TextLineSegment::deleteFunc (TextLine *line, bool tree_gone)
{
	//TODO nothing to do here? perhaps make the class abstract?
}


TextLineSegment* TextLineSegment::cleanupFunc (TextLineSegment *segPtr, TextLine *line)
{
	/* if nothing has to be done, just return segPtr */
	return segPtr;
}

void TextLineSegment::lineChangeFunc (TextLine *line)
{
	//TODO nothing to do here? perhaps make the class abstract?
}

void TextLineSegment::checkFunc (TextLine *line)
{
	//TODO nothing to do here? perhaps make the class abstract?
}

void TextLineSegment::self_check (void)
{
}

void TextLineSegmentChar::self_check (void)
{
  /* This function checks the segment itself, but doesn't
     assume the segment has been validly inserted into
     the btree. */

//  g_assert (seg != NULL);

  if (byte_count <= 0)
    {
      g_error ("segment has size <= 0");
    }

  if (strlen (body.chars) != byte_count)
    {
      g_error ("segment has wrong size");
    }

  if (g_utf8_strlen (body.chars, byte_count) != char_count)
    {
      g_error ("char segment has wrong character count");
    }
}

TextLineSegmentChar::TextLineSegmentChar (const gchar *text, guint len)
{
//  GtkTextLineSegment *seg;

  g_assert (text_byte_begins_utf8_char (text));

  //TODO implement
//  seg = g_malloc (CSEG_SIZE (len));
  body.chars = (char*)g_malloc(len);

  type = text_segment_char;
  next = NULL;
  byte_count = len;
  memcpy (body.chars, text, len);
  body.chars[len] = '\0';

  char_count = g_utf8_strlen (body.chars, byte_count);

  /*TODO if (gtk_debug_flags & GTK_DEBUG_TEXT)
    char_segment_self_check (seg);*/

//  return seg;
}

TextLineSegmentChar::TextLineSegmentChar (const gchar *text1, 
					guint        len1, 
					guint        chars1,
                                        const gchar *text2, 
					guint        len2, 
					guint        chars2)
{
//  GtkTextLineSegment *seg;

  g_assert (text_byte_begins_utf8_char (text1));
  g_assert (text_byte_begins_utf8_char (text2));

//  seg = g_malloc (CSEG_SIZE (len1+len2));
  body.chars = (char*)g_malloc(len1 + len2);

  type = text_segment_char;
  next = NULL;
  byte_count = len1 + len2;
  memcpy (body.chars, text1, len1);
  memcpy (body.chars + len1, text2, len2);
  body.chars[len1+len2] = '\0';

  char_count = chars1 + chars2;

  if (true) //TODO (gtk_debug_flags & GTK_DEBUG_TEXT)
    self_check();

//  return seg;
}

TextLineSegment* TextLineSegmentChar::split (gint index)
{
  TextLineSegmentChar *new1, *new2;

  g_assert (index < byte_count);

  if (true)//TODO(gtk_debug_flags & GTK_DEBUG_TEXT)
      self_check ();

  new1 = new TextLineSegmentChar (body.chars, index);
  new2 = new TextLineSegmentChar (body.chars + index, byte_count - index);

  g_assert (text_byte_begins_utf8_char (new1->body.chars));
  g_assert (text_byte_begins_utf8_char (new2->body.chars));
  g_assert (new1->byte_count + new2->byte_count == byte_count);
  g_assert (new1->char_count + new2->char_count == char_count);

  new1->next = new2;
  new2->next = next;

  if (true)//TODO(gtk_debug_flags & GTK_DEBUG_TEXT)
    {
      new1->self_check ();
      new2->self_check ();
    }

  //TODO implement delete seg;
  return new1;
}

TextLineSegment* TextLineSegmentChar::cleanupFunc (TextLineSegmentChar *segPtr, TextLine *line)
{
  TextLineSegment *segPtr2;
  TextLineSegmentChar *newPtr;

  if (true)//TODO(gtk_debug_flags & GTK_DEBUG_TEXT)
    segPtr->self_check ();

  segPtr2 = segPtr->next;
  if ((segPtr2 == NULL) || (segPtr2->type != text_segment_char))
    {
      return segPtr;
    }

  newPtr =
    new TextLineSegmentChar (segPtr->body.chars, 
			    segPtr->byte_count,
			    segPtr->char_count,
			    segPtr2->body.chars, 
			    segPtr2->byte_count,
			    segPtr2->char_count);

  newPtr->next = segPtr2->next;

  if (true)//TODO(gtk_debug_flags & GTK_DEBUG_TEXT)
    newPtr->self_check ();

  delete segPtr;
  delete segPtr2;

  return newPtr;
}

bool TextLineSegmentChar::deleteFunc (TextLineSegmentChar *segPtr, TextLine *line, bool treeGone)
{
  delete segPtr;
  return 0;
}

void TextLineSegmentChar::checkFunc (TextLine *line)
{
  self_check ();

  if (next != NULL)
    {
      if (next->type == text_segment_char)
        {
          g_error ("adjacent character segments weren't merged");
        }
    }
}

//TODO use initializers
TextLineSegmentToggle::TextLineSegmentToggle (TextTagInfo *info, bool on)
{
//  GtkTextLineSegment *seg;

//  seg = g_malloc (TSEG_SIZE);

  type = on ? text_segment_toggle_on : text_segment_toggle_off;

  next = NULL;

  byte_count = 0;
  char_count = 0;

  //TODO i added this here, is that ok?
  body.chars = NULL;

  body.toggle.info = info;
  body.toggle.inNodeCounts = 0;

//  return seg;
}

bool TextLineSegmentToggle::deleteFunc (TextLineSegmentToggle *segPtr, TextLine *line, bool treeGone)
{
  if (treeGone)
    {
      delete segPtr;
      return 0;
    }

  /*
   * This toggle is in the middle of a range of characters that's
   * being deleted.  Refuse to die.  We'll be moved to the end of
   * the deleted range and our cleanup procedure will be called
   * later.  Decrement GtkTextBTreeNode toggle counts here, and set a flag
   * so we'll re-increment them in the cleanup procedure.
   */

  if (segPtr->body.toggle.inNodeCounts)
    {
      line->parent->change_node_toggle_count (
                                     segPtr->body.toggle.info, -1);
      segPtr->body.toggle.inNodeCounts = 0;
    }
  return 1;
}

void TextLineSegmentToggle::checkFunc ( TextLine *line)
{
  Summary *summary;
  int needSummary;

  if (byte_count != 0)
    {
      g_error ("toggle_segment_check_func: segment had non-zero size");
    }
  if (!body.toggle.inNodeCounts)
    {
      g_error ("toggle_segment_check_func: toggle counts not updated in TextBTreeNodes");
    }
  needSummary = (body.toggle.info->tag_root != line->parent);
  for (summary = line->parent->summary; ;
       summary = summary->next)
    {
      if (summary == NULL)
        {
          if (needSummary)
            {
              g_error ("toggle_segment_check_func: tag not present in TextBTreeNode");
            }
          else
            {
              break;
            }
        }
      if (summary->info == body.toggle.info)
        {
          if (!needSummary)
            {
              g_error ("toggle_segment_check_func: tag present in root TextBTreeNode summary");
            }
          break;
        }
    }
}

TextLineSegment* TextLineSegmentToggle::cleanupFunc (TextLineSegmentToggle *segPtr, TextLine *line)
{
  TextLineSegment *segPtr2, *prevPtr;
  int counts;

  /*
   * If this is a toggle-off segment, look ahead through the next
   * segments to see if there's a toggle-on segment for the same tag
   * before any segments with non-zero size.  If so then the two
   * toggles cancel each other;  remove them both.
   */

  if (segPtr->type == text_segment_toggle_off)
    {
      for (prevPtr = segPtr, segPtr2 = prevPtr->next;
           (segPtr2 != NULL) && (segPtr2->byte_count == 0);
           prevPtr = segPtr2, segPtr2 = prevPtr->next)
        {
          if (segPtr2->type != text_segment_toggle_on)
            {
              continue;
            }
          if (segPtr2->body.toggle.info != segPtr->body.toggle.info)
            {
              continue;
            }
          counts = segPtr->body.toggle.inNodeCounts
            + segPtr2->body.toggle.inNodeCounts;
          if (counts != 0)
            {
              line->parent->change_node_toggle_count (
                                             segPtr->body.toggle.info, -counts);
            }
          prevPtr->next = segPtr2->next;
          delete segPtr2;
          segPtr2 = segPtr->next;
          delete segPtr;
          return segPtr2;
        }
    }

  if (!segPtr->body.toggle.inNodeCounts)
    {
      line->parent->change_node_toggle_count (
                                     segPtr->body.toggle.info, 1);
      segPtr->body.toggle.inNodeCounts = 1;
    }
  return segPtr;
}

void TextLineSegmentToggle::lineChangeFunc (TextLine *line)
{
  if (body.toggle.inNodeCounts)
    {
      line->parent->change_node_toggle_count (
                                     body.toggle.info, -1);
      body.toggle.inNodeCounts = 0;
    }
}

TextLineSegmentLeftMark::TextLineSegmentLeftMark (TextMark *mark_obj)
{
	body.mark.name = NULL;
	type = text_segment_left_mark;

	byte_count = 0;
	char_count = 0;

	body.mark.obj = mark_obj;
	mark_obj->segment = this;

	body.mark.tree = NULL;
	body.mark.line = NULL;
	next = NULL;

	body.mark.visible = false;
	body.mark.not_deleteable = false;
}

/*
 *--------------------------------------------------------------
 *
 * mark_segment_delete_func --
 *
 *      This procedure is invoked by the text B-tree code whenever
 *      a mark lies in a range of characters being deleted.
 *
 * Results:
 *      Returns 1 to indicate that deletion has been rejected,
 *      or 0 otherwise
 *
 * Side effects:
 *      Frees mark if tree is going away
 *
 *--------------------------------------------------------------
 */

bool TextLineSegmentLeftMark::deleteFunc (TextLineSegmentLeftMark *segPtr,
                          TextLine        *line,
                          bool            tree_gone)
{
  if (tree_gone)
    {
      segPtr->body.mark.tree->release_mark_segment (segPtr);
      return false;
    }
  else
    return true;
}

/*
 *--------------------------------------------------------------
 *
 * mark_segment_cleanup_func --
 *
 *      This procedure is invoked by the B-tree code whenever a
 *      mark segment is moved from one line to another.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      The line field of the segment gets updated.
 *
 *--------------------------------------------------------------
 */

TextLineSegment* TextLineSegmentLeftMark::cleanupFunc (TextLineSegmentLeftMark *segPtr, 
                           TextLine        *line)
{
  /* not sure why Tk did this here and not in LineChangeFunc */
  segPtr->body.mark.line = line;
  return segPtr;
}

/*
 *--------------------------------------------------------------
 *
 * mark_segment_check_func --
 *
 *      This procedure is invoked by the B-tree code to perform
 *      consistency checks on mark segments.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      The procedure panics if it detects anything wrong with
 *      the mark.
 *
 *--------------------------------------------------------------
 */

void TextLineSegmentLeftMark::checkFunc (
                         TextLine        *line)
{
  if (body.mark.line != line)
    g_error ("mark_segment_check_func: seg->body.mark.line bogus");
}
