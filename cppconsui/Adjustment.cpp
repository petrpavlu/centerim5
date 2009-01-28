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

#include "config.h"
#include "Adjustment.h"
//#include "gtkmarshalers.h"
//#include "gtkprivate.h"
//#include "gtkintl.h"
//#include "gtkalias.h"

/*

enum
{
  CHANGED,
  VALUE_CHANGED,
  LAST_SIGNAL
};*/

guint64 adjustment_changed_stamp = 0; /* protected by global gdk lock */


void
Adjustment::Adjustment (void)
{
  adjustment->value = 0.0;
  adjustment->lower = 0.0;
  adjustment->upper = 0.0;
  adjustment->step_increment = 0.0;
  adjustment->page_increment = 0.0;
  adjustment->page_size = 0.0;
}

/**
 * gtk_adjustment_get_value:
 * @adjustment: a #GtkAdjustment
 *
 * Gets the current value of the adjustment. See
 * gtk_adjustment_set_value ().
 *
 * Return value: The current value of the adjustment.
 **/
gdouble
Adjustment::get_value (void)
{
  //g_return_val_if_fail (GTK_IS_ADJUSTMENT (adjustment), 0.0);

  return value;
}

void
Adjustment::set_value ( gdouble        value)
{
  g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));

  value = CLAMP (value, adjustment->lower, adjustment->upper);

  if (value != adjustment->value)
    {
      adjustment->value = value;

      gtk_adjustment_value_changed (adjustment);
    }
}

/**
 * gtk_adjustment_get_lower:
 * @adjustment: a #GtkAdjustment
 *
 * Retrieves the minimum value of the adjustment.
 *
 * Return value: The current minimum value of the adjustment.
 *
 * Since: 2.14
 **/
gdouble
Adjustment::get_lower (void)
{
  g_return_val_if_fail (GTK_IS_ADJUSTMENT (adjustment), 0.0);

  return adjustment->lower;
}

/**
 * gtk_adjustment_set_lower:
 * @adjustment: a #GtkAdjustment
 * @lower: the new minimum value
 *
 * Sets the minimum value of the adjustment.
 *
 * When setting multiple adjustment properties via their individual
 * setters, multiple "changed" signals will be emitted. However, since
 * the emission of the "changed" signal is tied to the emission of the
 * "GObject::notify" signals of the changed properties, it's possible
 * to compress the "changed" signals into one by calling
 * g_object_freeze_notify() and g_object_thaw_notify() around the
 * calls to the individual setters.
 *
 * Alternatively, using a single g_object_set() for all the properties
 * to change, or using gtk_adjustment_configure() has the same effect
 * of compressing "changed" emissions.
 *
 * Since: 2.14
 **/
void
Adjustment::set_lower (
                          gdouble        lower)
{
  g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));

  if (lower != adjustment->lower)
    g_object_set (adjustment, "lower", lower, NULL);
}

/**
 * gtk_adjustment_get_upper:
 * @adjustment: a #GtkAdjustment
 *
 * Retrieves the maximum value of the adjustment.
 *
 * Return value: The current maximum value of the adjustment.
 *
 * Since: 2.14
 **/
gdouble
Adjustment::get_upper (void)
{
  g_return_val_if_fail (GTK_IS_ADJUSTMENT (adjustment), 0.0);

  return adjustment->upper;
}

/**
 * gtk_adjustment_set_upper:
 * @adjustment: a #GtkAdjustment
 * @upper: the new maximum value
 *
 * Sets the maximum value of the adjustment.
 *
 * Note that values will be restricted by
 * <literal>upper - page-size</literal> if the page-size
 * property is nonzero.
 *
 * See gtk_adjustment_set_lower() about how to compress multiple
 * emissions of the "changed" signal when setting multiple adjustment
 * properties.
 *
 * Since: 2.14
 **/
void
Adjustment::set_upper (
                          gdouble        upper)
{
  g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));

  if (upper != adjustment->upper)
    g_object_set (adjustment, "upper", upper, NULL);
}

/**
 * gtk_adjustment_get_step_increment:
 * @adjustment: a #GtkAdjustment
 *
 * Retrieves the step increment of the adjustment.
 *
 * Return value: The current step increment of the adjustment.
 *
 * Since: 2.14
 **/
gdouble
Adjustment::get_step_increment (void)
{
  g_return_val_if_fail (GTK_IS_ADJUSTMENT (adjustment), 0.0);

  return adjustment->step_increment;
}

/**
 * gtk_adjustment_set_step_increment:
 * @adjustment: a #GtkAdjustment
 * @step_increment: the new step increment
 *
 * Sets the step increment of the adjustment.
 *
 * See gtk_adjustment_set_lower() about how to compress multiple
 * emissions of the "changed" signal when setting multiple adjustment
 * properties.
 *
 * Since: 2.14
 **/
void
Adjustment::set_step_increment (
                                   gdouble        step_increment)
{
  g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));

  if (step_increment != adjustment->step_increment)
    g_object_set (adjustment, "step-increment", step_increment, NULL);
}

/**
 * gtk_adjustment_get_page_increment:
 * @adjustment: a #GtkAdjustment
 *
 * Retrieves the page increment of the adjustment.
 *
 * Return value: The current page increment of the adjustment.
 *
 * Since: 2.14
 **/
gdouble
Adjustment::get_page_increment (void)
{
  g_return_val_if_fail (GTK_IS_ADJUSTMENT (adjustment), 0.0);

  return adjustment->page_increment;
}

/**
 * gtk_adjustment_set_page_increment:
 * @adjustment: a #GtkAdjustment
 * @page_increment: the new page increment
 *
 * Sets the page increment of the adjustment.
 *
 * See gtk_adjustment_set_lower() about how to compress multiple
 * emissions of the "changed" signal when setting multiple adjustment
 * properties.
 *
 * Since: 2.14
 **/
void
Adjustment::set_page_increment (
                                   gdouble        page_increment)
{
  g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));

  if (page_increment != adjustment->page_increment)
    g_object_set (adjustment, "page-increment", page_increment, NULL);
}

/**
 * gtk_adjustment_get_page_size:
 * @adjustment: a #GtkAdjustment
 *
 * Retrieves the page size of the adjustment.
 *
 * Return value: The current page size of the adjustment.
 *
 * Since: 2.14
 **/
gdouble
Adjustment::get_page_size (void)
{
  g_return_val_if_fail (GTK_IS_ADJUSTMENT (adjustment), 0.0);

  return adjustment->page_size;
}

/**
 * gtk_adjustment_set_page_size:
 * @adjustment: a #GtkAdjustment
 * @page_size: the new page size
 *
 * Sets the page size of the adjustment.
 *
 * See gtk_adjustment_set_lower() about how to compress multiple
 * emissions of the "changed" signal when setting multiple adjustment
 * properties.
 *
 * Since: 2.14
 **/
void
Adjustment::set_page_size (
                              gdouble        page_size)
{
  g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));

  if (page_size != adjustment->page_size)
    g_object_set (adjustment, "page-size", page_size, NULL);
}

/**
 * gtk_adjustment_configure:
 * @adjustment: a #GtkAdjustment
 * @value: the new value
 * @lower: the new minimum value
 * @upper: the new maximum value
 * @step_increment: the new step increment
 * @page_increment: the new page increment
 * @page_size: the new page size
 *
 * Sets all properties of the adjustment at once.
 *
 * Use this function to avoid multiple emissions of the "changed"
 * signal. See gtk_adjustment_set_lower() for an alternative way
 * of compressing multiple emissions of "changed" into one.
 *
 * Since: 2.14
 **/
void
Adjustment::configure (
                          gdouble        value,
                          gdouble        lower,
                          gdouble        upper,
                          gdouble        step_increment,
                          gdouble        page_increment,
                          gdouble        page_size)
{
  gboolean value_changed = FALSE;
  guint64 old_stamp = adjustment_changed_stamp;

  g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));

  g_object_freeze_notify (G_OBJECT (adjustment));

  g_object_set (adjustment,
                "lower", lower,
                "upper", upper,
                "step-increment", step_increment,
                "page-increment", page_increment,
                "page-size", page_size,
                NULL);

  /* don't use CLAMP() so we don't end up below lower if upper - page_size
   * is smaller than lower
   */
  value = MIN (value, upper - page_size);
  value = MAX (value, lower);

  if (value != adjustment->value)
    {
      /* set value manually to make sure "changed" is emitted with the
       * new value in place and is emitted before "value-changed"
       */
      adjustment->value = value;
      value_changed = TRUE;
    }

  g_object_thaw_notify (G_OBJECT (adjustment));

  if (old_stamp == adjustment_changed_stamp)
    gtk_adjustment_changed (adjustment); /* force emission before ::value-changed */

  if (value_changed)
    gtk_adjustment_value_changed (adjustment);
}

void
Adjustment::changed (void)
{
  g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));

  g_signal_emit (adjustment, adjustment_signals[CHANGED], 0);
}

void
Adjustment::value_changed (void)
{
  g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));

  g_signal_emit (adjustment, adjustment_signals[VALUE_CHANGED], 0);
  g_object_notify (G_OBJECT (adjustment), "value");
}

void
Adjustment::clamp_page (
			   gdouble        lower,
			   gdouble        upper)
{
  gboolean need_emission;

  g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));

  lower = CLAMP (lower, adjustment->lower, adjustment->upper);
  upper = CLAMP (upper, adjustment->lower, adjustment->upper);

  need_emission = FALSE;

  if (adjustment->value + adjustment->page_size < upper)
    {
      adjustment->value = upper - adjustment->page_size;
      need_emission = TRUE;
    }
  if (adjustment->value > lower)
    {
      adjustment->value = lower;
      need_emission = TRUE;
    }

  if (need_emission)
    gtk_adjustment_value_changed (adjustment);
}

#define __GTK_ADJUSTMENT_C__
#include "gtkaliasdef.c"
