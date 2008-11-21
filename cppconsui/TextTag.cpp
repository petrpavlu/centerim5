/* gtktexttag.c - text tag object
 *
 * Copyright (c) 1992-1994 The Regents of the University of California.
 * Copyright (c) 1994-1997 Sun Microsystems, Inc.
 * Copyright (c) 2000      Red Hat, Inc.
 * Tk -> Gtk port by Havoc Pennington <hp@redhat.com>
 * Copyright (c) 2008      Mark Pustjens <pustjens@dds.nl>
 * Gtk -> C++ port
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

#include "config.h"
//#include "gtkmain.h"
#include "TextTag.h"
#include "TextTypes.h"
#include "TextTagTable.h"
//#include "gtkintl.h"
//#include "gtkmarshalers.h"
//#include "gtkprivate.h"
//#include "gtkalias.h"

//#include <stdlib.h>
//#include <string.h>

enum {
  EVENT,
  LAST_SIGNAL
};

enum {
  PROP_0,
  /* Construct args */
  PROP_NAME,

  /* Style args */
  PROP_BACKGROUND,
  PROP_FOREGROUND,
  /*PROP_BACKGROUND_GDK,
  PROP_FOREGROUND_GDK,
  PROP_FONT,
  PROP_FONT_DESC,
  PROP_FAMILY,
  PROP_STYLE,
  PROP_VARIANT,
  PROP_WEIGHT,
  PROP_STRETCH,
  PROP_SIZE,
  PROP_SIZE_POINTS,
  PROP_SCALE,
  PROP_PIXELS_ABOVE_LINES,
  PROP_PIXELS_BELOW_LINES,
  PROP_PIXELS_INSIDE_WRAP,*/
  PROP_EDITABLE,
  PROP_WRAP_MODE,
  PROP_JUSTIFICATION,
  PROP_DIRECTION,
  PROP_LEFT_MARGIN,
  PROP_INDENT,
  PROP_STRIKETHROUGH,
  PROP_RIGHT_MARGIN,
  PROP_UNDERLINE,
  PROP_RISE,
  //PROP_BACKGROUND_FULL_HEIGHT,
  PROP_LANGUAGE,
  PROP_TABS,
  PROP_INVISIBLE,
  PROP_PARAGRAPH_BACKGROUND,
  PROP_PARAGRAPH_BACKGROUND_GDK,

  /* Behavior args */
  PROP_ACCUMULATIVE_MARGIN,
  
  /* Whether-a-style-arg-is-set args */
  PROP_BACKGROUND_SET,
  PROP_FOREGROUND_SET,
  /*PROP_BACKGROUND_STIPPLE_SET,
  PROP_FOREGROUND_STIPPLE_SET,
  PROP_FAMILY_SET,
  PROP_STYLE_SET,
  PROP_VARIANT_SET,
  PROP_WEIGHT_SET,
  PROP_STRETCH_SET,
  PROP_SIZE_SET,
  PROP_SCALE_SET,
  PROP_PIXELS_ABOVE_LINES_SET,
  PROP_PIXELS_BELOW_LINES_SET,
  PROP_PIXELS_INSIDE_WRAP_SET,
  PROP_EDITABLE_SET,
  PROP_WRAP_MODE_SET,*/
  PROP_JUSTIFICATION_SET,
  PROP_LEFT_MARGIN_SET,
  PROP_INDENT_SET,
  PROP_STRIKETHROUGH_SET,
  PROP_RIGHT_MARGIN_SET,
  PROP_UNDERLINE_SET,
  PROP_RISE_SET,
  PROP_BACKGROUND_FULL_HEIGHT_SET,
  PROP_LANGUAGE_SET,
  PROP_TABS_SET,
  PROP_INVISIBLE_SET,
  PROP_PARAGRAPH_BACKGROUND_SET,

  LAST_ARG
};

/*
static void gtk_text_tag_finalize     (GObject         *object);
static void gtk_text_tag_set_property (GObject         *object,
                                       guint            prop_id,
                                       const GValue    *value,
                                       GParamSpec      *pspec);
static void gtk_text_tag_get_property (GObject         *object,
                                       guint            prop_id,
                                       GValue          *value,
                                       GParamSpec      *pspec);
*/

static guint signals[LAST_SIGNAL] = { 0 };

//G_DEFINE_TYPE (GtkTextTag, gtk_text_tag, G_TYPE_OBJECT)

TextTag::TextTag (const gchar *name)
{
	//TODO what to do with name?
	values = new TextAttributes();
	this->name = g_strdup(name);
}

TextTag::~TextTag(void)
{
	if (table)
		delete table;

 	values = NULL; 

	g_free(name);
	name = NULL;
}

void TextTag::set_bg_color (Color *color)
{
  if (color)
    {
      if (!bg_color_set)
        {
          bg_color_set = TRUE;
          //g_object_notify (G_OBJECT (tag), "background-set");
        }
      
      values->appearance.bg_color = *color;
    }
  else
    {
      if (bg_color_set)
        {
          bg_color_set = FALSE;
          //g_object_notify (G_OBJECT (tag), "background-set");
        }
    }
}

void TextTag::set_fg_color (Color *color)
{
  if (color)
    {
      if (!fg_color_set)
        {
          fg_color_set = TRUE;
          //g_object_notify (G_OBJECT (tag), "foreground-set");
        }
      values->appearance.fg_color = *color;
    }
  else
    {
      if (fg_color_set)
        {
          fg_color_set = FALSE;
          //g_object_notify (G_OBJECT (tag), "foreground-set");
        }
    }
}

void TextTag::set_pg_bg_color (Color *color)
{
  if (color)
    {
      if (!pg_bg_color_set)
        {
          pg_bg_color_set = TRUE;
          //g_object_notify (G_OBJECT (tag), "paragraph-background-set");
        }
      else
	delete values->pg_bg_color;

      values->pg_bg_color = new Color(*color);
    }
  else
    {
      if (pg_bg_color_set)
        {
          pg_bg_color_set = FALSE;
          //g_object_notify (G_OBJECT (tag), "paragraph-background-set");
	  delete values->pg_bg_color;
        }

      values->pg_bg_color = NULL;
    }
}

/*
static PangoFontMask
get_property_font_set_mask (guint prop_id)
{
  switch (prop_id)
    {
    case PROP_FAMILY_SET:
      return PANGO_FONT_MASK_FAMILY;
    case PROP_STYLE_SET:
      return PANGO_FONT_MASK_STYLE;
    case PROP_VARIANT_SET:
      return PANGO_FONT_MASK_VARIANT;
    case PROP_WEIGHT_SET:
      return PANGO_FONT_MASK_WEIGHT;
    case PROP_STRETCH_SET:
      return PANGO_FONT_MASK_STRETCH;
    case PROP_SIZE_SET:
      return PANGO_FONT_MASK_SIZE;
    }

  return 0;
}

static PangoFontMask
set_font_desc_fields (PangoFontDescription *desc,
		      PangoFontMask         to_set)
{
  PangoFontMask changed_mask = 0;
  
  if (to_set & PANGO_FONT_MASK_FAMILY)
    {
      const char *family = pango_font_description_get_family (desc);
      if (!family)
	{
	  family = "sans";
	  changed_mask |= PANGO_FONT_MASK_FAMILY;
	}

      pango_font_description_set_family (desc, family);
    }
  if (to_set & PANGO_FONT_MASK_STYLE)
    pango_font_description_set_style (desc, pango_font_description_get_style (desc));
  if (to_set & PANGO_FONT_MASK_VARIANT)
    pango_font_description_set_variant (desc, pango_font_description_get_variant (desc));
  if (to_set & PANGO_FONT_MASK_WEIGHT)
    pango_font_description_set_weight (desc, pango_font_description_get_weight (desc));
  if (to_set & PANGO_FONT_MASK_STRETCH)
    pango_font_description_set_stretch (desc, pango_font_description_get_stretch (desc));
  if (to_set & PANGO_FONT_MASK_SIZE)
    {
      gint size = pango_font_description_get_size (desc);
      if (size <= 0)
	{
	  size = 10 * PANGO_SCALE;
	  changed_mask |= PANGO_FONT_MASK_SIZE;
	}
      
      pango_font_description_set_size (desc, size);
    }

  return changed_mask;
}

static void
notify_set_changed (GObject       *object,
		    PangoFontMask  changed_mask)
{
  if (changed_mask & PANGO_FONT_MASK_FAMILY)
    g_object_notify (object, "family-set");
  if (changed_mask & PANGO_FONT_MASK_STYLE)
    g_object_notify (object, "style-set");
  if (changed_mask & PANGO_FONT_MASK_VARIANT)
    g_object_notify (object, "variant-set");
  if (changed_mask & PANGO_FONT_MASK_WEIGHT)
    g_object_notify (object, "weight-set");
  if (changed_mask & PANGO_FONT_MASK_STRETCH)
    g_object_notify (object, "stretch-set");
  if (changed_mask & PANGO_FONT_MASK_SIZE)
    g_object_notify (object, "size-set");
}

static void
notify_fields_changed (GObject       *object,
		       PangoFontMask  changed_mask)
{
  if (changed_mask & PANGO_FONT_MASK_FAMILY)
    g_object_notify (object, "family");
  if (changed_mask & PANGO_FONT_MASK_STYLE)
    g_object_notify (object, "style");
  if (changed_mask & PANGO_FONT_MASK_VARIANT)
    g_object_notify (object, "variant");
  if (changed_mask & PANGO_FONT_MASK_WEIGHT)
    g_object_notify (object, "weight");
  if (changed_mask & PANGO_FONT_MASK_STRETCH)
    g_object_notify (object, "stretch");
  if (changed_mask & PANGO_FONT_MASK_SIZE)
    g_object_notify (object, "size");
}

static void
set_font_description (GtkTextTag           *text_tag,
                      PangoFontDescription *font_desc)
{
  GObject *object = G_OBJECT (text_tag);
  PangoFontDescription *new_font_desc;
  PangoFontMask old_mask, new_mask, changed_mask, set_changed_mask;
  
  if (font_desc)
    new_font_desc = pango_font_description_copy (font_desc);
  else
    new_font_desc = pango_font_description_new ();

  if (text_tag->values->font)
    old_mask = pango_font_description_get_set_fields (text_tag->values->font);
  else
    old_mask = 0;
  
  new_mask = pango_font_description_get_set_fields (new_font_desc);

  changed_mask = old_mask | new_mask;
  set_changed_mask = old_mask ^ new_mask;

  if (text_tag->values->font)
    pango_font_description_free (text_tag->values->font);
  text_tag->values->font = new_font_desc;
  
  g_object_freeze_notify (object);

  g_object_notify (object, "font-desc");
  g_object_notify (object, "font");
  
  if (changed_mask & PANGO_FONT_MASK_FAMILY)
    g_object_notify (object, "family");
  if (changed_mask & PANGO_FONT_MASK_STYLE)
    g_object_notify (object, "style");
  if (changed_mask & PANGO_FONT_MASK_VARIANT)
    g_object_notify (object, "variant");
  if (changed_mask & PANGO_FONT_MASK_WEIGHT)
    g_object_notify (object, "weight");
  if (changed_mask & PANGO_FONT_MASK_STRETCH)
    g_object_notify (object, "stretch");
  if (changed_mask & PANGO_FONT_MASK_SIZE)
    {
      g_object_notify (object, "size");
      g_object_notify (object, "size-points");
    }

  notify_set_changed (object, set_changed_mask);
  
  g_object_thaw_notify (object);
}

static void
gtk_text_tag_ensure_font (GtkTextTag *text_tag)
{
  if (!text_tag->values->font)
    text_tag->values->font = pango_font_description_new ();
}
*/

/*static void TextTag::gtk_text_tag_set_property (
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  GtkTextTag *text_tag;
  gboolean size_changed = FALSE;

  text_tag = GTK_TEXT_TAG (object);

  g_return_if_fail (!text_tag->values->realized);

  switch (prop_id)
    {
    case PROP_NAME:
      g_return_if_fail (text_tag->name == NULL);
      text_tag->name = g_strdup (g_value_get_string (value));
      break;

    case PROP_BACKGROUND:
      {
        GdkColor color;

        if (!g_value_get_string (value))
          set_bg_color (text_tag, NULL);       * reset to background_set to FALSE *
        else if (gdk_color_parse (g_value_get_string (value), &color))
          set_bg_color (text_tag, &color);
        else
          g_warning ("Don't know color `%s'", g_value_get_string (value));

        g_object_notify (object, "background-gdk");
      }
      break;

    case PROP_FOREGROUND:
      {
        GdkColor color;

        if (!g_value_get_string (value))
          set_fg_color (text_tag, NULL);       * reset to foreground_set to FALSE *
        else if (gdk_color_parse (g_value_get_string (value), &color))
          set_fg_color (text_tag, &color);
        else
          g_warning ("Don't know color `%s'", g_value_get_string (value));

        g_object_notify (object, "foreground-gdk");
      }
      break;

    case PROP_BACKGROUND_GDK:
      {
        GdkColor *color = g_value_get_boxed (value);

        set_bg_color (text_tag, color);
      }
      break;

    case PROP_FOREGROUND_GDK:
      {
        GdkColor *color = g_value_get_boxed (value);

        set_fg_color (text_tag, color);
      }
      break;

    /*
    case PROP_BACKGROUND_STIPPLE:
      {
        GdkBitmap *bitmap = g_value_get_object (value);

        text_tag->bg_stipple_set = TRUE;
        g_object_notify (object, "background-stipple-set");
        
        if (text_tag->values->appearance.bg_stipple != bitmap)
          {
            if (bitmap != NULL)
              g_object_ref (bitmap);

            if (text_tag->values->appearance.bg_stipple)
              g_object_unref (text_tag->values->appearance.bg_stipple);

            text_tag->values->appearance.bg_stipple = bitmap;
          }
      }
      break;

    case PROP_FOREGROUND_STIPPLE:
      {
        GdkBitmap *bitmap = g_value_get_object (value);

        text_tag->fg_stipple_set = TRUE;
        g_object_notify (object, "foreground-stipple-set");

        if (text_tag->values->appearance.fg_stipple != bitmap)
          {
            if (bitmap != NULL)
              g_object_ref (bitmap);

            if (text_tag->values->appearance.fg_stipple)
              g_object_unref (text_tag->values->appearance.fg_stipple);

            text_tag->values->appearance.fg_stipple = bitmap;
          }
      }
      break;

    case PROP_FONT:
      {
        PangoFontDescription *font_desc = NULL;
        const gchar *name;

        name = g_value_get_string (value);

        if (name)
          font_desc = pango_font_description_from_string (name);

        set_font_description (text_tag, font_desc);
	if (font_desc)
	  pango_font_description_free (font_desc);
        
        size_changed = TRUE;
      }
      break;

    case PROP_FONT_DESC:
      {
        PangoFontDescription *font_desc;

        font_desc = g_value_get_boxed (value);

        set_font_description (text_tag, font_desc);

        size_changed = TRUE;
      }
      break;

    case PROP_FAMILY:
    case PROP_STYLE:
    case PROP_VARIANT:
    case PROP_WEIGHT:
    case PROP_STRETCH:
    case PROP_SIZE:
    case PROP_SIZE_POINTS:
      {
	PangoFontMask old_set_mask;

	gtk_text_tag_ensure_font (text_tag);
	old_set_mask = pango_font_description_get_set_fields (text_tag->values->font);
 
	switch (prop_id)
	  {
	  case PROP_FAMILY:
	    pango_font_description_set_family (text_tag->values->font,
					       g_value_get_string (value));
	    break;
	  case PROP_STYLE:
	    pango_font_description_set_style (text_tag->values->font,
					      g_value_get_enum (value));
	    break;
	  case PROP_VARIANT:
	    pango_font_description_set_variant (text_tag->values->font,
						g_value_get_enum (value));
	    break;
	  case PROP_WEIGHT:
	    pango_font_description_set_weight (text_tag->values->font,
					       g_value_get_int (value));
	    break;
	  case PROP_STRETCH:
	    pango_font_description_set_stretch (text_tag->values->font,
						g_value_get_enum (value));
	    break;
	  case PROP_SIZE:
	    pango_font_description_set_size (text_tag->values->font,
					     g_value_get_int (value));
	    g_object_notify (object, "size-points");
	    break;
	  case PROP_SIZE_POINTS:
	    pango_font_description_set_size (text_tag->values->font,
					     g_value_get_double (value) * PANGO_SCALE);
	    g_object_notify (object, "size");
	    break;
	  }

	size_changed = TRUE;
	notify_set_changed (object, old_set_mask & pango_font_description_get_set_fields (text_tag->values->font));
	g_object_notify (object, "font-desc");
	g_object_notify (object, "font");

	break;
      }
      
    case PROP_SCALE:
      text_tag->values->font_scale = g_value_get_double (value);
      text_tag->scale_set = TRUE;
      g_object_notifyg_object_notify (object, "scale-set");
      size_changed = TRUE;
      break;
      
    case PROP_PIXELS_ABOVE_LINES:
      text_tag->pixels_above_lines_set = TRUE;
      text_tag->values->pixels_above_lines = g_value_get_int (value);
      g_object_notify (object, "pixels-above-lines-set");
      size_changed = TRUE;
      break;

    case PROP_PIXELS_BELOW_LINES:
      text_tag->pixels_below_lines_set = TRUE;
      text_tag->values->pixels_below_lines = g_value_get_int (value);
      g_object_notify (object, "pixels-below-lines-set");
      size_changed = TRUE;
      break;

    case PROP_PIXELS_INSIDE_WRAP:
      text_tag->pixels_inside_wrap_set = TRUE;
      text_tag->values->pixels_inside_wrap = g_value_get_int (value);
      g_object_notify (object, "pixels-inside-wrap-set");
      size_changed = TRUE;
      break;*

    case PROP_EDITABLE:
      text_tag->editable_set = TRUE;
      text_tag->values->editable = g_value_get_boolean (value);
      g_object_notify (object, "editable-set");
      break;

    case PROP_WRAP_MODE:
      text_tag->wrap_mode_set = TRUE;
      text_tag->values->wrap_mode = g_value_get_enum (value);
      g_object_notify (object, "wrap-mode-set");
      size_changed = TRUE;
      break;

    case PROP_JUSTIFICATION:
      text_tag->justification_set = TRUE;
      text_tag->values->justification = g_value_get_enum (value);
      g_object_notify (object, "justification-set");
      size_changed = TRUE;
      break;

    case PROP_DIRECTION:
      text_tag->values->direction = g_value_get_enum (value);
      break;

    case PROP_LEFT_MARGIN:
      text_tag->left_margin_set = TRUE;
      text_tag->values->left_margin = g_value_get_int (value);
      g_object_notify (object, "left-margin-set");
      size_changed = TRUE;
      break;

    case PROP_INDENT:
      text_tag->indent_set = TRUE;
      text_tag->values->indent = g_value_get_int (value);
      g_object_notify (object, "indent-set");
      size_changed = TRUE;
      break;

    case PROP_STRIKETHROUGH:
      text_tag->strikethrough_set = TRUE;
      text_tag->values->appearance.strikethrough = g_value_get_boolean (value);
      g_object_notify (object, "strikethrough-set");
      break;

    case PROP_RIGHT_MARGIN:
      text_tag->right_margin_set = TRUE;
      text_tag->values->right_margin = g_value_get_int (value);
      g_object_notify (object, "right-margin-set");
      size_changed = TRUE;
      break;

    case PROP_UNDERLINE:
      text_tag->underline_set = TRUE;
      text_tag->values->appearance.underline = g_value_get_enum (value);
      g_object_notify (object, "underline-set");
      break;

    case PROP_RISE:
      text_tag->rise_set = TRUE;
      text_tag->values->appearance.rise = g_value_get_int (value);
      g_object_notify (object, "rise-set");
      size_changed = TRUE;      
      break;

    case PROP_BACKGROUND_FULL_HEIGHT:
      text_tag->bg_full_height_set = TRUE;
      text_tag->values->bg_full_height = g_value_get_boolean (value);
      g_object_notify (object, "background-full-height-set");
      break;

    case PROP_LANGUAGE:
      text_tag->language_set = TRUE;
      text_tag->values->language = pango_language_from_string (g_value_get_string (value));
      g_object_notify (object, "language-set");
      break;

    case PROP_TABS:
      text_tag->tabs_set = TRUE;

      if (text_tag->values->tabs)
        pango_tab_array_free (text_tag->values->tabs);

      * FIXME I'm not sure if this is a memleak or not *
      text_tag->values->tabs =
        pango_tab_array_copy (g_value_get_boxed (value));

      g_object_notify (object, "tabs-set");
      
      size_changed = TRUE;
      break;

    case PROP_INVISIBLE:
      text_tag->invisible_set = TRUE;
      text_tag->values->invisible = g_value_get_boolean (value);
      g_object_notify (object, "invisible-set");
      size_changed = TRUE;
      break;
      
    case PROP_PARAGRAPH_BACKGROUND:
      {
        GdkColor color;

        if (!g_value_get_string (value))
          set_pg_bg_color (text_tag, NULL);       * reset to paragraph_background_set to FALSE *
        else if (gdk_color_parse (g_value_get_string (value), &color))
          set_pg_bg_color (text_tag, &color);
        else
          g_warning ("Don't know color `%s'", g_value_get_string (value));

        g_object_notify (object, "paragraph-background-gdk");
      }
      break;

    case PROP_PARAGRAPH_BACKGROUND_GDK:
      {
        GdkColor *color = g_value_get_boxed (value);

        set_pg_bg_color (text_tag, color);
      }
      break;

    case PROP_ACCUMULATIVE_MARGIN:
      text_tag->accumulative_margin = g_value_get_boolean (value);
      g_object_notify (object, "accumulative-margin");
      size_changed = TRUE;
      break;

      * Whether the value should be used... *

    case PROP_BACKGROUND_SET:
      text_tag->bg_color_set = g_value_get_boolean (value);
      break;

    case PROP_FOREGROUND_SET:
      text_tag->fg_color_set = g_value_get_boolean (value);
      break;

    *case PROP_BACKGROUND_STIPPLE_SET:
      text_tag->bg_stipple_set = g_value_get_boolean (value);
      if (!text_tag->bg_stipple_set &&
          text_tag->values->appearance.bg_stipple)
        {
          g_object_unref (text_tag->values->appearance.bg_stipple);
          text_tag->values->appearance.bg_stipple = NULL;
        }
      break;

    case PROP_FOREGROUND_STIPPLE_SET:
      text_tag->fg_stipple_set = g_value_get_boolean (value);
      if (!text_tag->fg_stipple_set &&
          text_tag->values->appearance.fg_stipple)
        {
          g_object_unref (text_tag->values->appearance.fg_stipple);
          text_tag->values->appearance.fg_stipple = NULL;
        }
      break;

    case PROP_FAMILY_SET:
    case PROP_STYLE_SET:
    case PROP_VARIANT_SET:
    case PROP_WEIGHT_SET:
    case PROP_STRETCH_SET:
    case PROP_SIZE_SET:
      if (!g_value_get_boolean (value))
	{
	  if (text_tag->values->font)
	    pango_font_description_unset_fields (text_tag->values->font,
						 get_property_font_set_mask (prop_id));
	}
      else
	{
	  PangoFontMask changed_mask;
	  
	  gtk_text_tag_ensure_font (text_tag);
	  changed_mask = set_font_desc_fields (text_tag->values->font,
					       get_property_font_set_mask (prop_id));
	  notify_fields_changed (G_OBJECT (text_tag), changed_mask);
	}
      break;

    case PROP_SCALE_SET:
      text_tag->scale_set = g_value_get_boolean (value);
      size_changed = TRUE;
      break;
      
    case PROP_PIXELS_ABOVE_LINES_SET:
      text_tag->pixels_above_lines_set = g_value_get_boolean (value);
      size_changed = TRUE;
      break;

    case PROP_PIXELS_BELOW_LINES_SET:
      text_tag->pixels_below_lines_set = g_value_get_boolean (value);
      size_changed = TRUE;
      break;

    case PROP_PIXELS_INSIDE_WRAP_SET:
      text_tag->pixels_inside_wrap_set = g_value_get_boolean (value);
      size_changed = TRUE;
      break;*

    case PROP_EDITABLE_SET:
      text_tag->editable_set = g_value_get_boolean (value);
      break;

    case PROP_WRAP_MODE_SET:
      text_tag->wrap_mode_set = g_value_get_boolean (value);
      size_changed = TRUE;
      break;

    case PROP_JUSTIFICATION_SET:
      text_tag->justification_set = g_value_get_boolean (value);
      size_changed = TRUE;
      break;
      
    case PROP_LEFT_MARGIN_SET:
      text_tag->left_margin_set = g_value_get_boolean (value);
      size_changed = TRUE;
      break;

    case PROP_INDENT_SET:
      text_tag->indent_set = g_value_get_boolean (value);
      size_changed = TRUE;
      break;

    case PROP_STRIKETHROUGH_SET:
      text_tag->strikethrough_set = g_value_get_boolean (value);
      break;

    case PROP_RIGHT_MARGIN_SET:
      text_tag->right_margin_set = g_value_get_boolean (value);
      size_changed = TRUE;
      break;

    case PROP_UNDERLINE_SET:
      text_tag->underline_set = g_value_get_boolean (value);
      break;

    case PROP_RISE_SET:
      text_tag->rise_set = g_value_get_boolean (value);
      size_changed = TRUE;
      break;

    case PROP_BACKGROUND_FULL_HEIGHT_SET:
      text_tag->bg_full_height_set = g_value_get_boolean (value);
      break;

    case PROP_LANGUAGE_SET:
      text_tag->language_set = g_value_get_boolean (value);
      size_changed = TRUE;
      break;

    case PROP_TABS_SET:
      text_tag->tabs_set = g_value_get_boolean (value);
      size_changed = TRUE;
      break;

    case PROP_INVISIBLE_SET:
      text_tag->invisible_set = g_value_get_boolean (value);
      size_changed = TRUE;
      break;
      
    case PROP_PARAGRAPH_BACKGROUND_SET:
      text_tag->pg_bg_color_set = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }

  * FIXME I would like to do this after all set_property in a single
   * g_object_set () have been called. But an idle function won't
   * work; we need to emit when the tag is changed, not when we get
   * around to the event loop. So blah, we eat some inefficiency.
   *

  * This is also somewhat weird since we emit another object's
   * signal here, but the two objects are already tightly bound.
   *

  if (text_tag->table)
    g_signal_emit_by_name (text_tag->table,
                           "tag_changed",
                           text_tag, size_changed);
}*/

/*static void TextTag::gtk_text_tag_get_property (
                           guint         prop_id,
                           GValue       *value,
                           GParamSpec   *pspec)
{
  switch (prop_id)
    {
    case PROP_NAME:
      g_value_set_string (value, tag->name);
      break;

    case PROP_BACKGROUND_GDK:
      g_value_set_boxed (value, &tag->values->appearance.bg_color);
      break;

    case PROP_FOREGROUND_GDK:
      g_value_set_boxed (value, &tag->values->appearance.fg_color);
      break;

    case PROP_BACKGROUND_STIPPLE:
      if (tag->bg_stipple_set)
        g_value_set_object (value, tag->values->appearance.bg_stipple);
      break;

    case PROP_FOREGROUND_STIPPLE:
      if (tag->fg_stipple_set)
        g_value_set_object (value, tag->values->appearance.fg_stipple);
      break;

    case PROP_FONT:
        {
          gchar *str;

	  gtk_text_tag_ensure_font (tag);
	  
	  str = pango_font_description_to_string (tag->values->font);
          g_value_take_string (value, str);
        }
      break;

    case PROP_FONT_DESC:
      gtk_text_tag_ensure_font (tag);
      g_value_set_boxed (value, tag->values->font);
      break;

    case PROP_FAMILY:
    case PROP_STYLE:
    case PROP_VARIANT:
    case PROP_WEIGHT:
    case PROP_STRETCH:
    case PROP_SIZE:
    case PROP_SIZE_POINTS:
      gtk_text_tag_ensure_font (tag);
      switch (prop_id)
	{
	case PROP_FAMILY:
	  g_value_set_string (value, pango_font_description_get_family (tag->values->font));
	  break;
	  
	case PROP_STYLE:
	  g_value_set_enum (value, pango_font_description_get_style (tag->values->font));
	  break;
	  
	case PROP_VARIANT:
	  g_value_set_enum (value, pango_font_description_get_variant (tag->values->font));
	  break;
	  
	case PROP_WEIGHT:
	  g_value_set_int (value, pango_font_description_get_weight (tag->values->font));
	  break;
	  
	case PROP_STRETCH:
	  g_value_set_enum (value, pango_font_description_get_stretch (tag->values->font));
	  break;
	  
	case PROP_SIZE:
	  g_value_set_int (value, pango_font_description_get_size (tag->values->font));
	  break;
	  
	case PROP_SIZE_POINTS:
	  g_value_set_double (value, ((double)pango_font_description_get_size (tag->values->font)) / (double)PANGO_SCALE);
	  break;
	}
      break;
      
    case PROP_SCALE:
      g_value_set_double (value, tag->values->font_scale);
      break;
      
    case PROP_PIXELS_ABOVE_LINES:
      g_value_set_int (value,  tag->values->pixels_above_lines);
      break;

    case PROP_PIXELS_BELOW_LINES:
      g_value_set_int (value,  tag->values->pixels_below_lines);
      break;

    case PROP_PIXELS_INSIDE_WRAP:
      g_value_set_int (value,  tag->values->pixels_inside_wrap);
      break;

    case PROP_EDITABLE:
      g_value_set_boolean (value, tag->values->editable);
      break;

    case PROP_WRAP_MODE:
      g_value_set_enum (value, tag->values->wrap_mode);
      break;

    case PROP_JUSTIFICATION:
      g_value_set_enum (value, tag->values->justification);
      break;

    case PROP_DIRECTION:
      g_value_set_enum (value, tag->values->direction);
      break;
      
    case PROP_LEFT_MARGIN:
      g_value_set_int (value,  tag->values->left_margin);
      break;

    case PROP_INDENT:
      g_value_set_int (value,  tag->values->indent);
      break;

    case PROP_STRIKETHROUGH:
      g_value_set_boolean (value, tag->values->appearance.strikethrough);
      break;

    case PROP_RIGHT_MARGIN:
      g_value_set_int (value, tag->values->right_margin);
      break;

    case PROP_UNDERLINE:
      g_value_set_enum (value, tag->values->appearance.underline);
      break;

    case PROP_RISE:
      g_value_set_int (value, tag->values->appearance.rise);
      break;

    case PROP_BACKGROUND_FULL_HEIGHT:
      g_value_set_boolean (value, tag->values->bg_full_height);
      break;

    case PROP_LANGUAGE:
      g_value_set_string (value, pango_language_to_string (tag->values->language));
      break;

    case PROP_TABS:
      if (tag->values->tabs)
        g_value_set_boxed (value, tag->values->tabs);
      break;

    case PROP_INVISIBLE:
      g_value_set_boolean (value, tag->values->invisible);
      break;
      
    case PROP_PARAGRAPH_BACKGROUND_GDK:
      g_value_set_boxed (value, tag->values->pg_bg_color);
      break;

    case PROP_ACCUMULATIVE_MARGIN:
      g_value_set_boolean (value, tag->accumulative_margin);
      break;

    case PROP_BACKGROUND_SET:
      g_value_set_boolean (value, tag->bg_color_set);
      break;

    case PROP_FOREGROUND_SET:
      g_value_set_boolean (value, tag->fg_color_set);
      break;

    case PROP_BACKGROUND_STIPPLE_SET:
      g_value_set_boolean (value, tag->bg_stipple_set);
      break;

    case PROP_FOREGROUND_STIPPLE_SET:
      g_value_set_boolean (value, tag->fg_stipple_set);
      break;

    case PROP_FAMILY_SET:
    case PROP_STYLE_SET:
    case PROP_VARIANT_SET:
    case PROP_WEIGHT_SET:
    case PROP_STRETCH_SET:
    case PROP_SIZE_SET:
      {
	PangoFontMask set_mask = tag->values->font ? pango_font_description_get_set_fields (tag->values->font) : 0;
	PangoFontMask test_mask = get_property_font_set_mask (prop_id);
	g_value_set_boolean (value, (set_mask & test_mask) != 0);

	break;
      }

    case PROP_SCALE_SET:
      g_value_set_boolean (value, tag->scale_set);
      break;
      
    case PROP_PIXELS_ABOVE_LINES_SET:
      g_value_set_boolean (value, tag->pixels_above_lines_set);
      break;

    case PROP_PIXELS_BELOW_LINES_SET:
      g_value_set_boolean (value, tag->pixels_below_lines_set);
      break;

    case PROP_PIXELS_INSIDE_WRAP_SET:
      g_value_set_boolean (value, tag->pixels_inside_wrap_set);
      break;

    case PROP_EDITABLE_SET:
      g_value_set_boolean (value, tag->editable_set);
      break;

    case PROP_WRAP_MODE_SET:
      g_value_set_boolean (value, tag->wrap_mode_set);
      break;

    case PROP_JUSTIFICATION_SET:
      g_value_set_boolean (value, tag->justification_set);
      break;
      
    case PROP_LEFT_MARGIN_SET:
      g_value_set_boolean (value, tag->left_margin_set);
      break;

    case PROP_INDENT_SET:
      g_value_set_boolean (value, tag->indent_set);
      break;

    case PROP_STRIKETHROUGH_SET:
      g_value_set_boolean (value, tag->strikethrough_set);
      break;

    case PROP_RIGHT_MARGIN_SET:
      g_value_set_boolean (value, tag->right_margin_set);
      break;

    case PROP_UNDERLINE_SET:
      g_value_set_boolean (value, tag->underline_set);
      break;

    case PROP_RISE_SET:
      g_value_set_boolean (value, tag->rise_set);
      break;

    case PROP_BACKGROUND_FULL_HEIGHT_SET:
      g_value_set_boolean (value, tag->bg_full_height_set);
      break;

    case PROP_LANGUAGE_SET:
      g_value_set_boolean (value, tag->language_set);
      break;

    case PROP_TABS_SET:
      g_value_set_boolean (value, tag->tabs_set);
      break;

    case PROP_INVISIBLE_SET:
      g_value_set_boolean (value, tag->invisible_set);
      break;
      
    case PROP_PARAGRAPH_BACKGROUND_SET:
      g_value_set_boolean (value, tag->pg_bg_color_set);
      break;

    case PROP_BACKGROUND:
    case PROP_FOREGROUND:
    case PROP_PARAGRAPH_BACKGROUND:
      g_warning ("'foreground', 'background' and 'paragraph_background' properties are not readable, use 'foreground_gdk', 'background_gdk' and 'paragraph_background_gdk'");
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}*/

/*
 * Tag operations
 */

void TextTag::delta_priority_foreach(TextTag *tag, DeltaData *user_data)
{
  DeltaData *dd = user_data;

  if (tag->priority >= dd->low && tag->priority <= dd->high)
    tag->priority += dd->delta;
}

/**
 * gtk_text_tag_get_priority:
 * @tag: a #GtkTextTag
 * 
 * Get the tag priority.
 * 
 * Return value: The tag's priority.
 **/
gint TextTag::get_priority(void)
{
  return priority;
}

/**
 * gtk_text_tag_set_priority:
 * @tag: a #GtkTextTag
 * @priority: the new priority
 * 
 * Sets the priority of a #GtkTextTag. Valid priorities are
 * start at 0 and go to one less than gtk_text_tag_table_get_size().
 * Each tag in a table has a unique priority; setting the priority
 * of one tag shifts the priorities of all the other tags in the
 * table to maintain a unique priority for each tag. Higher priority
 * tags "win" if two tags both set the same text attribute. When adding
 * a tag to a tag table, it will be assigned the highest priority in
 * the table by default; so normally the precedence of a set of tags
 * is the order in which they were added to the table, or created with
 * gtk_text_buffer_create_tag(), which adds the tag to the buffer's table
 * automatically.
 **/
void TextTag::set_priority(gint priority)
{
  DeltaData dd;

  g_return_if_fail (table != NULL);
  g_return_if_fail (priority >= 0);
  g_return_if_fail (priority < table->get_size());

  if (priority == this->priority)
    return;

  if (priority < this->priority)
    {
      dd.low = priority;
      dd.high = this->priority - 1;
      dd.delta = 1;
    }
  else
    {
      dd.low = this->priority + 1;
      dd.high = priority;
      dd.delta = -1;
    }

  table->foreach ( (TextTagTableForeach)TextTag::delta_priority_foreach, (void*)&dd);

  this->priority = priority;
}

/**
 * gtk_text_tag_event:
 * @tag: a #GtkTextTag
 * @event_object: object that received the event, such as a widget
 * @event: the event
 * @iter: location where the event was received
 * 
 * Emits the "event" signal on the #GtkTextTag.
 * 
 * Return value: result of signal emission (whether the event was handled)
 **/
/*gboolean TextTag::event (
                    GObject           *event_object,
                    GdkEvent          *event,
                    const GtkTextIter *iter)
{
  gboolean retval = FALSE;

  g_return_val_if_fail (GTK_IS_TEXT_TAG (tag), FALSE);
  g_return_val_if_fail (G_IS_OBJECT (event_object), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  g_signal_emit (tag,
                 signals[EVENT],
                 0,
                 event_object,
                 event,
                 iter,
                 &retval);

  return retval;
}*/

bool TextTag::tag_sort_func (TextTag *first, TextTag *second)
{
  return first->priority < second->priority;
}

void TextTag::array_sort (TextTag** tag_array_p,
                          guint len)
{
  int i, j, prio;
  TextTag **tag;
  TextTag **maxPtrPtr, *tmp;

  g_return_if_fail (tag_array_p != NULL);
  g_return_if_fail (len > 0);

  if (len < 2) {
    return;
  }
  if (len < 20) {
    TextTag **iter = tag_array_p;

    for (i = len-1; i > 0; i--, iter++) {
      maxPtrPtr = tag = iter;
      prio = tag[0]->priority;
      for (j = i, tag++; j > 0; j--, tag++) {
        if (tag[0]->priority < prio) {
          prio = tag[0]->priority;
          maxPtrPtr = tag;
        }
      }
      tmp = *maxPtrPtr;
      *maxPtrPtr = *iter;
      *iter = tmp;
    }
  } else {
    qsort ((void *) tag_array_p, (unsigned) len, sizeof (TextTag *),
           (int (*)(const void*, const void*))tag_sort_func);
  }

#if 0
  {
    printf ("Sorted tag array: \n");
    i = 0;
    while (i < len)
      {
        GtkTextTag *t = tag_array_p[i];
        printf ("  %s priority %d\n", t->name, t->priority);

        ++i;
      }
  }
#endif
}

/*
 * Attributes
 */

/**
 * gtk_text_attributes_new:
 * 
 * Creates a #GtkTextAttributes, which describes
 * a set of properties on some text.
 * 
 * Return value: a new #GtkTextAttributes
 **/
TextAttributes::TextAttributes ()
{
  editable = true;
}

/**
 * gtk_text_attributes_copy:
 * @src: a #GtkTextAttributes to be copied
 * 
 * Copies @src and returns a new #GtkTextAttributes.
 * 
 * Return value: a copy of @src
 **/
/*TODO do we need a copy constructor (and hence an operator= and destructor)?
GtkTextAttributes*
gtk_text_attributes_copy (GtkTextAttributes *src)
{
  GtkTextAttributes *dest;

  dest = gtk_text_attributes_new ();
  gtk_text_attributes_copy_values (src, dest);

  return dest;
}*/

//TODO ??
/*GType TextTag::gtk_text_attributes_get_type (void)
{
  static GType our_type = 0;
  
  if (our_type == 0)
    our_type = g_boxed_type_register_static (I_("GtkTextAttributes"),
					     (GBoxedCopyFunc) gtk_text_attributes_ref,
					     (GBoxedFreeFunc) gtk_text_attributes_unref);

  return our_type;
}*/

/**
 * gtk_text_attributes_copy_values:
 * @src: a #GtkTextAttributes
 * @dest: another #GtkTextAttributes
 * 
 * Copies the values from @src to @dest so that @dest has the same values
 * as @src. Frees existing values in @dest.
 **/
/*TODO we would need a copy constructor and operator=*/
void TextAttributes::copy_values (TextAttributes *src,
                                 TextAttributes *dest)
{
  guint orig_refcount;

  g_return_if_fail (!dest->realized);

  if (src == dest)
    return;

  /* Add refs */

  /*if (src->appearance.bg_stipple)
    g_object_ref (src->appearance.bg_stipple);

  if (src->appearance.fg_stipple)
    g_object_ref (src->appearance.fg_stipple);*/

  /* Remove refs */

  /*if (dest->appearance.bg_stipple)
    g_object_unref (dest->appearance.bg_stipple);

  if (dest->appearance.fg_stipple)
    g_object_unref (dest->appearance.fg_stipple);

  if (dest->font)
    pango_font_description_free (dest->font);*/
  
  /* Copy */
  orig_refcount = dest->refcount;

  *dest = *src;

  /*if (src->tabs)
    dest->tabs = pango_tab_array_copy (src->tabs);*/

  //dest->language = src->language;

  /*if (dest->font)
    dest->font = pango_font_description_copy (src->font);*/
  
  if (src->pg_bg_color)
	 ;//TODO do something sensible
    //dest->pg_bg_color = gdk_color_copy (src->pg_bg_color);

  dest->refcount = orig_refcount;
  dest->realized = FALSE;
}

/**
 * gtk_text_attributes_ref:
 * @values: a #GtkTextAttributes
 * 
 * Increments the reference count on @values.
 *
 * Returns: the #GtkTextAttributes that were passed in
 **/
TextAttributes * TextAttributes::ref (void)
{
  //g_return_val_if_fail (values != NULL, NULL);

  refcount += 1;

  return this;
}

/**
 * gtk_text_attributes_unref:
 * @values: a #GtkTextAttributes
 * 
 * Decrements the reference count on @values, freeing the structure
 * if the reference count reaches 0.
 **/
void TextAttributes::unref (void)
{
//  g_return_if_fail (values != NULL);
  g_return_if_fail (refcount > 0);

  refcount -= 1;

  if (refcount == 0)
    {
      g_assert (!realized);

      /*
      if (appearance.bg_stipple)
        g_object_unref (appearance.bg_stipple);

      if (appearance.fg_stipple)
        g_object_unref (appearance.fg_stipple);

      if (tabs)
        pango_tab_array_free (tabs);

      if (font)
	pango_font_description_free (font);

      if (pg_bg_color)
	gdk_color_free (pg_bg_color);
	*/
      //TODOg_free (values);
    }
}

TextAttributes::TextAttributes( Colormap *cmap/*,
                              GdkVisual *visual*/)
{
 // g_return_if_fail (values != NULL);
  g_return_if_fail (refcount > 0);
  g_return_if_fail (!realized);

  /* It is wrong to use this colormap, FIXME */
  /*TODO
  gdk_colormap_alloc_color (cmap,
                            &values->appearance.fg_color,
                            FALSE, TRUE);

  gdk_colormap_alloc_color (cmap,
                            &values->appearance.bg_color,
                            FALSE, TRUE);*/

  if (pg_bg_color)
  {}/*gdk_colormap_alloc_color (cmap,
			      values->pg_bg_color,
			      FALSE, TRUE);*/

  realized = true;
}

void TextAttributes::unrealize (Colormap *cmap/*,
                                GdkVisual *visual*/)
{
 // g_return_if_fail (values != NULL);
  g_return_if_fail (refcount > 0);
  g_return_if_fail (realized);

  /*TODO
  gdk_colormap_free_colors (cmap,
                            &values->appearance.fg_color, 1);


  gdk_colormap_free_colors (cmap,
                            &values->appearance.bg_color, 1);*/

  appearance.fg_color.pixel = 0;
  appearance.bg_color.pixel = 0;

  if (pg_bg_color)
    {
      //gdk_colormap_free_colors (cmap, values->pg_bg_color, 1);
      
      pg_bg_color->pixel = 0;
    }

  realized = false;
}

void TextAttributes::fill_from_tags (TextTag **tags, guint n_tags)
{
  guint n = 0;

  guint left_margin_accumulative = 0;
  guint right_margin_accumulative = 0;

  g_return_if_fail (!realized);

  while (n < n_tags)
    {
      TextTag *tag = tags[n];
      TextAttributes *vals = tag->values;

      g_assert (tag->table != NULL);
      if (n > 0)
        g_assert (tags[n]->priority > tags[n-1]->priority);

      /*if (tag->bg_color_set)
        {
          appearance.bg_color = vals->appearance.bg_color;

          appearance.draw_bg = TRUE;
        }
      if (tag->fg_color_set)
        appearance.fg_color = vals->appearance.fg_color;
      
      if (tag->pg_bg_color_set)
        {
          pg_bg_color = gdk_color_copy (vals->pg_bg_color);
        }*/

      /*if (tag->bg_stipple_set)
        {
          g_object_ref (vals->appearance.bg_stipple);
          if (appearance.bg_stipple)
            g_object_unref (appearance.bg_stipple);
          appearance.bg_stipple = vals->appearance.bg_stipple;

          appearance.draw_bg = TRUE;
        }

      if (tag->fg_stipple_set)
        {
          g_object_ref (vals->appearance.fg_stipple);
          if (appearance.fg_stipple)
            g_object_unref (appearance.fg_stipple);
          appearance.fg_stipple = vals->appearance.fg_stipple;
        }

      if (vals->font)
	{
	  if (font)
	    pango_font_description_merge (font, vals->font, TRUE);
	  else
	    font = pango_font_description_copy (vals->font);
	}

      * multiply all the scales together to get a composite *
      if (tag->scale_set)
        font_scale *= vals->font_scale;
      
      if (tag->justification_set)
        justification = vals->justification;

      if (vals->direction != GTK_TEXT_DIR_NONE)
        direction = vals->direction;

      if (tag->left_margin_set) 
        {
          if (tag->accumulative_margin)
            left_margin_accumulative += vals->left_margin;
          else
            left_margin = vals->left_margin;
        }

      if (tag->indent_set)
        indent = vals->indent;

      if (tag->rise_set)
        appearance.rise = vals->appearance.rise;

      if (tag->right_margin_set) 
        {
          if (tag->accumulative_margin)
            right_margin_accumulative += vals->right_margin;
          else
            right_margin = vals->right_margin;
        }

      if (tag->pixels_above_lines_set)
        pixels_above_lines = vals->pixels_above_lines;

      if (tag->pixels_below_lines_set)
        pixels_below_lines = vals->pixels_below_lines;

      if (tag->pixels_inside_wrap_set)
        pixels_inside_wrap = vals->pixels_inside_wrap;

      if (tag->tabs_set)
        {
          if (tabs)
            pango_tab_array_free (tabs);
          tabs = pango_tab_array_copy (vals->tabs);
        }*/

      if (tag->wrap_mode_set)
        wrap_mode = vals->wrap_mode;

      if (tag->underline_set)
        appearance.underline = vals->appearance.underline;

      /*if (tag->strikethrough_set)
        appearance.strikethrough = vals->appearance.strikethrough;*/

      if (tag->invisible_set)
        invisible = vals->invisible;

      if (tag->editable_set)
        editable = vals->editable;

      if (tag->bg_full_height_set)
        bg_full_height = vals->bg_full_height;

      /*if (tag->language_set)
	language = vals->language;*/

      ++n;
    }

  left_margin += left_margin_accumulative;
  right_margin += right_margin_accumulative;
}

bool TextTag::affects_size (void)
{
//  g_return_val_if_fail (GTK_IS_TEXT_TAG (tag), FALSE);

  return
    /*(values->font && pango_font_description_get_set_fields (values->font) != 0) ||*/
    scale_set ||
    justification_set ||
    left_margin_set ||
    indent_set ||
    rise_set ||
    right_margin_set ||
    pixels_above_lines_set ||
    pixels_below_lines_set ||
    pixels_inside_wrap_set ||
    tabs_set ||
    underline_set ||
    wrap_mode_set ||
    invisible_set;
}

bool TextTag::affects_nonsize_appearance (void)
{
  return
    bg_color_set ||
    //tag->bg_stipple_set ||
    fg_color_set ||
    //tag->fg_stipple_set ||
    strikethrough_set ||
    //tag->bg_full_height_set ||
    pg_bg_color_set;
}
