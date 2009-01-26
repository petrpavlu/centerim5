/* GTK - The GIMP Toolkit
 * gtktextbuffer.c Copyright (C) 2000 Red Hat, Inc.
 *                 Copyright (C) 2004 Nokia Corporation
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

//#include "config.h"
#include <string.h>
//#include <stdarg.h>

#include "TextBuffer.h"
//#include "TextBTree.h"
//#include "TextIter.h"

/*#define GTK_TEXT_USE_INTERNAL_UNSUPPORTED_API
#include "gtkclipboard.h"
#include "gtkdnd.h"*/
//#include "gtkinvisible.h"
//#include "gtkmarshalers.h"
//#include "gtktextbuffer.h"
//#include "gtktextbufferrichtext.h"
//#include "gtktextbtree.h"
//#include "gtktextiterprivate.h"
//#include "gtkprivate.h"
//#include "gtkintl.h"
//#include "gtkalias.h"

#define TEXT_UNKNOWN_CHAR 0xFFFC

//#define GTK_TEXT_BUFFER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GTK_TYPE_TEXT_BUFFER, TextBufferPrivate))

/*
typedef struct
{
  GtkTargetList  *copy_target_list;
  GtkTargetEntry *copy_target_entries;
  gint            n_copy_target_entries;

  GtkTargetList  *paste_target_list;
  GtkTargetEntry *paste_target_entries;
  gint            n_paste_target_entries;
} TextBufferPrivate;
*/

/*typedef struct _ClipboardRequest ClipboardRequest;

struct _ClipboardRequest
{
  TextBuffer *buffer;
  bool interactive;
  bool default_editable;
  bool is_clipboard;
  bool replace_selection;
};*/

enum Signals {
  INSERT_TEXT,
  INSERT_PIXBUF,
  INSERT_CHILD_ANCHOR,
  DELETE_RANGE,
  CHANGED,
  MODIFIED_CHANGED,
  MARK_SET,
  MARK_DELETED,
  APPLY_TAG,
  REMOVE_TAG,
  BEGIN_USER_ACTION,
  END_USER_ACTION,
  LAST_SIGNAL
};

enum {
  PROP_0,

  /* Construct */
  PROP_TAG_TABLE,

  /* Normal */
  PROP_TEXT,
  PROP_HAS_SELECTION,
  PROP_CURSOR_POSITION,
  PROP_COPY_TARGET_LIST,
  PROP_PASTE_TARGET_LIST
};

static Signals signals[LAST_SIGNAL];

//static void free_log_attr_cache (TextLogAttrCache *cache);

TextBuffer::TextBuffer(void)
: tag_table(NULL)
, btree(NULL)
, log_attr_cache(NULL)
, user_action_count(0)
, modified(0)
, has_selection(0)
{
  /*GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gtk_text_buffer_finalize;
  object_class->set_property = gtk_text_buffer_set_property;
  object_class->get_property = gtk_text_buffer_get_property;
  object_class->notify       = gtk_text_buffer_notify;
 
  klass->insert_text = gtk_text_buffer_real_insert_text;
  klass->insert_pixbuf = gtk_text_buffer_real_insert_pixbuf;
  klass->insert_child_anchor = gtk_text_buffer_real_insert_anchor;
  klass->delete_range = gtk_text_buffer_real_delete_range;
  klass->apply_tag = gtk_text_buffer_real_apply_tag;
  klass->remove_tag = gtk_text_buffer_real_remove_tag;
  klass->changed = gtk_text_buffer_real_changed;
  klass->mark_set = gtk_text_buffer_real_mark_set;

  * Construct *
  g_object_class_install_property (object_class,
                                   PROP_TAG_TABLE,
                                   g_param_spec_object ("tag-table",
                                                        P_("Tag Table"),
                                                        P_("Text Tag Table"),
                                                        GTK_TYPE_TEXT_TAG_TABLE,
                                                        GTK_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

  * Normal properties*
  
  **
   * TextBuffer:text:
   *
   * The text content of the buffer. Without child widgets and images,
   * see gtk_text_buffer_get_text() for more information.
   *
   * Since: 2.8
   *
  g_object_class_install_property (object_class,
                                   PROP_TEXT,
                                   g_param_spec_string ("text",
                                                        P_("Text"),
                                                        P_("Current text of the buffer"),
							"",
                                                        GTK_PARAM_READWRITE));

  **
   * TextBuffer:has-selection:
   *
   * Whether the buffer has some text currently selected.
   *
   * Since: 2.10
   *
  g_object_class_install_property (object_class,
                                   PROP_HAS_SELECTION,
                                   g_param_spec_boolean ("has-selection",
                                                         P_("Has selection"),
                                                         P_("Whether the buffer has some text currently selected"),
                                                         FALSE,
                                                         GTK_PARAM_READABLE));

  **
   * TextBuffer:cursor-position:
   *
   * The position of the insert mark (as offset from the beginning 
   * of the buffer). It is useful for getting notified when the 
   * cursor moves.
   *
   * Since: 2.10
   *
  g_object_class_install_property (object_class,
                                   PROP_CURSOR_POSITION,
                                   g_param_spec_int ("cursor-position",
                                                     P_("Cursor position"),
                                                     P_("The position of the insert mark (as offset from the beginning of the buffer)"),
						     0, G_MAXINT, 0,
                                                     GTK_PARAM_READABLE));

  **
   * TextBuffer:copy-target-list:
   *
   * The list of targets this buffer supports for clipboard copying
   * and as DND source.
   *
   * Since: 2.10
   *
  g_object_class_install_property (object_class,
                                   PROP_COPY_TARGET_LIST,
                                   g_param_spec_boxed ("copy-target-list",
                                                       P_("Copy target list"),
                                                       P_("The list of targets this buffer supports for clipboard copying and DND source"),
                                                       GTK_TYPE_TARGET_LIST,
                                                       GTK_PARAM_READABLE));

  **
   * TextBuffer:paste-target-list:
   *
   * The list of targets this buffer supports for clipboard pasting
   * and as DND destination.
   *
   * Since: 2.10
   *
  g_object_class_install_property (object_class,
                                   PROP_PASTE_TARGET_LIST,
                                   g_param_spec_boxed ("paste-target-list",
                                                       P_("Paste target list"),
                                                       P_("The list of targets this buffer supports for clipboard pasting and DND destination"),
                                                       GTK_TYPE_TARGET_LIST,
                                                       GTK_PARAM_READABLE));

  **
   * TextBuffer::insert-text:
   * @textbuffer: the object which received the signal
   * @location: position to insert @text in @textbuffer
   * @text: the UTF-8 text to be inserted
   * @len: length of the inserted text in bytes
   * 
   * The ::insert-text signal is emitted to insert text in a #TextBuffer.
   * Insertion actually occurs in the default handler.  
   * 
   * Note that if your handler runs before the default handler it must not 
   * invalidate the @location iter (or has to revalidate it). 
   * The default signal handler revalidates it to point to the end of the 
   * inserted text.
   * 
   * See also: 
   * gtk_text_buffer_insert(), 
   * gtk_text_buffer_insert_range().
   *
  signals[INSERT_TEXT] =
    g_signal_new (I_("insert-text"),
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (TextBufferClass, insert_text),
                  NULL, NULL,
                  _gtk_marshal_VOID__BOXED_STRING_INT,
                  G_TYPE_NONE,
                  3,
                  GTK_TYPE_TEXT_ITER | G_SIGNAL_TYPE_STATIC_SCOPE,
                  G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE,
                  G_TYPE_INT);

  **
   * TextBuffer::insert-pixbuf:
   * @textbuffer: the object which received the signal
   * @location: position to insert @pixbuf in @textbuffer
   * @pixbuf: the #GdkPixbuf to be inserted
   * 
   * The ::insert-pixbuf signal is emitted to insert a #GdkPixbuf 
   * in a #TextBuffer. Insertion actually occurs in the default handler.
   * 
   * Note that if your handler runs before the default handler it must not 
   * invalidate the @location iter (or has to revalidate it). 
   * The default signal handler revalidates it to be placed after the 
   * inserted @pixbuf.
   * 
   * See also: gtk_text_buffer_insert_pixbuf().
   *
  signals[INSERT_PIXBUF] =
    g_signal_new (I_("insert-pixbuf"),
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (TextBufferClass, insert_pixbuf),
                  NULL, NULL,
                  _gtk_marshal_VOID__BOXED_OBJECT,
                  G_TYPE_NONE,
                  2,
                  GTK_TYPE_TEXT_ITER | G_SIGNAL_TYPE_STATIC_SCOPE,
                  GDK_TYPE_PIXBUF);


  **
   * TextBuffer::insert-child-anchor:
   * @textbuffer: the object which received the signal
   * @location: position to insert @anchor in @textbuffer
   * @anchor: the #TextChildAnchor to be inserted
   * 
   * The ::insert-child-anchor signal is emitted to insert a
   * #TextChildAnchor in a #TextBuffer.
   * Insertion actually occurs in the default handler.
   * 
   * Note that if your handler runs before the default handler it must
   * not invalidate the @location iter (or has to revalidate it). 
   * The default signal handler revalidates it to be placed after the 
   * inserted @anchor.
   * 
   * See also: gtk_text_buffer_insert_child_anchor().
   *
  signals[INSERT_CHILD_ANCHOR] =
    g_signal_new (I_("insert-child-anchor"),
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (TextBufferClass, insert_child_anchor),
                  NULL, NULL,
                  _gtk_marshal_VOID__BOXED_OBJECT,
                  G_TYPE_NONE,
                  2,
                  GTK_TYPE_TEXT_ITER | G_SIGNAL_TYPE_STATIC_SCOPE,
                  GTK_TYPE_TEXT_CHILD_ANCHOR);
  
  **
   * TextBuffer::delete-range:
   * @textbuffer: the object which received the signal
   * @start: the start of the range to be deleted
   * @end: the end of the range to be deleted
   * 
   * The ::delete-range signal is emitted to delete a range 
   * from a #TextBuffer. 
   * 
   * Note that if your handler runs before the default handler it must not 
   * invalidate the @start and @end iters (or has to revalidate them). 
   * The default signal handler revalidates the @start and @end iters to 
   * both point point to the location where text was deleted. Handlers
   * which run after the default handler (see g_signal_connect_after())
   * do not have access to the deleted text.
   * 
   * See also: gtk_text_buffer_delete().
   *
  signals[DELETE_RANGE] =
    g_signal_new (I_("delete-range"),
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (TextBufferClass, delete_range),
                  NULL, NULL,
                  _gtk_marshal_VOID__BOXED_BOXED,
                  G_TYPE_NONE,
                  2,
                  GTK_TYPE_TEXT_ITER | G_SIGNAL_TYPE_STATIC_SCOPE,
                  GTK_TYPE_TEXT_ITER | G_SIGNAL_TYPE_STATIC_SCOPE);

  **
   * TextBuffer::changed:
   * @textbuffer: the object which received the signal
   * 
   * The ::changed signal is emitted when the content of a #TextBuffer 
   * has changed.
   *
  signals[CHANGED] =
    g_signal_new (I_("changed"),
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_LAST,                   
                  G_STRUCT_OFFSET (TextBufferClass, changed),
                  NULL, NULL,
                  _gtk_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);

  **
   * TextBuffer::modified-changed:
   * @textbuffer: the object which received the signal
   * 
   * The ::modified-changed signal is emitted when the modified bit of a 
   * #TextBuffer flips.
   * 
   * See also:
   * gtk_text_buffer_set_modified().
   *
  signals[MODIFIED_CHANGED] =
    g_signal_new (I_("modified-changed"),
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (TextBufferClass, modified_changed),
                  NULL, NULL,
                  _gtk_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);

  **
   * TextBuffer::mark-set:
   * @textbuffer: the object which received the signal
   * @location: The location of @mark in @textbuffer
   * @mark: The mark that is set
   * 
   * The ::mark-set signal is emitted as notification
   * after a #TextMark is set.
   * 
   * See also: 
   * gtk_text_buffer_create_mark(),
   * gtk_text_buffer_move_mark().
   *
  signals[MARK_SET] =
    g_signal_new (I_("mark-set"),
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_LAST,                   
                  G_STRUCT_OFFSET (TextBufferClass, mark_set),
                  NULL, NULL,
                  _gtk_marshal_VOID__BOXED_OBJECT,
                  G_TYPE_NONE,
                  2,
                  GTK_TYPE_TEXT_ITER,
                  GTK_TYPE_TEXT_MARK);

  **
   * TextBuffer::mark-deleted:
   * @textbuffer: the object which received the signal
   * @mark: The mark that was deleted
   * 
   * The ::mark-deleted signal is emitted as notification
   * after a #TextMark is deleted. 
   * 
   * See also:
   * gtk_text_buffer_delete_mark().
   *
  signals[MARK_DELETED] =
    g_signal_new (I_("mark-deleted"),
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_LAST,                   
                  G_STRUCT_OFFSET (TextBufferClass, mark_deleted),
                  NULL, NULL,
                  _gtk_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  GTK_TYPE_TEXT_MARK);

   **
   * TextBuffer::apply-tag:
   * @textbuffer: the object which received the signal
   * @tag: the applied tag
   * @start: the start of the range the tag is applied to
   * @end: the end of the range the tag is applied to
   * 
   * The ::apply-tag signal is emitted to apply a tag to a
   * range of text in a #TextBuffer. 
   * Applying actually occurs in the default handler.
   * 
   * Note that if your handler runs before the default handler it must not 
   * invalidate the @start and @end iters (or has to revalidate them). 
   * 
   * See also: 
   * gtk_text_buffer_apply_tag(),
   * gtk_text_buffer_insert_with_tags(),
   * gtk_text_buffer_insert_range().
   * 
  signals[APPLY_TAG] =
    g_signal_new (I_("apply-tag"),
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (TextBufferClass, apply_tag),
                  NULL, NULL,
                  _gtk_marshal_VOID__OBJECT_BOXED_BOXED,
                  G_TYPE_NONE,
                  3,
                  GTK_TYPE_TEXT_TAG,
                  GTK_TYPE_TEXT_ITER,
                  GTK_TYPE_TEXT_ITER);


   **
   * TextBuffer::remove-tag:
   * @textbuffer: the object which received the signal
   * @tag: the tag to be removed
   * @start: the start of the range the tag is removed from
   * @end: the end of the range the tag is removed from
   * 
   * The ::remove-tag signal is emitted to remove all occurrences of @tag from
   * a range of text in a #TextBuffer. 
   * Removal actually occurs in the default handler.
   * 
   * Note that if your handler runs before the default handler it must not 
   * invalidate the @start and @end iters (or has to revalidate them). 
   * 
   * See also: 
   * gtk_text_buffer_remove_tag(). 
   * 
  signals[REMOVE_TAG] =
    g_signal_new (I_("remove-tag"),
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (TextBufferClass, remove_tag),
                  NULL, NULL,
                  _gtk_marshal_VOID__OBJECT_BOXED_BOXED,
                  G_TYPE_NONE,
                  3,
                  GTK_TYPE_TEXT_TAG,
                  GTK_TYPE_TEXT_ITER,
                  GTK_TYPE_TEXT_ITER);

   **
   * TextBuffer::begin-user-action:
   * @textbuffer: the object which received the signal
   * 
   * The ::begin-user-action signal is emitted at the beginning of a single
   * user-visible operation on a #TextBuffer.
   * 
   * See also: 
   * gtk_text_buffer_begin_user_action(),
   * gtk_text_buffer_insert_interactive(),
   * gtk_text_buffer_insert_range_interactive(),
   * gtk_text_buffer_delete_interactive(),
   * gtk_text_buffer_backspace(),
   * gtk_text_buffer_delete_selection().
   * 
  signals[BEGIN_USER_ACTION] =
    g_signal_new (I_("begin-user-action"),
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_LAST,                   
                  G_STRUCT_OFFSET (TextBufferClass, begin_user_action),
                  NULL, NULL,
                  _gtk_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);

   **
   * TextBuffer::end-user-action:
   * @textbuffer: the object which received the signal
   * 
   * The ::end-user-action signal is emitted at the end of a single
   * user-visible operation #TextBuffer.
   * 
   * See also: 
   * gtk_text_buffer_end_user_action(),
   * gtk_text_buffer_insert_interactive(),
   * gtk_text_buffer_insert_range_interactive(),
   * gtk_text_buffer_delete_interactive(),
   * gtk_text_buffer_backspace(),
   * gtk_text_buffer_delete_selection(),
   * gtk_text_buffer_backspace().
   * 
  signals[END_USER_ACTION] =
    g_signal_new (I_("end-user-action"),
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_LAST,                   
                  G_STRUCT_OFFSET (TextBufferClass, end_user_action),
                  NULL, NULL,
                  _gtk_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);

  g_type_class_add_private (object_class, sizeof (TextBufferPrivate));*/

  //buffer->clipboard_contents_buffers = NULL;
  //tag_table = NULL;

  /* allow copying of arbiatray stuff in the internal rich text format */
  //gtk_text_buffer_register_serialize_tagset (buffer, NULL);
}

void TextBuffer::set_table (TextTagTable *table)
{
  TextBuffer* buffer=this;
  g_return_if_fail (tag_table == NULL);

  if (table)
    {
      tag_table = table;
      //g_object_ref (buffer->tag_table);
      table->add_buffer (buffer);
    }
}

TextTagTable* TextBuffer::get_table(void)
{
  if (tag_table == NULL)
    { //TODO careful with freeing memory
      tag_table = new TextTagTable();
      tag_table->add_buffer (this);
    }

  return tag_table;
}

/*static void
gtk_text_buffer_set_property (GObject         *object,
                              guint            prop_id,
                              const GValue    *value,
                              GParamSpec      *pspec)
{
  TextBuffer *text_buffer;

  text_buffer = GTK_TEXT_BUFFER (object);

  switch (prop_id)
    {
    case PROP_TAG_TABLE:
      set_table (text_buffer, g_value_get_object (value));
      break;

    case PROP_TEXT:
      gtk_text_buffer_set_text (text_buffer,
				g_value_get_string (value), -1);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}*/

/*static void
gtk_text_buffer_get_property (GObject         *object,
                              guint            prop_id,
                              GValue          *value,
                              GParamSpec      *pspec)
{
  TextBuffer *text_buffer;
  TextIter iter;

  text_buffer = GTK_TEXT_BUFFER (object);

  switch (prop_id)
    {
    case PROP_TAG_TABLE:
      g_value_set_object (value, get_table (text_buffer));
      break;

    case PROP_TEXT:
      {
        TextIter start, end;

        gtk_text_buffer_get_start_iter (text_buffer, &start);
        gtk_text_buffer_get_end_iter (text_buffer, &end);

        g_value_take_string (value,
                            gtk_text_buffer_get_text (text_buffer,
                                                      &start, &end, FALSE));
        break;
      }

    case PROP_HAS_SELECTION:
      g_value_set_boolean (value, text_buffer->has_selection);
      break;

    case PROP_CURSOR_POSITION:
      gtk_text_buffer_get_iter_at_mark (text_buffer, &iter, 
    				        gtk_text_buffer_get_insert (text_buffer));
      g_value_set_int (value, gtk_text_iter_get_offset (&iter));
      break;

    case PROP_COPY_TARGET_LIST:
      g_value_set_boxed (value, gtk_text_buffer_get_copy_target_list (text_buffer));
      break;

    case PROP_PASTE_TARGET_LIST:
      g_value_set_boxed (value, gtk_text_buffer_get_paste_target_list (text_buffer));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}*/

/*static void
gtk_text_buffer_notify (GObject    *object,
                        GParamSpec *pspec)
{
  if (!strcmp (pspec->name, "copy-target-list") ||
      !strcmp (pspec->name, "paste-target-list"))
    {
      gtk_text_buffer_free_target_lists (GTK_TEXT_BUFFER (object));
    }
}*/

/**
 * gtk_text_buffer_new:
 * @table: a tag table, or %NULL to create a new one
 *
 * Creates a new text buffer.
 *
 * Return value: a new text buffer
 **/
TextBuffer::TextBuffer (TextTagTable *table)
: tag_table(table)
, btree(NULL)
, log_attr_cache(NULL)
, user_action_count(0)
, modified(0)
, has_selection(0)
{
}

TextBuffer::~TextBuffer (void)
{
  //remove_all_selection_clipboards (buffer);

  if (tag_table)
    {
      tag_table->remove_buffer(this);
      //g_object_unref (buffer->tag_table);
      tag_table = NULL;
    }

  if (btree)
    {
      //_gtk_text_btree_unref (buffer->btree);
      //TODO make sure we never get a btree other than creating it ourselves
      delete btree;
      btree = NULL;
    }

  /*if (log_attr_cache)
    free_log_attr_cache (log_attr_cache);

  log_attr_cache = NULL;*/

  //TODO?
  //free_target_lists ();

  //G_OBJECT_CLASS (gtk_text_buffer_parent_class)->finalize (object);
}

TextBTree* TextBuffer::get_btree (void)
{
  if (btree == NULL)
    btree = new TextBTree(get_tag_table(), this);

  return btree;
}

/**
 * gtk_text_buffer_get_tag_table:
 * @buffer: a #TextBuffer
 *
 * Get the #TextTagTable associated with this buffer.
 *
 * Return value: the buffer's tag table
 **/
TextTagTable* TextBuffer::get_tag_table (void)
{
  return get_table ();
}

/**
 * gtk_text_buffer_set_text:
 * @buffer: a #TextBuffer
 * @text: UTF-8 text to insert
 * @len: length of @text in bytes
 *
 * Deletes current contents of @buffer, and inserts @text instead. If
 * @len is -1, @text must be nul-terminated. @text must be valid UTF-8.
 **/
void TextBuffer::set_text (const gchar *text, gint len)
{
  TextIter start, end;

  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (text != NULL);

  if (len < 0)
    len = strlen (text);

  get_bounds (&start, &end);

  delete_text (&start, &end);

  if (len > 0)
    {
      get_iter_at_offset (&start, 0);
      insert (&start, text, len);
    }
  
  //TODO g_object_notify (G_OBJECT (buffer), "text");
}

 

/*
 * Insertion
 */

//TODO merge with the other insert_text function
void TextBuffer::real_insert_text (TextIter   *iter,
                                  const gchar   *text,
                                  gint           len)
{
  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (iter != NULL);
  
  TextBTree::insert_text(iter, text, len);

  //TODO g_signal_emit (buffer, signals[CHANGED], 0);
  //g_object_notify (G_OBJECT (buffer), "cursor-position");
}

void TextBuffer::emit_insert (
                             TextIter   *iter,
                             const gchar   *text,
                             gint           len)
{
  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (iter != NULL);
  g_return_if_fail (text != NULL);

  if (len < 0)
    len = strlen (text);

  g_return_if_fail (g_utf8_validate (text, len, NULL));
  
  if (len > 0)
    {
      real_insert_text (iter, text, len);
	//TODO what to do here? preferably without signal does the line above work?
      //g_signal_emit (buffer, signals[INSERT_TEXT], 0,
      //               iter, text, len);
    }
}

/**
 * gtk_text_buffer_insert:
 * @buffer: a #TextBuffer
 * @iter: a position in the buffer
 * @text: UTF-8 format text to insert
 * @len: length of text in bytes, or -1
 *
 * Inserts @len bytes of @text at position @iter.  If @len is -1,
 * @text must be nul-terminated and will be inserted in its
 * entirety. Emits the "insert-text" signal; insertion actually occurs
 * in the default handler for the signal. @iter is invalidated when
 * insertion occurs (because the buffer contents change), but the
 * default signal handler revalidates it to point to the end of the
 * inserted text.
 **/
void TextBuffer::insert (
                        TextIter *iter,
                        const gchar   *text,
                        gint           len)
{
  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (iter != NULL);
  g_return_if_fail (text != NULL);
  g_return_if_fail (iter->get_buffer() == this);
  
  real_insert_text(iter, text, len);
  //gtk_text_buffer_emit_insert (buffer, iter, text, len);
}

/**
 * gtk_text_buffer_insert_at_cursor:
 * @buffer: a #TextBuffer
 * @text: some text in UTF-8 format
 * @len: length of text, in bytes
 *
 * Simply calls gtk_text_buffer_insert(), using the current
 * cursor position as the insertion point.
 **/
void TextBuffer::insert_at_cursor (
                                  const gchar   *text,
                                  gint           len)
{
  TextIter iter;

  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (text != NULL);

  get_iter_at_mark (&iter, get_insert());

  insert (&iter, text, len);
}

/**
 * gtk_text_buffer_insert_interactive:
 * @buffer: a #TextBuffer
 * @iter: a position in @buffer
 * @text: some UTF-8 text
 * @len: length of text in bytes, or -1
 * @default_editable: default editability of buffer
 *
 * Like gtk_text_buffer_insert(), but the insertion will not occur if
 * @iter is at a non-editable location in the buffer. Usually you
 * want to prevent insertions at ineditable locations if the insertion
 * results from a user action (is interactive).
 *
 * @default_editable indicates the editability of text that doesn't
 * have a tag affecting editability applied to it. Typically the
 * result of gtk_text_view_get_editable() is appropriate here.
 *
 * Return value: whether text was actually inserted
 **/
bool TextBuffer::insert_interactive (
                                    TextIter   *iter,
                                    const gchar   *text,
                                    gint           len,
                                    bool       default_editable)
{
  g_return_val_if_fail (text != NULL, FALSE);
  g_return_val_if_fail (iter->get_buffer() == this, FALSE);

  if (iter->can_insert(default_editable))
    {
      begin_user_action ();
      emit_insert (iter, text, len);
      end_user_action ();
      return TRUE;
    }
  else
    return FALSE;
}

/**
 * gtk_text_buffer_insert_interactive_at_cursor:
 * @buffer: a #TextBuffer
 * @text: text in UTF-8 format
 * @len: length of text in bytes, or -1
 * @default_editable: default editability of buffer
 *
 * Calls gtk_text_buffer_insert_interactive() at the cursor
 * position.
 *
 * @default_editable indicates the editability of text that doesn't
 * have a tag affecting editability applied to it. Typically the
 * result of gtk_text_view_get_editable() is appropriate here.
 * 
 * Return value: whether text was actually inserted
 **/
bool TextBuffer::insert_interactive_at_cursor (
                                              const gchar   *text,
                                              gint           len,
                                              bool       default_editable)
{
  TextIter iter;

  g_return_val_if_fail (text != NULL, FALSE);

  get_iter_at_mark (&iter, get_insert ());

  return insert_interactive (&iter, text, len,
                                             default_editable);
}

bool TextBuffer::possibly_not_text (gunichar ch, gpointer user_data)
{
  return ch == TEXT_UNKNOWN_CHAR;
}

void TextBuffer::insert_text_range (
                   TextIter       *iter,
                   TextIter *orig_start,
                   TextIter *orig_end,
                   bool           interactive)
{
  gchar *text;

  text = iter->get_text (orig_start, orig_end);

  //TODO do actual insert
  //gtk_text_buffer_emit_insert (buffer, iter, text, -1);

  g_free (text);
}

//TODO move up and/or to .h
typedef struct
{
  TextBuffer *buffer;
  TextMark *start_mark;
  TextMark *end_mark;
  TextMark *whole_end_mark;
  TextIter *range_start;
  TextIter *range_end;
  TextIter *whole_end;
} Range;

static Range*
save_range (TextIter *range_start,
            TextIter *range_end,
            TextIter *whole_end)
{
  Range *r;

  r = g_new (Range, 1);

  r->buffer = range_start->get_buffer();
  //g_object_ref (r->buffer); TODO??
  
  r->start_mark = 
    range_start->get_buffer()->create_mark (
                                 NULL,
                                 range_start,
                                 FALSE);
  r->end_mark = 
    range_start->get_buffer()->create_mark (
                                 NULL,
                                 range_end,
                                 TRUE);

  r->whole_end_mark = 
    range_start->get_buffer()->create_mark (
                                 NULL,
                                 whole_end,
                                 TRUE);

  r->range_start = range_start;
  r->range_end = range_end;
  r->whole_end = whole_end;

  return r;
}

static void
restore_range (Range *r)
{
  r->buffer->get_iter_at_mark(
                                    r->range_start,
                                    r->start_mark);
      
  r->buffer->get_iter_at_mark(
                                    r->range_end,
                                    r->end_mark);
      
  r->buffer->get_iter_at_mark(
                                    r->whole_end,
                                    r->whole_end_mark);  
  
  r->buffer->delete_mark (r->start_mark);
  r->buffer->delete_mark (r->end_mark);
  r->buffer->delete_mark (r->whole_end_mark);

  /* Due to the gravities on the marks, the ordering could have
   * gotten mangled; we switch to an empty range in that
   * case
   */
  
  if (TextIter::compare (r->range_start, r->range_end) > 0)
    *r->range_start = *r->range_end;

  if (TextIter::compare (r->range_end, r->whole_end) > 0)
    *r->range_end = *r->whole_end;
  
  //TODOg_object_unref (r->buffer);
  g_free (r); 
}

void TextBuffer::insert_range_untagged (
                       TextIter       *iter,
                       TextIter *orig_start,
                       TextIter *orig_end,
                       bool           interactive)
{
  TextIter range_start;
  TextIter range_end;
  TextIter start, end;
  Range *r;
  
  if (TextIter::equal (orig_start, orig_end))
    return;

  start = *orig_start;
  end = *orig_end;
  
  range_start = start;
  range_end = start;
  
  while (TRUE)
    {
      if (TextIter::equal (&range_start, &range_end))
        {
          /* Figure out how to move forward */

          g_assert (TextIter::compare (&range_end, &end) <= 0);
          
          if (TextIter::equal (&range_end, &end))
            {
              /* nothing left to do */
              break;
            }
          else if ((&range_end)->get_char() == TEXT_UNKNOWN_CHAR)
            {
              //GdkPixbuf *pixbuf = NULL;
              TextChildAnchor *anchor = NULL;
              //pixbuf = gtk_text_iter_get_pixbuf (&range_end);
              //anchor = (&range_end)->get_child_anchor();

              /*if (pixbuf)
                {
                  r = save_range (&range_start,
                                  &range_end,
                                  &end);

                  gtk_text_buffer_insert_pixbuf (buffer,
                                                 iter,
                                                 pixbuf);

                  restore_range (r);
                  r = NULL;
                  
                  gtk_text_iter_forward_char (&range_end);
                  
                  range_start = range_end;
                }
              else*/ if (anchor)
                {
                  /* Just skip anchors */

                  (&range_end)->forward_char();
                  range_start = range_end;
                }
              else
                {
                  /* The TEXT_UNKNOWN_CHAR was in a text segment, so
                   * keep going. 
                   */
                  (&range_end)->forward_find_char(
                                                   &TextBuffer::possibly_not_text, NULL,
                                                   &end);
                  
                  g_assert (TextIter::compare (&range_end, &end) <= 0);
                }
            }
          else
            {
              /* Text segment starts here, so forward search to
               * find its possible endpoint
               */
              (&range_end)->forward_find_char(
                                               &TextBuffer::possibly_not_text, NULL,
                                               &end);
              
              g_assert (TextIter::compare (&range_end, &end) <= 0);
            }
        }
      else
        {
          r = save_range (&range_start,
                          &range_end,
                          &end);
          
          insert_text_range ( iter,
                             &range_start,
                             &range_end,
                             interactive);

          restore_range (r);
          r = NULL;
          
          range_start = range_end;
        }
    }
}

void TextBuffer::insert_range_not_inside_self (
                              TextIter       *iter,
                              TextIter *orig_start,
                              TextIter *orig_end,
                              bool           interactive)
{
  /* Find each range of uniformly-tagged text, insert it,
   * then apply the tags.
   */
  TextIter start = *orig_start;
  TextIter end = *orig_end;
  TextIter range_start;
  TextIter range_end;
  
  if (TextIter::equal (orig_start, orig_end))
    return;
  
  TextIter::TextIter::order (&start, &end);

  range_start = start;
  range_end = start;  
  
  while (TRUE)
    {
      gint start_offset;
      TextIter start_iter;
      GSList *tags;
      GSList *tmp_list;
      Range *r;
      
      if (TextIter::equal (&range_start, &end))
        break; /* All done */

      g_assert (TextIter::compare (&range_start, &end) < 0);
      
      (&range_end)->forward_to_tag_toggle(NULL);

      g_assert (!TextIter::equal (&range_start, &range_end));

      /* Clamp to the end iterator */
      if (TextIter::compare (&range_end, &end) > 0)
        range_end = end;
      
      /* We have a range with unique tags; insert it, and
       * apply all tags.
       */
      start_offset = iter->get_offset();

      r = save_range (&range_start, &range_end, &end);
      
      insert_range_untagged (iter, &range_start, &range_end, interactive);

      restore_range (r);
      r = NULL;
      
      get_iter_at_offset( &start_iter, start_offset);
      
      tags = (&range_start)->get_tags();
      tmp_list = tags;
      while (tmp_list != NULL)
        {
          apply_tag(
                                     (TextTag*)tmp_list->data,
                                     &start_iter,
                                     iter);
          
          tmp_list = g_slist_next (tmp_list);
        }
      g_slist_free (tags);

      range_start = range_end;
    }
}

void TextBuffer::real_insert_range (
                                   TextIter       *iter,
                                   TextIter *orig_start,
                                   TextIter *orig_end,
                                   bool           interactive)
{
  TextBuffer *src_buffer;
  
  /* Find each range of uniformly-tagged text, insert it,
   * then apply the tags.
   */  
  if (TextIter::equal (orig_start, orig_end))
    return;

  if (interactive)
    begin_user_action();
  
  src_buffer = orig_start->get_buffer();
  
  if (iter->get_buffer() != src_buffer ||
      !iter->in_range(orig_start, orig_end))
    {
      insert_range_not_inside_self(iter, orig_start, orig_end, interactive);
    }
  else
    {
      /* If you insert a range into itself, it could loop infinitely
       * because the region being copied keeps growing as we insert. So
       * we have to separately copy the range before and after
       * the insertion point.
       */
      TextIter start = *orig_start;
      TextIter end = *orig_end;
      TextIter range_start;
      TextIter range_end;
      Range *first_half;
      Range *second_half;

      TextIter::order (&start, &end);
      
      range_start = start;
      range_end = *iter;
      first_half = save_range (&range_start, &range_end, &end);

      range_start = *iter;
      range_end = end;
      second_half = save_range (&range_start, &range_end, &end);

      restore_range (first_half);
      insert_range_not_inside_self (iter, &range_start, &range_end, interactive);

      restore_range (second_half);
      insert_range_not_inside_self (iter, &range_start, &range_end, interactive);
    }
  
  if (interactive)
    end_user_action ();
}

/**
 * gtk_text_buffer_insert_range:
 * @buffer: a #TextBuffer
 * @iter: a position in @buffer
 * @start: a position in a #TextBuffer
 * @end: another position in the same buffer as @start
 *
 * Copies text, tags, and pixbufs between @start and @end (the order
 * of @start and @end doesn't matter) and inserts the copy at @iter.
 * Used instead of simply getting/inserting text because it preserves
 * images and tags. If @start and @end are in a different buffer from
 * @buffer, the two buffers must share the same tag table.
 *
 * Implemented via emissions of the insert_text and apply_tag signals,
 * so expect those.
 **/
void TextBuffer::insert_range (TextIter *iter, TextIter *start, TextIter *end)
{
  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (iter != NULL);
  g_return_if_fail (start != NULL);
  g_return_if_fail (end != NULL);
  g_return_if_fail (start->get_buffer() == end->get_buffer());
  g_return_if_fail (start->get_buffer()->tag_table == tag_table);  
  g_return_if_fail (iter->get_buffer() == this);
  
  real_insert_range (iter, start, end, FALSE);
}

/**
 * gtk_text_buffer_insert_range_interactive:
 * @buffer: a #TextBuffer
 * @iter: a position in @buffer
 * @start: a position in a #TextBuffer
 * @end: another position in the same buffer as @start
 * @default_editable: default editability of the buffer
 *
 * Same as gtk_text_buffer_insert_range(), but does nothing if the
 * insertion point isn't editable. The @default_editable parameter
 * indicates whether the text is editable at @iter if no tags
 * enclosing @iter affect editability. Typically the result of
 * gtk_text_view_get_editable() is appropriate here.
 *
 * Returns: whether an insertion was possible at @iter
 **/
bool TextBuffer::insert_range_interactive(TextIter *iter, TextIter *start, TextIter *end, bool default_editable)
{
  g_return_val_if_fail (iter != NULL, FALSE);
  g_return_val_if_fail (start != NULL, FALSE);
  g_return_val_if_fail (end != NULL, FALSE);
  g_return_val_if_fail (start->get_buffer() == end->get_buffer(), FALSE);
  g_return_val_if_fail (start->get_buffer()->tag_table == tag_table, FALSE);

  if (iter->can_insert( default_editable))
    {
      real_insert_range (iter, start, end, TRUE);
      return TRUE;
    }
  else
    return FALSE;
}

/**
 * gtk_text_buffer_insert_with_tags:
 * @buffer: a #TextBuffer
 * @iter: an iterator in @buffer
 * @text: UTF-8 text
 * @len: length of @text, or -1
 * @first_tag: first tag to apply to @text
 * @Varargs: NULL-terminated list of tags to apply
 *
 * Inserts @text into @buffer at @iter, applying the list of tags to
 * the newly-inserted text. The last tag specified must be NULL to
 * terminate the list. Equivalent to calling gtk_text_buffer_insert(),
 * then gtk_text_buffer_apply_tag() on the inserted text;
 * gtk_text_buffer_insert_with_tags() is just a convenience function.
 **/
void TextBuffer::insert_with_tags (
                                  TextIter   *iter,
                                  const gchar   *text,
                                  gint           len,
                                  TextTag    *first_tag,
                                  ...)
{
  gint start_offset;
  TextIter start;
  va_list args;
  TextTag *tag;

  g_return_if_fail (iter != NULL);
  g_return_if_fail (text != NULL);
  g_return_if_fail (iter->get_buffer() == this);
  
  start_offset = iter->get_offset ();

  real_insert_text (iter, text, len);

  if (first_tag == NULL)
    return;

  get_iter_at_offset (&start, start_offset);

  va_start (args, first_tag);
  tag = first_tag;
  while (tag)
    {
      apply_tag (tag, &start, iter);

      tag = va_arg (args, TextTag*);
    }

  va_end (args);
}

/**
 * gtk_text_buffer_insert_with_tags_by_name:
 * @buffer: a #TextBuffer
 * @iter: position in @buffer
 * @text: UTF-8 text
 * @len: length of @text, or -1
 * @first_tag_name: name of a tag to apply to @text
 * @Varargs: more tag names
 *
 * Same as gtk_text_buffer_insert_with_tags(), but allows you
 * to pass in tag names instead of tag objects.
 **/
void TextBuffer::insert_with_tags_by_name  (
                                           TextIter   *iter,
                                           const gchar   *text,
                                           gint           len,
                                           const gchar   *first_tag_name,
                                           ...)
{
  gint start_offset;
  TextIter start;
  va_list args;
  const gchar *tag_name;

  g_return_if_fail (iter != NULL);
  g_return_if_fail (text != NULL);
  g_return_if_fail (iter->get_buffer() == this);
  
  start_offset = iter->get_offset();

  real_insert_text (iter, text, len);

  if (first_tag_name == NULL)
    return;

  get_iter_at_offset (&start, start_offset);

  va_start (args, first_tag_name);
  tag_name = first_tag_name;
  while (tag_name)
    {
      TextTag *tag;

      tag = tag_table->lookup ( tag_name);

      if (tag == NULL)
        {
          g_warning ("%s: no tag with name '%s'!", G_STRLOC, tag_name);
          return;
        }

      apply_tag (tag, &start, iter);

      tag_name = va_arg (args, const gchar*);
    }

  va_end (args);
}


/*
 * Deletion
 */

void TextBuffer::real_delete_range ( TextIter   *start,
                                   TextIter   *end)
{
  bool has_selection;

  g_return_if_fail (start != NULL);
  g_return_if_fail (end != NULL);

  TextBTree::delete_text (start, end);

  /* may have deleted the selection... */
  //update_selection_clipboards (buffer);

  has_selection = get_selection_bounds (NULL, NULL);
  if (has_selection != this->has_selection)
    {
      this->has_selection = has_selection;
      //TODOg_object_notify (G_OBJECT (buffer), "has-selection");
    }

  //TODOg_signal_emit (buffer, signals[CHANGED], 0);
  //TODOg_object_notify (G_OBJECT (buffer), "cursor-position");
}

void TextBuffer::emit_delete (TextIter *start, TextIter *end)
{
  g_return_if_fail (start != NULL);
  g_return_if_fail (end != NULL);

  if (TextIter::equal (start, end))
    return;

  TextIter::order (start, end);

  //TODO do actual delete
  /*g_signal_emit (buffer,
                 signals[DELETE_RANGE],
                 0,
                 start, end);*/
}

/**
 * gtk_text_buffer_delete:
 * @buffer: a #TextBuffer
 * @start: a position in @buffer
 * @end: another position in @buffer
 *
 * Deletes text between @start and @end. The order of @start and @end
 * is not actually relevant; gtk_text_buffer_delete() will reorder
 * them. This function actually emits the "delete-range" signal, and
 * the default handler of that signal deletes the text. Because the
 * buffer is modified, all outstanding iterators become invalid after
 * calling this function; however, the @start and @end will be
 * re-initialized to point to the location where text was deleted.
 **/
void TextBuffer::delete_text (
                        TextIter   *start,
                        TextIter   *end)
{
  g_return_if_fail (start != NULL);
  g_return_if_fail (end != NULL);
  g_return_if_fail (start->get_buffer() == this);
  g_return_if_fail (end->get_buffer() == this);
  
  emit_delete (start, end);
}

/**
 * gtk_text_buffer_delete_interactive:
 * @buffer: a #TextBuffer
 * @start_iter: start of range to delete
 * @end_iter: end of range
 * @default_editable: whether the buffer is editable by default
 *
 * Deletes all <emphasis>editable</emphasis> text in the given range.
 * Calls gtk_text_buffer_delete() for each editable sub-range of
 * [@start,@end). @start and @end are revalidated to point to
 * the location of the last deleted range, or left untouched if
 * no text was deleted.
 *
 * Return value: whether some text was actually deleted
 **/
bool TextBuffer::delete_text_interactive (
                                    TextIter   *start_iter,
                                    TextIter   *end_iter,
                                    bool       default_editable)
{
  TextMark *end_mark;
  TextMark *start_mark;
  TextIter iter;
  bool current_state;
  bool deleted_stuff = FALSE;

  /* Delete all editable text in the range start_iter, end_iter */

  g_return_val_if_fail (start_iter != NULL, FALSE);
  g_return_val_if_fail (end_iter != NULL, FALSE);
  g_return_val_if_fail (start_iter->get_buffer() == this, FALSE);
  g_return_val_if_fail (end_iter->get_buffer() == this, FALSE);

  
  begin_user_action ();
  
  TextIter::order (start_iter, end_iter);

  start_mark = create_mark (NULL, start_iter, TRUE);
  end_mark = create_mark (NULL, end_iter, FALSE);

  get_iter_at_mark (&iter, start_mark);

  current_state = (&iter)->editable (default_editable);

  while (TRUE)
    {
      bool new_state;
      bool done = FALSE;
      TextIter end;

      (&iter)->forward_to_tag_toggle (NULL);

      get_iter_at_mark (&end, end_mark);

      if (TextIter::compare (&iter, &end) >= 0)
        {
          done = TRUE;
          iter = end; /* clamp to the last boundary */
        }

      new_state = (&iter)->editable (default_editable);

      if (current_state == new_state)
        {
          if (done)
            {
              if (current_state)
                {
                  /* We're ending an editable region. Delete said region. */
                  TextIter start;

                  get_iter_at_mark (&start, start_mark);

                  emit_delete (&start, &iter);

                  deleted_stuff = TRUE;

                  /* revalidate user's iterators. */
                  *start_iter = start;
                  *end_iter = iter;
                }

              break;
            }
          else
            continue;
        }

      if (current_state && !new_state)
        {
          /* End of an editable region. Delete it. */
          TextIter start;

          get_iter_at_mark (&start, start_mark);

          emit_delete (&start, &iter);

	  /* It's more robust to ask for the state again then to assume that
	   * we're on the next not-editable segment. We don't know what the
	   * ::delete-range handler did.... maybe it deleted the following
           * not-editable segment because it was associated with the editable
           * segment.
	   */
	  current_state = (&iter)->editable (default_editable);
          deleted_stuff = TRUE;

          /* revalidate user's iterators. */
          *start_iter = start;
          *end_iter = iter;
        }
      else
        {
          /* We are at the start of an editable region. We won't be deleting
           * the previous region. Move start mark to start of this region.
           */

          g_assert (!current_state && new_state);

          move_mark (start_mark, &iter);

          current_state = TRUE;
        }

      if (done)
        break;
    }

  delete_mark (start_mark);
  delete_mark (end_mark);

  end_user_action ();
  
  return deleted_stuff;
}

/*
 * Extracting textual buffer contents
 */

/**
 * gtk_text_buffer_get_text:
 * @buffer: a #TextBuffer
 * @start: start of a range
 * @end: end of a range
 * @include_hidden_chars: whether to include invisible text
 *
 * Returns the text in the range [@start,@end). Excludes undisplayed
 * text (text marked with tags that set the invisibility attribute) if
 * @include_hidden_chars is %FALSE. Does not include characters
 * representing embedded images, so byte and character indexes into
 * the returned string do <emphasis>not</emphasis> correspond to byte
 * and character indexes into the buffer. Contrast with
 * gtk_text_buffer_get_slice().
 *
 * Return value: an allocated UTF-8 string
 **/
gchar* TextBuffer::get_text (
                          TextIter *start,
                          TextIter *end,
                          bool include_hidden_chars)
{
  g_return_val_if_fail (start != NULL, NULL);
  g_return_val_if_fail (end != NULL, NULL);
  g_return_val_if_fail (start->get_buffer() == this, NULL);
  g_return_val_if_fail (end->get_buffer() == this, NULL);
  
  if (include_hidden_chars)
    return TextIter::get_text (start, end);
  else
    return TextIter::get_visible_text (start, end);
}

/**
 * gtk_text_buffer_get_slice:
 * @buffer: a #TextBuffer
 * @start: start of a range
 * @end: end of a range
 * @include_hidden_chars: whether to include invisible text
 *
 * Returns the text in the range [@start,@end). Excludes undisplayed
 * text (text marked with tags that set the invisibility attribute) if
 * @include_hidden_chars is %FALSE. The returned string includes a
 * 0xFFFC character whenever the buffer contains
 * embedded images, so byte and character indexes into
 * the returned string <emphasis>do</emphasis> correspond to byte
 * and character indexes into the buffer. Contrast with
 * gtk_text_buffer_get_text(). Note that 0xFFFC can occur in normal
 * text as well, so it is not a reliable indicator that a pixbuf or
 * widget is in the buffer.
 *
 * Return value: an allocated UTF-8 string
 **/
gchar* TextBuffer::get_slice (
                           TextIter *start,
                           TextIter *end,
                           bool           include_hidden_chars)
{
  g_return_val_if_fail (start != NULL, NULL);
  g_return_val_if_fail (end != NULL, NULL);
  g_return_val_if_fail (start->get_buffer() == this, NULL);
  g_return_val_if_fail (end->get_buffer() == this, NULL);
  
  if (include_hidden_chars)
    return TextIter::get_slice (start, end);
  else
    return TextIter::get_visible_slice (start, end);
}

/*
 * Pixbufs
 */
/*
static void
gtk_text_buffer_real_insert_pixbuf (TextBuffer *buffer,
                                    TextIter   *iter,
                                    GdkPixbuf     *pixbuf)
{ 
  _gtk_text_btree_insert_pixbuf (iter, pixbuf);

  g_signal_emit (buffer, signals[CHANGED], 0);
}
*/
/**
 * gtk_text_buffer_insert_pixbuf:
 * @buffer: a #TextBuffer
 * @iter: location to insert the pixbuf
 * @pixbuf: a #GdkPixbuf
 *
 * Inserts an image into the text buffer at @iter. The image will be
 * counted as one character in character counts, and when obtaining
 * the buffer contents as a string, will be represented by the Unicode
 * "object replacement character" 0xFFFC. Note that the "slice"
 * variants for obtaining portions of the buffer as a string include
 * this character for pixbufs, but the "text" variants do
 * not. e.g. see gtk_text_buffer_get_slice() and
 * gtk_text_buffer_get_text().
 **/
/*void
gtk_text_buffer_insert_pixbuf (TextBuffer *buffer,
                               TextIter   *iter,
                               GdkPixbuf     *pixbuf)
{
  g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (iter != NULL);
  g_return_if_fail (GDK_IS_PIXBUF (pixbuf));
  g_return_if_fail (gtk_text_iter_get_buffer (iter) == buffer);
  
  g_signal_emit (buffer, signals[INSERT_PIXBUF], 0,
                 iter, pixbuf);
}
*/
/*
 * Child anchor
 */


/*
void TextBuffer::real_insert_anchor (TextIter *iter, TextChildAnchor *anchor)
{
  TextBTree::insert_child_anchor (iter, anchor);

  //TODOg_signal_emit (buffer, signals[CHANGED], 0);
}*/

/**
 * gtk_text_buffer_insert_child_anchor:
 * @buffer: a #TextBuffer
 * @iter: location to insert the anchor
 * @anchor: a #TextChildAnchor
 *
 * Inserts a child widget anchor into the text buffer at @iter. The
 * anchor will be counted as one character in character counts, and
 * when obtaining the buffer contents as a string, will be represented
 * by the Unicode "object replacement character" 0xFFFC. Note that the
 * "slice" variants for obtaining portions of the buffer as a string
 * include this character for child anchors, but the "text" variants do
 * not. E.g. see gtk_text_buffer_get_slice() and
 * gtk_text_buffer_get_text(). Consider
 * gtk_text_buffer_create_child_anchor() as a more convenient
 * alternative to this function. The buffer will add a reference to
 * the anchor, so you can unref it after insertion.
 **/
void TextBuffer::insert_child_anchor (TextIter *iter, TextChildAnchor *anchor)
{
  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (iter != NULL);
  //TODO test for null? g_return_if_fail (GTK_IS_TEXT_CHILD_ANCHOR (anchor));
  g_return_if_fail (iter->get_buffer() == this);
  
  //TODOg_signal_emit (buffer, signals[INSERT_CHILD_ANCHOR], 0,
  //               iter, anchor);
}

/**
 * gtk_text_buffer_create_child_anchor:
 * @buffer: a #TextBuffer
 * @iter: location in the buffer
 * 
 * This is a convenience function which simply creates a child anchor
 * with gtk_text_child_anchor_new() and inserts it into the buffer
 * with gtk_text_buffer_insert_child_anchor(). The new anchor is
 * owned by the buffer; no reference count is returned to
 * the caller of gtk_text_buffer_create_child_anchor().
 * 
 * Return value: the created child anchor
 **/
TextChildAnchor* TextBuffer::create_child_anchor(TextIter *iter)
{
  TextChildAnchor *anchor;
  
  g_return_val_if_fail (iter != NULL, NULL);
  g_return_val_if_fail (iter->get_buffer() == this, NULL);
  
  anchor = new TextChildAnchor(); //TODO carefull with freeing

  insert_child_anchor (iter, anchor);

  //TODOg_object_unref (anchor);

  return anchor;
}

/*
 * Mark manipulation
 */

/**
 * gtk_text_buffer_set_mark:
 * @buffer:       a #TextBuffer
 * @mark_name:    name of the mark
 * @iter:         location for the mark
 * @left_gravity: if the mark is created by this function, gravity for 
 *                the new mark
 * @should_exist: if %TRUE, warn if the mark does not exist, and return
 *                immediately
 *
 * Move the mark to the given position, if not @should_exist, 
 * create the mark.
 *
 * Return value: mark
 **/
TextMark* TextBuffer::set_mark (
                          TextMark       *existing_mark,
                          const gchar       *mark_name,
                          TextIter *iter,
                          bool           left_gravity,
                          bool           should_exist)
{
  TextIter location;
  TextMark *mark;

  g_return_val_if_fail (iter->get_buffer() == this, NULL);
  
  mark = get_btree()->set_mark(
                                   existing_mark,
                                   mark_name,
                                   left_gravity,
                                   iter,
                                   should_exist);
  
  get_btree()->get_iter_at_mark ( &location, mark);

  mark_set (&location, mark);

  return mark;
}

/**
 * gtk_text_buffer_create_mark:
 * @buffer: a #TextBuffer
 * @mark_name: name for mark, or %NULL
 * @where: location to place mark
 * @left_gravity: whether the mark has left gravity
 *
 * Creates a mark at position @where. If @mark_name is %NULL, the mark
 * is anonymous; otherwise, the mark can be retrieved by name using
 * gtk_text_buffer_get_mark(). If a mark has left gravity, and text is
 * inserted at the mark's current location, the mark will be moved to
 * the left of the newly-inserted text. If the mark has right gravity
 * (@left_gravity = %FALSE), the mark will end up on the right of
 * newly-inserted text. The standard left-to-right cursor is a mark
 * with right gravity (when you type, the cursor stays on the right
 * side of the text you're typing).
 *
 * The caller of this function does <emphasis>not</emphasis> own a 
 * reference to the returned #TextMark, so you can ignore the 
 * return value if you like. Marks are owned by the buffer and go 
 * away when the buffer does.
 *
 * Emits the "mark-set" signal as notification of the mark's initial
 * placement.
 *
 * Return value: the new #TextMark object
 **/
TextMark* TextBuffer::create_mark (
                             const gchar       *mark_name,
                             TextIter *where,
                             bool           left_gravity)
{
  return set_mark (NULL, mark_name, where, left_gravity, FALSE);
}

/**
 * gtk_text_buffer_add_mark:
 * @buffer: a #TextBuffer
 * @mark: the mark to add
 * @where: location to place mark
 *
 * Adds the mark at position @where. The mark must not be added to
 * another buffer, and if its name is not %NULL then there must not
 * be another mark in the buffer with the same name.
 *
 * Emits the "mark-set" signal as notification of the mark's initial
 * placement.
 *
 * Since: 2.12
 **/
void TextBuffer::add_mark (
                          TextMark       *mark,
                          TextIter *where)
{
  const gchar *name;

  //TODO test for null? g_return_if_fail (GTK_IS_TEXT_MARK (mark));
  //TODO test fror null?? g_return_if_fail (where != NULL);
  g_return_if_fail (mark->get_buffer() == NULL);

  name = mark->get_name ();

  if (name != NULL && get_mark (name) != NULL)
    {
      g_critical ("Mark %s already exists in the buffer", name);
      return;
    }

  set_mark (mark, NULL, where, FALSE, FALSE);
}

/**
 * gtk_text_buffer_move_mark:
 * @buffer: a #TextBuffer
 * @mark: a #TextMark
 * @where: new location for @mark in @buffer
 *
 * Moves @mark to the new location @where. Emits the "mark-set" signal
 * as notification of the move.
 **/
void TextBuffer::move_mark (
                           TextMark       *mark,
                           TextIter *where)
{
  //TODO test for null? g_return_if_fail (GTK_IS_TEXT_MARK (mark));
  g_return_if_fail (!mark->get_deleted ());
  //TODO check null g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));

  set_mark (mark, NULL, where, FALSE, TRUE);
}

/**
 * gtk_text_buffer_get_iter_at_mark:
 * @buffer: a #TextBuffer
 * @iter: iterator to initialize
 * @mark: a #TextMark in @buffer
 *
 * Initializes @iter with the current position of @mark.
 **/
void TextBuffer::get_iter_at_mark (
                                  TextIter   *iter,
                                  TextMark   *mark)
{
  //g_return_if_fail (GTK_IS_TEXT_MARK (mark));
  g_return_if_fail (!mark->get_deleted ());
  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));

  get_btree()->get_iter_at_mark ( iter, mark);
}

/**
 * gtk_text_buffer_delete_mark:
 * @buffer: a #TextBuffer
 * @mark: a #TextMark in @buffer
 *
 * Deletes @mark, so that it's no longer located anywhere in the
 * buffer. Removes the reference the buffer holds to the mark, so if
 * you haven't called g_object_ref() on the mark, it will be freed. Even
 * if the mark isn't freed, most operations on @mark become
 * invalid, until it gets added to a buffer again with 
 * gtk_text_buffer_add_mark(). Use gtk_text_mark_get_deleted() to  
 * find out if a mark has been removed from its buffer.
 * The "mark-deleted" signal will be emitted as notification after 
 * the mark is deleted.
 **/
void TextBuffer::delete_mark (
                             TextMark   *mark)
{
  //g_return_if_fail (GTK_IS_TEXT_MARK (mark));
  g_return_if_fail (!mark->get_deleted ());
  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));

  //TODOg_object_ref (mark);

  get_btree()->remove_mark(mark);

  /* See rationale above for MARK_SET on why we emit this after
   * removing the mark, rather than removing the mark in a default
   * handler.
   */
  /*TODO g_signal_emit (buffer, signals[MARK_DELETED],
                 0,
                 mark);*/

  //TODOg_object_unref (mark);
}

/**
 * gtk_text_buffer_get_mark:
 * @buffer: a #TextBuffer
 * @name: a mark name
 *
 * Returns the mark named @name in buffer @buffer, or %NULL if no such
 * mark exists in the buffer.
 *
 * Return value: a #TextMark, or %NULL
 **/
TextMark* TextBuffer::get_mark (
                          const gchar   *name)
{
  TextMark *mark;

  //g_return_val_if_fail (GTK_IS_TEXT_BUFFER (buffer), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  mark = get_btree()->get_mark_by_name(name);

  return mark;
}

/**
 * gtk_text_buffer_move_mark_by_name:
 * @buffer: a #TextBuffer
 * @name: name of a mark
 * @where: new location for mark
 *
 * Moves the mark named @name (which must exist) to location @where.
 * See gtk_text_buffer_move_mark() for details.
 **/
void TextBuffer::move_mark_by_name (
                                   const gchar       *name,
                                   TextIter *where)
{
  TextMark *mark;

  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (name != NULL);

  mark = get_btree()->get_mark_by_name(name);

  if (mark == NULL)
    {
      g_warning ("%s: no mark named '%s'", G_STRLOC, name);
      return;
    }

  move_mark (mark, where);
}

/**
 * gtk_text_buffer_delete_mark_by_name:
 * @buffer: a #TextBuffer
 * @name: name of a mark in @buffer
 *
 * Deletes the mark named @name; the mark must exist. See
 * gtk_text_buffer_delete_mark() for details.
 **/
void TextBuffer::delete_mark_by_name ( const gchar   *name)
{
  TextMark *mark;

 // g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (name != NULL);

  mark = get_btree()->get_mark_by_name(name);

  if (mark == NULL)
    {
      g_warning ("%s: no mark named '%s'", G_STRLOC, name);
      return;
    }

  delete_mark (mark);
}

/**
 * gtk_text_buffer_get_insert:
 * @buffer: a #TextBuffer
 *
 * Returns the mark that represents the cursor (insertion point).
 * Equivalent to calling gtk_text_buffer_get_mark() to get the mark
 * named "insert", but very slightly more efficient, and involves less
 * typing.
 *
 * Return value: insertion point mark
 **/
TextMark* TextBuffer::get_insert (void)
{
  //g_return_val_if_fail (GTK_IS_TEXT_BUFFER (buffer), NULL);

	//TODO get_btree does not need to be a function? so replace everywhere
  return get_btree()->get_insert();
}

/**
 * gtk_text_buffer_get_selection_bound:
 * @buffer: a #TextBuffer
 *
 * Returns the mark that represents the selection bound.  Equivalent
 * to calling gtk_text_buffer_get_mark() to get the mark named
 * "selection_bound", but very slightly more efficient, and involves
 * less typing.
 *
 * The currently-selected text in @buffer is the region between the
 * "selection_bound" and "insert" marks. If "selection_bound" and
 * "insert" are in the same place, then there is no current selection.
 * gtk_text_buffer_get_selection_bounds() is another convenient function
 * for handling the selection, if you just want to know whether there's a
 * selection and what its bounds are.
 *
 * Return value: selection bound mark
 **/
TextMark* TextBuffer::get_selection_bound (void)
{
  //g_return_val_if_fail (GTK_IS_TEXT_BUFFER (buffer), NULL);

  return get_btree()->get_selection_bound ();
}

/**
 * gtk_text_buffer_get_iter_at_child_anchor:
 * @buffer: a #TextBuffer
 * @iter: an iterator to be initialized
 * @anchor: a child anchor that appears in @buffer
 *
 * Obtains the location of @anchor within @buffer.
 **/
void TextBuffer::get_iter_at_child_anchor (
                                          TextIter        *iter,
                                          TextChildAnchor *anchor)
{
  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (iter != NULL);
  //g_return_if_fail (GTK_IS_TEXT_CHILD_ANCHOR (anchor));
  //TODO we dont support text childs g_return_if_fail (!anchor->get_deleted ());
  
  get_btree()->get_iter_at_child_anchor (iter, anchor);
}

/**
 * gtk_text_buffer_place_cursor:
 * @buffer: a #TextBuffer
 * @where: where to put the cursor
 *
 * This function moves the "insert" and "selection_bound" marks
 * simultaneously.  If you move them to the same place in two steps
 * with gtk_text_buffer_move_mark(), you will temporarily select a
 * region in between their old and new locations, which can be pretty
 * inefficient since the temporarily-selected region will force stuff
 * to be recalculated. This function moves them as a unit, which can
 * be optimized.
 **/
void TextBuffer::place_cursor (const TextIter *where)
{
  select_range (where, where);
}

/**
 * gtk_text_buffer_select_range:
 * @buffer: a #TextBuffer
 * @ins: where to put the "insert" mark
 * @bound: where to put the "selection_bound" mark
 *
 * This function moves the "insert" and "selection_bound" marks
 * simultaneously.  If you move them in two steps
 * with gtk_text_buffer_move_mark(), you will temporarily select a
 * region in between their old and new locations, which can be pretty
 * inefficient since the temporarily-selected region will force stuff
 * to be recalculated. This function moves them as a unit, which can
 * be optimized.
 *
 * Since: 2.4
 **/
void TextBuffer::select_range (
			      const TextIter *ins,
                              const TextIter *bound)
{
  TextIter real_ins;
  TextIter real_bound;

  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));

  real_ins = *ins;
  real_bound = *bound;

  get_btree()->select_range (&real_ins, &real_bound);
  mark_set (&real_ins, get_insert ());
  mark_set (&real_bound, get_selection_bound ());
}

void TextBuffer::select_all(bool select)
{
  TextIter start_iter, end_iter, insert;

  if (select) 
    {
      get_bounds (&start_iter, &end_iter);
      select_range (&start_iter, &end_iter);
    }
  else 
    {
      get_iter_at_mark (&insert, get_insert ());
      move_mark_by_name ("selection_bound", &insert);
    }

  //TODO signal that redraw is needed
  //Redraw();
}

/*
 * Tags
 */

/**
 * gtk_text_buffer_create_tag:
 * @buffer: a #TextBuffer
 * @tag_name: name of the new tag, or %NULL
 * @first_property_name: name of first property to set, or %NULL
 * @Varargs: %NULL-terminated list of property names and values
 *
 *
 * Creates a tag and adds it to the tag table for @buffer.
 * Equivalent to calling gtk_text_tag_new() and then adding the
 * tag to the buffer's tag table. The returned tag is owned by
 * the buffer's tag table, so the ref count will be equal to one.
 *
 * If @tag_name is %NULL, the tag is anonymous.
 *
 * If @tag_name is non-%NULL, a tag called @tag_name must not already
 * exist in the tag table for this buffer.
 *
 * The @first_property_name argument and subsequent arguments are a list
 * of properties to set on the tag, as with g_object_set().
 *
 * Return value: a new tag
 **/
TextTag* TextBuffer::create_tag (
                            const gchar   *tag_name,
                            const gchar   *first_property_name,
                            ...)
{
  TextTag *tag;
  va_list list;
  
  //g_return_val_if_fail (GTK_IS_TEXT_BUFFER (buffer), NULL);

  tag = new TextTag(tag_name);

  get_table()->add_tag (tag);

  if (first_property_name)
    {
      va_start (list, first_property_name);
      //TODOg_object_set_valist (G_OBJECT (tag), first_property_name, list);
      va_end (list);
    }
  
  //TODOg_object_unref (tag);

  return tag;
}

void TextBuffer::real_changed (void)
{
  set_modified (TRUE);
}

void TextBuffer::mark_set ( const TextIter *iter, TextMark *mark)
{
  TextMark *insert;
  
  insert = get_insert ();

  if (mark == insert || mark == get_selection_bound ())
    {
      bool has_selection;

      //TODOupdate_selection_clipboards (buffer);
    
      has_selection = get_selection_bounds (
                                                            NULL,
                                                            NULL);

      if (has_selection != this->has_selection)
        {
          this->has_selection = has_selection;
          //TODOg_object_notify (G_OBJECT (buffer), "has-selection");
        }
    }
    
    if (mark == insert)
    {}//TODOg_object_notify (G_OBJECT (buffer), "cursor-position");
}

void TextBuffer::emit_tag (
                          TextTag        *tag,
                          bool           apply,
                          const TextIter *start,
                          const TextIter *end)
{
  TextIter start_tmp = *start;
  TextIter end_tmp = *end;

  g_return_if_fail (tag != NULL);

  TextIter::order (&start_tmp, &end_tmp);

  if (apply)
  {} /*TODOg_signal_emit (buffer, signals[APPLY_TAG],
                   0,
                   tag, &start_tmp, &end_tmp);*/
  else
  {}/*TODOg_signal_emit (buffer, signals[REMOVE_TAG],
                   0,
                   tag, &start_tmp, &end_tmp);*/
}

/**
 * gtk_text_buffer_apply_tag:
 * @buffer: a #TextBuffer
 * @tag: a #TextTag
 * @start: one bound of range to be tagged
 * @end: other bound of range to be tagged
 *
 * Emits the "apply-tag" signal on @buffer. The default
 * handler for the signal applies @tag to the given range.
 * @start and @end do not have to be in order.
 **/
void TextBuffer::apply_tag (
                           TextTag        *tag,
                           TextIter *start,
                           TextIter *end)
{
  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
  //g_return_if_fail (GTK_IS_TEXT_TAG (tag));
  g_return_if_fail (start != NULL);
  g_return_if_fail (end != NULL);
  g_return_if_fail (start->get_buffer() == this);
  g_return_if_fail (end->get_buffer() == this);
  g_return_if_fail (tag->table == tag_table);
  
  //TODO remove this check?
  if (tag->table != tag_table)
    {
      g_warning ("Can only apply tags that are in the tag table for the buffer");
      return;
    }
  
  TextBTree::tag (start, end, tag, TRUE);

}

/**
 * gtk_text_buffer_remove_tag:
 * @buffer: a #TextBuffer
 * @tag: a #TextTag
 * @start: one bound of range to be untagged
 * @end: other bound of range to be untagged
 *
 * Emits the "remove-tag" signal. The default handler for the signal
 * removes all occurrences of @tag from the given range. @start and
 * @end don't have to be in order.
 **/
void TextBuffer::remove_tag (
                            TextTag        *tag,
                            TextIter *start,
                            TextIter *end)

{
  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
  //g_return_if_fail (GTK_IS_TEXT_TAG (tag));
  g_return_if_fail (start != NULL);
  g_return_if_fail (end != NULL);
  g_return_if_fail (start->get_buffer() == this);
  g_return_if_fail (end->get_buffer() == this);
  g_return_if_fail (tag->table == tag_table);
 
  //TODO remove this check?
  if (tag->table != tag_table)
    {
      g_warning ("Can only remove tags that are in the tag table for the buffer");
      return;
    }
  
  TextBTree::tag (start, end, tag, FALSE);

}

/**
 * gtk_text_buffer_apply_tag_by_name:
 * @buffer: a #TextBuffer
 * @name: name of a named #TextTag
 * @start: one bound of range to be tagged
 * @end: other bound of range to be tagged
 *
 * Calls gtk_text_tag_table_lookup() on the buffer's tag table to
 * get a #TextTag, then calls gtk_text_buffer_apply_tag().
 **/
void TextBuffer::apply_tag_by_name (
                                   const gchar       *name,
                                   TextIter *start,
                                   TextIter *end)
{
  TextTag *tag;

  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (name != NULL);
  g_return_if_fail (start != NULL);
  g_return_if_fail (end != NULL);
  g_return_if_fail (start->get_buffer() == this);
  g_return_if_fail (end->get_buffer() == this);

  tag = get_table()->lookup( name);

  if (tag == NULL)
    {
      g_warning ("Unknown tag `%s'", name);
      return;
    }

  emit_tag (tag, TRUE, start, end);
}

/**
 * gtk_text_buffer_remove_tag_by_name:
 * @buffer: a #TextBuffer
 * @name: name of a #TextTag
 * @start: one bound of range to be untagged
 * @end: other bound of range to be untagged
 *
 * Calls gtk_text_tag_table_lookup() on the buffer's tag table to
 * get a #TextTag, then calls gtk_text_buffer_remove_tag().
 **/
void TextBuffer::remove_tag_by_name (
                                    const gchar       *name,
                                    TextIter *start,
                                    TextIter *end)
{
  TextTag *tag;

  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (name != NULL);
  g_return_if_fail (start != NULL);
  g_return_if_fail (end != NULL);
  g_return_if_fail (start->get_buffer() == this);
  g_return_if_fail (end->get_buffer() == this);
  
  tag = get_table()->lookup( name);

  if (tag == NULL)
    {
      g_warning ("Unknown tag `%s'", name);
      return;
    }

  emit_tag (tag, FALSE, start, end);
}

static gint
pointer_cmp (gconstpointer a,
             gconstpointer b)
{
  if (a < b)
    return -1;
  else if (a > b)
    return 1;
  else
    return 0;
}

/**
 * gtk_text_buffer_remove_all_tags:
 * @buffer: a #TextBuffer
 * @start: one bound of range to be untagged
 * @end: other bound of range to be untagged
 * 
 * Removes all tags in the range between @start and @end.  Be careful
 * with this function; it could remove tags added in code unrelated to
 * the code you're currently writing. That is, using this function is
 * probably a bad idea if you have two or more unrelated code sections
 * that add tags.
 **/
void TextBuffer::remove_all_tags (
                                 TextIter *start,
                                 TextIter *end)
{
  TextIter first, second, tmp;
  GSList *tags;
  GSList *tmp_list;
  GSList *prev, *next;
  TextTag *tag;
  
  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (start != NULL);
  g_return_if_fail (end != NULL);
  g_return_if_fail (start->get_buffer() == this);
  g_return_if_fail (end->get_buffer() == this);
  
  first = *start;
  second = *end;

  TextIter::order (&first, &second);

  /* Get all tags turned on at the start */
  tags = (&first)->get_tags();
  
  /* Find any that are toggled on within the range */
  tmp = first;
  while ((&tmp)->forward_to_tag_toggle (NULL))
    {
      GSList *toggled;
      GSList *tmp_list2;

      if (TextIter::compare (&tmp, &second) >= 0)
        break; /* past the end of the range */
      
      toggled = (&tmp)->get_toggled_tags (TRUE);

      /* We could end up with a really big-ass list here.
       * Fix it someday.
       */
      tmp_list2 = toggled;
      while (tmp_list2 != NULL)
        {
          tags = g_slist_prepend (tags, tmp_list2->data);

          tmp_list2 = g_slist_next (tmp_list2);
        }

      g_slist_free (toggled);
    }
  
  /* Sort the list */
  tags = g_slist_sort (tags, pointer_cmp);

  /* Strip duplicates */
  tag = NULL;
  prev = NULL;
  tmp_list = tags;
  while (tmp_list != NULL)
    {
      if (tag == tmp_list->data)
        {
          /* duplicate */
          next = tmp_list->next;
          if (prev)
            prev->next = next;

          tmp_list->next = NULL;

          g_slist_free (tmp_list);

          tmp_list = next;
          /* prev is unchanged */
        }
      else
        {
          /* not a duplicate */
          tag = (TextTag*)(tmp_list->data);
          prev = tmp_list;
          tmp_list = tmp_list->next;
        }
    }

  //TODOg_slist_foreach (tags, (GFunc) g_object_ref, NULL);
  
  tmp_list = tags;
  while (tmp_list != NULL)
    {
      tag = (TextTag*)(tmp_list->data);

      remove_tag (tag, &first, &second);
      
      tmp_list = tmp_list->next;
    }

  //TODOg_slist_foreach (tags, (GFunc) g_object_unref, NULL);
  
  g_slist_free (tags);
}


/*
 * Obtain various iterators
 */

/**
 * gtk_text_buffer_get_iter_at_line_offset:
 * @buffer: a #TextBuffer
 * @iter: iterator to initialize
 * @line_number: line number counting from 0
 * @char_offset: char offset from start of line
 *
 * Obtains an iterator pointing to @char_offset within the given
 * line. The @char_offset must exist, offsets off the end of the line
 * are not allowed. Note <emphasis>characters</emphasis>, not bytes;
 * UTF-8 may encode one character as multiple bytes.
 **/
void TextBuffer::get_iter_at_line_offset (
                                         TextIter   *iter,
                                         gint           line_number,
                                         gint           char_offset)
{
  g_return_if_fail (iter != NULL);
  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));

  get_btree()->get_iter_at_line_char ( iter, line_number, char_offset);
}

/**
 * gtk_text_buffer_get_iter_at_line_index:
 * @buffer: a #TextBuffer 
 * @iter: iterator to initialize 
 * @line_number: line number counting from 0
 * @byte_index: byte index from start of line
 *
 * Obtains an iterator pointing to @byte_index within the given line.
 * @byte_index must be the start of a UTF-8 character, and must not be
 * beyond the end of the line.  Note <emphasis>bytes</emphasis>, not
 * characters; UTF-8 may encode one character as multiple bytes.
 **/
void TextBuffer::get_iter_at_line_index  (
                                         TextIter   *iter,
                                         gint           line_number,
                                         gint           byte_index)
{
  g_return_if_fail (iter != NULL);
  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));

  get_btree()->get_iter_at_line_byte ( iter, line_number, byte_index);
}

/**
 * gtk_text_buffer_get_iter_at_line:
 * @buffer: a #TextBuffer 
 * @iter: iterator to initialize
 * @line_number: line number counting from 0
 * 
 * Initializes @iter to the start of the given line.
 **/
void TextBuffer::get_iter_at_line (
                                  TextIter   *iter,
                                  gint           line_number)
{
  g_return_if_fail (iter != NULL);
  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));

  get_iter_at_line_offset (iter, line_number, 0);
}

/**
 * gtk_text_buffer_get_iter_at_offset:
 * @buffer: a #TextBuffer 
 * @iter: iterator to initialize
 * @char_offset: char offset from start of buffer, counting from 0, or -1
 *
 * Initializes @iter to a position @char_offset chars from the start
 * of the entire buffer. If @char_offset is -1 or greater than the number
 * of characters in the buffer, @iter is initialized to the end iterator,
 * the iterator one past the last valid character in the buffer.
 **/
void TextBuffer::get_iter_at_offset (
                                    TextIter   *iter,
                                    gint           char_offset)
{
  g_return_if_fail (iter != NULL);
  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));

  get_btree()->get_iter_at_char (iter, char_offset);
}

/**
 * gtk_text_buffer_get_start_iter:
 * @buffer: a #TextBuffer
 * @iter: iterator to initialize
 *
 * Initialized @iter with the first position in the text buffer. This
 * is the same as using gtk_text_buffer_get_iter_at_offset() to get
 * the iter at character offset 0.
 **/
void TextBuffer::get_start_iter (TextIter   *iter)
{
  g_return_if_fail (iter != NULL);
  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));

  get_btree()->get_iter_at_char (iter, 0);
}

/**
 * gtk_text_buffer_get_end_iter:
 * @buffer: a #TextBuffer 
 * @iter: iterator to initialize
 *
 * Initializes @iter with the "end iterator," one past the last valid
 * character in the text buffer. If dereferenced with
 * gtk_text_iter_get_char(), the end iterator has a character value of
 * 0. The entire buffer lies in the range from the first position in
 * the buffer (call gtk_text_buffer_get_start_iter() to get
 * character position 0) to the end iterator.
 **/
void TextBuffer::get_end_iter (
                              TextIter   *iter)
{
  g_return_if_fail (iter != NULL);
  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));

  get_btree()->get_end_iter (iter);
}

/**
 * gtk_text_buffer_get_bounds:
 * @buffer: a #TextBuffer 
 * @start: iterator to initialize with first position in the buffer
 * @end: iterator to initialize with the end iterator
 *
 * Retrieves the first and last iterators in the buffer, i.e. the
 * entire buffer lies within the range [@start,@end).
 **/
void TextBuffer::get_bounds (
                            TextIter   *start,
                            TextIter   *end)
{
  g_return_if_fail (start != NULL);
  g_return_if_fail (end != NULL);
  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));

  get_btree()->get_iter_at_char (start, 0);
  get_btree()->get_end_iter (end);
}

/*
 * Modified flag
 */

/**
 * gtk_text_buffer_get_modified:
 * @buffer: a #TextBuffer 
 * 
 * Indicates whether the buffer has been modified since the last call
 * to gtk_text_buffer_set_modified() set the modification flag to
 * %FALSE. Used for example to enable a "save" function in a text
 * editor.
 * 
 * Return value: %TRUE if the buffer has been modified
 **/
bool TextBuffer::get_modified (void)
{
  //g_return_val_if_fail (GTK_IS_TEXT_BUFFER (buffer), FALSE);

  return modified;
}

/**
 * gtk_text_buffer_set_modified:
 * @buffer: a #TextBuffer 
 * @setting: modification flag setting
 *
 * Used to keep track of whether the buffer has been modified since the
 * last time it was saved. Whenever the buffer is saved to disk, call
 * gtk_text_buffer_set_modified (@buffer, FALSE). When the buffer is modified,
 * it will automatically toggled on the modified bit again. When the modified
 * bit flips, the buffer emits a "modified-changed" signal.
 **/
void TextBuffer::set_modified (
                              bool       setting)
{
  bool fixed_setting;

  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));

  fixed_setting = setting != FALSE;

  if (modified == fixed_setting)
    return;
  else
    {
      modified = fixed_setting;
      //TODOg_signal_emit (buffer, signals[MODIFIED_CHANGED], 0);
    }
}

/**
 * gtk_text_buffer_get_has_selection:
 * @buffer: a #TextBuffer 
 * 
 * Indicates whether the buffer has some text currently selected.
 * 
 * Return value: %TRUE if the there is text selected
 *
 * Since: 2.10
 **/
bool TextBuffer::get_has_selection (void)
{
  //g_return_val_if_fail (GTK_IS_TEXT_BUFFER (buffer), FALSE);

  return has_selection;
}


/*
 * Assorted other stuff
 */

/**
 * gtk_text_buffer_get_line_count:
 * @buffer: a #TextBuffer 
 * 
 * Obtains the number of lines in the buffer. This value is cached, so
 * the function is very fast.
 * 
 * Return value: number of lines in the buffer
 **/
gint TextBuffer::get_line_count (void)
{
  //g_return_val_if_fail (GTK_IS_TEXT_BUFFER (buffer), 0);

  return get_btree()->get_line_count ();
}

/**
 * gtk_text_buffer_get_char_count:
 * @buffer: a #TextBuffer 
 * 
 * Gets the number of characters in the buffer; note that characters
 * and bytes are not the same, you can't e.g. expect the contents of
 * the buffer in string form to be this many bytes long. The character
 * count is cached, so this function is very fast.
 * 
 * Return value: number of characters in the buffer
 **/
gint TextBuffer::get_char_count (void)
{
  //g_return_val_if_fail (GTK_IS_TEXT_BUFFER (buffer), 0);

  return get_btree()->get_char_count ();
}

/* Called when we lose the primary selection.
 */
/*
static void
clipboard_clear_selection_cb (GtkClipboard *clipboard,
                              gpointer      data)
{
  * Move selection_bound to the insertion point *
  TextIter insert;
  TextIter selection_bound;
  TextBuffer *buffer = GTK_TEXT_BUFFER (data);

  gtk_text_buffer_get_iter_at_mark (buffer, &insert,
                                    gtk_text_buffer_get_insert (buffer));
  gtk_text_buffer_get_iter_at_mark (buffer, &selection_bound,
                                    gtk_text_buffer_get_selection_bound (buffer));

  if (!TextIter::equal (&insert, &selection_bound))
    gtk_text_buffer_move_mark (buffer,
                               gtk_text_buffer_get_selection_bound (buffer),
                               &insert);
}*/

/* Called when we have the primary selection and someone else wants our
 * data in order to paste it.
 */
/*static void
clipboard_get_selection_cb (GtkClipboard     *clipboard,
                            GtkSelectionData *selection_data,
                            guint             info,
                            gpointer          data)
{
  TextBuffer *buffer = GTK_TEXT_BUFFER (data);
  TextIter start, end;

  if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
    {
      if (info == GTK_TEXT_BUFFER_TARGET_INFO_BUFFER_CONTENTS)
        {
          * Provide the address of the buffer; this will only be
           * used within-process
           *
          gtk_selection_data_set (selection_data,
                                  selection_data->target,
                                  8, * bytes *
                                  (void*)&buffer,
                                  sizeof (buffer));
        }
      else if (info == GTK_TEXT_BUFFER_TARGET_INFO_RICH_TEXT)
        {
          guint8 *str;
          gsize   len;

          str = gtk_text_buffer_serialize (buffer, buffer,
                                           selection_data->target,
                                           &start, &end, &len);

          gtk_selection_data_set (selection_data,
                                  selection_data->target,
                                  8, * bytes *
                                  str, len);
          g_free (str);
        }
      else
        {
          gchar *str;

          str = gtk_text_iter_get_visible_text (&start, &end);
          gtk_selection_data_set_text (selection_data, str, -1);
          g_free (str);
        }
    }
}

static TextBuffer *
create_clipboard_contents_buffer (TextBuffer *buffer)
{
  TextBuffer *contents;

  contents = gtk_text_buffer_new (gtk_text_buffer_get_tag_table (buffer));

  g_object_set_data (G_OBJECT (contents), I_("gtk-text-buffer-clipboard-source"),
                     buffer);
  g_object_set_data (G_OBJECT (contents), I_("gtk-text-buffer-clipboard"),
                     GINT_TO_POINTER (1));

  *  Ref the source buffer as long as the clipboard contents buffer
   *  exists, because it's needed for serializing the contents buffer.
   *  See http://bugzilla.gnome.org/show_bug.cgi?id=339195
   *
  g_object_ref (buffer);
  g_object_weak_ref (G_OBJECT (contents), (GWeakNotify) g_object_unref, buffer);

  return contents;
}

* Provide cut/copied data *
static void
clipboard_get_contents_cb (GtkClipboard     *clipboard,
                           GtkSelectionData *selection_data,
                           guint             info,
                           gpointer          data)
{
  TextBuffer *contents = GTK_TEXT_BUFFER (data);

  g_assert (contents); * This should never be called unless we own the clipboard *

  if (info == GTK_TEXT_BUFFER_TARGET_INFO_BUFFER_CONTENTS)
    {
      * Provide the address of the clipboard buffer; this will only
       * be used within-process. OK to supply a NULL value for contents.
       *
      gtk_selection_data_set (selection_data,
                              selection_data->target,
                              8, * bytes *
                              (void*)&contents,
                              sizeof (contents));
    }
  else if (info == GTK_TEXT_BUFFER_TARGET_INFO_RICH_TEXT)
    {
      TextBuffer *clipboard_source_buffer;
      TextIter start, end;
      guint8 *str;
      gsize   len;

      clipboard_source_buffer = g_object_get_data (G_OBJECT (contents),
                                                   "gtk-text-buffer-clipboard-source");

      gtk_text_buffer_get_bounds (contents, &start, &end);

      str = gtk_text_buffer_serialize (clipboard_source_buffer, contents,
                                       selection_data->target,
                                       &start, &end, &len);

      gtk_selection_data_set (selection_data,
			      selection_data->target,
			      8, * bytes *
			      str, len);
      g_free (str);
    }
  else
    {
      gchar *str;
      TextIter start, end;

      gtk_text_buffer_get_bounds (contents, &start, &end);

      str = gtk_text_iter_get_visible_text (&start, &end);
      gtk_selection_data_set_text (selection_data, str, -1);
      g_free (str);
    }
}

static void
clipboard_clear_contents_cb (GtkClipboard *clipboard,
                             gpointer      data)
{
  TextBuffer *contents = GTK_TEXT_BUFFER (data);

  g_object_unref (contents);
}*/

/*
static void
get_paste_point (TextBuffer *buffer,
                 TextIter   *iter,
                 bool       clear_afterward)
{
  TextIter insert_point;
  TextMark *paste_point_override;

  paste_point_override = gtk_text_buffer_get_mark (buffer,
                                                   "gtk_paste_point_override");

  if (paste_point_override != NULL)
    {
      gtk_text_buffer_get_iter_at_mark (buffer, &insert_point,
                                        paste_point_override);
      if (clear_afterward)
        gtk_text_buffer_delete_mark (buffer,
                                     gtk_text_buffer_get_mark (buffer,
                                                               "gtk_paste_point_override"));
    }
  else
    {
      gtk_text_buffer_get_iter_at_mark (buffer, &insert_point,
                                        gtk_text_buffer_get_insert (buffer));
    }

  *iter = insert_point;
}

static void
pre_paste_prep (ClipboardRequest *request_data,
                TextIter      *insert_point)
{
  TextBuffer *buffer = request_data->buffer;
  
  get_paste_point (buffer, insert_point, TRUE);

  * If we're going to replace the selection, we insert before it to
   * avoid messing it up, then we delete the selection after inserting.
   *
  if (request_data->replace_selection)
    {
      TextIter start, end;
      
      if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
        *insert_point = start;
    }
}

static void
post_paste_cleanup (ClipboardRequest *request_data)
{
  if (request_data->replace_selection)
    {
      TextIter start, end;
      
      if (gtk_text_buffer_get_selection_bounds (request_data->buffer,
                                                &start, &end))
        {
          if (request_data->interactive)
            gtk_text_buffer_delete_interactive (request_data->buffer,
                                                &start,
                                                &end,
                                                request_data->default_editable);
          else
            gtk_text_buffer_delete (request_data->buffer, &start, &end);
        }
    }
}

static void
free_clipboard_request (ClipboardRequest *request_data)
{
  g_object_unref (request_data->buffer);
  g_free (request_data);
}

* Called when we request a paste and receive the text data
 *
static void
clipboard_text_received (GtkClipboard *clipboard,
                         const gchar  *str,
                         gpointer      data)
{
  ClipboardRequest *request_data = data;
  TextBuffer *buffer = request_data->buffer;

  if (str)
    {
      TextIter insert_point;
      
      if (request_data->interactive) 
	begin_user_action();

      pre_paste_prep (request_data, &insert_point);
      
      if (request_data->interactive) 
	gtk_text_buffer_insert_interactive (buffer, &insert_point,
					    str, -1, request_data->default_editable);
      else
        gtk_text_buffer_insert (buffer, &insert_point,
                                str, -1);

      post_paste_cleanup (request_data);
      
      if (request_data->interactive) 
	end_user_action();
    }

  free_clipboard_request (request_data);
}*/

/*
TextBuffer* TextBuffer::selection_data_get_buffer (GtkSelectionData *selection_data,
                           ClipboardRequest *request_data)
{
  GdkWindow *owner;
  TextBuffer *src_buffer = NULL;

  * If we can get the owner, the selection is in-process *
  owner = gdk_selection_owner_get_for_display (selection_data->display,
					       selection_data->selection);

  if (owner == NULL)
    return NULL;
  
  if (gdk_window_get_window_type (owner) == GDK_WINDOW_FOREIGN)
    return NULL;
 
  if (selection_data->type !=
      gdk_atom_intern_static_string ("GTK_TEXT_BUFFER_CONTENTS"))
    return NULL;

  if (selection_data->length != sizeof (src_buffer))
    return NULL;
          
  memcpy (&src_buffer, selection_data->data, sizeof (src_buffer));

  if (src_buffer == NULL)
    return NULL;
  
  g_return_val_if_fail (GTK_IS_TEXT_BUFFER (src_buffer), NULL);

  if (gtk_text_buffer_get_tag_table (src_buffer) !=
      gtk_text_buffer_get_tag_table (request_data->buffer))
    return NULL;
  
  return src_buffer;
}*/

#if 0
/* These are pretty handy functions; maybe something like them
 * should be in the public API. Also, there are other places in this
 * file where they could be used.
 */
static gpointer
save_iter (const TextIter *iter,
           bool           left_gravity)
{
  return gtk_text_buffer_create_mark (gtk_text_iter_get_buffer (iter),
                                      NULL,
                                      iter,
                                      TRUE);
}

static void
restore_iter (const TextIter *iter,
              gpointer           save_id)
{
  gtk_text_buffer_get_iter_at_mark (gtk_text_mark_get_buffer (save_id),
                                    (TextIter*) iter,
                                    save_id);
  gtk_text_buffer_delete_mark (gtk_text_mark_get_buffer (save_id),
                               save_id);
}
#endif

/*
static void
clipboard_rich_text_received (GtkClipboard *clipboard,
                              GdkAtom       format,
                              const guint8 *text,
                              gsize         length,
                              gpointer      data)
{
  ClipboardRequest *request_data = data;
  TextIter insert_point;
  bool retval = TRUE;
  GError *error = NULL;

  if (text != NULL && length > 0)
    {
      pre_paste_prep (request_data, &insert_point);

      if (request_data->interactive)
        gtk_text_buffer_begin_user_action (request_data->buffer);

      if (!request_data->interactive ||
          gtk_text_iter_can_insert (&insert_point,
                                    request_data->default_editable))
        {
          retval = gtk_text_buffer_deserialize (request_data->buffer,
                                                request_data->buffer,
                                                format,
                                                &insert_point,
                                                text, length,
                                                &error);
        }

      if (!retval)
        {
          g_warning ("error pasting: %s\n", error->message);
          g_clear_error (&error);
        }

      if (request_data->interactive)
        gtk_text_buffer_end_user_action (request_data->buffer);

      if (retval)
        {
          post_paste_cleanup (request_data);
          return;
        }
    }

  * Request the text selection instead *
  gtk_clipboard_request_text (clipboard,
                              clipboard_text_received,
                              data);
}

static void
paste_from_buffer (ClipboardRequest  *request_data,
                   TextBuffer     *src_buffer,
                   const TextIter *start,
                   const TextIter *end)
{
  TextIter insert_point;
  TextBuffer *buffer = request_data->buffer;
  
  * We're about to emit a bunch of signals, so be safe *
  g_object_ref (src_buffer);
  
  pre_paste_prep (request_data, &insert_point);
  
  if (request_data->interactive) 
    begin_user_action();

  if (!TextIter::equal (start, end))
    {
      if (!request_data->interactive ||
          (gtk_text_iter_can_insert (&insert_point,
                                     request_data->default_editable)))
        gtk_text_buffer_real_insert_range (buffer,
                                           &insert_point,
                                           start,
                                           end,
                                           request_data->interactive);
    }

  post_paste_cleanup (request_data);
      
  if (request_data->interactive) 
    end_user_action();

  g_object_unref (src_buffer);

  free_clipboard_request (request_data);
}

static void
clipboard_clipboard_buffer_received (GtkClipboard     *clipboard,
                                     GtkSelectionData *selection_data,
                                     gpointer          data)
{
  ClipboardRequest *request_data = data;
  TextBuffer *src_buffer;

  src_buffer = selection_data_get_buffer (selection_data, request_data); 

  if (src_buffer)
    {
      TextIter start, end;

      if (g_object_get_data (G_OBJECT (src_buffer), "gtk-text-buffer-clipboard"))
	{
	  gtk_text_buffer_get_bounds (src_buffer, &start, &end);

	  paste_from_buffer (request_data, src_buffer,
			     &start, &end);
	}
      else
	{
	  if (gtk_text_buffer_get_selection_bounds (src_buffer, &start, &end))
	    paste_from_buffer (request_data, src_buffer,
			       &start, &end);
	}
    }
  else
    {
      if (gtk_clipboard_wait_is_rich_text_available (clipboard,
                                                     request_data->buffer))
        {
          * Request rich text *
          gtk_clipboard_request_rich_text (clipboard,
                                           request_data->buffer,
                                           clipboard_rich_text_received,
                                           data);
        }
      else
        {
          * Request the text selection instead *
          gtk_clipboard_request_text (clipboard,
                                      clipboard_text_received,
                                      data);
        }
    }
}

typedef struct
{
  GtkClipboard *clipboard;
  guint ref_count;
} SelectionClipboard;

static void
update_selection_clipboards (TextBuffer *buffer)
{
  TextBufferPrivate *priv;
  GSList               *tmp_list = buffer->selection_clipboards;

  priv = GTK_TEXT_BUFFER_GET_PRIVATE (buffer);

  gtk_text_buffer_get_copy_target_list (buffer);

  while (tmp_list)
    {
      TextIter start;
      TextIter end;
      
      SelectionClipboard *selection_clipboard = tmp_list->data;
      GtkClipboard *clipboard = selection_clipboard->clipboard;

      * Determine whether we have a selection and adjust X selection
       * accordingly.
       *
      if (!gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
	{
	  if (gtk_clipboard_get_owner (clipboard) == G_OBJECT (buffer))
	    gtk_clipboard_clear (clipboard);
	}
      else
	{
	  * Even if we already have the selection, we need to update our
	   * timestamp.
	   *
	  if (!gtk_clipboard_set_with_owner (clipboard,
                                             priv->copy_target_entries,
                                             priv->n_copy_target_entries,
					     clipboard_get_selection_cb,
					     clipboard_clear_selection_cb,
					     G_OBJECT (buffer)))
	    clipboard_clear_selection_cb (clipboard, buffer);
	}

      tmp_list = tmp_list->next;
    }
}

static SelectionClipboard *
find_selection_clipboard (TextBuffer *buffer,
			  GtkClipboard  *clipboard)
{
  GSList *tmp_list = buffer->selection_clipboards;
  while (tmp_list)
    {
      SelectionClipboard *selection_clipboard = tmp_list->data;
      if (selection_clipboard->clipboard == clipboard)
	return selection_clipboard;
      
      tmp_list = tmp_list->next;
    }

  return NULL;
}

**
 * gtk_text_buffer_add_selection_clipboard:
 * @buffer: a #TextBuffer
 * @clipboard: a #GtkClipboard
 * 
 * Adds @clipboard to the list of clipboards in which the selection 
 * contents of @buffer are available. In most cases, @clipboard will be 
 * the #GtkClipboard of type %GDK_SELECTION_PRIMARY for a view of @buffer.
 **
void
gtk_text_buffer_add_selection_clipboard (TextBuffer *buffer,
					 GtkClipboard  *clipboard)
{
  SelectionClipboard *selection_clipboard;

  g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (clipboard != NULL);

  selection_clipboard = find_selection_clipboard (buffer, clipboard);
  if (selection_clipboard)
    {
      selection_clipboard->ref_count++;
    }
  else
    {
      selection_clipboard = g_new (SelectionClipboard, 1);

      selection_clipboard->clipboard = clipboard;
      selection_clipboard->ref_count = 1;

      buffer->selection_clipboards = g_slist_prepend (buffer->selection_clipboards, selection_clipboard);
    }
}

**
 * gtk_text_buffer_remove_selection_clipboard:
 * @buffer: a #TextBuffer
 * @clipboard: a #GtkClipboard added to @buffer by 
 *             gtk_text_buffer_add_selection_clipboard()
 * 
 * Removes a #GtkClipboard added with 
 * gtk_text_buffer_add_selection_clipboard().
 **
void 
gtk_text_buffer_remove_selection_clipboard (TextBuffer *buffer,
					    GtkClipboard  *clipboard)
{
  SelectionClipboard *selection_clipboard;

  g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (clipboard != NULL);

  selection_clipboard = find_selection_clipboard (buffer, clipboard);
  g_return_if_fail (selection_clipboard != NULL);

  selection_clipboard->ref_count--;
  if (selection_clipboard->ref_count == 0)
    {
      if (gtk_clipboard_get_owner (selection_clipboard->clipboard) == G_OBJECT (buffer))
	gtk_clipboard_clear (selection_clipboard->clipboard);

      buffer->selection_clipboards = g_slist_remove (buffer->selection_clipboards,
                                                     selection_clipboard);
      
      g_free (selection_clipboard);
    }
}

static void
remove_all_selection_clipboards (TextBuffer *buffer)
{
  g_slist_foreach (buffer->selection_clipboards, (GFunc)g_free, NULL);
  g_slist_free (buffer->selection_clipboards);
  buffer->selection_clipboards = NULL;
}

**
 * gtk_text_buffer_paste_clipboard:
 * @buffer: a #TextBuffer
 * @clipboard: the #GtkClipboard to paste from
 * @override_location: location to insert pasted text, or %NULL for 
 *                     at the cursor
 * @default_editable: whether the buffer is editable by default
 *
 * Pastes the contents of a clipboard at the insertion point, or at 
 * @override_location. (Note: pasting is asynchronous, that is, we'll 
 * ask for the paste data and return, and at some point later after 
 * the main loop runs, the paste data will be inserted.)
 **
void
gtk_text_buffer_paste_clipboard (TextBuffer *buffer,
				 GtkClipboard  *clipboard,
				 TextIter   *override_location,
                                 bool       default_editable)
{
  ClipboardRequest *data = g_new (ClipboardRequest, 1);
  TextIter paste_point;
  TextIter start, end;

  if (override_location != NULL)
    gtk_text_buffer_create_mark (buffer,
                                 "gtk_paste_point_override",
                                 override_location, FALSE);

  data->buffer = g_object_ref (buffer);
  data->interactive = TRUE;
  data->default_editable = default_editable;

  * When pasting with the cursor inside the selection area, you
   * replace the selection with the new text, otherwise, you
   * simply insert the new text at the point where the click
   * occured, unselecting any selected text. The replace_selection
   * flag toggles this behavior.
   *
  data->replace_selection = FALSE;
  
  get_paste_point (buffer, &paste_point, FALSE);
  if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end) &&
      (gtk_text_iter_in_range (&paste_point, &start, &end) ||
       TextIter::equal (&paste_point, &end)))
    data->replace_selection = TRUE;

  gtk_clipboard_request_contents (clipboard,
				  gdk_atom_intern_static_string ("GTK_TEXT_BUFFER_CONTENTS"),
				  clipboard_clipboard_buffer_received, data);
}*/

/**
 * gtk_text_buffer_delete_selection:
 * @buffer: a #TextBuffer 
 * @interactive: whether the deletion is caused by user interaction
 * @default_editable: whether the buffer is editable by default
 *
 * Deletes the range between the "insert" and "selection_bound" marks,
 * that is, the currently-selected text. If @interactive is %TRUE,
 * the editability of the selection will be considered (users can't delete
 * uneditable text).
 * 
 * Return value: whether there was a non-empty selection to delete
 **/
bool TextBuffer::delete_selection (
                                  bool interactive,
                                  bool default_editable)
{
  TextIter start;
  TextIter end;

  if (!get_selection_bounds (&start, &end))
    {
      return FALSE; /* No selection */
    }
  else
    {
      if (interactive)
        {
          begin_user_action ();
          delete_text_interactive (&start, &end, default_editable);
          end_user_action ();
        }
      else
        delete_text (&start, &end);

      return TRUE; /* We deleted stuff */
    }
}

/**
 * gtk_text_buffer_backspace:
 * @buffer: a #TextBuffer
 * @iter: a position in @buffer
 * @interactive: whether the deletion is caused by user interaction
 * @default_editable: whether the buffer is editable by default
 * 
 * Performs the appropriate action as if the user hit the delete
 * key with the cursor at the position specified by @iter. In the
 * normal case a single character will be deleted, but when
 * combining accents are involved, more than one character can
 * be deleted, and when precomposed character and accent combinations
 * are involved, less than one character will be deleted.
 * 
 * Because the buffer is modified, all outstanding iterators become 
 * invalid after calling this function; however, the @iter will be
 * re-initialized to point to the location where text was deleted. 
 *
 * Return value: %TRUE if the buffer was modified
 *
 * Since: 2.6
 **/
bool TextBuffer::backspace (
			   TextIter   *iter,
			   bool       interactive,
			   bool       default_editable)
{
  gchar *cluster_text;
  TextIter start;
  TextIter end;
  bool retval = FALSE;
  //const PangoLogAttr *attrs;
  int offset;
  bool backspace_deletes_character;

  //g_return_val_if_fail (GTK_IS_TEXT_BUFFER (buffer), FALSE);
  g_return_val_if_fail (iter != NULL, FALSE);

  start = *iter;
  end = *iter;

  //attrs = get_line_log_attrs (&start, NULL);

  /* For no good reason, attrs is NULL for the empty last line in
   * a buffer. Special case that here. (#156164)
   */
  /*if (attrs)
    {
      offset = (&start)->get_line_offset();
      backspace_deletes_character = attrs[offset].backspace_deletes_character;
    }
  else*/
    backspace_deletes_character = FALSE;

  (&start)->backward_cursor_position();

  if (TextIter::equal (&start, &end))
    return FALSE;
    
  cluster_text = TextIter::get_text (&start, &end);

  if (interactive)
    begin_user_action();
  
  if (delete_text_interactive(&start, &end, default_editable))
    {
      if (backspace_deletes_character)
	{
	  gchar *normalized_text = g_utf8_normalize (cluster_text,
						     strlen (cluster_text),
						     G_NORMALIZE_NFD);
	  glong len = g_utf8_strlen (normalized_text, -1);
	  
	  if (len > 1)
	    insert_interactive (
						&start,
						normalized_text,
						g_utf8_offset_to_pointer (normalized_text, len - 1) - normalized_text,
						default_editable);
	  
	  g_free (normalized_text);
	}

      retval = TRUE;
    }
  
  if (interactive)
    end_user_action();
  
  g_free (cluster_text);

  /* Revalidate the users iter */
  *iter = start;

  return retval;
}

/*
static void
cut_or_copy (TextBuffer *buffer,
	     GtkClipboard  *clipboard,
             bool       delete_region_after,
             bool       interactive,
             bool       default_editable)
{
  TextBufferPrivate *priv;

  * We prefer to cut the selected region between selection_bound and
   * insertion point. If that region is empty, then we cut the region
   * between the "anchor" and the insertion point (this is for
   * C-space and M-w and other Emacs-style copy/yank keys). Note that
   * insert and selection_bound are guaranteed to exist, but the
   * anchor only exists sometimes.
   *
  TextIter start;
  TextIter end;

  priv = GTK_TEXT_BUFFER_GET_PRIVATE (buffer);

  gtk_text_buffer_get_copy_target_list (buffer);

  if (!gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
    {
      * Let's try the anchor thing *
      TextMark * anchor = gtk_text_buffer_get_mark (buffer, "anchor");

      if (anchor == NULL)
        return;
      else
        {
          gtk_text_buffer_get_iter_at_mark (buffer, &end, anchor);
          TextIter::order (&start, &end);
        }
    }

  if (!TextIter::equal (&start, &end))
    {
      TextIter ins;
      TextBuffer *contents;

      contents = create_clipboard_contents_buffer (buffer);

      gtk_text_buffer_get_iter_at_offset (contents, &ins, 0);
      
      gtk_text_buffer_insert_range (contents, &ins, &start, &end);
                                    
      if (!gtk_clipboard_set_with_data (clipboard,
                                        priv->copy_target_entries,
                                        priv->n_copy_target_entries,
					clipboard_get_contents_cb,
					clipboard_clear_contents_cb,
					contents))
	g_object_unref (contents);
      else
	gtk_clipboard_set_can_store (clipboard,
                                     priv->copy_target_entries + 1,
                                     priv->n_copy_target_entries - 1);

      if (delete_region_after)
        {
          if (interactive)
            delete_interactive&start, &end,
                                                default_editable);
          else
            delete_text (&start, &end);
        }
    }
}*/

/**
 * gtk_text_buffer_cut_clipboard:
 * @buffer: a #TextBuffer
 * @clipboard: the #GtkClipboard object to cut to
 * @default_editable: default editability of the buffer
 *
 * Copies the currently-selected text to a clipboard, then deletes
 * said text if it's editable.
 **/
/*void
gtk_text_buffer_cut_clipboard (TextBuffer *buffer,
			       GtkClipboard  *clipboard,
                               bool       default_editable)
{
  begin_user_action();
  cut_or_copy (buffer, clipboard, TRUE, TRUE, default_editable);
  end_user_action();
}*/

/**
 * gtk_text_buffer_copy_clipboard:
 * @buffer: a #TextBuffer 
 * @clipboard: the #GtkClipboard object to copy to
 *
 * Copies the currently-selected text to a clipboard.
 **/
/*void
gtk_text_buffer_copy_clipboard (TextBuffer *buffer,
				GtkClipboard  *clipboard)
{
  begin_user_action();
  cut_or_copy (buffer, clipboard, FALSE, TRUE, TRUE);
  end_user_action();
}*/

/**
 * gtk_text_buffer_get_selection_bounds:
 * @buffer: a #TextBuffer a #TextBuffer
 * @start: iterator to initialize with selection start
 * @end: iterator to initialize with selection end
 *
 * Returns %TRUE if some text is selected; places the bounds
 * of the selection in @start and @end (if the selection has length 0,
 * then @start and @end are filled in with the same value).
 * @start and @end will be in ascending order. If @start and @end are
 * NULL, then they are not filled in, but the return value still indicates
 * whether text is selected.
 *
 * Return value: whether the selection has nonzero length
 **/
bool TextBuffer::get_selection_bounds ( TextIter   *start, TextIter   *end)
{
  //g_return_val_if_fail (GTK_IS_TEXT_BUFFER (buffer), FALSE);

  return get_btree()->get_selection_bounds (start, end);
}

/**
 * gtk_text_buffer_begin_user_action:
 * @buffer: a #TextBuffer
 * 
 * Called to indicate that the buffer operations between here and a
 * call to gtk_text_buffer_end_user_action() are part of a single
 * user-visible operation. The operations between
 * gtk_text_buffer_begin_user_action() and
 * gtk_text_buffer_end_user_action() can then be grouped when creating
 * an undo stack. #TextBuffer maintains a count of calls to
 * gtk_text_buffer_begin_user_action() that have not been closed with
 * a call to gtk_text_buffer_end_user_action(), and emits the 
 * "begin-user-action" and "end-user-action" signals only for the 
 * outermost pair of calls. This allows you to build user actions 
 * from other user actions.
 *
 * The "interactive" buffer mutation functions, such as
 * gtk_text_buffer_insert_interactive(), automatically call begin/end
 * user action around the buffer operations they perform, so there's
 * no need to add extra calls if you user action consists solely of a
 * single call to one of those functions.
 **/
void TextBuffer::begin_user_action (void)
{
  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));

  user_action_count += 1;
  
  if (user_action_count == 1)
    {
      /* Outermost nested user action begin emits the signal */
      //TODO make signal for this: g_signal_emit (buffer, signals[BEGIN_USER_ACTION], 0);
    }
}

/**
 * gtk_text_buffer_end_user_action:
 * @buffer: a #TextBuffer
 * 
 * Should be paired with a call to gtk_text_buffer_begin_user_action().
 * See that function for a full explanation.
 **/
void TextBuffer::end_user_action (void)
{
  //g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (user_action_count > 0);
  
  user_action_count -= 1;
  
  if (user_action_count == 0)
    {
      /* Ended the outermost-nested user action end, so emit the signal */
      //TODO make signal for this: g_signal_emit (buffer, signals[END_USER_ACTION], 0);
    }
}

/*void TextBuffer::free_target_lists(void)
{
  TextBufferPrivate *priv = GTK_TEXT_BUFFER_GET_PRIVATE (buffer);

  if (priv->copy_target_list)
    {
      gtk_target_list_unref (priv->copy_target_list);
      priv->copy_target_list = NULL;

      gtk_target_table_free (priv->copy_target_entries,
                             priv->n_copy_target_entries);
      priv->copy_target_entries = NULL;
      priv->n_copy_target_entries = 0;
    }

  if (priv->paste_target_list)
    {
      gtk_target_list_unref (priv->paste_target_list);
      priv->paste_target_list = NULL;

      gtk_target_table_free (priv->paste_target_entries,
                             priv->n_paste_target_entries);
      priv->paste_target_entries = NULL;
      priv->n_paste_target_entries = 0;
    }
}*/

/*static GtkTargetList *
gtk_text_buffer_get_target_list (TextBuffer   *buffer,
                                 bool         deserializable,
                                 GtkTargetEntry **entries,
                                 gint            *n_entries)
{
  GtkTargetList *target_list;

  target_list = gtk_target_list_new (NULL, 0);

  gtk_target_list_add (target_list,
                       gdk_atom_intern_static_string ("GTK_TEXT_BUFFER_CONTENTS"),
                       GTK_TARGET_SAME_APP,
                       GTK_TEXT_BUFFER_TARGET_INFO_BUFFER_CONTENTS);

  gtk_target_list_add_rich_text_targets (target_list,
                                         GTK_TEXT_BUFFER_TARGET_INFO_RICH_TEXT,
                                         deserializable,
                                         buffer);

  gtk_target_list_add_text_targets (target_list,
                                    GTK_TEXT_BUFFER_TARGET_INFO_TEXT);

  *entries = gtk_target_table_new_from_list (target_list, n_entries);

  return target_list;
}*/

/**
 * gtk_text_buffer_get_copy_target_list:
 * @buffer: a #TextBuffer
 *
 * This function returns the list of targets this text buffer can
 * provide for copying and as DND source. The targets in the list are
 * added with %info values from the #TextBufferTargetInfo enum,
 * using gtk_target_list_add_rich_text_targets() and
 * gtk_target_list_add_text_targets().
 *
 * Return value: the #GtkTargetList
 *
 * Since: 2.10
 **/
/*
GtkTargetList *
gtk_text_buffer_get_copy_target_list (TextBuffer *buffer)
{
  TextBufferPrivate *priv;

  g_return_val_if_fail (GTK_IS_TEXT_BUFFER (buffer), NULL);

  priv = GTK_TEXT_BUFFER_GET_PRIVATE (buffer);

  if (! priv->copy_target_list)
    priv->copy_target_list =
      gtk_text_buffer_get_target_list (buffer, FALSE,
                                       &priv->copy_target_entries,
                                       &priv->n_copy_target_entries);

  return priv->copy_target_list;
}*/

/**
 * gtk_text_buffer_get_paste_target_list:
 * @buffer: a #TextBuffer
 *
 * This function returns the list of targets this text buffer supports
 * for pasting and as DND destination. The targets in the list are
 * added with %info values from the #TextBufferTargetInfo enum,
 * using gtk_target_list_add_rich_text_targets() and
 * gtk_target_list_add_text_targets().
 *
 * Return value: the #GtkTargetList
 *
 * Since: 2.10
 **/
/*
GtkTargetList *
gtk_text_buffer_get_paste_target_list (TextBuffer *buffer)
{
  TextBufferPrivate *priv;

  g_return_val_if_fail (GTK_IS_TEXT_BUFFER (buffer), NULL);

  priv = GTK_TEXT_BUFFER_GET_PRIVATE (buffer);

  if (! priv->paste_target_list)
    priv->paste_target_list =
      gtk_text_buffer_get_target_list (buffer, TRUE,
                                       &priv->paste_target_entries,
                                       &priv->n_paste_target_entries);

  return priv->paste_target_list;
}*/

/*
 * Logical attribute cache
 */

/*
static void
free_log_attr_cache (TextLogAttrCache *cache)
{
  gint i = 0;
  while (i < ATTR_CACHE_SIZE)
    {
      g_free (cache->entries[i].attrs);
      ++i;
    }
  g_free (cache);
}

static void
clear_log_attr_cache (TextLogAttrCache *cache)
{
  gint i = 0;
  while (i < ATTR_CACHE_SIZE)
    {
      g_free (cache->entries[i].attrs);
      cache->entries[i].attrs = NULL;
      ++i;
    }
}

static PangoLogAttr*
compute_log_attrs (const TextIter *iter,
                   gint              *char_lenp)
{
  TextIter start;
  TextIter end;
  gchar *paragraph;
  gint char_len, byte_len;
  PangoLogAttr *attrs = NULL;
  
  start = *iter;
  end = *iter;

  gtk_text_iter_set_line_offset (&start, 0);
  gtk_text_iter_forward_line (&end);

  paragraph = gtk_text_iter_get_slice (&start, &end);
  char_len = g_utf8_strlen (paragraph, -1);
  byte_len = strlen (paragraph);

  g_assert (char_len > 0);

  if (char_lenp)
    *char_lenp = char_len;
  
  attrs = g_new (PangoLogAttr, char_len + 1);

  * FIXME we need to follow PangoLayout and allow different language
   * tags within the paragraph
   *
  pango_get_log_attrs (paragraph, byte_len, -1,
		       gtk_text_iter_get_language (&start),
                       attrs,
                       char_len + 1);
  
  g_free (paragraph);

  return attrs;
}

* The return value from this is valid until you call this a second time.
 *
const PangoLogAttr*
_gtk_text_buffer_get_line_log_attrs (TextBuffer     *buffer,
                                     const TextIter *anywhere_in_line,
                                     gint              *char_len)
{
  gint line;
  TextLogAttrCache *cache;
  gint i;
  
  g_return_val_if_fail (GTK_IS_TEXT_BUFFER (buffer), NULL);
  g_return_val_if_fail (anywhere_in_line != NULL, NULL);

  * special-case for empty last line in buffer *
  if (gtk_text_iter_is_end (anywhere_in_line) &&
      gtk_text_iter_get_line_offset (anywhere_in_line) == 0)
    {
      if (char_len)
        *char_len = 0;
      return NULL;
    }
  
  * FIXME we also need to recompute log attrs if the language tag at
   * the start of a paragraph changes
   *
  
  if (buffer->log_attr_cache == NULL)
    {
      buffer->log_attr_cache = g_new0 (TextLogAttrCache, 1);
      buffer->log_attr_cache->chars_changed_stamp =
        _gtk_text_btree_get_chars_changed_stamp (get_btree (buffer));
    }
  else if (buffer->log_attr_cache->chars_changed_stamp !=
           _gtk_text_btree_get_chars_changed_stamp (get_btree (buffer)))
    {
      clear_log_attr_cache (buffer->log_attr_cache);
    }
  
  cache = buffer->log_attr_cache;
  line = gtk_text_iter_get_line (anywhere_in_line);

  i = 0;
  while (i < ATTR_CACHE_SIZE)
    {
      if (cache->entries[i].attrs &&
          cache->entries[i].line == line)
        {
          if (char_len)
            *char_len = cache->entries[i].char_len;
          return cache->entries[i].attrs;
        }
      ++i;
    }
  
  * Not in cache; open up the first cache entry *
  g_free (cache->entries[ATTR_CACHE_SIZE-1].attrs);
  
  g_memmove (cache->entries + 1, cache->entries,
             sizeof (CacheEntry) * (ATTR_CACHE_SIZE - 1));

  cache->entries[0].line = line;
  cache->entries[0].attrs = compute_log_attrs (anywhere_in_line,
                                               &cache->entries[0].char_len);

  if (char_len)
    *char_len = cache->entries[0].char_len;
  
  return cache->entries[0].attrs;
}*/

void TextBuffer::notify_will_remove_tag (
                                         TextTag    *tag)
{
  /* This removes tag from the buffer, but DOESN'T emit the
   * remove-tag signal, because we can't afford to have user
   * code messing things up at this point; the tag MUST be removed
   * entirely.
   */
  if (btree)
    btree->notify_will_remove_tag (tag);
}

/*
 * Debug spew
 */
/*
void TextBuffer::spew (void)
{
  get_btree()->spew();
}*/
