/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
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

//#if defined(GTK_DISABLE_SINGLE_INCLUDES) && !defined (__GTK_H_INSIDE__) && !defined (GTK_COMPILATION)
//#error "Only <gtk/gtk.h> can be included directly."
//#endif

#ifndef __TEXT_TAG_TABLE_H__
#define __TEXT_TAG_TABLE_H__

#include "TextTag.h"

#include <algorithm>
#include <list>
#include <map>
#include <glib.h>

class TextTag;

typedef void (* TextTagTableForeach) (TextTag *tag, gpointer data);

#define GTK_TYPE_TEXT_TAG_TABLE            (gtk_text_tag_table_get_type ())
#define GTK_TEXT_TAG_TABLE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_TEXT_TAG_TABLE, GtkTextTagTable))
#define GTK_TEXT_TAG_TABLE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_TEXT_TAG_TABLE, GtkTextTagTableClass))
#define GTK_IS_TEXT_TAG_TABLE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_TEXT_TAG_TABLE))
#define GTK_IS_TEXT_TAG_TABLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_TEXT_TAG_TABLE))
#define GTK_TEXT_TAG_TABLE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_TEXT_TAG_TABLE, GtkTextTagTableClass))

class TextTagTable
{
	/*TODO make signals of these
  void (* tag_changed) (GtkTextTagTable *table, GtkTextTag *tag, gboolean size_changed);
  void (* tag_added) (GtkTextTagTable *table, GtkTextTag *tag);
  void (* tag_removed) (GtkTextTagTable *table, GtkTextTag *tag);
  */

	public:
		TextTagTable();
		~TextTagTable();

		/* INTERNAL private stuff - not even exported from the library on
		 * many platforms
		 */
		void add_buffer(gpointer buffer);
		void remove_buffer(gpointer buffer);

		void add_tag(TextTag *tag);
		void delete_tag(TextTag *tag);
		TextTag* lookup(const gchar *name);
		gint get_size (void);

		 GSList* list_of_tags (void);
		
		void foreach  (
                                              TextTagTableForeach  func,
                                              gpointer                data);
	protected:

	private:

		static void foreach_unref (TextTag *tag, gpointer data);
		static void hash_foreach (gpointer key, gpointer value, gpointer data);
		static void list_foreach (gpointer data, gpointer user_data);
		static void foreach_remove_tag (TextTag *tag, gpointer data);
		static void listify_foreach (TextTag *tag, gpointer user_data);
		
		int GetSize(void);

		std::map<guint, TextTag*> map;
		std::list<TextTag*> anonymous;

		  GHashTable *hash;
		  //GSList *anonymous;
		  gint anon_count;

		  GSList *buffers;

};

#endif /* __TEXT_TAG_TABLE_H__ */
