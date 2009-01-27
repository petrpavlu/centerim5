/*
 * Gtktextbtree.c --
 *
 *      This file contains code that manages the B-tree representation
 *      of text for the text buffer and implements character and
 *      toggle segment types.
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

#define GTK_TEXT_USE_INTERNAL_UNSUPPORTED_API
//#include "config.h"
#include "TextBTree.h"
#include <string.h>
//#include <stdlib.h>
//#include <stdio.h>
#include "TextTag.h"
#include "TextTagTable.h"
#include "TextSegment.h"
#include "TextTypes.h"
//#include "gtktextlayout.h"
//#include "gtktextiterprivate.h"
//#include "gtkdebug.h"
//#include "gtktextmarkprivate.h"
//#include "gtkalias.h"


#if 1
#define MAX_CHILDREN 12
#define MIN_CHILDREN 6
#else
#define MAX_CHILDREN 6
#define MIN_CHILDREN 3
#endif

class TextLineSegment;

/*
 * Prototypes
 */

/* Inline thingies */

inline void TextBTree::chars_changed (void)
{
  chars_changed_stamp += 1;
}


/*
 * BTree operations
 */

TextBTree::TextBTree(TextTagTable *table,
                     TextBuffer *buffer)
: root_node(NULL)
, table(table)
, mark_table(NULL)
, refcount(0)
, insert_mark(NULL)
, selection_bound_mark(NULL)
, buffer(buffer)
, views(NULL)
, tag_infos(NULL)
, tag_changed_handler(0)
, chars_changed_stamp(0)
, segments_changed_stamp(0)
, last_line(NULL)
, last_line_stamp(0)
, end_iter_segment(NULL)
, end_iter_segment_byte_index(0)
, end_iter_segment_char_offset(0)
, end_iter_line_stamp(0)
, end_iter_segment_stamp(0)
, child_anchor_table(NULL)
{
  //TextBTreeNode *root_node;
  TextLine *line, *line2;

  //TODOg_return_val_if_fail (GTK_IS_TEXT_TAG_TABLE (table), NULL);
  //TODOg_return_val_if_fail (GTK_IS_TEXT_BUFFER (buffer), NULL);

  /*
   * The tree will initially have two empty lines.  The second line
   * isn't actually part of the tree's contents, but its presence
   * makes several operations easier.  The tree will have one TextBTreeNode,
   * which is also the root of the tree.
   */

  /* Create the root node. */

  root_node = new TextBTreeNode();

  line = new TextLine();
  line2 = new TextLine();

  root_node->parent = NULL;
  root_node->next = NULL;
  root_node->summary = NULL;
  root_node->level = 0;
  root_node->children.line = line;
  root_node->num_children = 2;
  root_node->num_lines = 2;
  root_node->num_chars = 2;

  line->parent = root_node;
  line->next = line2;

  line->segments = new TextLineSegmentChar ("\n", 1);

  line2->parent = root_node;
  line2->next = NULL;
  line2->segments = new TextLineSegmentChar ("\n", 1);

  //already done in initializer. this->table = table;
  views = NULL;

  /* Set these to values that are unlikely to be found
   * in random memory garbage, and also avoid
   * duplicates between tree instances.
   */
  chars_changed_stamp = g_random_int ();
  segments_changed_stamp = g_random_int ();

  last_line_stamp = chars_changed_stamp - 1;
  last_line = NULL;

  end_iter_line_stamp = chars_changed_stamp - 1;
  end_iter_segment_stamp = segments_changed_stamp - 1;
  end_iter_line = NULL;
  end_iter_segment_byte_index = 0;
  end_iter_segment_char_offset = 0;
  
  //TODOg_object_ref (tree->table);

  /*TODOtree->tag_changed_handler = g_signal_connect (tree->table,
						"tag-changed",
						G_CALLBACK (tag_changed_cb),
						tree);*/

  //mark_table = g_hash_table_new (g_str_hash, g_str_equal);
  child_anchor_table = NULL;
  
  /* We don't ref the buffer, since the buffer owns us;
   * we'd have some circularity issues. The buffer always
   * lasts longer than the BTree
   */
  this->buffer = buffer;

  {
    TextIter start;
    TextLineSegment *seg;

    get_iter_at_line_char (&start, 0, 0);


    insert_mark = set_mark ( NULL, "insert", false, &start, false);

    seg = insert_mark->segment;

    seg->body.mark.not_deleteable = true;
    seg->body.mark.visible = true;

    selection_bound_mark = set_mark ( NULL, "selection_bound", false, &start, false);

    seg = selection_bound_mark->segment;

    seg->body.mark.not_deleteable = true;

    //TODOg_object_ref (tree->insert_mark);
    //TODOg_object_ref (tree->selection_bound_mark);
  }

  refcount = 1;
}

void TextBTree::ref (void)
{
  //g_return_if_fail (tree != NULL);
  g_return_if_fail (refcount > 0);

  refcount += 1;
}

void TextBTree::unref (void)
{
 // g_return_if_fail (tree != NULL);
  g_return_if_fail (refcount > 0);

  refcount -= 1;

  if (refcount == 0)
    {      
      /*TODOg_signal_handler_disconnect (table,
                                   tag_changed_handler);*/

      //TODOg_object_unref (tree->table);
      table = NULL;
      
      TextBTreeNode::node_destroy (this, root_node);
      root_node = NULL;
      
      g_assert (g_hash_table_size (mark_table) == 0);
      g_hash_table_destroy (mark_table);
      mark_table = NULL;
      if (child_anchor_table != NULL) 
	{
	  g_hash_table_destroy (child_anchor_table);
	  child_anchor_table = NULL;
	}

      //TODOg_object_unref (tree->insert_mark);
      insert_mark = NULL;
      //TODOg_object_unref (tree->selection_bound_mark);
      selection_bound_mark = NULL;

      //TODO??g_free (tree);
    }
}

TextBuffer* TextBTree::get_buffer (void)
{
  return buffer;
}

guint TextBTree::get_chars_changed_stamp (void)
{
  return chars_changed_stamp;
}

guint
TextBTree::get_segments_changed_stamp (void)
{
  return segments_changed_stamp;
}

void
TextBTree::segments_changed (void)
{
  //g_return_if_fail (tree != NULL);
  segments_changed_stamp += 1;
}

/*
 * Indexable segment mutation
 */

/*
 *  The following function is responsible for resolving the bidi direction
 *  for the lines between start and end. But it also calculates any
 *  dependent bidi direction for surrounding lines that change as a result
 *  of the bidi direction decisions within the range. The function is
 *  trying to do as little propagation as is needed.
 */
void
TextBTree::resolve_bidi (TextIter *start,
			     TextIter *end)
{
  TextBTree *tree = start->get_btree();
  TextLine *start_line, *end_line, *start_line_prev, *end_line_next, *line;
  //PangoDirection last_strong, dir_above_propagated, dir_below_propagated;

  /* Resolve the strong bidi direction for all lines between
   * start and end.
  */
  start_line = start->get_text_line ();
  start_line_prev = start_line->previous_line ();
  end_line = end->get_text_line ();
  end_line_next = end_line->next_line ();
  
  line = start_line;
  while (line && line != end_line_next)
    {
      /* Loop through the segments and search for a strong character
       */
      TextLineSegment *seg = line->segments;
      //TODOline->dir_strong = PANGO_DIRECTION_NEUTRAL;
      
      while (seg)
        {
          /*TODO??if (seg->type == text_segment_char && seg->byte_count > 0)
            {
	      PangoDirection pango_dir;

              pango_dir = pango_find_base_dir (seg->body.chars,
					       seg->byte_count);
	      
              if (pango_dir != PANGO_DIRECTION_NEUTRAL)
                {
                  line->dir_strong = pango_dir;
                  break;
                }
            }*/
          seg = seg->next;
        }

      line = line->next_line();
    }

  /* Sweep forward */

  /* The variable dir_above_propagated contains the forward propagated
   * direction before start. It is neutral if start is in the beginning
   * of the buffer.
   */
  //TODOdir_above_propagated = PANGO_DIRECTION_NEUTRAL;
  if (start_line_prev)
  {}//TODOdir_above_propagated = start_line_prev->dir_propagated_forward;

  /* Loop forward and propagate the direction of each paragraph 
   * to all neutral lines.
   */
  line = start_line;
  //last_strong = dir_above_propagated;
  while (line != end_line_next)
    {
      //TODOif (line->dir_strong != PANGO_DIRECTION_NEUTRAL)
      //  last_strong = line->dir_strong;
      
      //line->dir_propagated_forward = last_strong;
      
      line = line->next_line();
    }

  /* Continue propagating as long as the previous resolved forward
   * is different from last_strong.
   */
  {
    TextIter end_propagate;
    
    /*TODO
    while (line &&
	   line->dir_strong == PANGO_DIRECTION_NEUTRAL &&
	   line->dir_propagated_forward != last_strong)
      {
        TextLine *prev = line;
        line->dir_propagated_forward = last_strong;
        
        line = _text_line_next(line);
        if (!line)
          {
            line = prev;
            break;
          }
      }*/

    /* The last line to invalidate is the last line before the
     * line with the strong character. Or in case of the end of the
     * buffer, the last line of the buffer. (There seems to be an
     * extra "virtual" last line in the buffer that must not be used
     * calling TextBTree::get_iter_at_line (causes crash). Thus the
     * _text_line_previous is ok in that case as well.)
     */
    line = line->previous_line();
    get_iter_at_line (&end_propagate, line, 0);
    invalidate_region (end, &end_propagate, false);
  }
  
  /* Sweep backward */

  /* The variable dir_below_propagated contains the backward propagated
   * direction after end. It is neutral if end is at the end of
   * the buffer.
  */
  //dir_below_propagated = PANGO_DIRECTION_NEUTRAL;
  //if (end_line_next)
  //  dir_below_propagated = end_line_next->dir_propagated_back;

  /* Loop backward and propagate the direction of each paragraph 
   * to all neutral lines.
   */
  line = end_line;
  //last_strong = dir_below_propagated;
  while (line != start_line_prev)
    {
      //if (line->dir_strong != PANGO_DIRECTION_NEUTRAL)
      //  last_strong = line->dir_strong;

      //line->dir_propagated_back = last_strong;

      line = line->previous_line();
    }

  /* Continue propagating as long as the resolved backward dir
   * is different from last_strong.
   */
  {
    TextIter start_propagate;

    /*TODO
    while (line &&
	   line->dir_strong == PANGO_DIRECTION_NEUTRAL &&
	   line->dir_propagated_back != last_strong)
      {
        TextLine *prev = line;
        line->dir_propagated_back = last_strong;

        line = _text_line_previous (line);
        if (!line)
          {
            line = prev;
            break;
          }
      }*/

    /* We only need to invalidate for backwards propagation if the
     * line we ended up on didn't get a direction from forwards
     * propagation.
     */
    /*TODOif (line && line->dir_propagated_forward == PANGO_DIRECTION_NEUTRAL)
      {
        get_iter_at_line (&start_propagate, line, 0);
        invalidate_region (&start_propagate, start, false);
      }*/
  }
}

void TextBTree::delete_text (TextIter *start, TextIter *end)
{
  TextLineSegment *prev_seg;             /* The segment just before the start
                                             * of the deletion range. */
  TextLineSegment *last_seg;             /* The segment just after the end
                                             * of the deletion range. */
  TextLineSegment *seg, *next, *next2;
  TextLine *curline;
  TextBTreeNode *curnode, *node;
  TextBTree *tree;
  TextLine *start_line;
  TextLine *end_line;
  TextLine *line;
  TextLine *deleted_lines = NULL;        /* List of lines we've deleted */
  gint start_byte_offset;

  g_return_if_fail (start != NULL);
  g_return_if_fail (end != NULL);
  g_return_if_fail (start->get_btree() == end->get_btree());

  TextIter::order (start, end);

  tree = start->get_btree();
 
  /*TODOif (gtk_debug_flags & GTK_DEBUG_TEXT)
    _text_btree_check (tree);*/
  
  /* Broadcast the need for redisplay before we break the iterators */
  DV (g_print ("invalidating due to deleting some text (%s)\n", G_STRLOC));
  tree->invalidate_region (start, end, false);

  /* Save the byte offset so we can reset the iterators */
  start_byte_offset = start->get_line_index ();

  start_line = start->get_text_line ();
  end_line = end->get_text_line ();

  /*
   * Split the start and end segments, so we have a place
   * to insert our new text.
   *
   * Tricky point:  split at end first;  otherwise the split
   * at end may invalidate seg and/or prev_seg. This allows
   * us to avoid invalidating segments for start.
   */

  last_seg = TextLineSegment::split_segment (end);
  if (last_seg != NULL)
    last_seg = last_seg->next;
  else
    last_seg = end_line->segments;

  prev_seg = TextLineSegment::split_segment (start);
  if (prev_seg != NULL)
    {
      seg = prev_seg->next;
      prev_seg->next = last_seg;
    }
  else
    {
      seg = start_line->segments;
      start_line->segments = last_seg;
    }

  /* notify iterators that their segments need recomputation,
     just for robustness. */
  tree->segments_changed ();

  /*
   * Delete all of the segments between prev_seg and last_seg.
   */

  curline = start_line;
  curnode = curline->parent;
  while (seg != last_seg)
    {
      gint char_count = 0;

      if (seg == NULL)
        {
          TextLine *nextline;

          /*
           * We just ran off the end of a line.  First find the
           * next line, then go back to the old line and delete it
           * (unless it's the starting line for the range).
           */

          nextline = curline->next_line();
          if (curline != start_line)
            {
              if (curnode == start_line->parent)
                start_line->next = curline->next;
              else
                curnode->children.line = curline->next;

              for (node = curnode; node != NULL;
                   node = node->parent)
                {
                  /* Don't update node->num_chars, because
                   * that was done when we deleted the segments.
                   */
                  node->num_lines -= 1;
                }

              curnode->num_children -= 1;
              curline->next = deleted_lines;
              deleted_lines = curline;
            }

          curline = nextline;
          seg = curline->segments;

          /*
           * If the TextBTreeNode is empty then delete it and its parents,
           * recursively upwards until a non-empty TextBTreeNode is found.
           */

          while (curnode->num_children == 0)
            {
              TextBTreeNode *parent;

              parent = curnode->parent;
              if (parent->children.node == curnode)
                {
                  parent->children.node = curnode->next;
                }
              else
                {
                  TextBTreeNode *prevnode = parent->children.node;
                  while (prevnode->next != curnode)
                    {
                      prevnode = prevnode->next;
                    }
                  prevnode->next = curnode->next;
                }
              parent->num_children--;
	      TextBTreeNode::free_empty (curnode);
              curnode = parent;
            }
          curnode = curline->parent;
          continue;
        }

      next = seg->next;
      char_count = seg->char_count;

      if (seg->deleteFunc(curline, false) != 0)
        {
          /*
           * This segment refuses to die.  Move it to prev_seg and
           * advance prev_seg if the segment has left gravity.
           */

          if (prev_seg == NULL)
            {
              seg->next = start_line->segments;
              start_line->segments = seg;
            }
          else if (prev_seg->next &&
		   prev_seg->next != last_seg &&
		   seg->type == text_segment_toggle_off &&
		   prev_seg->next->type == text_segment_toggle_on &&
		   seg->body.toggle.info == prev_seg->next->body.toggle.info)
	    {
	      /* Try to match an off toggle with the matching on toggle
	       * if it immediately follows. This is a common case, and
	       * handling it here prevents quadratic blowup in
	       * cleanup_line() below. See bug 317125.
	       */
	      next2 = prev_seg->next->next;
	      g_free ((char *)prev_seg->next);
	      prev_seg->next = next2;
	      g_free ((char *)seg);
	      seg = NULL;
	    }
	  else
	    {
              seg->next = prev_seg->next;
              prev_seg->next = seg;
            }

          if (seg && seg->leftGravity)
            {
              prev_seg = seg;
            }
        }
      else
        {
          /* Segment is gone. Decrement the char count of the node and
             all its parents. */
          for (node = curnode; node != NULL;
               node = node->parent)
            {
              node->num_chars -= char_count;
            }
        }

      seg = next;
    }

  /*
   * If the beginning and end of the deletion range are in different
   * lines, join the two lines together and discard the ending line.
   */

  if (start_line != end_line)
    {
      BTreeView *view;
      TextBTreeNode *ancestor_node;
      TextLine *prevline;
      int chars_moved;      

      /* last_seg was appended to start_line up at the top of this function */
      chars_moved = 0;
      for (seg = last_seg; seg != NULL;
           seg = seg->next)
        {
          chars_moved += seg->char_count;
          //if (seg->type->lineChangeFunc != NULL)
          //  {
              seg->lineChangeFunc(end_line);
          //  }
        }

      for (node = start_line->parent; node != NULL;
           node = node->parent)
        {
          node->num_chars += chars_moved;
        }
      
      curnode = end_line->parent;
      for (node = curnode; node != NULL;
           node = node->parent)
        {
          node->num_chars -= chars_moved;
          node->num_lines--;
        }
      curnode->num_children--;
      prevline = curnode->children.line;
      if (prevline == end_line)
        {
          curnode->children.line = end_line->next;
        }
      else
        {
          while (prevline->next != end_line)
            {
              prevline = prevline->next;
            }
          prevline->next = end_line->next;
        }
      end_line->next = deleted_lines;
      deleted_lines = end_line;

      /* We now fix up the per-view aggregates. We add all the height and
       * width for the deleted lines to the start line, so that when revalidation
       * occurs, the correct change in size is seen.
       */
      ancestor_node = TextBTreeNode::common_parent (curnode, start_line->parent);
      view = tree->views;
      while (view)
        {
          TextLineData *ld;

          gint deleted_width = 0;
          gint deleted_height = 0;

          line = deleted_lines;
          while (line)
            {
              TextLine *next_line = line->next;
              ld = (TextLineData*)line->get_data (view->view_id);

              if (ld)
                {
                  deleted_width = MAX (deleted_width, ld->width);
                  deleted_height += ld->height;
                }

              line = next_line;
            }

          if (deleted_width > 0 || deleted_height > 0)
            {
              ld = (TextLineData*)start_line->get_data (view->view_id);
              
              if (ld == NULL)
                {
                  /* This means that start_line has never been validated.
                   * We don't really want to do the validation here but
                   * we do need to store our temporary sizes. So we
                   * create the line data and assume a line w/h of 0.
                   */
                  ld = new TextLineData(view->layout, start_line);
                  start_line->add_data (ld);
                  ld->width = 0;
                  ld->height = 0;
                  ld->valid = false;
                }
              
              ld->width = MAX (deleted_width, ld->width);
              ld->height += deleted_height;
              ld->valid = false;
            }

          ancestor_node->check_valid_downward (view->view_id);
          if (ancestor_node->parent)
            TextBTreeNode::check_valid_upward (ancestor_node->parent, view->view_id);

          view = view->next;
        }

      line = deleted_lines;
      while (line)
        {
          TextLine *next_line = line->next;

          tree->text_line_destroy (line);





          line = next_line;
        }

      /* avoid dangling pointer */
      deleted_lines = NULL;
      
      tree->rebalance (curnode);
    }

  /*
   * Cleanup the segments in the new line.
   */

  start_line->cleanup_line();

  /*
   * Lastly, rebalance the first TextBTreeNode of the range.
   */

  tree->rebalance (start_line->parent);

  /* Notify outstanding iterators that they
     are now hosed */
  tree->chars_changed ();
  tree->segments_changed ();

  /*TODOif (gtk_debug_flags & GTK_DEBUG_TEXT)
    _text_btree_check (tree);*/

  /* Re-initialize our iterators */
  tree->get_iter_at_line (start, start_line, start_byte_offset);
  *end = *start;

  tree->resolve_bidi (start, end);
}

void TextBTree::insert_text (TextIter *iter,
                        const gchar *text,
                        gint         len)
{
  TextLineSegment *prev_seg;     /* The segment just before the first
                                     * new segment (NULL means new segment
                                     * is at beginning of line). */
  TextLineSegment *cur_seg;              /* Current segment;  new characters
                                             * are inserted just after this one.
                                             * NULL means insert at beginning of
                                             * line. */
  TextLine *line;           /* Current line (new segments are
                                * added to this line). */
  TextLineSegment *seg;
  TextLine *newline;
  int chunk_len;                        /* # characters in current chunk. */
  gint sol;                           /* start of line */
  gint eol;                           /* Pointer to character just after last
                                       * one in current chunk.
                                       */
  gint delim;                          /* index of paragraph delimiter */
  int line_count_delta;                /* Counts change to total number of
                                        * lines in file.
                                        */

  int char_count_delta;                /* change to number of chars */
  TextBTree *tree;
  gint start_byte_index;
  TextLine *start_line;

  g_return_if_fail (text != NULL);
  g_return_if_fail (iter != NULL);

  if (len < 0)
    len = strlen (text);

  /* extract iterator info */
  tree = iter->get_btree ();
  line = iter->get_text_line ();
  
  start_line = line;
  start_byte_index = iter->get_line_index ();

  /* Get our insertion segment split. Note this assumes line allows
   * char insertions, which isn't true of the "last" line. But iter
   * should not be on that line, as we assert here.
   */
  g_assert (!tree->line_is_last (line));
  prev_seg = TextLineSegment::split_segment (iter);
  cur_seg = prev_seg;

  /* Invalidate all iterators */
  tree->chars_changed ();
  tree->segments_changed ();
  
  /*
   * Chop the text up into lines and create a new segment for
   * each line, plus a new line for the leftovers from the
   * previous line.
   */

  eol = 0;
  sol = 0;
  line_count_delta = 0;
  char_count_delta = 0;
  while (eol < len)
    {
      sol = eol;
      
      find_paragraph_boundary (text + sol,
                                     len - sol,
                                     &delim,
                                     &eol);

      /* make these relative to the start of the text */
      delim += sol;
      eol += sol;

      g_assert (eol >= sol);
      g_assert (delim >= sol);
      g_assert (eol >= delim);
      g_assert (sol >= 0);
      g_assert (eol <= len);
      
      chunk_len = eol - sol;

      g_assert (g_utf8_validate (&text[sol], chunk_len, NULL));
      seg = new TextLineSegmentChar (&text[sol], chunk_len);

      char_count_delta += seg->char_count;

      if (cur_seg == NULL)
        {
          seg->next = line->segments;
          line->segments = seg;
        }
      else
        {
          seg->next = cur_seg->next;
          cur_seg->next = seg;
        }

      if (delim == eol)
        {
          /* chunk didn't end with a paragraph separator */
          g_assert (eol == len);
          break;
        }

      /*
       * The chunk ended with a newline, so create a new TextLine
       * and move the remainder of the old line to it.
       */

      newline = new TextLine();
      newline->set_parent (line->parent);
      newline->next = line->next;
      line->next = newline;
      newline->segments = seg->next;
      seg->next = NULL;
      line = newline;
      cur_seg = NULL;
      line_count_delta++;
    }

  /*
   * Cleanup the starting line for the insertion, plus the ending
   * line if it's different.
   */

  start_line->cleanup_line();
  if (line != start_line)
    {
      line->cleanup_line();
    }

  tree->post_insert_fixup (line, line_count_delta, char_count_delta);

  /* Invalidate our region, and reset the iterator the user
     passed in to point to the end of the inserted text. */
  {
    TextIter start;
    TextIter end;


    tree->get_iter_at_line (
                                      &start,
                                      start_line,
                                      start_byte_index);
    end = start;

    /* We could almost certainly be more efficient here
       by saving the information from the insertion loop
       above. FIXME */
    (&end)->forward_chars (char_count_delta);

    DV (g_print ("invalidating due to inserting some text (%s)\n", G_STRLOC));
    tree->invalidate_region (&start, &end, false);


    /* Convenience for the user */
    *iter = end;

    tree->resolve_bidi (&start, &end);
  }
}

/* void
insert_pixbuf_or_widget_segment (TextIter        *iter,
                                 TextLineSegment *seg)

{
  TextIter start;
  TextLineSegment *prevPtr;
  TextLine *line;
  TextBTree *tree;
  gint start_byte_offset;

  line = iter->get_text_line ();
  tree = iter->get_btree ();
  start_byte_offset = iter->get_line_index ();

  prevPtr = TextLineSegment::split(iter);
  if (prevPtr == NULL)
    {
      seg->next = line->segments;
      line->segments = seg;
    }
  else
    {
      seg->next = prevPtr->next;
      prevPtr->next = seg;
    }

  post_insert_fixup (line, 0, seg->char_count);

  chars_changed ();
  segments_changed ();

  * reset *iter for the user, and invalidate tree nodes *

  get_iter_at_line (&start, line, start_byte_offset);

  *iter = start;
  iter->forward_char (); * skip forward past the segment *

  DV (g_print ("invalidating due to inserting pixbuf/widget (%s)\n", G_STRLOC));
  invalidate_region (&start, iter, false);
}*/
     
/*void
_text_btree_insert_pixbuf (TextIter *iter,
                              GdkPixbuf   *pixbuf)
{
  TextLineSegment *seg;
  
  seg = _gtk_pixbuf_segment_new (pixbuf);

  insert_pixbuf_or_widget_segment (iter, seg);
}*/

/*void
_text_btree_insert_child_anchor (TextIter        *iter,
                                     TextChildAnchor *anchor)
{
  TextLineSegment *seg;
  TextBTree *tree;

  if (anchor->segment != NULL)
    {
      g_warning (G_STRLOC": Same child anchor can't be inserted twice");
      return;
    }
  
  seg = _gtk_widget_segment_new (anchor);

  tree = seg->body.child.tree = iter->get_btree();
  seg->body.child.line = _text_iter_get_text_line (iter);
  
  insert_pixbuf_or_widget_segment (iter, seg);

  if (tree->child_anchor_table == NULL)
    tree->child_anchor_table = g_hash_table_new (NULL, NULL);

  g_hash_table_insert (tree->child_anchor_table,
                       seg->body.child.obj,
                       seg->body.child.obj);
}

void
_text_btree_unregister_child_anchor (TextChildAnchor *anchor)
{
  TextLineSegment *seg;

  seg = anchor->segment;
  
  g_hash_table_remove (seg->body.child.tree->child_anchor_table,
                       anchor);
}*/

/*
 * View stuff
 */

TextLine* TextBTree::find_line_by_y ( BTreeView *view, TextBTreeNode *node, gint y, gint *line_top, TextLine *last_line)
{
  gint current_y = 0;

  /*TODOif (gtk_debug_flags & GTK_DEBUG_TEXT)
    _text_btree_check (tree);*/

  if (node->level == 0)
    {
      TextLine *line;

      line = node->children.line;

      while (line != NULL && line != last_line)
        {
          TextLineData *ld;

          ld = line->get_data (view->view_id);

          if (ld)
            {
              if (y < (current_y + (ld ? ld->height : 0)))
                return line;

              current_y += ld->height;
              *line_top += ld->height;
            }

          line = line->next;
        }
      return NULL;
    }
  else
    {
      TextBTreeNode *child;

      child = node->children.node;

      while (child != NULL)
        {
          gint width;
          gint height;

          child->get_size (view->view_id,
                                        &width, &height);

          if (y < (current_y + height))
            return find_line_by_y (view, child,
                                   y - current_y, line_top,
                                   last_line);

          current_y += height;
          *line_top += height;

          child = child->next;
        }

      return NULL;
    }
}

//TODO probably dont need this
TextLine * TextBTree::find_line_by_y (
                                gpointer      view_id,
                                gint          ypixel,
                                gint         *line_top_out)
{
  TextLine *line;
  BTreeView *view;
  TextLine *last_line;
  gint line_top = 0;

  view = get_view (view_id);
  g_return_val_if_fail (view != NULL, NULL);

  last_line = get_last_line ();

  line = find_line_by_y (view, root_node, ypixel, &line_top,
                         last_line);

  if (line_top_out)
    *line_top_out = line_top;

  return line;
}

 gint TextBTree::find_line_top_in_line_list (
                            BTreeView *view,
                            TextLine *line,
                            TextLine *target_line,
                            gint y)
{
  while (line != NULL)
    {
      TextLineData *ld;

      if (line == target_line)
        return y;

      ld = line->get_data (view->view_id);
      if (ld)
        y += ld->height;

      line = line->next;
    }

  g_assert_not_reached (); /* If we get here, our
                              target line didn't exist
                              under its parent node */
  return 0;
}

gint TextBTree::find_line_top (
                              TextLine *target_line,
                              gpointer view_id)
{
  gint y = 0;
  BTreeView *view;
  GSList *nodes;
  GSList *iter;
  TextBTreeNode *node;

  view = get_view (view_id);

  g_return_val_if_fail (view != NULL, 0);

  nodes = NULL;
  node = target_line->parent;
  while (node != NULL)
    {
      nodes = g_slist_prepend (nodes, node);
      node = node->parent;
    }

  iter = nodes;
  while (iter != NULL)
    {
      node = (TextBTreeNode*)iter->data;

      if (node->level == 0)
        {
          g_slist_free (nodes);
          return find_line_top_in_line_list (view,
                                             node->children.line,
                                             target_line, y);
        }
      else
        {
          TextBTreeNode *child;
          TextBTreeNode *target_node;

          g_assert (iter->next != NULL); /* not at level 0 */
          target_node = (TextBTreeNode*)iter->next->data;

          child = node->children.node;

          while (child != NULL)
            {
              gint width;
              gint height;

              if (child == target_node)
                break;
              else
                {
                  child->get_size (view->view_id,
                                                &width, &height);
                  y += height;
                }
              child = child->next;
            }
          g_assert (child != NULL); /* should have broken out before we
                                       ran out of nodes */
        }

      iter = g_slist_next (iter);
    }

  g_assert_not_reached (); /* we return when we find the target line */
  return 0;
}

void TextBTree::add_view (
                         TextLayout *layout)
{
  BTreeView *view;
  TextLine *last_line;
  TextLineData *line_data;

  //g_return_if_fail (tree != NULL);
  
  view = g_new (BTreeView, 1);

  view->view_id = layout;
  view->layout = layout;

  view->next = views;
  view->prev = NULL;

  if (views)
    {
      g_assert (views->prev == NULL);
      views->prev = view;
    }
  
  views = view;

  /* The last line in the buffer has identity values for the per-view
   * data so that we can avoid special case checks for it in a large
   * number of loops
   */
  last_line = get_last_line ();

  line_data = g_new (TextLineData, 1);
  line_data->view_id = layout;
  line_data->next = NULL;
  line_data->width = 0;
  line_data->height = 0;
  line_data->valid = true;

  last_line->add_data (line_data);
}

void TextBTree::remove_view (
                             gpointer      view_id)
{
  BTreeView *view;
  TextLine *last_line;
  TextLineData *line_data;

  //g_return_if_fail (tree != NULL);
  
  view = views;

  while (view != NULL)
    {
      if (view->view_id == view_id)
        break;

      view = view->next;
    }

  g_return_if_fail (view != NULL);

  if (view->next)
    view->next->prev = view->prev;

  if (view->prev)
    view->prev->next = view->next;

  if (view == views)
    views = view->next;

  /* Remove the line data for the last line which we added ourselves.
   * (Do this first, so that we don't try to call the view's line data destructor on it.)
   */
  last_line = get_last_line ();
  line_data = last_line->remove_data (view_id);
  g_free (line_data);

  root_node->remove_view (view, view_id);

  view->layout = (TextLayout*) 0xdeadbeef;
  view->view_id = (gpointer) 0xdeadbeef;
  
  g_free (view);
}

void TextBTree::invalidate_region (
                                   TextIter *start,
                                   TextIter *end,
                                   bool           cursors_only)
{
  BTreeView *view;

  view = views;

  while (view != NULL)
    {
      if (cursors_only)
	view->layout->invalidate_cursors (start, end);
      else
	view->layout->invalidate (start, end);

      view = view->next;
    }
}

void TextBTree::get_view_size (
                              gpointer view_id,
                              gint *width,
                              gint *height)
{
  //g_return_if_fail (tree != NULL);
  g_return_if_fail (view_id != NULL);

  root_node->get_size (view_id, width, height);
}

/*
 * Tag
 */

typedef struct {
  TextIter *iters;
  guint count;
  guint alloced;
} IterStack;

 IterStack*
iter_stack_new (void)
{
  IterStack *stack;
  stack = g_slice_new (IterStack);
  stack->iters = NULL;
  stack->count = 0;
  stack->alloced = 0;
  return stack;
}

 void
iter_stack_push (IterStack         *stack, 
		 const TextIter *iter)
{
  stack->count += 1;
  if (stack->count > stack->alloced)
    {
      stack->alloced = stack->count*2;
      stack->iters = (TextIter*)g_realloc (stack->iters,
                                stack->alloced * sizeof (TextIter));
    }
  stack->iters[stack->count-1] = *iter;
}

 bool
iter_stack_pop (IterStack   *stack, 
		TextIter *iter)
{
  if (stack->count == 0)
    return false;
  else
    {
      stack->count -= 1;
      *iter = stack->iters[stack->count];
      return true;
    }
}

 void
iter_stack_free (IterStack *stack)
{
  g_free (stack->iters);
  g_slice_free (IterStack, stack);
}

 void
iter_stack_invert (IterStack *stack)
{
  if (stack->count > 0)
    {
      guint i = 0;
      guint j = stack->count - 1;
      while (i < j)
        {
          TextIter tmp;

          tmp = stack->iters[i];
          stack->iters[i] = stack->iters[j];
          stack->iters[j] = tmp;

          ++i;
          --j;
        }
    }
}

void TextBTree::queue_tag_redisplay (
                     TextTag        *tag,
                     TextIter *start,
                     TextIter *end)
{
  if (tag->affects_size ())
    {
      DV (g_print ("invalidating due to size-affecting tag (%s)\n", G_STRLOC));
      invalidate_region (start, end, false);
    }
  else if (tag->affects_nonsize_appearance ())
    {
      /* We only need to queue a redraw, not a relayout */
      redisplay_region (start, end, false);
    }

  /* We don't need to do anything if the tag doesn't affect display */
}

void TextBTree::tag (TextIter *start_orig,
                     TextIter *end_orig,
                     TextTag        *tag,
                     bool           add)
{
  TextLineSegment *seg, *prev;
  TextLine *cleanupline;
  bool toggled_on;
  TextLine *start_line;
  TextLine *end_line;
  TextIter iter;
  TextIter start, end;
  TextBTree *tree;
  IterStack *stack;
  TextTagInfo *info;

  g_return_if_fail (start_orig != NULL);
  g_return_if_fail (end_orig != NULL);
  //g_return_if_fail (GTK_IS_TEXT_TAG (tag));
  g_return_if_fail (start_orig->get_btree() == end_orig->get_btree());
  g_return_if_fail (tag->table == start_orig->get_btree()->table);
  
#if 0
  printf ("%s tag %s from %d to %d\n",
          add ? "Adding" : "Removing",
          tag->name,
          text_buffer_get_offset (start_orig),
          text_buffer_get_offset (end_orig));
#endif

  //TODO with an operator== on textiter
  if (TextIter::equal (start_orig, end_orig))
    return;

  start = *start_orig;
  end = *end_orig;

  TextIter::order (&start, &end);

  tree = (&start)->get_btree();

  tree->queue_tag_redisplay (tag, &start, &end);

  info = tree->get_tag_info (tag);

  start_line = (&start)->get_text_line ();
  end_line = (&end)->get_text_line();

  /* Find all tag toggles in the region; we are going to delete them.
     We need to find them in advance, because
     forward_find_tag_toggle () won't work once we start playing around
     with the tree. */
  stack = iter_stack_new ();
  iter = start;

  /* forward_to_tag_toggle() skips a toggle at the start iterator,
   * which is deliberate - we don't want to delete a toggle at the
   * start.
   */
  while ((&iter)->forward_to_tag_toggle (tag))
    {
	    //TODO operator
      if (TextIter::compare (&iter, &end) >= 0)
        break;
      else
        iter_stack_push (stack, &iter);
    }

  /* We need to traverse the toggles in order. */
  iter_stack_invert (stack);

  /*
   * See whether the tag is present at the start of the range.  If
   * the state doesn't already match what we want then add a toggle
   * there.
   */

  toggled_on = (&start)->has_tag (tag);
  if ( (add && !toggled_on) ||
       (!add && toggled_on) )
    {
      /* This could create a second toggle at the start position;
         ->cleanup_line() will remove it if so. */
      seg = new TextLineSegmentToggle (info, add);

      prev = TextLineSegment::split_segment (&start);
      if (prev == NULL)
        {
          seg->next = start_line->segments;
          start_line->segments = seg;
        }
      else
        {
          seg->next = prev->next;
          prev->next = seg;
        }

      /* cleanup_line adds the new toggle to the node counts. */
#if 0
      printf ("added toggle at start\n");
#endif
      /* we should probably call segments_changed, but in theory
         any still-cached segments in the iters we are about to
         use are still valid, since they're in front
         of this spot. */
    }

  /*
   *
   * Scan the range of characters and delete any internal tag
   * transitions.  Keep track of what the old state was at the end
   * of the range, and add a toggle there if it's needed.
   *
   */

  cleanupline = start_line;
  while (iter_stack_pop (stack, &iter))
    {
      TextLineSegment *indexable_seg;
      TextLine *line;

      line = (&iter)->get_text_line ();
      seg = (&iter)->get_any_segment ();
      indexable_seg = (&iter)->get_indexable_segment ();

      g_assert (seg != NULL);
      g_assert (indexable_seg != NULL);
      g_assert (seg != indexable_seg);

      prev = line->segments;

      /* Find the segment that actually toggles this tag. */
      while (seg != indexable_seg)
        {
          g_assert (seg != NULL);
          g_assert (indexable_seg != NULL);
          g_assert (seg != indexable_seg);
          
          if ( (seg->type == text_segment_toggle_on ||
                seg->type == text_segment_toggle_off) &&
               (seg->body.toggle.info == info) )
            break;
          else
            seg = seg->next;
        }

      g_assert (seg != NULL);
      g_assert (indexable_seg != NULL);

      g_assert (seg != indexable_seg); /* If this happens, then
                                          forward_to_tag_toggle was
                                          full of shit. */
      g_assert (seg->body.toggle.info->tag == tag);

      /* If this happens, when previously tagging we didn't merge
         overlapping tags. */
      g_assert ( (toggled_on && seg->type == text_segment_toggle_off) ||
                 (!toggled_on && seg->type == text_segment_toggle_on) );

      toggled_on = !toggled_on;

#if 0
      printf ("deleting %s toggle\n",
              seg->type == text_segment_toggle_on_type ? "on" : "off");
#endif
      /* Remove toggle segment from the list. */
      if (prev == seg)
        {
          line->segments = seg->next;
        }
      else
        {
          while (prev->next != seg)
            {
              prev = prev->next;
            }
          prev->next = seg->next;
        }

      /* Inform iterators we've hosed them. This actually reflects a
         bit of inefficiency; if you have the same tag toggled on and
         off a lot in a single line, we keep having the rescan from
         the front of the line. Of course we have to do that to get
         "prev" anyway, but here we are doing it an additional
         time. FIXME */
      tree->segments_changed ();

      /* Update node counts */
      if (seg->body.toggle.inNodeCounts)
        {
          line->parent->change_node_toggle_count (
                                         info, -1);
          seg->body.toggle.inNodeCounts = false;
        }

      g_free (seg);

      /* We only clean up lines when we're done with them, saves some
         gratuitous line-segment-traversals */

      if (cleanupline != line)
        {
          cleanupline->cleanup_line();
          cleanupline = line;
        }
    }

  iter_stack_free (stack);

  /* toggled_on now reflects the toggle state _just before_ the
     end iterator. The end iterator could already have a toggle
     on or a toggle off. */
  if ( (add && !toggled_on) ||
       (!add && toggled_on) )
    {
      /* This could create a second toggle at the start position;
         ->cleanup_line() will remove it if so. */

      seg = new TextLineSegmentToggle (info, !add);

      prev = TextLineSegment::split_segment (&end);
      if (prev == NULL)
        {
          seg->next = end_line->segments;
          end_line->segments = seg;
        }
      else
        {
          seg->next = prev->next;
          prev->next = seg;
        }
      /* cleanup_line adds the new toggle to the node counts. */
      g_assert (seg->body.toggle.inNodeCounts == false);
#if 0
      printf ("added toggle at end\n");
#endif
    }

  /*
   * Cleanup cleanupline and the last line of the range, if
   * these are different.
   */

  cleanupline->cleanup_line();
  if (cleanupline != end_line)
    {
      end_line->cleanup_line();
    }

  tree->segments_changed ();

  tree->queue_tag_redisplay (tag, &start, &end);

  /*TODOif (gtk_debug_flags & GTK_DEBUG_TEXT)
    _text_btree_check (tree);*/
}


/*
 * "Getters"
 */

TextLine* TextBTree::get_line_internal (
                   gint          line_number,
                   gint         *real_line_number,
                   bool      include_last)
{
  TextBTreeNode *node;
  TextLine *line;
  int lines_left;
  int line_count;

  line_count = get_line_count ();
  if (!include_last)
    line_count -= 1;
  
  if (line_number < 0)
    {
      line_number = line_count;
    }
  else if (line_number > line_count)
    {
      line_number = line_count;
    }

  if (real_line_number)
    *real_line_number = line_number;

  node = root_node;
  lines_left = line_number;

  /*
   * Work down through levels of the tree until a TextBTreeNode is found at
   * level 0.
   */

  while (node->level != 0)
    {
      for (node = node->children.node;
           node->num_lines <= lines_left;
           node = node->next)
        {
#if 0
          if (node == NULL)
            {
              g_error ("text_btree_find_line ran out of TextBTreeNodes");
            }
#endif
          lines_left -= node->num_lines;
        }
    }

  /*
   * Work through the lines attached to the level-0 TextBTreeNode.
   */

  for (line = node->children.line; lines_left > 0;
       line = line->next)
    {
#if 0
      if (line == NULL)
        {
          g_error ("text_btree_find_line ran out of lines");
        }
#endif
      lines_left -= 1;
    }
  return line;
}

TextLine* TextBTree::get_end_iter_line ()
{
  return
    get_line ( get_line_count () - 1,
                              NULL);
}

TextLine* TextBTree::get_line (
                          gint          line_number,
                          gint         *real_line_number)
{
  return get_line_internal (line_number, real_line_number, true);
}

TextLine* TextBTree::get_line_no_last (
                                  gint               line_number,
                                  gint              *real_line_number)
{
  return get_line_internal (line_number, real_line_number, false);
}

TextLine* TextBTree::get_line_at_char (
                                  gint               char_index,
                                  gint              *line_start_index,
                                  gint              *real_char_index)
{
  TextBTreeNode *node;
  TextLine *line;
  TextLineSegment *seg;
  int chars_left;
  int chars_in_line;

  node = root_node;

  /* Clamp to valid indexes (-1 is magic for "highest index"),
   * node->num_chars includes the two newlines that aren't really
   * in the buffer.
   */
  if (char_index < 0 || char_index >= (node->num_chars - 1))
    {
      char_index = node->num_chars - 2;
    }

  *real_char_index = char_index;

  /*
   * Work down through levels of the tree until a TextBTreeNode is found at
   * level 0.
   */

  chars_left = char_index;
  while (node->level != 0)
    {
      for (node = node->children.node;
           chars_left >= node->num_chars;
           node = node->next)
        {
          chars_left -= node->num_chars;

          g_assert (chars_left >= 0);
        }
    }

  if (chars_left == 0)
    {
      /* Start of a line */

      *line_start_index = char_index;
      return node->children.line;
    }

  /*
   * Work through the lines attached to the level-0 TextBTreeNode.
   */

  chars_in_line = 0;
  seg = NULL;
  for (line = node->children.line; line != NULL; line = line->next)
    {
      seg = line->segments;
      while (seg != NULL)
        {
          if (chars_in_line + seg->char_count > chars_left)
            goto found; /* found our line/segment */

          chars_in_line += seg->char_count;

          seg = seg->next;
        }

      chars_left -= chars_in_line;

      chars_in_line = 0;
      seg = NULL;
    }

 found:

  g_assert (line != NULL); /* hosage, ran out of lines */
  g_assert (seg != NULL);

  *line_start_index = char_index - chars_left;
  return line;
}

/* It returns an array sorted by tags priority, ready to pass to
 * _text_attributes_fill_from_tags() */
TextTag** TextBTree::get_tags (TextIter *iter,
                         gint *num_tags)
{
  TextBTreeNode *node;
  TextLine *siblingline;
  TextLineSegment *seg;
  int src, dst, index;
  TagInfo tagInfo;
  TextLine *line;
  gint byte_index;

#define NUM_TAG_INFOS 10

  line = iter->get_text_line ();
  byte_index = iter->get_line_index ();

  tagInfo.numTags = 0;
  tagInfo.arraySize = NUM_TAG_INFOS;
  tagInfo.tags = g_new (TextTag*, NUM_TAG_INFOS);
  tagInfo.counts = g_new (int, NUM_TAG_INFOS);

  /*
   * Record tag toggles within the line of indexPtr but preceding
   * indexPtr. Note that if this loop segfaults, your
   * byte_index probably points past the sum of all
   * seg->byte_count */

  for (index = 0, seg = line->segments;
       (index + seg->byte_count) <= byte_index;
       index += seg->byte_count, seg = seg->next)
    {
      if ((seg->type == text_segment_toggle_on)
          || (seg->type == text_segment_toggle_off))
        {
          inc_count (seg->body.toggle.info->tag, 1, &tagInfo);
        }
    }

  /*
   * Record toggles for tags in lines that are predecessors of
   * line but under the same level-0 TextBTreeNode.
   */

  for (siblingline = line->parent->children.line;
       siblingline != line;
       siblingline = siblingline->next)
    {
      for (seg = siblingline->segments; seg != NULL;
           seg = seg->next)
        {
          if ((seg->type == text_segment_toggle_on)
              || (seg->type == text_segment_toggle_off))
            {
              inc_count (seg->body.toggle.info->tag, 1, &tagInfo);
            }
        }
    }

  /*
   * For each TextBTreeNode in the ancestry of this line, record tag
   * toggles for all siblings that precede that TextBTreeNode.
   */

  for (node = line->parent; node->parent != NULL;
       node = node->parent)
    {
      TextBTreeNode *siblingPtr;
      Summary *summary;

      for (siblingPtr = node->parent->children.node;
           siblingPtr != node; siblingPtr = siblingPtr->next)
        {
          for (summary = siblingPtr->summary; summary != NULL;
               summary = summary->next)
            {
              if (summary->toggle_count & 1)
                {
                  inc_count (summary->info->tag, summary->toggle_count,
                             &tagInfo);
                }
            }
        }
    }

  /*
   * Go through the tag information and squash out all of the tags
   * that have even toggle counts (these tags exist before the point
   * of interest, but not at the desired character itself).
   */

  for (src = 0, dst = 0; src < tagInfo.numTags; src++)
    {
      if (tagInfo.counts[src] & 1)
        {
          //g_assert (GTK_IS_TEXT_TAG (tagInfo.tags[src]));
          tagInfo.tags[dst] = tagInfo.tags[src];
          dst++;
        }
    }

  *num_tags = dst;
  g_free (tagInfo.counts);
  if (dst == 0)
    {
      g_free (tagInfo.tags);
      return NULL;
    }

  /* Sort tags in ascending order of priority */
  TextTag::array_sort (tagInfo.tags, dst);

  return tagInfo.tags;
}

void
copy_segment (GString *string,
              bool include_hidden,
              bool include_nonchars,
              TextIter *start,
              TextIter *end)
{
  TextLineSegment *end_seg;
  TextLineSegment *seg;

  if (TextIter::equal (start, end))
    return;

  seg = start->get_indexable_segment ();
  end_seg = end->get_indexable_segment ();

  if (seg->type == text_segment_char)
    {
      bool copy = true;
      gint copy_bytes = 0;
      gint copy_start = 0;

      /* Don't copy if we're invisible; segments are invisible/not
         as a whole, no need to check each char */
      if (!include_hidden &&
          TextBTree::char_is_invisible (start))
        {
          copy = false;
          /* printf (" <invisible>\n"); */
        }

      copy_start = start->get_segment_byte ();

      if (seg == end_seg)
        {
          /* End is in the same segment; need to copy fewer bytes. */
          gint end_byte = end->get_segment_byte ();

          copy_bytes = end_byte - copy_start;
        }
      else
        copy_bytes = seg->byte_count - copy_start;

      g_assert (copy_bytes != 0); /* Due to iter equality check at
                                     front of this function. */

      if (copy)
        {
          g_assert ((copy_start + copy_bytes) <= seg->byte_count);

          g_string_append_len (string,
                               seg->body.chars + copy_start,
                               copy_bytes);
        }

      /* printf ("  :%s\n", string->str); */
    }
  /*TODO else if (seg->type == text_pixbuf_type ||
           seg->type == text_child_type)
    {
      bool copy = true;

      if (!include_nonchars)
        {
          copy = false;
        }
      else if (!include_hidden &&
               TextBTree::char_is_invisible (start))
        {
          copy = false;
        }

      if (copy)
        {
          g_string_append_len (string,
                               text_unknown_char_utf8,
                               3);

        }
    }*/
}

gchar* TextBTree::get_text (TextIter *start_orig,
                         TextIter *end_orig,
                         bool include_hidden,
                         bool include_nonchars)
{
  TextLineSegment *seg;
  TextLineSegment *end_seg;
  GString *retval;
  gchar *str;
  TextIter iter;
  TextIter start;
  TextIter end;

  g_return_val_if_fail (start_orig != NULL, NULL);
  g_return_val_if_fail (end_orig != NULL, NULL);
  g_return_val_if_fail (start_orig->get_btree() == end_orig->get_btree(), NULL);

  start = *start_orig;
  end = *end_orig;

  TextIter::order (&start, &end);

  retval = g_string_new (NULL);

  end_seg = (&end)->get_indexable_segment ();
  iter = start;
  seg = (&iter)->get_indexable_segment ();
  while (seg != end_seg)
    {
      copy_segment (retval, include_hidden, include_nonchars,
                    &iter, &end);

      (&iter)->forward_indexable_segment ();

      seg = (&iter)->get_indexable_segment ();
    }

  copy_segment (retval, include_hidden, include_nonchars, &iter, &end);

  str = retval->str;
  g_string_free (retval, false);
  return str;
}

gint TextBTree::get_line_count ()
{
  /* Subtract bogus line at the end; we return a count
     of usable lines. */
  return root_node->num_lines - 1;
}

gint TextBTree::get_char_count ()
{
  /* Exclude newline in bogus last line and the
   * one in the last line that is after the end iterator
   */
  return root_node->num_chars - 2;
}

#define LOTSA_TAGS 1000
//TODO move to textiter??
bool TextBTree::char_is_invisible (TextIter *iter)
{
  bool invisible = false;  /* if nobody says otherwise, it's visible */

  int deftagCnts[LOTSA_TAGS] = { 0, };
  int *tagCnts = deftagCnts;
  TextTag *deftags[LOTSA_TAGS];
  TextTag **tags = deftags;
  int numTags;
  TextBTreeNode *node;
  TextLine *siblingline;
  TextLineSegment *seg;
  TextTag *tag;
  int i, index;
  TextLine *line;
  TextBTree *tree;
  gint byte_index;

  line = iter->get_text_line ();
  tree = iter->get_btree ();
  byte_index = iter->get_line_index ();

  numTags = tree->table->get_size();

  /* almost always avoid malloc, so stay out of system calls */
  if (LOTSA_TAGS < numTags)
    {
      tagCnts = g_new0 (int, numTags);
      tags = g_new (TextTag*, numTags);
    }

  /*
   * Record tag toggles within the line of indexPtr but preceding
   * indexPtr.
   */

  for (index = 0, seg = line->segments;
       (index + seg->byte_count) <= byte_index; /* segfault here means invalid index */
       index += seg->byte_count, seg = seg->next)
    {
      if ((seg->type == text_segment_toggle_on)
          || (seg->type == text_segment_toggle_off))
        {
          tag = seg->body.toggle.info->tag;
          if (tag->invisible_set)
            {
              tags[tag->priority] = tag;
              tagCnts[tag->priority]++;
            }
        }
    }

  /*
   * Record toggles for tags in lines that are predecessors of
   * line but under the same level-0 TextBTreeNode.
   */

  for (siblingline = line->parent->children.line;
       siblingline != line;
       siblingline = siblingline->next)
    {
      for (seg = siblingline->segments; seg != NULL;
           seg = seg->next)
        {
          if ((seg->type == text_segment_toggle_on)
              || (seg->type == text_segment_toggle_off))
            {
              tag = seg->body.toggle.info->tag;
              if (tag->invisible_set)
                {
                  tags[tag->priority] = tag;
                  tagCnts[tag->priority]++;
                }
            }
        }
    }

  /*
   * For each TextBTreeNode in the ancestry of this line, record tag toggles
   * for all siblings that precede that TextBTreeNode.
   */

  for (node = line->parent; node->parent != NULL;
       node = node->parent)
    {
      TextBTreeNode *siblingPtr;
      Summary *summary;

      for (siblingPtr = node->parent->children.node;
           siblingPtr != node; siblingPtr = siblingPtr->next)
        {
          for (summary = siblingPtr->summary; summary != NULL;
               summary = summary->next)
            {
              if (summary->toggle_count & 1)
                {
                  tag = summary->info->tag;
                  if (tag->invisible_set)
                    {
                      tags[tag->priority] = tag;
                      tagCnts[tag->priority] += summary->toggle_count;
                    }
                }
            }
        }
    }

  /*
   * Now traverse from highest priority to lowest,
   * take invisible value from first odd count (= on)
   */

  for (i = numTags-1; i >=0; i--)
    {
      if (tagCnts[i] & 1)
        {
          /* FIXME not sure this should be if 0 */
#if 0
#ifndef ALWAYS_SHOW_SELECTION
          /* who would make the selection invisible? */
          if ((tag == tkxt->seltag)
              && !(tkxt->flags & GOT_FOCUS))
            {
              continue;
            }
#endif
#endif
          invisible = tags[i]->values->invisible;
          break;
        }
    }

  if (LOTSA_TAGS < numTags)
    {
      g_free (tagCnts);
      g_free (tags);
    }

  return invisible;
}


/*
 * Manipulate marks
 */

void TextBTree::redisplay_region (
                  TextIter *start,
                  TextIter *end,
                  bool           cursors_only)
{
  BTreeView *view;
  TextLine *start_line, *end_line;

  if (TextIter::compare (start, end) > 0)
    {
      TextIter *tmp = start;
      start = end;
      end = tmp;
    }

  start_line = start->get_text_line ();
  end_line = end->get_text_line ();

  view = views;
  while (view != NULL)
    {
      gint start_y, end_y;
      TextLineData *ld;

      start_y = find_line_top (start_line, view->view_id);

      if (end_line == start_line)
        end_y = start_y;
      else
        end_y = find_line_top (end_line, view->view_id);

      ld = end_line->get_data (view->view_id);
      if (ld)
        end_y += ld->height;

      if (cursors_only)
	view->layout->cursors_changed (start_y,
					 end_y - start_y,
					  end_y - start_y);
      else
	view->layout->changed (start_y,
				 end_y - start_y,
				 end_y - start_y);

      view = view->next;
    }
}

void TextBTree::redisplay_mark (TextLineSegment *mark)
{
  TextIter iter;
  TextIter end;
  bool cursor_only;

  mark->body.mark.tree->get_iter_at_mark (
                                   &iter,
                                   mark->body.mark.obj);

  end = iter;
  (&end)->forward_char ();

  DV (g_print ("invalidating due to moving visible mark (%s)\n", G_STRLOC));
  cursor_only = mark == mark->body.mark.tree->insert_mark->segment;
  mark->body.mark.tree->invalidate_region(&iter, &end, cursor_only);
}

void TextBTree::redisplay_mark_if_visible (TextLineSegment *mark)
{
  if (!mark->body.mark.visible)
    return;
  else
    redisplay_mark (mark);
}

void TextBTree::ensure_not_off_end (
                    TextLineSegment *mark,
                    TextIter *iter)
{
  if (iter->get_line () == get_line_count ())
    iter->backward_char ();
}

TextLineSegment*
TextBTree::real_set_mark (
               TextMark       *existing_mark,
               const gchar       *name,
               bool           left_gravity,
               TextIter *where,
               bool           should_exist,
               bool           redraw_selections)
{
  TextLineSegment *mark;
  TextIter iter;

  //g_return_val_if_fail (tree != NULL, NULL);
  g_return_val_if_fail (where != NULL, NULL);
  g_return_val_if_fail (where->get_btree() == this, NULL);

  if (existing_mark != NULL)
    {
      if (existing_mark->get_buffer () != NULL)
	mark = existing_mark->segment;
      else
	mark = NULL;
    }
  else if (name != NULL) 
    {
      mark = (TextLineSegment*)g_hash_table_lookup (mark_table, name);
    }
  else
    mark = NULL;

  if (should_exist && mark == NULL)
    {
      g_warning ("No mark `%s' exists!", name);
      return NULL;
    }

  /* OK if !should_exist and it does already exist, in that case
   * we just move it.
   */
  
  iter = *where;

  /*TODOif (gtk_debug_flags & GTK_DEBUG_TEXT)
    _text_iter_check (&iter);*/
  
  if (mark != NULL)
    {
      if (redraw_selections &&
          (mark == insert_mark->segment ||
           mark == selection_bound_mark->segment))
        {
          TextIter old_pos;

          get_iter_at_mark (&old_pos,
                                           mark->body.mark.obj);
          redisplay_region (&old_pos, where, true);
        }

      /*
       * don't let visible marks be after the final newline of the
       *  file.
       */

      if (mark->body.mark.visible)
        {
          ensure_not_off_end (mark, &iter);
        }

      /* Redraw the mark's old location. */
      redisplay_mark_if_visible (mark);

      /* Unlink mark from its current location.
         This could hose our iterator... */
      unlink_segment (mark,
                                     mark->body.mark.line);
      mark->body.mark.line = (&iter)->get_text_line ();
      g_assert (mark->body.mark.line == (&iter)->get_text_line ());

      segments_changed (); /* make sure the iterator recomputes its
                                  segment */
    }
  else
    {
      if (existing_mark)
      {}//TODOg_object_ref (existing_mark);
      else
	existing_mark = new TextMark (name, left_gravity);

      mark = existing_mark->segment;
      mark->set_tree (this);

      mark->body.mark.line = (&iter)->get_text_line ();

      if (mark->body.mark.name)
        g_hash_table_insert (mark_table,
                             mark->body.mark.name,
                             mark);
    }

  /*TODOif (gtk_debug_flags & GTK_DEBUG_TEXT)
    _text_iter_check (&iter);*/
  
  /* Link mark into new location */
  TextBTree::link_segment (mark, &iter);

  /* Invalidate some iterators. */
  segments_changed ();

  /*
   * update the screen at the mark's new location.
   */

  redisplay_mark_if_visible (mark);

  /*TODOif (gtk_debug_flags & GTK_DEBUG_TEXT)
    _text_iter_check (&iter);*/

  /*TODOif (gtk_debug_flags & GTK_DEBUG_TEXT)
    _text_btree_check (tree);*/
  
  return mark;
}


TextMark* TextBTree::set_mark (
                         TextMark  *existing_mark,
                         const gchar *name,
                         bool left_gravity,
                         TextIter *iter,
                         bool should_exist)
{
  TextLineSegment *seg;

  seg = real_set_mark (existing_mark,
                       name, left_gravity, iter, should_exist,
                       true);

  return seg ? seg->body.mark.obj : NULL;
}

bool TextBTree::get_selection_bounds (
                                     TextIter  *start,
                                     TextIter  *end)
{
  TextIter tmp_start, tmp_end;

  get_iter_at_mark (&tmp_start, insert_mark);
  get_iter_at_mark (&tmp_end, selection_bound_mark);

  if (TextIter::equal (&tmp_start, &tmp_end))
    {
      if (start)
        *start = tmp_start;

      if (end)
        *end = tmp_end;

      return false;
    }
  else
    {
	    TextIter::order (&tmp_start, &tmp_end);

      if (start)
        *start = tmp_start;

      if (end)
        *end = tmp_end;

      return true;
    }
}

void TextBTree::place_cursor (
                             TextIter *iter)
{
  select_range (iter, iter);
}

void TextBTree::select_range (
			      TextIter *ins,
                              TextIter *bound)
{
  TextIter old_ins, old_bound;

  get_iter_at_mark (&old_ins, insert_mark);
  get_iter_at_mark (&old_bound, selection_bound_mark);

  /* Check if it's no-op since text_buffer_place_cursor()
   * also calls this, and this will redraw the cursor line. */
  if (!TextIter::equal (&old_ins, ins) ||
      !TextIter::equal (&old_bound, bound))
    {
      redisplay_region (&old_ins, &old_bound, true);

      /* Move insert AND selection_bound before we redisplay */
      real_set_mark (insert_mark,
		     "insert", false, ins, true, false);
      real_set_mark (selection_bound_mark,
		     "selection_bound", false, bound, true, false);

      redisplay_region (ins, bound, true);
    }
}


void TextBTree::remove_mark_by_name (
                                    const gchar *name)
{
  TextMark *mark;

 // g_return_if_fail (tree != NULL);
  g_return_if_fail (name != NULL);

  mark = (TextMark*)g_hash_table_lookup (mark_table,
                              name);

  remove_mark (mark);
}

void TextBTree::release_mark_segment (
                                      TextLineSegment *segment)
{

  if (segment->body.mark.name)
    g_hash_table_remove (mark_table, segment->body.mark.name);

  segment->body.mark.tree = NULL;
  segment->body.mark.line = NULL;
  
  /* Remove the ref on the mark, which frees segment as a side effect
   * if this is the last reference.
   */
  //TODOg_object_unref (segment->body.mark.obj);
}

void TextBTree::remove_mark (
                             TextMark *mark)
{
  TextLineSegment *segment;

  g_return_if_fail (mark != NULL);
  //g_return_if_fail (tree != NULL);

  segment = mark->segment;

  if (segment->body.mark.not_deleteable)
    {
      g_warning ("Can't delete special mark `%s'", segment->body.mark.name);
      return;
    }

  /* This calls cleanup_line and segments_changed */
  unlink_segment (segment, segment->body.mark.line);
  
  release_mark_segment (segment);
}

bool TextBTree::mark_is_insert (
                                TextMark *segment)
{
  return segment == insert_mark;
}

bool TextBTree::mark_is_selection_bound (
                                         TextMark *segment)
{
  return segment == selection_bound_mark;
}

TextMark * TextBTree::get_insert (void)
{
  return insert_mark;
}

TextMark * TextBTree::get_selection_bound (void)
{
  return selection_bound_mark;
}

TextMark* TextBTree::get_mark_by_name (
                                  const gchar *name)
{
  TextLineSegment *seg;

 // g_return_val_if_fail (tree != NULL, NULL);
  g_return_val_if_fail (name != NULL, NULL);

  seg = (TextLineSegment*)g_hash_table_lookup (mark_table, name);

  return seg ? seg->body.mark.obj : NULL;
}

/**
 * text_mark_set_visible:
 * @mark: a #TextMark
 * @setting: visibility of mark
 * 
 * Sets the visibility of @mark; the insertion point is normally
 * visible, i.e. you can see it as a vertical bar. Also, the text
 * widget uses a visible mark to indicate where a drop will occur when
 * dragging-and-dropping text. Most other marks are not visible.
 * Marks are not visible by default.
 * 
 **/
void TextBTree::mark_set_visible (TextMark       *mark,
                           bool           setting)
{
  TextLineSegment *seg;

  g_return_if_fail (mark != NULL);

  seg = mark->segment;

  if (seg->body.mark.visible == setting)
    return;
  else
    {
      seg->body.mark.visible = setting;

      if (seg->body.mark.tree)
	redisplay_mark (seg);
    }
}

TextLine* TextBTree::first_could_contain_tag (
                                        TextTag *tag)
{
  TextBTreeNode *node;
  TextTagInfo *info;

  //g_return_val_if_fail (tree != NULL, NULL);

  if (tag != NULL)
    {
      info = get_existing_tag_info (tag);

      if (info == NULL)
        return NULL;

      if (info->tag_root == NULL)
        return NULL;

      node = info->tag_root;

      /* We know the tag root has instances of the given
         tag below it */

    continue_outer_loop:
      g_assert (node != NULL);
      while (node->level > 0)
        {
          g_assert (node != NULL); /* Failure probably means bad tag summaries. */
          node = node->children.node;
          while (node != NULL)
            {
              if (node->has_tag (tag))
                goto continue_outer_loop;

              node = node->next;
            }
          g_assert (node != NULL);
        }

      g_assert (node != NULL); /* The tag summaries said some node had
                                  tag toggles... */

      g_assert (node->level == 0);

      return node->children.line;
    }
  else
    {
      /* Looking for any tag at all (tag == NULL).
         Unfortunately this can't be done in a simple and efficient way
         right now; so I'm just going to return the
         first line in the btree. FIXME */
      return get_line (0, NULL);
    }
}

TextLine* TextBTree::last_could_contain_tag (
                                       TextTag *tag)
{
  TextBTreeNode *node;
  TextBTreeNode *last_node;
  TextLine *line;
  TextTagInfo *info;

  //g_return_val_if_fail (tree != NULL, NULL);

  if (tag != NULL)
    {
      info = get_existing_tag_info (tag);

      if (info->tag_root == NULL)
        return NULL;

      node = info->tag_root;
      /* We know the tag root has instances of the given
         tag below it */

      while (node->level > 0)
        {
          g_assert (node != NULL); /* Failure probably means bad tag summaries. */
          last_node = NULL;
          node = node->children.node;
          while (node != NULL)
            {
              if (node->has_tag (tag))
                last_node = node;
              node = node->next;
            }

          node = last_node;
        }

      g_assert (node != NULL); /* The tag summaries said some node had
                                  tag toggles... */

      g_assert (node->level == 0);

      /* Find the last line in this node */
      line = node->children.line;
      while (line->next != NULL)
        line = line->next;

      return line;
    }
  else
    {
      /* This search can't be done efficiently at the moment,
         at least not without complexity.
         So, we just return the last line.
      */
      return get_end_iter_line ();
    }
}


/*
 * Lines
 */

gint TextLine::get_number (void)
{
  TextLine *line2;
  TextBTreeNode *node, *node2, *parent_iter;
  int index;

  /*
   * First count how many lines precede this one in its level-0
   * TextBTreeNode.
   */

  node = parent;
  index = 0;
  for (line2 = node->children.line; line2 != this;
       line2 = line2->next)
    {
      if (line2 == NULL)
        {
          g_error ("text_btree_line_number couldn't find line");
        }
      index += 1;
    }

  /*
   * Now work up through the levels of the tree one at a time,
   * counting how many lines are in TextBTreeNodes preceding the current
   * TextBTreeNode.
   */

  for (parent_iter = node->parent ; parent_iter != NULL;
       node = parent_iter, parent_iter = parent_iter->parent)
    {
      for (node2 = parent_iter->children.node; node2 != node;
           node2 = node2->next)
        {
          if (node2 == NULL)
            {
              g_error ("text_btree_line_number couldn't find TextBTreeNode");
            }
          index += node2->num_lines;
        }
    }
  return index;
}

TextLineSegment* TextLine::find_toggle_segment_before_char (
                                 gint char_in_line,
                                 TextTag *tag)
{
  TextLineSegment *seg;
  TextLineSegment *toggle_seg;
  int index;

  toggle_seg = NULL;
  index = 0;
  seg = segments;
  while ( (index + seg->char_count) <= char_in_line )
    {
      if (((seg->type == text_segment_toggle_on)
           || (seg->type == text_segment_toggle_off))
          && (seg->body.toggle.info->tag == tag))
        toggle_seg = seg;

      index += seg->char_count;
      seg = seg->next;
    }

  return toggle_seg;
}

TextLineSegment* TextLine::find_toggle_segment_before_byte (
                                 gint byte_in_line,
                                 TextTag *tag)
{
  TextLineSegment *seg;
  TextLineSegment *toggle_seg;
  int index;

  toggle_seg = NULL;
  index = 0;
  seg = segments;
  while ( (index + seg->byte_count) <= byte_in_line )
    {
      if (((seg->type == text_segment_toggle_on)
           || (seg->type == text_segment_toggle_off))
          && (seg->body.toggle.info->tag == tag))
        toggle_seg = seg;

      index += seg->byte_count;
      seg = seg->next;
    }

  return toggle_seg;
}

bool TextLine::find_toggle_outside_current_line (
				TextBTree *tree,
                                  TextTag *tag)
{
  TextBTreeNode *node;
  TextLine *sibling_line;
  TextLineSegment *seg;
  TextLineSegment *toggle_seg;
  int toggles;
  TextTagInfo *info = NULL;

  /*
   * No toggle in this line.  Look for toggles for the tag in lines
   * that are predecessors of line but under the same
   * level-0 TextBTreeNode.
   */
  toggle_seg = NULL;
  sibling_line = parent->children.line;
  while (sibling_line != this)
    {
      seg = sibling_line->segments;
      while (seg != NULL)
        {
          if (((seg->type == text_segment_toggle_on)
               || (seg->type == text_segment_toggle_off))
              && (seg->body.toggle.info->tag == tag))
            toggle_seg = seg;

          seg = seg->next;
        }

      sibling_line = sibling_line->next;
    }

  if (toggle_seg != NULL)
    return (toggle_seg->type == text_segment_toggle_on);

  /*
   * No toggle in this TextBTreeNode.  Scan upwards through the ancestors of
   * this TextBTreeNode, counting the number of toggles of the given tag in
   * siblings that precede that TextBTreeNode.
   */

  info = tree->get_existing_tag_info (tag);

  if (info == NULL)
    return false;

  toggles = 0;
  node = parent;
  while (node->parent != NULL)
    {
      TextBTreeNode *sibling_node;

      sibling_node = node->parent->children.node;
      while (sibling_node != node)
        {
          Summary *summary;

          summary = sibling_node->summary;
          while (summary != NULL)
            {
              if (summary->info == info)
                toggles += summary->toggle_count;

              summary = summary->next;
            }

          sibling_node = sibling_node->next;
        }

      if (node == info->tag_root)
        break;

      node = node->parent;
    }

  /*
   * An odd number of toggles means that the tag is present at the
   * given point.
   */

  return (toggles & 1) != 0;
}

/* FIXME this function is far too slow, for no good reason. */
bool TextLine::char_has_tag (
                             TextBTree *tree,
                             gint char_in_line,
                             TextTag *tag)
{
  TextLineSegment *toggle_seg;

  //g_return_val_if_fail (line != NULL, false);

  /*
   * Check for toggles for the tag in the line but before
   * the char.  If there is one, its type indicates whether or
   * not the character is tagged.
   */

  toggle_seg = find_toggle_segment_before_char (char_in_line, tag);

  if (toggle_seg != NULL)
    return (toggle_seg->type == text_segment_toggle_on);
  else
    return find_toggle_outside_current_line (tree, tag);
}

bool TextLine::byte_has_tag ( TextBTree *tree, gint byte_in_line, TextTag *tag)
{
  TextLineSegment *toggle_seg;

  //g_return_val_if_fail (line != NULL, false);

  /*
   * Check for toggles for the tag in the line but before
   * the char.  If there is one, its type indicates whether or
   * not the character is tagged.
   */

  toggle_seg = find_toggle_segment_before_byte (byte_in_line, tag);

  if (toggle_seg != NULL)
    return (toggle_seg->type == text_segment_toggle_on);
  else
    return find_toggle_outside_current_line (tree, tag);
}

bool TextBTree::line_is_last (TextLine *line)
{
  return line == get_last_line ();
}

void TextBTree::ensure_end_iter_line (void)
{
  if (end_iter_line_stamp != chars_changed_stamp)
    {
      gint real_line;
	
       /* n_lines is without the magic line at the end */
      g_assert (get_line_count () >= 1);

      end_iter_line = get_line_no_last (-1, &real_line);
      
      end_iter_line_stamp = chars_changed_stamp;
    }
}

void TextBTree::ensure_end_iter_segment (void)
{
  if (end_iter_segment_stamp != segments_changed_stamp)
    {
      TextLineSegment *seg;
      TextLineSegment *last_with_chars;

      ensure_end_iter_line ();

      last_with_chars = NULL;
      
      seg = end_iter_line->segments;
      while (seg != NULL)
        {
          if (seg->char_count > 0)
            last_with_chars = seg;
          seg = seg->next;
        }

      end_iter_segment = last_with_chars;

      /* We know the last char in the last line is '\n' */
      end_iter_segment_byte_index = last_with_chars->byte_count - 1;
      end_iter_segment_char_offset = last_with_chars->char_count - 1;
      
      end_iter_segment_stamp = segments_changed_stamp;

      g_assert (end_iter_segment->type == text_segment_char);
      g_assert (end_iter_segment->body.chars[end_iter_segment_byte_index] == '\n');
    }
}

bool TextLine::contains_end_iter (TextBTree *tree)
{ 
	tree->ensure_end_iter_line ();

  return this == tree->end_iter_line;
}

bool TextBTree::is_end (
                        TextLine        *line,
                        TextLineSegment *seg,
                        int                 byte_index,
                        int                 char_offset)
{
  g_return_val_if_fail (byte_index >= 0 || char_offset >= 0, false);
  
  /* Do this first to avoid walking segments in most cases */
  if (!line->contains_end_iter (this))
    return false;

  ensure_end_iter_segment ();

  if (seg != end_iter_segment)
    return false;

  if (byte_index >= 0)
    return byte_index == end_iter_segment_byte_index;
  else
    return char_offset == end_iter_segment_char_offset;
}

TextLine* TextLine::next_line (void)
{
  TextBTreeNode *node;

  if (next != NULL)
    return next;
  else
    {
      /*
       * This was the last line associated with the particular parent
       * TextBTreeNode.  Search up the tree for the next TextBTreeNode,
       * then search down from that TextBTreeNode to find the first
       * line.
       */

      node = parent;
      while (node != NULL && node->next == NULL)
        node = node->parent;

      if (node == NULL)
        return NULL;

      node = node->next;
      while (node->level > 0)
        {
          node = node->children.node;
        }

      g_assert (node->children.line != this);

      return node->children.line;
    }
}

TextLine* TextLine::next_excluding_last (void)
{
  TextLine *next;
  
  next = next_line();

  /* If we were on the end iter line, we can't go to
   * the last line
   */
  if (next && next->next == NULL && /* these checks are optimization only */
      next->next_line () == NULL)
    return NULL;

  return next;
}

TextLine* TextLine::previous_line (void)
{
  TextBTreeNode *node;
  TextBTreeNode *node2;
  TextLine *prev;

  /*
   * Find the line under this TextBTreeNode just before the starting line.
   */
  prev = parent->children.line;        /* First line at leaf */
  while (prev != this)
    {
      if (prev->next == this)
        return prev;

      prev = prev->next;

      if (prev == NULL)
        g_error ("text_btree_previous_line ran out of lines");
    }

  /*
   * This was the first line associated with the particular parent
   * TextBTreeNode.  Search up the tree for the previous TextBTreeNode,
   * then search down from that TextBTreeNode to find its last line.
   */
  for (node = parent; ; node = node->parent)
    {
      if (node == NULL || node->parent == NULL)
        return NULL;
      else if (node != node->parent->children.node)
        break;
    }

  for (node2 = node->parent->children.node; ;
       node2 = node2->children.node)
    {
      while (node2->next != node)
        node2 = node2->next;

      if (node2->level == 0)
        break;

      node = NULL;
    }

  for (prev = node2->children.line ; ; prev = prev->next)
    {
      if (prev->next == NULL)
        return prev;
    }

  g_assert_not_reached ();
  return NULL;
}


TextLineData::TextLineData (TextLayout *layout,
                         TextLine   *line)
{
  view_id = layout;
  next = NULL;
  width = 0;
  height = 0;
  valid = false;
}

void TextLine::add_data (
                         TextLineData *data)
{
 // g_return_if_fail (line != NULL);
  g_return_if_fail (data != NULL);
  g_return_if_fail (data->view_id != NULL);

  if (views)
    {
      data->next = views;
      views = data;
    }
  else
    {
      views = data;
    }
}

TextLineData* TextLine::remove_data ( gpointer view_id)
{
  TextLineData *prev;
  TextLineData *iter;

//  g_return_val_if_fail (line != NULL, NULL);
  g_return_val_if_fail (view_id != NULL, NULL);

  prev = NULL;
  iter = views;
  while (iter != NULL)
    {
      if (iter->view_id == view_id)
        break;
      prev = iter;
      iter = iter->next;
    }

  if (iter)
    {
      if (prev)
        prev->next = iter->next;
      else
        views = iter->next;

      return iter;
    }
  else
    return NULL;
}

TextLineData* TextLine::get_data ( gpointer view_id)
{
  TextLineData *iter;

//  g_return_val_if_fail (line != NULL, NULL);
  g_return_val_if_fail (view_id != NULL, NULL);

  iter = views;
  while (iter != NULL)
    {
      if (iter->view_id == view_id)
        break;
      iter = iter->next;
    }

  return iter;
}

void TextLine::invalidate_wrap ( TextLineData *ld)
{
  /* For now this is totally unoptimized. FIXME?

     We could probably optimize the case where the width removed
     is less than the max width for the parent node,
     and the case where the height is unchanged when we re-wrap.
  */
  
  g_return_if_fail (ld != NULL);
  
  ld->valid = false;
  parent->invalidate_upward (ld->view_id);
}

gint TextLine::char_count (void)
{
  TextLineSegment *seg;
  gint size;

  size = 0;
  seg = segments;
  while (seg != NULL)
    {
      size += seg->char_count;
      seg = seg->next;
    }
  return size;
}

gint TextLine::byte_count (void)
{
  TextLineSegment *seg;
  gint size;

  size = 0;
  seg = segments;
  while (seg != NULL)
    {
      size += seg->byte_count;
      seg = seg->next;
    }

  return size;
}

gint TextLine::char_index ()
{
  GSList *node_stack = NULL;
  TextBTreeNode *iter;
  TextLine *line;
  gint num_chars;

  /* Push all our parent nodes onto a stack */
  iter = parent;

  g_assert (iter != NULL);

  while (iter != NULL)
    {
      node_stack = g_slist_prepend (node_stack, iter);

      iter = iter->parent;
    }

  /* Check that we have the root node on top of the stack. */
  g_assert (node_stack != NULL &&
            node_stack->data != NULL &&
            ((TextBTreeNode*)node_stack->data)->parent == NULL);

  /* Add up chars in all nodes before the nodes in our stack.
   */

  num_chars = 0;
  iter = (TextBTreeNode*)node_stack->data;
  while (iter != NULL)
    {
      TextBTreeNode *child_iter;
      TextBTreeNode *next_node;

      next_node = node_stack->next ?
        (TextBTreeNode*)node_stack->next->data : NULL;
      node_stack = g_slist_remove (node_stack, node_stack->data);

      if (iter->level == 0)
        {
          /* stack should be empty when we're on the last node */
          g_assert (node_stack == NULL);
          break; /* Our children are now lines */
        }

      g_assert (next_node != NULL);
      g_assert (iter != NULL);
      g_assert (next_node->parent == iter);

      /* Add up chars before us in the tree */
      child_iter = iter->children.node;
      while (child_iter != next_node)
        {
          g_assert (child_iter != NULL);

          num_chars += child_iter->num_chars;

          child_iter = child_iter->next;
        }

      iter = next_node;
    }

  g_assert (iter != NULL);
  g_assert (iter == parent);

  /* Since we don't store char counts in lines, only in segments, we
     have to iterate over the lines adding up segment char counts
     until we find our line.  */
  line = iter->children.line;
  while (line != this)
    {
      g_assert (line != NULL);

      num_chars += line->char_count ();

      line = line->next;
    }

  g_assert (line == this);

  return num_chars;
}

TextLineSegment* TextLine::byte_to_segment ( gint byte_offset, gint *seg_offset)
{
  TextLineSegment *seg;
  int offset;

//  g_return_val_if_fail (line != NULL, NULL);

  offset = byte_offset;
  seg = segments;

  while (offset >= seg->byte_count)
    {
      offset -= seg->byte_count;
      seg = seg->next;
      g_assert (seg != NULL); /* means an invalid byte index */
    }

  if (seg_offset)
    *seg_offset = offset;

  return seg;
}

TextLineSegment* TextLine::char_to_segment (
                               gint char_offset,
                               gint *seg_offset)
{
  TextLineSegment *seg;
  int offset;

//  g_return_val_if_fail (line != NULL, NULL);

  offset = char_offset;
  seg = segments;

  while (offset >= seg->char_count)
    {
      offset -= seg->char_count;
      seg = seg->next;
      g_assert (seg != NULL); /* means an invalid char index */
    }

  if (seg_offset)
    *seg_offset = offset;

  return seg;
}

TextLineSegment* TextLine::byte_to_any_segment (
                                   gint byte_offset,
                                   gint *seg_offset)
{
  TextLineSegment *seg;
  int offset;

//  g_return_val_if_fail (line != NULL, NULL);

  offset = byte_offset;
  seg = segments;

  while (offset > 0 && offset >= seg->byte_count)
    {
      offset -= seg->byte_count;
      seg = seg->next;
      g_assert (seg != NULL); /* means an invalid byte index */
    }

  if (seg_offset)
    *seg_offset = offset;

  return seg;
}

TextLineSegment*
_text_line_char_to_any_segment (TextLine *line,
                                   gint char_offset,
                                   gint *seg_offset)
{
  TextLineSegment *seg;
  int offset;

  g_return_val_if_fail (line != NULL, NULL);

  offset = char_offset;
  seg = line->segments;

  while (offset > 0 && offset >= seg->char_count)
    {
      offset -= seg->char_count;
      seg = seg->next;
      g_assert (seg != NULL); /* means an invalid byte index */
    }

  if (seg_offset)
    *seg_offset = offset;

  return seg;
}

gint
_text_line_byte_to_char (TextLine *line,
                            gint byte_offset)
{
  gint char_offset;
  TextLineSegment *seg;

  g_return_val_if_fail (line != NULL, 0);
  g_return_val_if_fail (byte_offset >= 0, 0);

  char_offset = 0;
  seg = line->segments;
  while (byte_offset >= seg->byte_count) /* while (we need to go farther than
                                            the next segment) */
    {
      byte_offset -= seg->byte_count;
      char_offset += seg->char_count;
      seg = seg->next;
      g_assert (seg != NULL); /* our byte_index was bogus if this happens */
    }

  g_assert (seg != NULL);

  /* Now byte_offset is the offset into the current segment,
     and char_offset is the start of the current segment.
     Optimize the case where no chars use > 1 byte */
  if (seg->byte_count == seg->char_count)
    return char_offset + byte_offset;
  else
    {
      if (seg->type == text_segment_char)
        return char_offset + g_utf8_strlen (seg->body.chars, byte_offset);
      else
        {
          g_assert (seg->char_count == 1);
          g_assert (byte_offset == 0);

          return char_offset;
        }
    }
}

gint
_text_line_char_to_byte (TextLine *line,
                            gint         char_offset)
{
  g_warning ("FIXME not implemented");

  return 0;
}

/* FIXME sync with char_locate (or figure out a clean
   way to merge the two functions) */
bool
TextLine::byte_locate (
                            gint byte_offset,
                            TextLineSegment **segment,
                            TextLineSegment **any_segment,
                            gint *seg_byte_offset,
                            gint *line_byte_offset)
{
  TextLineSegment *seg;
  TextLineSegment *after_last_indexable;
  TextLineSegment *last_indexable;
  gint offset;
  gint bytes_in_line;

//  g_return_val_if_fail (line != NULL, false);
  g_return_val_if_fail (byte_offset >= 0, false);

  *segment = NULL;
  *any_segment = NULL;
  bytes_in_line = 0;

  offset = byte_offset;

  last_indexable = NULL;
  after_last_indexable = segments;
  seg = segments;

  /* The loop ends when we're inside a segment;
     last_indexable refers to the last segment
     we passed entirely. */
  while (seg && offset >= seg->byte_count)
    {
      if (seg->char_count > 0)
        {
          offset -= seg->byte_count;
          bytes_in_line += seg->byte_count;
          last_indexable = seg;
          after_last_indexable = last_indexable->next;
        }

      seg = seg->next;
    }

  if (seg == NULL)
    {
      /* We went off the end of the line */
      if (offset != 0)
        g_warning ("%s: byte index off the end of the line", G_STRLOC);

      return false;
    }
  else
    {
      *segment = seg;
      if (after_last_indexable != NULL)
        *any_segment = after_last_indexable;
      else
        *any_segment = *segment;
    }

  /* Override any_segment if we're in the middle of a segment. */
  if (offset > 0)
    *any_segment = *segment;

  *seg_byte_offset = offset;

  g_assert (*segment != NULL);
  g_assert (*any_segment != NULL);
  g_assert (*seg_byte_offset < (*segment)->byte_count);

  *line_byte_offset = bytes_in_line + *seg_byte_offset;

  return true;
}

/* FIXME sync with byte_locate (or figure out a clean
   way to merge the two functions) */
bool TextLine::char_locate     (
                                gint              char_offset,
                                TextLineSegment **segment,
                                TextLineSegment **any_segment,
                                gint             *seg_char_offset,
                                gint             *line_char_offset)
{
  TextLineSegment *seg;
  TextLineSegment *after_last_indexable;
  TextLineSegment *last_indexable;
  gint offset;
  gint chars_in_line;

  //g_return_val_if_fail (line != NULL, false);
  g_return_val_if_fail (char_offset >= 0, false);
  
  *segment = NULL;
  *any_segment = NULL;
  chars_in_line = 0;

  offset = char_offset;

  last_indexable = NULL;
  after_last_indexable = segments;
  seg = segments;

  /* The loop ends when we're inside a segment;
     last_indexable refers to the last segment
     we passed entirely. */
  while (seg && offset >= seg->char_count)
    {
      if (seg->char_count > 0)
        {
          offset -= seg->char_count;
          chars_in_line += seg->char_count;
          last_indexable = seg;
          after_last_indexable = last_indexable->next;
        }

      seg = seg->next;
    }

  if (seg == NULL)
    {
      /* end of the line */
      if (offset != 0)
        g_warning ("%s: char offset off the end of the line", G_STRLOC);

      return false;
    }
  else
    {
      *segment = seg;
      if (after_last_indexable != NULL)
        *any_segment = after_last_indexable;
      else
        *any_segment = *segment;
    }

  /* Override any_segment if we're in the middle of a segment. */
  if (offset > 0)
    *any_segment = *segment;

  *seg_char_offset = offset;

  g_assert (*segment != NULL);
  g_assert (*any_segment != NULL);
  g_assert (*seg_char_offset < (*segment)->char_count);

  *line_char_offset = chars_in_line + *seg_char_offset;

  return true;
}

void TextLine::byte_to_char_offsets (
                                    gint byte_offset,
                                    gint *line_char_offset,
                                    gint *seg_char_offset)
{
  TextLineSegment *seg;
  int offset;

//  g_return_if_fail (line != NULL);
  g_return_if_fail (byte_offset >= 0);

  *line_char_offset = 0;

  offset = byte_offset;
  seg = segments;

  while (offset >= seg->byte_count)
    {
      offset -= seg->byte_count;
      *line_char_offset += seg->char_count;
      seg = seg->next;
      g_assert (seg != NULL); /* means an invalid char offset */
    }

  g_assert (seg->char_count > 0); /* indexable. */

  /* offset is now the number of bytes into the current segment we
   * want to go. Count chars into the current segment.
   */

  if (seg->type == text_segment_char)
    {
      *seg_char_offset = g_utf8_strlen (seg->body.chars, offset);

      g_assert (*seg_char_offset < seg->char_count);

      *line_char_offset += *seg_char_offset;
    }
  else
    {
      g_assert (offset == 0);
      *seg_char_offset = 0;
    }
}

void TextLine::char_to_byte_offsets (
                                    gint char_offset,
                                    gint *line_byte_offset,
                                    gint *seg_byte_offset)
{
  TextLineSegment *seg;
  int offset;

//  g_return_if_fail (line != NULL);
  g_return_if_fail (char_offset >= 0);

  *line_byte_offset = 0;

  offset = char_offset;
  seg = segments;

  while (offset >= seg->char_count)
    {
      offset -= seg->char_count;
      *line_byte_offset += seg->byte_count;
      seg = seg->next;
      g_assert (seg != NULL); /* means an invalid char offset */
    }

  g_assert (seg->char_count > 0); /* indexable. */

  /* offset is now the number of chars into the current segment we
     want to go. Count bytes into the current segment. */

  if (seg->type == text_segment_char)
    {
      const char *p;

      /* if in the last fourth of the segment walk backwards */
      if (seg->char_count - offset < seg->char_count / 4)
        p = g_utf8_offset_to_pointer (seg->body.chars + seg->byte_count, 
                                      offset - seg->char_count);
      else
        p = g_utf8_offset_to_pointer (seg->body.chars, offset);

      *seg_byte_offset = p - seg->body.chars;

      g_assert (*seg_byte_offset < seg->byte_count);

      *line_byte_offset += *seg_byte_offset;
    }
  else
    {
      g_assert (offset == 0);
      *seg_byte_offset = 0;
    }
}

gint
node_compare (TextBTreeNode *lhs,
              TextBTreeNode *rhs)
{
  TextBTreeNode *iter;
  TextBTreeNode *node;
  TextBTreeNode *common_parent;
  TextBTreeNode *parent_of_lower;
  TextBTreeNode *parent_of_higher;
  bool lhs_is_lower;
  TextBTreeNode *lower;
  TextBTreeNode *higher;

  /* This function assumes that lhs and rhs are not underneath each
   * other.
   */

  if (lhs == rhs)
    return 0;

  if (lhs->level < rhs->level)
    {
      lhs_is_lower = true;
      lower = lhs;
      higher = rhs;
    }
  else
    {
      lhs_is_lower = false;
      lower = rhs;
      higher = lhs;
    }

  /* Algorithm: find common parent of lhs/rhs. Save the child nodes
   * of the common parent we used to reach the common parent; the
   * ordering of these child nodes in the child list is the ordering
   * of lhs and rhs.
   */

  /* Get on the same level (may be on same level already) */
  node = lower;
  while (node->level < higher->level)
    node = node->parent;

  g_assert (node->level == higher->level);

  g_assert (node != higher); /* Happens if lower is underneath higher */

  /* Go up until we have two children with a common parent.
   */
  parent_of_lower = node;
  parent_of_higher = higher;

  while (parent_of_lower->parent != parent_of_higher->parent)
    {
      parent_of_lower = parent_of_lower->parent;
      parent_of_higher = parent_of_higher->parent;
    }

  g_assert (parent_of_lower->parent == parent_of_higher->parent);

  common_parent = parent_of_lower->parent;

  g_assert (common_parent != NULL);

  /* See which is first in the list of common_parent's children */
  iter = common_parent->children.node;
  while (iter != NULL)
    {
      if (iter == parent_of_higher)
        {
          /* higher is less than lower */

          if (lhs_is_lower)
            return 1; /* lhs > rhs */
          else
            return -1;
        }
      else if (iter == parent_of_lower)
        {
          /* lower is less than higher */

          if (lhs_is_lower)
            return -1; /* lhs < rhs */
          else
            return 1;
        }

      iter = iter->next;
    }

  g_assert_not_reached ();
  return 0;
}

/* remember that tag == NULL means "any tag" */
TextLine* TextLine::next_could_contain_tag (
					TextBTree *tree,
                                       TextTag   *tag)
{
  TextBTreeNode *node;
  TextTagInfo *info;
  bool below_tag_root;

  //g_return_val_if_fail (line != NULL, NULL);

  /*TODOif (gtk_debug_fla & GTK_DEBUG_TEXT)
    _text_btree_check (tree);*/

  if (tag == NULL)
    {
      /* Right now we can only offer linear-search if the user wants
       * to know about any tag toggle at all.
       */
      return next_excluding_last ();
    }

  /* Our tag summaries only have node precision, not line
   * precision. This means that if any line under a node could contain a
   * tag, then any of the others could also contain a tag.
   * 
   * In the future we could have some mechanism to keep track of how
   * many toggles we've found under a node so far, since we have a
   * count of toggles under the node. But for now I'm going with KISS.
   */

  /* return same-node line, if any. */
  if (next)
    return next;

  info = tree->get_existing_tag_info (tag);
  if (info == NULL)
    return NULL;

  if (info->tag_root == NULL)
    return NULL;

  if (info->tag_root == parent)
    return NULL; /* we were at the last line under the tag root */

  /* We need to go up out of this node, and on to the next one with
     toggles for the target tag. If we're below the tag root, we need to
     find the next node below the tag root that has tag summaries. If
     we're not below the tag root, we need to see if the tag root is
     after us in the tree, and if so, return the first line underneath
     the tag root. */

  node = parent;
  below_tag_root = false;
  while (node != NULL)
    {
      if (node == info->tag_root)
        {
          below_tag_root = true;
          break;
        }

      node = node->parent;
    }

  if (below_tag_root)
    {
      node = parent;
      while (node != info->tag_root)
        {
          if (node->next == NULL)
            node = node->parent;
          else
            {
              node = node->next;

              if (node->has_tag (tag))
                goto found;
            }
        }
      return NULL;
    }
  else
    {
      gint ordering;

      ordering = node_compare (parent, info->tag_root);

      if (ordering < 0)
        {
          /* Tag root is ahead of us, so search there. */
          node = info->tag_root;
          goto found;
        }
      else
        {
          /* Tag root is after us, so no more lines that
           * could contain the tag.
           */
          return NULL;
        }

      g_assert_not_reached ();
    }

 found:

  g_assert (node != NULL);

  /* We have to find the first sub-node of this node that contains
   * the target tag.
   */

  while (node->level > 0)
    {
      g_assert (node != NULL); /* If this fails, it likely means an
                                  incorrect tag summary led us on a
                                  wild goose chase down this branch of
                                  the tree. */
      node = node->children.node;
      while (node != NULL)
        {
          if (node->has_tag (tag))
            break;
          node = node->next;
        }
    }

  g_assert (node != NULL);
  g_assert (node->level == 0);

  return node->children.line;
}

TextLine* TextBTreeNode::prev_line_under_node ( TextLine      *line)
{
  TextLine *prev;

  prev = children.line;

  g_assert (prev);

  if (prev != line)
    {
      while (prev->next != line)
        prev = prev->next;

      return prev;
    }

  return NULL;
}

TextLine* TextLine::previous_could_contain_tag (TextBTree *tree,
                                          TextTag   *tag)
{
  TextBTreeNode *node;
  TextBTreeNode *found_node = NULL;
  TextTagInfo *info;
  bool below_tag_root;
  TextLine *prev;
  TextBTreeNode *line_ancestor;
  TextBTreeNode *line_ancestor_parent;

  /* See next_could_contain_tag () for more extensive comments
   * on what's going on here.
   */

  //g_return_val_if_fail (line != NULL, NULL);

  /*TODOif (gtk_debug_flags & GTK_DEBUG_TEXT)
    _text_btree_check (tree);*/

  if (tag == NULL)
    {
      /* Right now we can only offer linear-search if the user wants
       * to know about any tag toggle at all.
       */
      return previous_line ();
    }

  /* Return same-node line, if any. */
  prev = parent->prev_line_under_node (this);
  if (prev)
    return prev;

  info = tree->get_existing_tag_info (tag);
  if (info == NULL)
    return NULL;

  if (info->tag_root == NULL)
    return NULL;

  if (info->tag_root == parent)
    return NULL; /* we were at the first line under the tag root */

  /* Are we below the tag root */
  node = parent;
  below_tag_root = false;
  while (node != NULL)
    {
      if (node == info->tag_root)
        {
          below_tag_root = true;
          break;
        }

      node = node->parent;
    }

  if (below_tag_root)
    {
      /* Look for a previous node under this tag root that has our
       * tag.
       */

      /* this assertion holds because line->parent is not the
       * tag root, we are below the tag root, and the tag
       * root exists.
       */
      g_assert (parent->parent != NULL);

      line_ancestor = parent;
      line_ancestor_parent = parent->parent;

      while (line_ancestor != info->tag_root)
        {
          GSList *child_nodes = NULL;
          GSList *tmp;

          /* Create reverse-order list of nodes before
           * line_ancestor
           */
          if (line_ancestor_parent != NULL)
	    node = line_ancestor_parent->children.node;
	  else
	    node = line_ancestor;

          while (node != line_ancestor && node != NULL)
            {
              child_nodes = g_slist_prepend (child_nodes, node);

              node = node->next;
            }

          /* Try to find a node with our tag on it in the list */
          tmp = child_nodes;
          while (tmp != NULL)
            {
              TextBTreeNode *this_node = (TextBTreeNode*)tmp->data;

              g_assert (this_node != line_ancestor);

              if (this_node->has_tag (tag))
                {
                  found_node = this_node;
                  g_slist_free (child_nodes);
                  goto found;
                }

              tmp = g_slist_next (tmp);
            }

          g_slist_free (child_nodes);

          /* Didn't find anything on this level; go up one level. */
          line_ancestor = line_ancestor_parent;
          line_ancestor_parent = line_ancestor->parent;
        }

      /* No dice. */
      return NULL;
    }
  else
    {
      gint ordering;

      ordering = node_compare (parent, info->tag_root);

      if (ordering < 0)
        {
          /* Tag root is ahead of us, so no more lines
           * with this tag.
           */
          return NULL;
        }
      else
        {
          /* Tag root is after us, so grab last tagged
           * line underneath the tag root.
           */
          found_node = info->tag_root;
          goto found;
        }

      g_assert_not_reached ();
    }

 found:

  g_assert (found_node != NULL);

  /* We have to find the last sub-node of this node that contains
   * the target tag.
   */
  node = found_node;

  while (node->level > 0)
    {
      GSList *child_nodes = NULL;
      GSList *iter;
      g_assert (node != NULL); /* If this fails, it likely means an
                                  incorrect tag summary led us on a
                                  wild goose chase down this branch of
                                  the tree. */

      node = node->children.node;
      while (node != NULL)
        {
          child_nodes = g_slist_prepend (child_nodes, node);
          node = node->next;
        }

      node = NULL; /* detect failure to find a child node. */

      iter = child_nodes;
      while (iter != NULL)
        {
          if (((TextBTreeNode*)iter->data)->has_tag (tag))
            {
              /* recurse into this node. */
              node = (TextBTreeNode*)iter->data;
              break;
            }

          iter = g_slist_next (iter);
        }

      g_slist_free (child_nodes);

      g_assert (node != NULL);
    }

  g_assert (node != NULL);
  g_assert (node->level == 0);

  /* this assertion is correct, but slow. */
  /* g_assert (node_compare (node, line->parent) < 0); */

  /* Return last line in this node. */

  prev = node->children.line;
  while (prev->next)
    prev = prev->next;

  return prev;
}

/*
 * Non-public function implementations
 */

void
summary_list_destroy (Summary *summary)
{
  g_slice_free_chain (Summary, summary, next);
}

TextLine* TextBTree::get_last_line ()
{
  if (last_line_stamp != chars_changed_stamp)
    {
      gint n_lines;
      TextLine *line;
      gint real_line;

      n_lines = get_line_count ();

      g_assert (n_lines >= 1); /* num_lines doesn't return bogus last line. */

      line = get_line (n_lines, &real_line);

      last_line_stamp = chars_changed_stamp;
      last_line = line;
    }

  return last_line;
}

/*
 * Lines
 */

TextLine::TextLine ()
: parent(NULL)
, next(NULL)
, segments(NULL)
, views(NULL)
, dir_strong(NULL)
, dir_propagated_back(NULL)
, dir_propagated_forward(NULL)
{
  /*TODOline->dir_strong = PANGO_DIRECTION_NEUTRAL;
  line->dir_propagated_forward = PANGO_DIRECTION_NEUTRAL;
  line->dir_propagated_back = PANGO_DIRECTION_NEUTRAL;

  return line;*/
}

void TextBTree::text_line_destroy (TextLine *line)
{
  TextLineData *ld;
  TextLineData *next;

  g_return_if_fail (line != NULL);

  ld = line->views;
  while (ld != NULL)
    {
      BTreeView *view;

      view = get_view (ld->view_id);

      g_assert (view != NULL);

      next = ld->next;
      view->layout->free_line_data (line, ld);

      ld = next;
    }

  g_free (line); //TODO use delete?
}

void TextLine::set_parent (
                          TextBTreeNode *node)
{
  if (parent == node)
    return;
  parent = node;
  node->invalidate_upward (NULL);
}

void TextLine::cleanup_line()
{
  TextLineSegment *seg, **prev_p;
  bool changed;

  /*
   * Make a pass over all of the segments in the line, giving each
   * a chance to clean itself up.  This could potentially change
   * the structure of the line, e.g. by merging two segments
   * together or having two segments cancel themselves;  if so,
   * then repeat the whole process again, since the first structure
   * change might make other structure changes possible.  Repeat
   * until eventually there are no changes.
   */

  changed = true;
  while (changed)
    {
      changed = false;
      prev_p = &segments;
      for (seg = *prev_p; seg != NULL; seg = *prev_p)
        {
		/* we always have a cleanupFunc */
	  //TODO if (seg->cleanupFunc != NULL)
            {
              *prev_p = seg->cleanupFunc(seg, this);
              if (seg != *prev_p)
		{
		  changed = true;
		  continue;
		}
            }

	  prev_p = &(*prev_p)->next;
        }
    }
}

/*
 * Nodes
 */

NodeData*
node_data_new (gpointer view_id)
{
  NodeData *nd;
  
  nd = g_slice_new (NodeData);

  nd->view_id = view_id;
  nd->next = NULL;
  nd->width = 0;
  nd->height = 0;
  nd->valid = false;

  return nd;
}

void TextBTree::node_data_destroy (NodeData *nd)
{
  g_slice_free (NodeData, nd);
}

void
node_data_list_destroy (NodeData *nd)
{
  g_slice_free_chain (NodeData, nd, next);
}

NodeData* TextBTree::node_data_find (NodeData *nd, gpointer  view_id)
{
  while (nd != NULL)
    {
      if (nd->view_id == view_id)
        break;
      nd = nd->next;
    }
  return nd;
}

//TODO move to a summary class?
void TextBTree::summary_destroy (Summary *summary)
{
  /* Fill with error-triggering garbage */
  summary->info = (TextTagInfo*)0x1;
  summary->toggle_count = 567;
  summary->next = (Summary*)0x1;
  g_slice_free (Summary, summary);
}

TextBTreeNode::TextBTreeNode (void)
: parent(NULL)
, next(NULL)
, summary(NULL)
, level(0)
, num_children(0)
, num_lines(0)
, num_chars(0)
{
//  TextBTreeNode *node;

//  node = g_new (TextBTreeNode, 1);

  node_data = NULL;

//  return node;
}

TextBTreeNode::~TextBTreeNode(void)
{
	//TODO figure out what to do here
}

void TextBTreeNode::adjust_toggle_count (
                                         TextTagInfo  *info,
                                         gint adjust)
{
  Summary *summary;

  summary = this->summary;
  while (summary != NULL)
    {
      if (summary->info == info)
        {
          summary->toggle_count += adjust;
          break;
        }

      summary = summary->next;
    }

  if (summary == NULL)
    {
      /* didn't find a summary for our tag. */
      g_return_if_fail (adjust > 0);
      summary = g_slice_new (Summary);
      summary->info = info;
      summary->toggle_count = adjust;
      summary->next = summary;
      summary = summary;
    }
}

/* Note that the tag root and above do not have summaries
   for the tag; only nodes below the tag root have
   the summaries. */
bool TextBTreeNode::has_tag (TextTag *tag)
{
  Summary *summary;

  summary = this->summary;
  while (summary != NULL)
    {
      if (tag == NULL ||
          summary->info->tag == tag)
        return true;

      summary = summary->next;
    }

  return false;
}

/* Add node and all children to the damage region. */
void TextBTreeNode::invalidate_downward (void)
{
  NodeData *nd;

  nd = node_data;
  while (nd != NULL)
    {
      nd->valid = false;
      nd = nd->next;
    }

  if (level == 0)
    {
      TextLine *line;

      line = children.line;
      while (line != NULL)
        {
          TextLineData *ld;

          ld = line->views;
          while (ld != NULL)
            {
              ld->valid = false;
              ld = ld->next;
            }

          line = line->next;
        }
    }
  else
    {
      TextBTreeNode *child;

      child = children.node;

      while (child != NULL)
        {
          child->invalidate_downward ();

          child = child->next;
        }
    }
}

void TextBTreeNode::invalidate_upward (gpointer view_id)
{
  TextBTreeNode *iter;

  iter = this;
  while (iter != NULL)
    {
      NodeData *nd;

      if (view_id)
        {
          nd = TextBTree::node_data_find (iter->node_data, view_id);

          if (nd == NULL || !nd->valid)
            break; /* Once a node is invalid, we know its parents are as well. */

          nd->valid = false;
        }
      else
        {
          bool should_continue = false;

          nd = iter->node_data;
          while (nd != NULL)
            {
              if (nd->valid)
                {
                  should_continue = true;
                  nd->valid = false;
                }

              nd = nd->next;
            }

          if (!should_continue)
            break; /* This node was totally invalidated, so are its
                      parents */
        }

      iter = iter->parent;
    }
}


/**
 * _text_btree_is_valid:
 * @tree: a #TextBTree
 * @view_id: ID for the view
 *
 * Check to see if the entire #TextBTree is valid or not for
 * the given view.
 *
 * Return value: %true if the entire #TextBTree is valid
 **/
bool TextBTree::is_valid (
                         gpointer      view_id)
{
  NodeData *nd;
//  g_return_val_if_fail (tree != NULL, false);

  nd = node_data_find (root_node->node_data, view_id);
  return (nd && nd->valid);
}

void TextBTreeNode::validate (BTreeView         *view,
                              gpointer           view_id,
                              ValidateState     *state)
{
  gint node_valid = true;
  gint node_width = 0;
  gint node_height = 0;

  NodeData *nd = ensure_data (view_id);
  g_return_if_fail (!nd->valid);

  if (level == 0)
    {
      TextLine *line = children.line;
      TextLineData *ld;

      /* Iterate over leading valid lines */
      while (line != NULL)
        {
          ld = line->get_data (view_id);

          if (!ld || !ld->valid)
            break;
          else if (state->in_validation)
            {
              state->in_validation = false;
              return;
            }
          else
            {
              state->y += ld->height;
              node_width = MAX (ld->width, node_width);
              node_height += ld->height;
            }

          line = line->next;
        }

      state->in_validation = true;

      /* Iterate over invalid lines */
      while (line != NULL)
        {
          ld = line->get_data (view_id);

          if (ld && ld->valid)
            break;
          else
            {
              if (ld)
                state->old_height += ld->height;
              ld = view->layout->wrap (line, ld);
              state->new_height += ld->height;

              node_width = MAX (ld->width, node_width);
              node_height += ld->height;

              state->remaining_pixels -= ld->height;
              if (state->remaining_pixels <= 0)
                {
                  line = line->next;
                  break;
                }
            }

          line = line->next;
        }

      /* Iterate over the remaining lines */
      while (line != NULL)
        {
          ld = line->get_data (view_id);
          state->in_validation = false;

          if (!ld || !ld->valid)
            node_valid = false;

          if (ld)
            {
              node_width = MAX (ld->width, node_width);
              node_height += ld->height;
            }

          line = line->next;
        }
    }
  else
    {
      TextBTreeNode *child;
      NodeData *child_nd;

      child = children.node;

      /* Iterate over leading valid nodes */
      while (child)
        {
          child_nd = child->ensure_data (view_id);

          if (!child_nd->valid)
            break;
          else if (state->in_validation)
            {
              state->in_validation = false;
              return;
            }
          else
            {
              state->y += child_nd->height;
              node_width = MAX (node_width, child_nd->width);
              node_height += child_nd->height;
            }

          child = child->next;
        }

      /* Iterate over invalid nodes */
      while (child)
        {
          child_nd = child->ensure_data (view_id);

          if (child_nd->valid)
            break;
          else
            {
              child->validate (view, view_id, state);

              if (!child_nd->valid)
                node_valid = false;
              node_width = MAX (node_width, child_nd->width);
              node_height += child_nd->height;

              if (!state->in_validation || state->remaining_pixels <= 0)
                {
                  child = child->next;
                  break;
                }
            }

          child = child->next;
        }

      /* Iterate over the remaining lines */
      while (child)
        {
          child_nd = child->ensure_data (view_id);
          state->in_validation = false;

          if (!child_nd->valid)
            node_valid = false;

          node_width = MAX (child_nd->width, node_width);
          node_height += child_nd->height;

          child = child->next;
        }
    }

  nd->width = node_width;
  nd->height = node_height;
  nd->valid = node_valid;
}

/**
 * _text_btree_validate:
 * @tree: a #TextBTree
 * @view_id: view id
 * @max_pixels: the maximum number of pixels to validate. (No more
 *              than one paragraph beyond this limit will be validated)
 * @y: location to store starting y coordinate of validated region
 * @old_height: location to store old height of validated region
 * @new_height: location to store new height of validated region
 *
 * Validate a single contiguous invalid region of a #TextBTree for
 * a given view.
 *
 * Return value: %true if a region has been validated, %false if the
 * entire tree was already valid.
 **/
bool TextBTree::validate (
                         gpointer      view_id,
                         gint          max_pixels,
                         gint         *y,
                         gint         *old_height,
                         gint         *new_height)
{
  BTreeView *view;

 // g_return_val_if_fail (tree != NULL, false);

  view = get_view (view_id);
  g_return_val_if_fail (view != NULL, false);

  if (!is_valid (view_id))
    {
      ValidateState state;

      state.remaining_pixels = max_pixels;
      state.in_validation = false;
      state.y = 0;
      state.old_height = 0;
      state.new_height = 0;

      root_node->validate (view, view_id, &state);

      if (y)
        *y = state.y;
      if (old_height)
        *old_height = state.old_height;
      if (new_height)
        *new_height = state.new_height;

      /*TODOif (gtk_debug_flags & GTK_DEBUG_TEXT)
        _text_btree_check (tree);*/

      return true;
    }
  else
    return false;
}

void TextBTreeNode::compute_view_aggregates (
                                             gpointer          view_id,
                                             gint             *width_out,
                                             gint             *height_out,
                                             bool         *valid_out)
{
  gint width = 0;
  gint height = 0;
  bool valid = true;

  if (level == 0)
    {
      TextLine *line = children.line;

      while (line != NULL)
        {
          TextLineData *ld = (TextLineData*)line->get_data (view_id);

          if (!ld || !ld->valid)
            valid = false;

          if (ld)
            {
              width = MAX (ld->width, width);
              height += ld->height;
            }

          line = line->next;
        }
    }
  else
    {
      TextBTreeNode *child = children.node;

      while (child)
        {
          NodeData *child_nd = TextBTree::node_data_find (child->node_data, view_id);

          if (!child_nd || !child_nd->valid)
            valid = false;

          if (child_nd)
            {
              width = MAX (child_nd->width, width);
              height += child_nd->height;
            }

          child = child->next;
        }
    }

  *width_out = width;
  *height_out = height;
  *valid_out = valid;
}


/* Recompute the validity and size of the view data for a given
 * view at this node from the immediate children of the node
 */
NodeData * TextBTreeNode::check_valid (
                                 gpointer          view_id)
{
  NodeData *nd = ensure_data (view_id);
  bool valid;
  gint width;
  gint height;

  compute_view_aggregates (view_id, &width, &height, &valid);
  nd->width = width;
  nd->height = height;
  nd->valid = valid;

  return nd;
}

void TextBTreeNode::check_valid_upward (TextBTreeNode *node,
                                        gpointer          view_id)
{
  while (node)
    {
      node->check_valid (view_id);
      node = node->parent;
    }
}

NodeData * TextBTreeNode::check_valid_downward (
                                          gpointer          view_id)
{
  if (level == 0)
    {
      return check_valid (view_id);
    }
  else
    {
      TextBTreeNode *child = children.node;

      NodeData *nd = ensure_data (view_id);

      nd->valid = true;
      nd->width = 0;
      nd->height = 0;

      while (child)
        {
          NodeData *child_nd = child->check_valid_downward (view_id);

          if (!child_nd->valid)
            nd->valid = false;
          nd->width = MAX (child_nd->width, nd->width);
          nd->height += child_nd->height;

          child = child->next;
        }
      return nd;
    }
}



/**
 * _text_btree_validate_line:
 * @tree: a #TextBTree
 * @line: line to validate
 * @view_id: view ID for the view to validate
 *
 * Revalidate a single line of the btree for the given view, propagate
 * results up through the entire tree.
 **/
void TextBTree::validate_line (
                               TextLine      *line,
                               gpointer          view_id)
{
  TextLineData *ld;
  BTreeView *view;

  //g_return_if_fail (tree != NULL);
  g_return_if_fail (line != NULL);

  view = get_view (view_id);
  g_return_if_fail (view != NULL);
  
  ld = (TextLineData*)line->get_data (view_id);
  if (!ld || !ld->valid)
    {
      ld = view->layout->wrap (line, ld);
      
      TextBTreeNode::check_valid_upward (line->parent, view_id);
    }
}

void TextBTreeNode::remove_view (BTreeView *view, gpointer view_id)
{
  if (level == 0)
    {
      TextLine *line;

      line = children.line;
      while (line != NULL)
        {
          TextLineData *ld;

          ld = line->remove_data (view_id);

          if (ld)
            view->layout->free_line_data (line, ld);

          line = line->next;
        }
    }
  else
    {
      TextBTreeNode *child;

      child = children.node;

      while (child != NULL)
        {
          /* recurse */
          child->remove_view (view, view_id);

          child = child->next;
        }
    }

  remove_data (view_id);
}

void TextBTreeNode::node_destroy (TextBTree *tree, TextBTreeNode *node)
{
  if (node->level == 0)
    {
      TextLine *line;
      TextLineSegment *seg;

      while (node->children.line != NULL)
        {
          line = node->children.line;
          node->children.line = line->next;
          while (line->segments != NULL)
            {
              seg = line->segments;
              line->segments = seg->next;

              seg->deleteFunc (line, true);
            }
          tree->text_line_destroy (line);
        }
    }
  else
    {
      TextBTreeNode *childPtr;

      while (node->children.node != NULL)
        {
          childPtr = node->children.node;
          node->children.node = childPtr->next;
          node_destroy (tree, childPtr);
        }
    }

  TextBTreeNode::free_empty (node);
}

 void TextBTreeNode::free_empty ( TextBTreeNode *node)
{
  g_return_if_fail ((node->level > 0 && node->children.node == NULL) ||
                    (node->level == 0 && node->children.line == NULL));

  summary_list_destroy (node->summary);
  node_data_list_destroy (node->node_data);
  delete node;
}

NodeData* TextBTreeNode::ensure_data (gpointer view_id)
{
  NodeData *nd;

  nd = node_data;
  while (nd != NULL)
    {
      if (nd->view_id == view_id)
        break;

      nd = nd->next;
    }

  if (nd == NULL)
    {
      nd = node_data_new (view_id);
      
      if (node_data)
        nd->next = node_data;
      
      node_data = nd;
    }

  return nd;
}

 void TextBTreeNode::remove_data (gpointer view_id)
{
  NodeData *nd;
  NodeData *prev;

  prev = NULL;
  nd = node_data;
  while (nd != NULL)
    {
      if (nd->view_id == view_id)
        break;

      prev = nd;
      nd = nd->next;
    }

  if (nd == NULL)
    return;

  if (prev != NULL)
    prev->next = nd->next;

  if (node_data == nd)
    node_data = nd->next;

  nd->next = NULL;

  TextBTree::node_data_destroy (nd);
}

void TextBTreeNode::get_size (gpointer view_id,
                              gint *width, gint *height)
{
  NodeData *nd;

  g_return_if_fail (width != NULL);
  g_return_if_fail (height != NULL);

  nd = ensure_data (view_id);

  if (width)
    *width = nd->width;
  if (height)
    *height = nd->height;
}

/* Find the closest common ancestor of the two nodes. FIXME: The interface
 * here isn't quite right, since for a lot of operations we want to
 * know which children of the common parent correspond to the two nodes
 * (e.g., when computing the order of two iters)
 */
TextBTreeNode * TextBTreeNode::common_parent (TextBTreeNode *node1,
                                   TextBTreeNode *node2)
{
  while (node1->level < node2->level)
    node1 = node1->parent;
  while (node2->level < node1->level)
    node2 = node2->parent;
  while (node1 != node2)
    {
      node1 = node1->parent;
      node2 = node2->parent;
    }

  return node1;
}

/*
 * BTree
 */

 BTreeView* TextBTree::get_view (gpointer view_id)
{
  BTreeView *view;

  view = views;
  while (view != NULL)
    {
      if (view->view_id == view_id)
        break;
      view = view->next;
    }

  return view;
}

 void TextBTree::get_tree_bounds (
                 TextIter *start,
                 TextIter *end)
{
  get_iter_at_line_char (start, 0, 0);
  get_end_iter (end);
}

 void TextBTree::tag_changed_cb (TextTagTable *table,
                TextTag      *tag,
                bool         size_changed,
                TextBTree    *tree)
{
  if (size_changed)
    {
      /* We need to queue a relayout on all regions that are tagged with
       * this tag.
       */

      TextIter start;
      TextIter end;

      if (tree->get_iter_at_first_toggle (&start, tag))
        {
          /* Must be a last toggle if there was a first one. */
          tree->get_iter_at_last_toggle (&end, tag);
          DV (g_print ("invalidating due to tag change (%s)\n", G_STRLOC));
          tree->invalidate_region (&start, &end, false);

        }
    }
  else
    {
      /* We only need to queue a redraw, not a relayout */
      BTreeView *view;

      view = tree->views;

      while (view != NULL)
        {
          gint width, height;

          get_view_size (view->view_id, &width, &height);
          view->layout->changed (0, height, height);

          view = view->next;
        }
    }
}

void TextBTree::notify_will_remove_tag (
                                        TextTag      *tag)
{
  /* Remove the tag from the tree */

  TextIter start;
  TextIter end;

  TextBTree::get_tree_bounds (&start, &end);

  TextBTree::tag (&start, &end, tag, false);
  remove_tag_info (tag);
}


/* Rebalance the out-of-whack node "node" */
 void TextBTree::rebalance (
                          TextBTreeNode *node)
{
  /*
   * Loop over the entire ancestral chain of the TextBTreeNode, working
   * up through the tree one TextBTreeNode at a time until the root
   * TextBTreeNode has been processed.
   */

  while (node != NULL)
    {
      TextBTreeNode *new_node, *child;
      TextLine *line;
      int i;

      /*
       * Check to see if the TextBTreeNode has too many children.  If it does,
       * then split off all but the first MIN_CHILDREN into a separate
       * TextBTreeNode following the original one.  Then repeat until the
       * TextBTreeNode has a decent size.
       */

      if (node->num_children > MAX_CHILDREN)
        {
          while (1)
            {
              /*
               * If the TextBTreeNode being split is the root
               * TextBTreeNode, then make a new root TextBTreeNode above
               * it first.
               */

              if (node->parent == NULL)
                {
                  new_node = new TextBTreeNode ();
                  new_node->parent = NULL;
                  new_node->next = NULL;
                  new_node->summary = NULL;
                  new_node->level = node->level + 1;
                  new_node->children.node = node;
                  recompute_node_counts (new_node);
                  root_node = new_node;
                }
              new_node = new TextBTreeNode ();
              new_node->parent = node->parent;
              new_node->next = node->next;
              node->next = new_node;
              new_node->summary = NULL;
              new_node->level = node->level;
              new_node->num_children = node->num_children - MIN_CHILDREN;
              if (node->level == 0)
                {
                  for (i = MIN_CHILDREN-1,
                         line = node->children.line;
                       i > 0; i--, line = line->next)
                    {
                      /* Empty loop body. */
                    }
                  new_node->children.line = line->next;
                  line->next = NULL;
                }
              else
                {
                  for (i = MIN_CHILDREN-1,
                         child = node->children.node;
                       i > 0; i--, child = child->next)
                    {
                      /* Empty loop body. */
                    }
                  new_node->children.node = child->next;
                  child->next = NULL;
                }
              recompute_node_counts (node);
              node->parent->num_children++;
              node = new_node;
              if (node->num_children <= MAX_CHILDREN)
                {
                  recompute_node_counts (node);
                  break;
                }
            }
        }

      while (node->num_children < MIN_CHILDREN)
        {
          TextBTreeNode *other;
          TextBTreeNode *halfwaynode = NULL; /* Initialization needed only */
          TextLine *halfwayline = NULL; /* to prevent cc warnings. */
          int total_children, first_children, i;

          /*
           * Too few children for this TextBTreeNode.  If this is the root then,
           * it's OK for it to have less than MIN_CHILDREN children
           * as long as it's got at least two.  If it has only one
           * (and isn't at level 0), then chop the root TextBTreeNode out of
           * the tree and use its child as the new root.
           */

          if (node->parent == NULL)
            {
              if ((node->num_children == 1) && (node->level > 0))
                {
                  root_node = node->children.node;
                  root_node->parent = NULL;

                  node->children.node = NULL;
		  TextBTreeNode::free_empty (node);
                }
              return;
            }

          /*
           * Not the root.  Make sure that there are siblings to
           * balance with.
           */

          if (node->parent->num_children < 2)
            {
              rebalance (node->parent);
              continue;
            }

          /*
           * Find a sibling neighbor to borrow from, and arrange for
           * node to be the earlier of the pair.
           */

          if (node->next == NULL)
            {
              for (other = node->parent->children.node;
                   other->next != node;
                   other = other->next)
                {
                  /* Empty loop body. */
                }
              node = other;
            }
          other = node->next;

          /*
           * We're going to either merge the two siblings together
           * into one TextBTreeNode or redivide the children among them to
           * balance their loads.  As preparation, join their two
           * child lists into a single list and remember the half-way
           * point in the list.
           */

          total_children = node->num_children + other->num_children;
          first_children = total_children/2;
          if (node->children.node == NULL)
            {
              node->children = other->children;
              other->children.node = NULL;
              other->children.line = NULL;
            }
          if (node->level == 0)
            {
              TextLine *line;

              for (line = node->children.line, i = 1;
                   line->next != NULL;
                   line = line->next, i++)
                {
                  if (i == first_children)
                    {
                      halfwayline = line;
                    }
                }
              line->next = other->children.line;
              while (i <= first_children)
                {
                  halfwayline = line;
                  line = line->next;
                  i++;
                }
            }
          else
            {
              TextBTreeNode *child;

              for (child = node->children.node, i = 1;
                   child->next != NULL;
                   child = child->next, i++)
                {
                  if (i <= first_children)
                    {
                      if (i == first_children)
                        {
                          halfwaynode = child;
                        }
                    }
                }
              child->next = other->children.node;
              while (i <= first_children)
                {
                  halfwaynode = child;
                  child = child->next;
                  i++;
                }
            }

          /*
           * If the two siblings can simply be merged together, do it.
           */

          if (total_children <= MAX_CHILDREN)
            {
              recompute_node_counts (node);
              node->next = other->next;
              node->parent->num_children--;

              other->children.node = NULL;
              other->children.line = NULL;
	      TextBTreeNode::free_empty (other);
              continue;
            }

          /*
           * The siblings can't be merged, so just divide their
           * children evenly between them.
           */

          if (node->level == 0)
            {
              other->children.line = halfwayline->next;
              halfwayline->next = NULL;
            }
          else
            {
              other->children.node = halfwaynode->next;
              halfwaynode->next = NULL;
            }

          recompute_node_counts (node);
          recompute_node_counts (other);
        }

      node = node->parent;
    }
}

void TextBTree::post_insert_fixup (
                   TextLine *line,
                   gint line_count_delta,
                   gint char_count_delta)

{
  TextBTreeNode *node;

  /*
   * Increment the line counts in all the parent TextBTreeNodes of the insertion
   * point, then rebalance the tree if necessary.
   */

  for (node = line->parent ; node != NULL;
       node = node->parent)
    {
      node->num_lines += line_count_delta;
      node->num_chars += char_count_delta;
    }
  node = line->parent;
  node->num_children += line_count_delta;

  if (node->num_children > MAX_CHILDREN)
    {
      rebalance (node);
    }

  /*TODOif (gtk_debug_flags & GTK_DEBUG_TEXT)
    _text_btree_check (tree);*/
}

 TextTagInfo* TextBTree::get_existing_tag_info (
                                      TextTag   *tag)
{
  TextTagInfo *info;
  GSList *list;


  list = tag_infos;
  while (list != NULL)
    {
      info = (TextTagInfo*)list->data; //TODO use texttaginfo type for data?
      if (info->tag == tag)
        return info;

      list = g_slist_next (list);
    }

  return NULL;
}

 TextTagInfo* TextBTree::get_tag_info (
                             TextTag   *tag)
{
  TextTagInfo *info;

  info = get_existing_tag_info (tag);

  if (info == NULL)
    {
      /* didn't find it, create. */

      info = g_slice_new (TextTagInfo);

      info->tag = tag;
      //TODOg_object_ref (tag);
      info->tag_root = NULL;
      info->toggle_count = 0;

      tag_infos = g_slist_prepend (tag_infos, info);

#if 0
      g_print ("Created tag info %p for tag %s(%p)\n",
               info, info->tag->name ? info->tag->name : "anon",
               info->tag);
#endif
    }

  return info;
}

 void TextBTree::remove_tag_info ( TextTag   *tag)
{
  TextTagInfo *info;
  GSList *list;
  GSList *prev;

  prev = NULL;
  list = tag_infos;
  while (list != NULL)
    {
      info = (TextTagInfo*)list->data; //TODO change datatype for data
      if (info->tag == tag)
        {
#if 0
          g_print ("Removing tag info %p for tag %s(%p)\n",
                   info, info->tag->name ? info->tag->name : "anon",
                   info->tag);
#endif
          
          if (prev != NULL)
            {
              prev->next = list->next;
            }
          else
            {
              tag_infos = list->next;
            }
          list->next = NULL;
          g_slist_free (list);

          //TODOg_object_unref (info->tag);

          g_slice_free (TextTagInfo, info);
          return;
        }

      prev = list;
      list = g_slist_next (list);
    }
}

 void TextBTreeNode::recompute_level_zero_counts (void)
{
  TextLine *line;
  TextLineSegment *seg;

  g_assert (level == 0);

  line = children.line;
  while (line != NULL)
    {
      num_children++;
      num_lines++;

      if (line->parent != this)
        line->set_parent (this);

      seg = line->segments;
      while (seg != NULL)
        {

          num_chars += seg->char_count;

          if (((seg->type != text_segment_toggle_on)
               && (seg->type != text_segment_toggle_off))
              || !(seg->body.toggle.inNodeCounts))
            {
              ; /* nothing */
            }
          else
            {
              TextTagInfo *info;

              info = seg->body.toggle.info;

              adjust_toggle_count (info, 1);
            }

          seg = seg->next;
        }

      line = line->next;
    }
}

 void TextBTreeNode::recompute_level_nonzero_counts (void)
{
  Summary *summary;
  TextBTreeNode *child;

  g_assert (level > 0);

  child = children.node;
  while (child != NULL)
    {
      num_children += 1;
      num_lines += child->num_lines;
      num_chars += child->num_chars;

      if (child->parent != this)
        {
          child->parent = this;
          invalidate_upward (NULL);
        }

      summary = child->summary;
      while (summary != NULL)
        {
          adjust_toggle_count ( summary->info, summary->toggle_count);

          summary = summary->next;
        }

      child = child->next;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * recompute_node_counts --
 *
 *      This procedure is called to recompute all the counts in a TextBTreeNode
 *      (tags, child information, etc.) by scanning the information in
 *      its descendants.  This procedure is called during rebalancing
 *      when a TextBTreeNode's child structure has changed.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      The tag counts for node are modified to reflect its current
 *      child structure, as are its num_children, num_lines, num_chars fields.
 *      Also, all of the childrens' parent fields are made to point
 *      to node.
 *
 *----------------------------------------------------------------------
 */

 void TextBTree::recompute_node_counts (TextBTreeNode *node)
{
  BTreeView *view;
  Summary *summary, *summary2;

  /*
   * Zero out all the existing counts for the TextBTreeNode, but don't delete
   * the existing Summary records (most of them will probably be reused).
   */

  summary = node->summary;
  while (summary != NULL)
    {
      summary->toggle_count = 0;
      summary = summary->next;
    }

  node->num_children = 0;
  node->num_lines = 0;
  node->num_chars = 0;

  /*
   * Scan through the children, adding the childrens' tag counts into
   * the TextBTreeNode's tag counts and adding new Summary structures if
   * necessary.
   */

  if (node->level == 0)
    node->recompute_level_zero_counts ();
  else
    node->recompute_level_nonzero_counts ();

  view = views;
  while (view)
    {
      node->check_valid (view->view_id);
      view = view->next;
    }
  
  /*
   * Scan through the TextBTreeNode's tag records again and delete any Summary
   * records that still have a zero count, or that have all the toggles.
   * The TextBTreeNode with the children that account for all the tags toggles
   * have no summary information, and they become the tag_root for the tag.
   */

  summary2 = NULL;
  for (summary = node->summary; summary != NULL; )
    {
      if (summary->toggle_count > 0 &&
          summary->toggle_count < summary->info->toggle_count)
        {
          if (node->level == summary->info->tag_root->level)
            {
              /*
               * The tag's root TextBTreeNode split and some toggles left.
               * The tag root must move up a level.
               */
              summary->info->tag_root = node->parent;
            }
          summary2 = summary;
          summary = summary->next;
          continue;
        }
      if (summary->toggle_count == summary->info->toggle_count)
        {
          /*
           * A TextBTreeNode merge has collected all the toggles under
           * one TextBTreeNode.  Push the root down to this level.
           */
          summary->info->tag_root = node;
        }
      if (summary2 != NULL)
        {
          summary2->next = summary->next;
          summary_destroy (summary);
          summary = summary2->next;
        }
      else
        {
          node->summary = summary->next;
          summary_destroy (summary);
          summary = node->summary;
        }
    }
}

void TextBTreeNode::change_node_toggle_count (
                               TextTagInfo   *info,
                               gint              delta) /* may be negative */
{
  Summary *summary, *prevPtr;
  TextBTreeNode *node2Ptr;
  int rootLevel;                        /* Level of original tag root */
  TextBTreeNode *node;

  node = this;

  info->toggle_count += delta;

  if (info->tag_root == (TextBTreeNode *) NULL)
    {
      info->tag_root = node;
      return;
    }

  /*
   * Note the level of the existing root for the tag so we can detect
   * if it needs to be moved because of the toggle count change.
   */

  rootLevel = info->tag_root->level;

  /*
   * Iterate over the TextBTreeNode and its ancestors up to the tag root, adjusting
   * summary counts at each TextBTreeNode and moving the tag's root upwards if
   * necessary.
   */

  for ( ; node != info->tag_root; node = node->parent)
    {
      /*
       * See if there's already an entry for this tag for this TextBTreeNode.  If so,
       * perhaps all we have to do is adjust its count.
       */

      for (prevPtr = NULL, summary = node->summary;
           summary != NULL;
           prevPtr = summary, summary = summary->next)
        {
          if (summary->info == info)
            {
              break;
            }
        }
      if (summary != NULL)
        {
          summary->toggle_count += delta;
          if (summary->toggle_count > 0 &&
              summary->toggle_count < info->toggle_count)
            {
              continue;
            }
          if (summary->toggle_count != 0)
            {
              /*
               * Should never find a TextBTreeNode with max toggle count at this
               * point (there shouldn't have been a summary entry in the
               * first place).
               */

              g_error ("%s: bad toggle count (%d) max (%d)",
                       G_STRLOC, summary->toggle_count, info->toggle_count);
            }

          /*
           * Zero toggle count;  must remove this tag from the list.
           */

          if (prevPtr == NULL)
            {
              node->summary = summary->next;
            }
          else
            {
              prevPtr->next = summary->next;
            }
	  TextBTree::summary_destroy (summary);
        }
      else
        {
          /*
           * This tag isn't currently in the summary information list.
           */

          if (rootLevel == node->level)
            {

              /*
               * The old tag root is at the same level in the tree as this
               * TextBTreeNode, but it isn't at this TextBTreeNode.  Move the tag root up
               * a level, in the hopes that it will now cover this TextBTreeNode
               * as well as the old root (if not, we'll move it up again
               * the next time through the loop).  To push it up one level
               * we copy the original toggle count into the summary
               * information at the old root and change the root to its
               * parent TextBTreeNode.
               */

              TextBTreeNode *rootnode = info->tag_root;
              summary = g_slice_new (Summary);
              summary->info = info;
              summary->toggle_count = info->toggle_count - delta;
              summary->next = rootnode->summary;
              rootnode->summary = summary;
              rootnode = rootnode->parent;
              rootLevel = rootnode->level;
              info->tag_root = rootnode;
            }
          summary = g_slice_new (Summary);
          summary->info = info;
          summary->toggle_count = delta;
          summary->next = node->summary;
          node->summary = summary;
        }
    }

  /*
   * If we've decremented the toggle count, then it may be necessary
   * to push the tag root down one or more levels.
   */

  if (delta >= 0)
    {
      return;
    }
  if (info->toggle_count == 0)
    {
      info->tag_root = (TextBTreeNode *) NULL;
      return;
    }
  node = info->tag_root;
  while (node->level > 0)
    {
      /*
       * See if a single child TextBTreeNode accounts for all of the tag's
       * toggles.  If so, push the root down one level.
       */

      for (node2Ptr = node->children.node;
           node2Ptr != (TextBTreeNode *)NULL ;
           node2Ptr = node2Ptr->next)
        {
          for (prevPtr = NULL, summary = node2Ptr->summary;
               summary != NULL;
               prevPtr = summary, summary = summary->next)
            {
              if (summary->info == info)
                {
                  break;
                }
            }
          if (summary == NULL)
            {
              continue;
            }
          if (summary->toggle_count != info->toggle_count)
            {
              /*
               * No TextBTreeNode has all toggles, so the root is still valid.
               */

              return;
            }

          /*
           * This TextBTreeNode has all the toggles, so push down the root.
           */

          if (prevPtr == NULL)
            {
              node2Ptr->summary = summary->next;
            }
          else
            {
              prevPtr->next = summary->next;
            }
	  TextBTree::summary_destroy (summary);
          info->tag_root = node2Ptr;
          break;
        }
      node = info->tag_root;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * inc_count --
 *
 *      This is a utility procedure used by _text_btree_get_tags.  It
 *      increments the count for a particular tag, adding a new
 *      entry for that tag if there wasn't one previously.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      The information at *tagInfoPtr may be modified, and the arrays
 *      may be reallocated to make them larger.
 *
 *----------------------------------------------------------------------
 */

 void TextBTree::inc_count (TextTag *tag, int inc, TagInfo *tagInfoPtr)
{
  TextTag **tag_p;
  int count;

  for (tag_p = tagInfoPtr->tags, count = tagInfoPtr->numTags;
       count > 0; tag_p++, count--)
    {
      if (*tag_p == tag)
        {
          tagInfoPtr->counts[tagInfoPtr->numTags-count] += inc;
          return;
        }
    }

  /*
   * There isn't currently an entry for this tag, so we have to
   * make a new one.  If the arrays are full, then enlarge the
   * arrays first.
   */

  if (tagInfoPtr->numTags == tagInfoPtr->arraySize)
    {
      TextTag **newTags;
      int *newCounts, newSize;

      newSize = 2*tagInfoPtr->arraySize;
      newTags = (TextTag **) g_malloc ((unsigned)
                                          (newSize*sizeof (TextTag *)));
      memcpy ((void *) newTags, (void *) tagInfoPtr->tags,
              tagInfoPtr->arraySize  *sizeof (TextTag *));
      g_free ((char *) tagInfoPtr->tags);
      tagInfoPtr->tags = newTags;
      newCounts = (int *) g_malloc ((unsigned) (newSize*sizeof (int)));
      memcpy ((void *) newCounts, (void *) tagInfoPtr->counts,
              tagInfoPtr->arraySize  *sizeof (int));
      g_free ((char *) tagInfoPtr->counts);
      tagInfoPtr->counts = newCounts;
      tagInfoPtr->arraySize = newSize;
    }

  tagInfoPtr->tags[tagInfoPtr->numTags] = tag;
  tagInfoPtr->counts[tagInfoPtr->numTags] = inc;
  tagInfoPtr->numTags++;
}

//TODO move to textsegment.h
 void TextBTree::link_segment (TextLineSegment *seg,
                             TextIter *iter)
{
  TextLineSegment *prev;
  TextLine *line;
  TextBTree *tree;

  line = iter->get_text_line ();
  tree = iter->get_btree();

  prev = TextLineSegment::split_segment (iter);
  if (prev == NULL)
    {
      seg->next = line->segments;
      line->segments = seg;
    }
  else
    {
      seg->next = prev->next;
      prev->next = seg;
    }
  line->cleanup_line();
  tree->segments_changed ();

  /*TODOif (gtk_debug_flags & GTK_DEBUG_TEXT)
    _text_btree_check (tree);*/
}

 void TextBTree::unlink_segment ( TextLineSegment *seg, TextLine *line)
{
  TextLineSegment *prev;

  if (line->segments == seg)
    {
      line->segments = seg->next;
    }
  else
    {
      for (prev = line->segments; prev->next != seg;
           prev = prev->next)
        {
          /* Empty loop body. */
        }
      prev->next = seg->next;
    }
  line->cleanup_line();
  segments_changed ();
}

/*
 * This is here because it requires BTree internals, it logically
 * belongs in gtktextsegment.c
 */




/*
 * Debug
 */

 void TextBTree::node_view_check_consistency (
                                            TextBTreeNode *node,
                                            NodeData         *nd)
{
  gint width;
  gint height;
  bool valid;
  BTreeView *view;
  
  view = views;

  while (view != NULL)
    {
      if (view->view_id == nd->view_id)
        break;

      view = view->next;
    }

  if (view == NULL)
    g_error ("Node has data for a view %p no longer attached to the tree",
             nd->view_id);
  
  node->compute_view_aggregates (nd->view_id,
                                               &width, &height, &valid);

  /* valid aggregate not checked the same as width/height, because on
   * btree rebalance we can have invalid nodes where all lines below
   * them are actually valid, due to moving lines around between
   * nodes.
   *
   * The guarantee is that if there are invalid lines the node is
   * invalid - we don't guarantee that if the node is invalid there
   * are invalid lines.
   */
  
  if (nd->width != width ||
      nd->height != height ||
      (nd->valid && !valid))
    {
      g_error ("Node aggregates for view %p are invalid:\n"
               "Are (%d,%d,%s), should be (%d,%d,%s)",
               nd->view_id,
               nd->width, nd->height, nd->valid ? "true" : "false",
               width, height, valid ? "true" : "false");
    }
}

 void TextBTreeNode::check_consistency (TextBTree *tree)
{
  TextBTreeNode *childnode;
  Summary *summary, *summary2;
  TextLine *line;
  TextLineSegment *segPtr;
  int num_children, num_lines, num_chars, toggle_count, min_children;
  TextLineData *ld;
  NodeData *nd;

  if (parent != NULL)
    {
      min_children = MIN_CHILDREN;
    }
  else if (level > 0)
    {
      min_children = 2;
    }
  else  {
    min_children = 1;
  }
  if ((num_children < min_children)
      || (num_children > MAX_CHILDREN))
    {
      g_error ("text_btree_node_check_consistency: bad child count (%d)",
               num_children);
    }

  nd = node_data;
  while (nd != NULL)
    {
      tree->node_view_check_consistency (this, nd);
      nd = nd->next;
    }

  num_children = 0;
  num_lines = 0;
  num_chars = 0;
  if (level == 0)
    {
      for (line = children.line; line != NULL;
           line = line->next)
        {
          if (line->parent != this)
            {
              g_error ("text_btree_node_check_consistency: line doesn't point to parent");
            }
          if (line->segments == NULL)
            {
              g_error ("text_btree_node_check_consistency: line has no segments");
            }

          ld = line->views;
          while (ld != NULL)
            {
              /* Just ensuring we don't segv while doing this loop */

              ld = ld->next;
            }

          for (segPtr = line->segments; segPtr != NULL; segPtr = segPtr->next)
            {
              //if (segPtr->checkFunc != NULL)
              //  {
                  segPtr->checkFunc(line);
              //  }
              if ((segPtr->byte_count == 0) && (!segPtr->leftGravity)
                  && (segPtr->next != NULL)
                  && (segPtr->next->byte_count == 0)
                  && (segPtr->next->leftGravity))
                {
                  g_error ("text_btree_node_check_consistency: wrong segment order for gravity");
                }
              if ((segPtr->next == NULL)
                  && (segPtr->type != text_segment_char))
                {
                  g_error ("text_btree_node_check_consistency: line ended with wrong type");
                }

              num_chars += segPtr->char_count;
            }

          num_children++;
          num_lines++;
        }
    }
  else
    {
      for (childnode = children.node; childnode != NULL;
           childnode = childnode->next)
        {
          if (childnode->parent != this)
            {
              g_error ("text_btree_node_check_consistency: TextBTreeNode doesn't point to parent");
            }
          if (childnode->level != (this->level-1))
            {
              g_error ("text_btree_node_check_consistency: level mismatch (%d %d)",
                       level, childnode->level);
            }
          childnode->check_consistency (tree);
          for (summary = childnode->summary; summary != NULL;
               summary = summary->next)
            {
              for (summary2 = this->summary; ;
                   summary2 = summary2->next)
                {
                  if (summary2 == NULL)
                    {
                      if (summary->info->tag_root == this)
                        {
                          break;
                        }
                      g_error ("text_btree_node_check_consistency: TextBTreeNode tag \"%s\" not %s",
                               summary->info->tag->name,
                               "present in parent summaries");
                    }
                  if (summary->info == summary2->info)
                    {
                      break;
                    }
                }
            }
          num_children++;
          num_lines += childnode->num_lines;
          num_chars += childnode->num_chars;
        }
    }
  if (num_children != this->num_children)
    {
      g_error ("text_btree_node_check_consistency: mismatch in num_children (%d %d)",
               num_children, this->num_children);
    }
  if (num_lines != this->num_lines)
    {
      g_error ("text_btree_node_check_consistency: mismatch in num_lines (%d %d)",
               num_lines, this->num_lines);
    }
  if (num_chars != this->num_chars)
    {
      g_error ("text_btree_node_check_consistency: mismatch in num_chars (%d %d)",
               num_chars, this->num_chars);
    }

  for (summary = this->summary; summary != NULL;
       summary = summary->next)
    {
      if (summary->info->toggle_count == summary->toggle_count)
        {
          g_error ("text_btree_node_check_consistency: found unpruned root for \"%s\"",
                   summary->info->tag->name);
        }
      toggle_count = 0;
      if (level == 0)
        {
          for (line = children.line; line != NULL;
               line = line->next)
            {
              for (segPtr = line->segments; segPtr != NULL;
                   segPtr = segPtr->next)
                {
                  if ((segPtr->type != text_segment_toggle_on)
                      && (segPtr->type != text_segment_toggle_off))
                    {
                      continue;
                    }
                  if (segPtr->body.toggle.info == summary->info)
                    {
                      if (!segPtr->body.toggle.inNodeCounts)
                        g_error ("Toggle segment not in the node counts");

                      toggle_count ++;
                    }
                }
            }
        }
      else
        {
          for (childnode = children.node;
               childnode != NULL;
               childnode = childnode->next)
            {
              for (summary2 = childnode->summary;
                   summary2 != NULL;
                   summary2 = summary2->next)
                {
                  if (summary2->info == summary->info)
                    {
                      toggle_count += summary2->toggle_count;
                    }
                }
            }
        }
      if (toggle_count != summary->toggle_count)
        {
          g_error ("text_btree_node_check_consistency: mismatch in toggle_count (%d %d)",
                   toggle_count, summary->toggle_count);
        }
      for (summary2 = summary->next; summary2 != NULL;
           summary2 = summary2->next)
        {
          if (summary2->info == summary->info)
            {
              g_error ("text_btree_node_check_consistency: duplicated TextBTreeNode tag: %s",
                       summary->info->tag->name);
            }
        }
    }
}

void TextBTree::check (void)
{
  Summary *summary;
  TextBTreeNode *node;
  TextLine *line;
  TextLineSegment *seg;
  TextTag *tag;
  GSList *all_tags, *taglist = NULL;
  int count;
  TextTagInfo *info;

  /*
   * Make sure that the tag toggle counts and the tag root pointers are OK.
   */
  all_tags = table->list_of_tags ();
  for (taglist = all_tags; taglist != NULL ; taglist = taglist->next)
    {
      tag = (TextTag*)taglist->data; //TODO use texttag* as data's type
      info = get_existing_tag_info (tag);
      if (info != NULL)
        {
          node = info->tag_root;
          if (node == NULL)
            {
              if (info->toggle_count != 0)
                {
                  g_error ("_text_btree_check found \"%s\" with toggles (%d) but no root",
                           tag->name, info->toggle_count);
                }
              continue;         /* no ranges for the tag */
            }
          else if (info->toggle_count == 0)
            {
              g_error ("_text_btree_check found root for \"%s\" with no toggles",
                       tag->name);
            }
          else if (info->toggle_count & 1)
            {
              g_error ("_text_btree_check found odd toggle count for \"%s\" (%d)",
                       tag->name, info->toggle_count);
            }
          for (summary = node->summary; summary != NULL;
               summary = summary->next)
            {
              if (summary->info->tag == tag)
                {
                  g_error ("_text_btree_check found root TextBTreeNode with summary info");
                }
            }
          count = 0;
          if (node->level > 0)
            {
              for (node = node->children.node ; node != NULL ;
                   node = node->next)
                {
                  for (summary = node->summary; summary != NULL;
                       summary = summary->next)
                    {
                      if (summary->info->tag == tag)
                        {
                          count += summary->toggle_count;
                        }
                    }
                }
            }
          else
            {
              TextLineSegmentType last = text_segment_none;

              for (line = node->children.line ; line != NULL ;
                   line = line->next)
                {
                  for (seg = line->segments; seg != NULL;
                       seg = seg->next)
                    {
                      if ((seg->type == text_segment_toggle_on ||
                           seg->type == text_segment_toggle_off) &&
                          seg->body.toggle.info->tag == tag)
                        {
                          if (last == seg->type)
                            g_error ("Two consecutive toggles on or off weren't merged");
                          if (!seg->body.toggle.inNodeCounts)
                            g_error ("Toggle segment not in the node counts");

                          last = seg->type;

                          count++;
                        }
                    }
                }
            }
          if (count != info->toggle_count)
            {
              g_error ("_text_btree_check toggle_count (%d) wrong for \"%s\" should be (%d)",
                       info->toggle_count, tag->name, count);
            }
        }
    }

  g_slist_free (all_tags);

  /*
   * Call a recursive procedure to do the main body of checks.
   */

  node = root_node;
  root_node->check_consistency (this);

  /*
   * Make sure that there are at least two lines in the text and
   * that the last line has no characters except a newline.
   */

  if (node->num_lines < 2)
    {
      g_error ("_text_btree_check: less than 2 lines in tree");
    }
  if (node->num_chars < 2)
    {
      g_error ("_text_btree_check: less than 2 chars in tree");
    }
  while (node->level > 0)
    {
      node = node->children.node;
      while (node->next != NULL)
        {
          node = node->next;
        }
    }
  line = node->children.line;
  while (line->next != NULL)
    {
      line = line->next;
    }
  seg = line->segments;
  while ((seg->type == text_segment_toggle_off)
         || (seg->type == text_segment_right_mark)
         || (seg->type == text_segment_left_mark))
    {
      /*
       * It's OK to toggle a tag off in the last line, but
       * not to start a new range.  It's also OK to have marks
       * in the last line.
       */

      seg = seg->next;
    }
  if (seg->type != text_segment_char)
    {
      g_error ("_text_btree_check: last line has bogus segment type");
    }
  if (seg->next != NULL)
    {
      g_error ("_text_btree_check: last line has too many segments");
    }
  if (seg->byte_count != 1)
    {
      g_error ("_text_btree_check: last line has wrong # characters: %d",
               seg->byte_count);
    }
  if ((seg->body.chars[0] != '\n') || (seg->body.chars[1] != 0))
    {
      g_error ("_text_btree_check: last line had bad value: %s",
               seg->body.chars);
    }
}

/*
void _text_btree_spew_line (TextBTree* tree, TextLine* line);
void _text_btree_spew_segment (TextBTree* tree, TextLineSegment* seg);
void _text_btree_spew_node (TextBTreeNode *node, int indent);
void _text_btree_spew_line_short (TextLine *line, int indent);

void TextBTree::spew ()
{
  TextLine * line;
  int real_line;

  printf ("%d lines in tree %p\n",
          get_line_count (), this);

  line = get_line (0, &real_line);

  while (line != NULL)
    {
      spew_line (line);
      line = line->next ();
    }

  printf ("=================== Tag information\n");

  {
    GSList * list;

    list = tag_infos;

    while (list != NULL)
      {
        TextTagInfo *info;

        info = (TextTagInfo*)list->data;

        printf ("  tag `%s': root at %p, toggle count %d\n",
                info->tag->name, info->tag_root, info->toggle_count);

        list = g_slist_next (list);
      }

    if (tag_infos == NULL)
      {
        printf ("  (no tags in the tree)\n");
      }
  }

  printf ("=================== Tree nodes\n");

  {
    spew_node (root_node, 0);
  }
}

void TextBTree::spew_line_short (TextLine *line, int indent)
{
  gchar * spaces;
  TextLineSegment *seg;

  spaces = g_strnfill (indent, ' ');

  printf ("%sline %p chars %d bytes %d\n",
          spaces, line,
          line->char_count (),
          line->byte_count ());

  seg = line->segments;
  while (seg != NULL)
    {
      if (seg->type == text_segment_char)
        {
          gchar* str = g_strndup (seg->body.chars, MIN (seg->byte_count, 10));
          gchar* s;
          s = str;
          while (*s)
            {
              if (*s == '\n' || *s == '\r')
                *s = '\\';
              ++s;
            }
          printf ("%s chars `%s'...\n", spaces, str);
          g_free (str);
        }
      else if (seg->type == &text_segment_right_mark_type)
        {
          printf ("%s right mark `%s' visible: %d\n",
                  spaces,
                  seg->body.mark.name,
                  seg->body.mark.visible);
        }
      else if (seg->type == &text_segment_left_mark_type)
        {
          printf ("%s left mark `%s' visible: %d\n",
                  spaces,
                  seg->body.mark.name,
                  seg->body.mark.visible);
        }
      else if (seg->type == text_segment_toggle_on_type ||
               seg->type == text_segment_toggle_off_type)
        {
          printf ("%s tag `%s' %s\n",
                  spaces, seg->body.toggle.info->tag->name,
                  seg->type == text_segment_toggle_off_type ? "off" : "on");
        }

      seg = seg->next;
    }

  g_free (spaces);
}

void
_text_btree_spew_node (TextBTreeNode *node, int indent)
{
  gchar * spaces;
  TextBTreeNode *iter;
  Summary *s;

  spaces = g_strnfill (indent, ' ');

  printf ("%snode %p level %d children %d lines %d chars %d\n",
          spaces, node, node->level,
          node->num_children, node->num_lines, node->num_chars);

  s = node->summary;
  while (s)
    {
      printf ("%s %d toggles of `%s' below this node\n",
              spaces, s->toggle_count, s->info->tag->name);
      s = s->next;
    }

  g_free (spaces);

  if (node->level > 0)
    {
      iter = node->children.node;
      while (iter != NULL)
        {
          _text_btree_spew_node (iter, indent + 2);

          iter = iter->next;
        }
    }
  else
    {
      TextLine *line = node->children.line;
      while (line != NULL)
        {
          _text_btree_spew_line_short (line, indent + 2);

          line = line->next;
        }
    }
}

void
_text_btree_spew_line (TextBTree* tree, TextLine* line)
{
  TextLineSegment * seg;

  printf ("%4d| line: %p parent: %p next: %p\n",
          _text_line_get_number (line), line, line->parent, line->next);

  seg = line->segments;

  while (seg != NULL)
    {
      _text_btree_spew_segment (tree, seg);
      seg = seg->next;
    }
}

void
_text_btree_spew_segment (TextBTree* tree, TextLineSegment * seg)
{
  printf ("     segment: %p type: %s bytes: %d chars: %d\n",
          seg, seg->type->name, seg->byte_count, seg->char_count);

  if (seg->type == text_segment_char)
    {
      gchar* str = g_strndup (seg->body.chars, seg->byte_count);
      printf ("       `%s'\n", str);
      g_free (str);
    }
  else if (seg->type == &text_segment_right_mark_type)
    {
      printf ("       right mark `%s' visible: %d not_deleteable: %d\n",
              seg->body.mark.name,
              seg->body.mark.visible,
              seg->body.mark.not_deleteable);
    }
  else if (seg->type == &text_segment_left_mark_type)
    {
      printf ("       left mark `%s' visible: %d not_deleteable: %d\n",
              seg->body.mark.name,
              seg->body.mark.visible,
              seg->body.mark.not_deleteable);
    }
  else if (seg->type == text_segment_toggle_on_type ||
           seg->type == text_segment_toggle_off_type)
    {
      printf ("       tag `%s' priority %d\n",
              seg->body.toggle.info->tag->name,
              seg->body.toggle.info->tag->priority);
    }
}*/

/*
 * Init iterators from the BTree
 */

//TODO move to textiter class? (and have a textbtree as param?)
//then also make the fields of textiter class private.
void TextBTree::get_iter_at_line_char (
                                       TextIter  *iter,
                                       gint          line_number,
                                       gint          char_on_line)
{
  //TextRealIter *real = (TextRealIter*)iter;
  TextLine *line;
  gint real_line;

  g_return_if_fail (iter != NULL);
  //g_return_if_fail (tree != NULL);

  line = get_line_no_last (line_number, &real_line);
  
  iter->init_from_char_offset (this, line, char_on_line);

  /* We might as well cache this, since we know it. */
  /*real*/iter->cached_line_number = real_line;

  iter->check_invariants ();
}

void
TextBTree::get_iter_at_line_byte (
                                       TextIter    *iter,
                                       gint            line_number,
                                       gint            byte_index)
{
  //TextRealIter *real = (TextRealIter*)iter;
  TextLine *line;
  gint real_line;

  g_return_if_fail (iter != NULL);
  //g_return_if_fail (tree != NULL);

  line = get_line_no_last (line_number, &real_line);

  iter->init_from_byte_offset (this, line, byte_index);

  /* We might as well cache this, since we know it. */
  /*real*/iter->cached_line_number = real_line;

  iter->check_invariants ();
}

void
TextBTree::get_iter_at_line      (
                                       TextIter    *iter,
                                       TextLine    *line,
                                       gint            byte_offset)
{
  g_return_if_fail (iter != NULL);
  //g_return_if_fail (tree != NULL);
  g_return_if_fail (line != NULL);

  iter->init_from_byte_offset (this, line, byte_offset);

  iter->check_invariants ();
}

bool
TextBTree::get_iter_at_first_toggle (
                                          TextIter    *iter,
                                          TextTag     *tag)
{
  TextLine *line;

  g_return_val_if_fail (iter != NULL, false);
  //g_return_val_if_fail (tree != NULL, false);

  line = first_could_contain_tag (tag);

  if (line == NULL)
    {
      /* Set iter to last in tree */
      get_end_iter (iter);
      iter->check_invariants ();
      return false;
    }
  else
    {
      iter->init_from_byte_offset (this, line, 0);

      if (!iter->toggles_tag (tag))
        iter->forward_to_tag_toggle (tag);

      iter->check_invariants ();
      return true;
    }
}

bool
TextBTree::get_iter_at_last_toggle  (
                                          TextIter    *iter,
                                          TextTag     *tag)
{
  g_return_val_if_fail (iter != NULL, false);
  //g_return_val_if_fail (tree != NULL, false);

  get_end_iter (iter);
  iter->backward_to_tag_toggle (tag);
  iter->check_invariants ();
  
  return true;
}

bool
TextBTree::get_iter_at_mark_name (
                                       TextIter *iter,
                                       const gchar *mark_name)
{
  TextMark *mark;

  g_return_val_if_fail (iter != NULL, false);
  //g_return_val_if_fail (tree != NULL, false);

  mark = get_mark_by_name (mark_name);

  if (mark == NULL)
    return false;
  else
    {
      get_iter_at_mark (iter, mark);
      iter->check_invariants ();
      return true;
    }
}

void
TextBTree::get_iter_at_mark (
                                  TextIter *iter,
                                  TextMark *mark)
{
  TextLineSegment *seg;

  g_return_if_fail (iter != NULL);
  //g_return_if_fail (tree != NULL);
  //TODOg_return_if_fail (GTK_IS_TEXT_MARK (mark));

  seg = mark->segment;

  iter->init_from_segment (this, seg->body.mark.line, seg);
  g_assert (seg->body.mark.line == iter->get_text_line ());
  iter->check_invariants ();
}

void
TextBTree::get_iter_at_child_anchor (
                                          TextIter        *iter,
                                          TextChildAnchor *anchor)
{
  TextLineSegment *seg;

  g_return_if_fail (iter != NULL);
  //g_return_if_fail (tree != NULL);
  //TODOg_return_if_fail (GTK_IS_TEXT_CHILD_ANCHOR (anchor));
  
  seg = anchor->segment;  

  /*g_assert (seg->body.child.line != NULL);
  
  iter_init_from_segment (iter, tree,
                          seg->body.child.line, seg);
  g_assert (seg->body.child.line == _text_iter_get_text_line (iter));*/
  iter->check_invariants ();
}

void
TextBTree::get_end_iter         (
                                      TextIter    *iter)
{
  g_return_if_fail (iter != NULL);
 // g_return_if_fail (tree != NULL);

  get_iter_at_char ( iter, get_char_count ());
  iter->check_invariants ();
}

void TextBTree::get_iter_at_char ( TextIter *iter,
                                  gint char_index)
{
//  TextRealIter *real = (TextRealIter*)iter;
  gint real_char_index;
  gint line_start;
  TextLine *line;

  g_return_if_fail (iter != NULL);
//  g_return_if_fail (tree != NULL);

  line = get_line_at_char (char_index,
                                           &line_start, &real_char_index);

  iter->init_from_char_offset (this, line, real_char_index - line_start);

  iter->cached_char_index = real_char_index;

  iter->check_invariants ();
}
