/* gtktextchild.c - child pixmaps and widgets
 *
 * Copyright (c) 1994 The Regents of the University of California.
 * Copyright (c) 1994-1997 Sun Microsystems, Inc.
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
#include "TextChild.h"
#include "TextBTree.h"
//#include "gtktextlayout.h"
//#include "gtkintl.h"
//#include "gtkalias.h"

#define CHECK_IN_BUFFER(anchor)                                         \
  G_STMT_START {                                                        \
    if ((anchor)->segment == NULL)                                      \
      {                                                                 \
        g_warning ("%s: TextChildAnchor hasn't been in a buffer yet",\
                   G_STRFUNC);                                          \
      }                                                                 \
  } G_STMT_END

#define CHECK_IN_BUFFER_RETURN(anchor, val)                             \
  G_STMT_START {                                                        \
    if ((anchor)->segment == NULL)                                      \
      {                                                                 \
        g_warning ("%s: TextChildAnchor hasn't been in a buffer yet",\
                   G_STRFUNC);                                          \
        return (val);                                                   \
      }                                                                 \
  } G_STMT_END

/*
static TextLineSegment *
pixbuf_segment_cleanup_func (TextLineSegment *seg,
                             TextLine        *line)
{
  * nothing *
  return seg;
}

static int
pixbuf_segment_delete_func (TextLineSegment *seg,
                            TextLine        *line,
                            gboolean            tree_gone)
{
  if (seg->body.pixbuf.pixbuf)
    g_object_unref (seg->body.pixbuf.pixbuf);

  g_free (seg);

  return 0;
}

static void
pixbuf_segment_check_func (TextLineSegment *seg,
                           TextLine        *line)
{
  if (seg->next == NULL)
    g_error ("pixbuf segment is the last segment in a line");

  if (seg->byte_count != 3)
    g_error ("pixbuf segment has byte count of %d", seg->byte_count);

  if (seg->char_count != 1)
    g_error ("pixbuf segment has char count of %d", seg->char_count);
}


const TextLineSegmentClass gtk_text_pixbuf_type = {
  "pixbuf",                     * name *
  FALSE,                        * leftGravity *
  NULL,                         * splitFunc *
  pixbuf_segment_delete_func,   * deleteFunc *
  pixbuf_segment_cleanup_func,  * cleanupFunc *
  NULL,                         * lineChangeFunc *
  pixbuf_segment_check_func     * checkFunc *

};

#define PIXBUF_SEG_SIZE ((unsigned) (G_STRUCT_OFFSET (TextLineSegment, body) \
        + sizeof (TextPixbuf)))

TextLineSegment *
_gtk_pixbuf_segment_new (GdkPixbuf *pixbuf)
{
  TextLineSegment *seg;

  seg = g_malloc (PIXBUF_SEG_SIZE);

  seg->type = &gtk_text_pixbuf_type;

  seg->next = NULL;

  seg->byte_count = 3; * We convert to the 0xFFFC "unknown character",
                        * a 3-byte sequence in UTF-8
                        *
  seg->char_count = 1;

  seg->body.pixbuf.pixbuf = pixbuf;

  g_object_ref (pixbuf);

  return seg;
}*/


static TextLineSegment *
child_segment_cleanup_func (TextLineSegment *seg,
                            TextLine        *line)
{
  //we dont allow child widgets seg->body.child.line = line;

  return seg;
}

static int
child_segment_delete_func (TextLineSegment *seg,
                           TextLine       *line,
                           gboolean           tree_gone)
{
  GSList *tmp_list;
  GSList *copy;

  /*no child widgets _gtk_text_btree_unregister_child_anchor (seg->body.child.obj);
  
  seg->body.child.tree = NULL;
  seg->body.child.line = NULL;*/

  /* avoid removing widgets while walking the list */
  /*copy = g_slist_copy (seg->body.child.widgets);
  tmp_list = copy;
  while (tmp_list != NULL)
    {
      GtkWidget *child = tmp_list->data;

      gtk_widget_destroy (child);
      
      tmp_list = g_slist_next (tmp_list);
    }*/

  /* On removal from the widget's parents (TextView),
   * the widget should have been removed from the anchor.
   */
  /*g_assert (seg->body.child.widgets == NULL);

  g_slist_free (copy);
  
  _gtk_widget_segment_unref (seg);  */
  
  return 0;
}

static void
child_segment_check_func (TextLineSegment *seg,
                          TextLine        *line)
{
  if (seg->next == NULL)
    g_error ("child segment is the last segment in a line");

  if (seg->byte_count != 3)
    g_error ("child segment has byte count of %d", seg->byte_count);

  if (seg->char_count != 1)
    g_error ("child segment has char count of %d", seg->char_count);
}

/*const TextLineSegmentType text_child_type = {
  "child-widget",                                        * name *
  FALSE,                                                 * leftGravity *
  NULL,                                                  * splitFunc *
  child_segment_delete_func,                             * deleteFunc *
  child_segment_cleanup_func,                            * cleanupFunc *
  NULL,                                                  * lineChangeFunc *
  child_segment_check_func                               * checkFunc *
};*/

/*
#define WIDGET_SEG_SIZE ((unsigned) (G_STRUCT_OFFSET (TextLineSegment, body) \
        + sizeof (TextChildBody)))

TextLineSegment *
_gtk_widget_segment_new (TextChildAnchor *anchor)
{
  TextLineSegment *seg;

  seg = g_malloc (WIDGET_SEG_SIZE);

  seg->type = &gtk_text_child_type;

  seg->next = NULL;

  seg->byte_count = 3; * We convert to the 0xFFFC "unknown character",
                        * a 3-byte sequence in UTF-8
                        *
  seg->char_count = 1;

  seg->body.child.obj = anchor;
  seg->body.child.obj->segment = seg;
  seg->body.child.widgets = NULL;
  seg->body.child.tree = NULL;
  seg->body.child.line = NULL;

  g_object_ref (anchor);
  
  return seg;
}

void
_gtk_widget_segment_add    (TextLineSegment *widget_segment,
                            GtkWidget          *child)
{
  g_return_if_fail (widget_segment->type == &gtk_text_child_type);
  g_return_if_fail (widget_segment->body.child.tree != NULL);

  g_object_ref (child);
  
  widget_segment->body.child.widgets =
    g_slist_prepend (widget_segment->body.child.widgets,
                     child);
}

void
_gtk_widget_segment_remove (TextLineSegment *widget_segment,
                            GtkWidget          *child)
{
  g_return_if_fail (widget_segment->type == &gtk_text_child_type);
  
  widget_segment->body.child.widgets =
    g_slist_remove (widget_segment->body.child.widgets,
                    child);

  g_object_unref (child);
}

void
_gtk_widget_segment_ref (TextLineSegment *widget_segment)
{
  g_assert (widget_segment->type == &gtk_text_child_type);

  g_object_ref (widget_segment->body.child.obj);
}

void
_gtk_widget_segment_unref (TextLineSegment *widget_segment)
{
  g_assert (widget_segment->type == &gtk_text_child_type);

  g_object_unref (widget_segment->body.child.obj);
}*/

/*TextLayout*
_gtk_anchored_child_get_layout (GtkWidget *child)
{
  return g_object_get_data (G_OBJECT (child), "gtk-text-child-anchor-layout");  
}

static void
_gtk_anchored_child_set_layout (GtkWidget     *child,
                                TextLayout *layout)
{
  g_object_set_data (G_OBJECT (child),
                     I_("gtk-text-child-anchor-layout"),
                     layout);  
}*/
     
//static void gtk_text_child_anchor_finalize (GObject *obj);

//G_DEFINE_TYPE (TextChildAnchor, gtk_text_child_anchor, G_TYPE_OBJECT)

static void
gtk_text_child_anchor_init (TextChildAnchor *child_anchor)
{
  child_anchor->segment = NULL;
}

/*
static void
gtk_text_child_anchor_class_init (TextChildAnchorType *type)
{
  //GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gtk_text_child_anchor_finalize;
}*/

/**
 * gtk_text_child_anchor_new:
 * 
 * Creates a new #TextChildAnchor. Usually you would then insert
 * it into a #TextBuffer with gtk_text_buffer_insert_child_anchor().
 * To perform the creation and insertion in one step, use the
 * convenience function gtk_text_buffer_create_child_anchor().
 * 
 * Return value: a new #TextChildAnchor
 **/
TextChildAnchor::TextChildAnchor()
{
 // return g_object_new (GTK_TYPE_TEXT_CHILD_ANCHOR, NULL);
}

TextChildAnchor::~TextChildAnchor()
{
 // TextChildAnchor *anchor;
  GSList *tmp_list;
  TextLineSegment *seg;
  
//  anchor = GTK_TEXT_CHILD_ANCHOR (obj);

  seg = segment;
  
  if (seg)
    {
      /*if (seg->body.child.tree != NULL)
        {
          g_warning ("Someone removed a reference to a TextChildAnchor "
                     "they didn't own; the anchor is still in the text buffer "
                     "and the refcount is 0.");
          return;
        }*/
      
      //tmp_list = seg->body.child.widgets;
      while (tmp_list)
        {
          //TODOg_object_unref (tmp_list->data);
          tmp_list = g_slist_next (tmp_list);
        }
  
      //g_slist_free (seg->body.child.widgets);
  
      g_free (seg);
    }

  segment = NULL;

  //TODO G_OBJECT_CLASS (gtk_text_child_anchor_parent_class)->finalize (obj);
}

/**
 * gtk_text_child_anchor_get_widgets:
 * @anchor: a #TextChildAnchor
 * 
 * Gets a list of all widgets anchored at this child anchor.
 * The returned list should be freed with g_list_free().
 * 
 * 
 * Return value: list of widgets anchored at @anchor
 **/
/*
GList*
gtk_text_child_anchor_get_widgets (TextChildAnchor *anchor)
{
  TextLineSegment *seg = anchor->segment;
  GList *list = NULL;
  GSList *iter;

  CHECK_IN_BUFFER_RETURN (anchor, NULL);
  
  g_return_val_if_fail (seg->type == &gtk_text_child_type, NULL);

  iter = seg->body.child.widgets;
  while (iter != NULL)
    {
      list = g_list_prepend (list, iter->data);

      iter = g_slist_next (iter);
    }

  * Order is not relevant, so we don't need to reverse the list
   * again.
   *
  return list;
}*/

/**
 * gtk_text_child_anchor_get_deleted:
 * @anchor: a #TextChildAnchor
 * 
 * Determines whether a child anchor has been deleted from
 * the buffer. Keep in mind that the child anchor will be
 * unreferenced when removed from the buffer, so you need to
 * hold your own reference (with g_object_ref()) if you plan
 * to use this function &mdash; otherwise all deleted child anchors
 * will also be finalized.
 * 
 * Return value: %TRUE if the child anchor has been deleted from its buffer
 **/
/*
bool TextChildAnchor::get_deleted (void)
{
  TextLineSegment *seg = segment;

//TODO  CHECK_IN_BUFFER_RETURN (anchor, TRUE);
  
  g_return_val_if_fail (seg->type == &text_child_type, true);

  return seg->body.child.tree == NULL;
}*/

/*void
gtk_text_child_anchor_register_child (TextChildAnchor *anchor,
                                      GtkWidget          *child,
                                      TextLayout      *layout)
{
  g_return_if_fail (GTK_IS_TEXT_CHILD_ANCHOR (anchor));
  g_return_if_fail (GTK_IS_WIDGET (child));

  CHECK_IN_BUFFER (anchor);
  
  _gtk_anchored_child_set_layout (child, layout);
  
  _gtk_widget_segment_add (anchor->segment, child);

  gtk_text_child_anchor_queue_resize (anchor, layout);
}

void
gtk_text_child_anchor_unregister_child (TextChildAnchor *anchor,
                                        GtkWidget          *child)
{
  g_return_if_fail (GTK_IS_TEXT_CHILD_ANCHOR (anchor));
  g_return_if_fail (GTK_IS_WIDGET (child));

  CHECK_IN_BUFFER (anchor);
  
  if (_gtk_anchored_child_get_layout (child))
    {
      gtk_text_child_anchor_queue_resize (anchor,
                                          _gtk_anchored_child_get_layout (child));
    }
  
  _gtk_anchored_child_set_layout (child, NULL);
  
  _gtk_widget_segment_remove (anchor->segment, child);
}*/

void TextChildAnchor::queue_resize ( TextLayout      *layout)
{
  TextIter start;
  TextIter end;
  TextLineSegment *seg;
  
 // g_return_if_fail (GTK_IS_TEXT_CHILD_ANCHOR (anchor));
 //TODO g_return_if_fail (GTK_IS_TEXT_LAYOUT (layout));

  CHECK_IN_BUFFER (this);
  
  seg = segment;

  //if (seg->body.child.tree == NULL)
    return;
  
  layout->buffer->get_iter_at_child_anchor ( &start, this);
  end = start;
  (&end)->forward_char ();
  
  layout->invalidate (&start, &end);
}

/*void
gtk_text_anchored_child_set_layout (GtkWidget     *child,
                                    TextLayout *layout)
{
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (layout == NULL || GTK_IS_TEXT_LAYOUT (layout));
  
  _gtk_anchored_child_set_layout (child, layout);
}*/
