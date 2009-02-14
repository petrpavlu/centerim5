/* GTK - The GIMP Toolkit
 * gtktextbuffer.h Copyright (C) 2000 Red Hat, Inc.
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

#ifndef __TEXTBUFFER_H__
#define __TEXTBUFFER_H__

//#include "TextTagTable.h"
//#include "TextBTree.h"
//#include "TextBuffer.h"
//#include <gtkwidget.h>
//#include <gtkclipboard.h>
//#include "TextTagTable.h"
#include "TextIter.h"
#include "TextMark.h"
#include "TextChild.h"

//G_BEGIN_DECLS

/*
 * This is the PUBLIC representation of a text buffer.
 * TextBTree is the PRIVATE internal representation of it.
 */

#define GTK_TYPE_TEXT_BUFFER            (get_type ())
#define GTK_TEXT_BUFFER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_TEXT_BUFFER, TextBuffer))
#define GTK_TEXT_BUFFER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_TEXT_BUFFER, TextBufferClass))
#define GTK_IS_TEXT_BUFFER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_TEXT_BUFFER))
#define GTK_IS_TEXT_BUFFER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_TEXT_BUFFER))
#define GTK_TEXT_BUFFER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_TEXT_BUFFER, TextBufferClass))

#define ATTR_CACHE_SIZE 2

typedef struct _CacheEntry CacheEntry; //TODO do we need this?
struct _CacheEntry
{
  gint line;
  gint char_len;
  //PangoLogAttr *attrs;
};

struct TextLogAttrCache //TODO move inside textbuffer class and do we need this?
{
  gint chars_changed_stamp;
  CacheEntry entries[ATTR_CACHE_SIZE];
};

class TextBTree;
class TextIter;
class TextChildAnchor;
class TextMark;

class TextBuffer
{
	public:
		/* these values are used as "info" for the targets contained in the
		 * lists returned by get_copy,paste_target_list()
		 *
		 * the enum counts down from G_MAXUINT to avoid clashes with application
		 * added drag destinations which usually start at 0.
		 */
		typedef enum
		{
		  TEXT_BUFFER_TARGET_INFO_BUFFER_CONTENTS = -1,
		  TEXT_BUFFER_TARGET_INFO_RICH_TEXT       = -2,
		  TEXT_BUFFER_TARGET_INFO_TEXT            = -3
		} TextBufferTargetInfo;

		TextBuffer(void);
		/* table is NULL to create a new one */
		TextBuffer(TextTagTable *table);

		~TextBuffer(void);

/* Delete whole buffer, then insert */
void set_text (const gchar *text, gint len);

/* Insert into the buffer */
void insert ( TextIter   *iter, const gchar   *text, gint           len);
void insert_at_cursor  ( const gchar   *text, gint           len);

bool insert_interactive           (
                                                       TextIter   *iter,
                                                       const gchar   *text,
                                                       gint           len,
                                                       bool default_editable);
bool insert_interactive_at_cursor (
                                                       const gchar   *text,
                                                       gint           len,
                                                       bool default_editable);

void     insert_range             (
                                                   TextIter       *iter,
                                                   TextIter *start,
                                                   TextIter *end);

		bool insert_range_interactive(TextIter *iter,
				TextIter *start,
				TextIter *end,
				bool default_editable);

void insert_with_tags (TextIter *iter, const gchar *text, gint len, TextTag *first_tag, ...) G_GNUC_NULL_TERMINATED;

void    insert_with_tags_by_name  (
                                                   TextIter       *iter,
                                                   const gchar       *text,
                                                   gint               len,
                                                   const gchar       *first_tag_name,
                                                   ...) G_GNUC_NULL_TERMINATED;

/* Delete from the buffer */
void     delete_text             (
					     TextIter   *start,
					     TextIter   *end);
bool delete_text_interactive (
					     TextIter   *start_iter,
					     TextIter   *end_iter,
					     bool       default_editable);
bool backspace          (
					     TextIter   *iter,
					     bool       interactive,
					     bool       default_editable);

/* Obtain strings from the buffer */
gchar          *get_text            (
                                                     TextIter *start,
                                                     TextIter *end,
                                                     bool           include_hidden_chars);

gchar          *get_slice           (
                                                     TextIter *start,
                                                     TextIter *end,
                                                     bool           include_hidden_chars);

/* Insert a pixbuf */
/*void insert_pixbuf         (TextBuffer *buffer,
                                            TextIter   *iter,
                                            GdkPixbuf     *pixbuf);*/

		/* Insert a child anchor */
		//void insert_child_anchor (TextIter *iter, TextChildAnchor *anchor);

		/* Convenience, create and insert a child anchor */
		//TextChildAnchor *create_child_anchor (TextIter *iter);

/* Mark manipulation */
void           add_mark    (
                                            TextMark       *mark,
                                            TextIter *where);
TextMark   *create_mark (
                                            const gchar       *mark_name,
                                            TextIter *where,
                                            bool           left_gravity);
void           move_mark   (
                                            TextMark       *mark,
                                            TextIter *where);
void           delete_mark (
                                            TextMark       *mark);
TextMark*   get_mark    (
                                            const gchar       *name);

void move_mark_by_name   (
                                          const gchar       *name,
                                          TextIter *where);
void delete_mark_by_name (
                                          const gchar       *name);

TextMark* get_insert          (void);
TextMark* get_selection_bound (void);

/* efficiently move insert and selection_bound at the same time */
void place_cursor (
                                   const TextIter *where);
void select_range (
                                   const TextIter *ins,
				   const TextIter *bound);

void select_all(bool select);


/* Tag manipulation */
void apply_tag             (
                                            TextTag        *tag,
                                            TextIter *start,
                                            TextIter *end);
void remove_tag            (
                                            TextTag        *tag,
                                            TextIter *start,
                                            TextIter *end);
void apply_tag_by_name     (
                                            const gchar       *name,
                                            TextIter *start,
                                            TextIter *end);
void remove_tag_by_name    (
                                            const gchar       *name,
                                            TextIter *start,
                                            TextIter *end);
void remove_all_tags       (
                                            TextIter *start,
                                            TextIter *end);


/* You can either ignore the return value, or use it to
 * set the attributes of the tag. tag_name can be NULL
 */
TextTag    *create_tag (
                                           const gchar   *tag_name,
                                           const gchar   *first_property_name,
                                           ...);

/* Obtain iterators pointed at various places, then you can move the
 * iterator around using the TextIter operators
 */
void get_iter_at_line_offset (
                                              TextIter   *iter,
                                              gint           line_number,
                                              gint           char_offset);
void get_iter_at_line_index  (
                                              TextIter   *iter,
                                              gint           line_number,
                                              gint           byte_index);
void get_iter_at_offset      (
                                              TextIter   *iter,
                                              gint           char_offset);
void get_iter_at_line        (
                                              TextIter   *iter,
                                              gint           line_number);
		void get_start_iter ( TextIter   *iter);
void get_end_iter            (
                                              TextIter   *iter);
void get_bounds              ( TextIter   *start, TextIter   *end);
void get_iter_at_mark        (
                                              TextIter   *iter,
                                              TextMark   *mark);

void get_iter_at_child_anchor (
                                               TextIter        *iter,
                                               TextChildAnchor *anchor);

/* There's no get_first_iter because you just get the iter for
   line or char 0 */

/* Used to keep track of whether the buffer needs saving; anytime the
   buffer contents change, the modified flag is turned on. Whenever
   you save, turn it off. Tags and marks do not affect the modified
   flag, but if you would like them to you can connect a handler to
   the tag/mark signals and call set_modified in your handler */

bool        get_modified            (void);
void            set_modified            (
                                                         bool       setting);

bool        get_has_selection       (void);

/*void add_selection_clipboard    (TextBuffer     *buffer,
						 GtkClipboard      *clipboard);
void remove_selection_clipboard (TextBuffer     *buffer,
						 GtkClipboard      *clipboard);*/

/*void            cut_clipboard           (TextBuffer *buffer,
							 GtkClipboard  *clipboard,
                                                         bool       default_editable);
void            copy_clipboard          (TextBuffer *buffer,
							 GtkClipboard  *clipboard);
void            paste_clipboard         (TextBuffer *buffer,
							 GtkClipboard  *clipboard,
							 TextIter   *override_location,
                                                         bool       default_editable);*/

bool        get_selection_bounds    (
                                                         TextIter   *start,
                                                         TextIter   *end);
bool        delete_selection        (
                                                         bool       interactive,
                                                         bool       default_editable);

/* Called to specify atomic user actions, used to implement undo */
void            begin_user_action       (void);
void            end_user_action         (void);

//TODO  cim probably needs this
//GtkTargetList * get_copy_target_list    (TextBuffer *buffer);
//GtkTargetList * get_paste_target_list   (TextBuffer *buffer);

/* INTERNAL private stuff */
void            spew                  (void);

		TextBTree* get_btree(void);

/*const PangoLogAttr* _get_line_log_attrs (TextBuffer     *buffer,
                                                         const TextIter *anywhere_in_line,
                                                         gint              *char_len);*/

void notify_will_remove_tag ( TextTag    *tag);


		static bool possibly_not_text (gunichar ch, gpointer user_data);

		void insert_text_range ( TextIter *iter, TextIter *orig_start, TextIter *orig_end, bool interactive);

  TextTagTable *tag_table;

	protected:
	private:

  TextBTree *btree;

  //GSList *GSEAL (clipboard_contents_buffers);
  //GSList *GSEAL (selection_clipboards);

  TextLogAttrCache *log_attr_cache;

  guint user_action_count;

  /* Whether the buffer has been modified since last save */
  guint modified ;//: 1;

  guint has_selection;// : 1;

/*struct _TextBufferClass
{
  GObjectClass parent_class;

  void (* insert_text)     (TextBuffer *buffer,
                            TextIter *pos,
                            const gchar *text,
                            gint length);

  void (* insert_pixbuf)   (TextBuffer *buffer,
                            TextIter   *pos,
                            GdkPixbuf     *pixbuf);

  void (* insert_child_anchor)   (TextBuffer      *buffer,
                                  TextIter        *pos,
                                  TextChildAnchor *anchor);

  void (* delete_range)     (TextBuffer *buffer,
                             TextIter   *start,
                             TextIter   *end);

  * Only for text/widgets/pixbuf changed, marks/tags don't cause this
   * to be emitted
   *
  void (* changed)         (TextBuffer *buffer);


  * New value for the modified flag *
  void (* modified_changed)   (TextBuffer *buffer);
*/
  /* Mark moved or created */
  void mark_set (const TextIter *location, TextMark *mark);
/*
  void (* mark_deleted)       (TextBuffer *buffer,
                               TextMark *mark);

  void (* apply_tag)          (TextBuffer *buffer,
                               TextTag *tag,
                               const TextIter *start_char,
                               const TextIter *end_char);

  void (* remove_tag)         (TextBuffer *buffer,
                               TextTag *tag,
                               const TextIter *start_char,
                               const TextIter *end_char);

  * Called at the start and end of an atomic user action *
  void (* begin_user_action)  (TextBuffer *buffer);
  void (* end_user_action)    (TextBuffer *buffer);

  * Padding for future expansion *
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
  void (*_gtk_reserved5) (void);
  void (*_gtk_reserved6) (void);
};*/

//GType        get_type       (void) G_GNUC_CONST;




		gint get_line_count (void);
		gint get_char_count (void);


		TextTagTable* get_tag_table(void);

	private:
		void set_table(TextTagTable *table);
		TextTagTable* get_table(void);

		//TODO this might be for copy/paste, which we don't support right now
		//void free_target_lists(void);

		void real_insert_text ( TextIter  *iter,
                                  const gchar   *text,
                                  gint           len);


		void emit_insert ( TextIter *iter, const gchar   *text, gint len);

		void insert_range_untagged (
                       TextIter       *iter,
                       TextIter *orig_start,
                       TextIter *orig_end,
                       bool           interactive);

		void insert_range_not_inside_self (
                              TextIter       *iter,
                              TextIter *orig_start,
                              TextIter *orig_end,
                              bool           interactive);

		void real_insert_range (
                                   TextIter       *iter,
                                   TextIter *orig_start,
                                   TextIter *orig_end,
                                   bool           interactive);

		void real_delete_range ( TextIter   *start,
                                   TextIter   *end);

		void real_insert_anchor (TextIter *iter, TextChildAnchor *anchor);

		TextMark* set_mark (
                          TextMark       *existing_mark,
                          const gchar       *mark_name,
                          TextIter *iter,
                          bool           left_gravity,
                          bool           should_exist);

		void emit_delete (TextIter *start, TextIter *end);

		void real_changed (void);

		void emit_tag (
                          TextTag        *tag,
                          bool           apply,
                          const TextIter *start,
                          const TextIter *end);

};

#endif /* __TEXTBUFFER_H__ */
