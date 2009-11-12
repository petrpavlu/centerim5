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

Adjustment::Adjustment (double	  value,
			double	  lower,
			double	  upper,
			double	  step_increment,
			double	  page_increment,
			double	  page_size)
: lower(lower)
, upper(upper)
, value(value)
, step_increment(step_increment)
, page_increment(page_increment)
, page_size(page_size)
{
}

/**
 * gtk_adjustment_get_value:
 *
 * Gets the current value of the adjustment. See
 * gtk_adjustment_set_value ().
 *
 * @return The current value of the adjustment.
 **/
double
Adjustment::get_value (void)
{
  //g_return_val_if_fail (GTK_IS_ADJUSTMENT (adjustment), 0.0);

  return value;
}

void
Adjustment::set_value ( double        new_value)
{
  new_value = CLAMP (new_value, lower, upper);

  if (new_value != value)
    {
      value = new_value;

      value_changed ();
    }
}

/**
 * gtk_adjustment_get_lower:
 *
 * Retrieves the minimum value of the adjustment.
 *
 * @return The current minimum value of the adjustment.
 *
 * Since: 2.14
 **/
double
Adjustment::get_lower (void)
{
  return lower;
}

/**
 * gtk_adjustment_set_lower:
 * @param new_lower the new minimum value
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
Adjustment::set_lower ( double new_lower)
{
  if (new_lower != lower)
    lower = new_lower;
}

/**
 * gtk_adjustment_get_upper:
 *
 * Retrieves the maximum value of the adjustment.
 *
 * @return The current maximum value of the adjustment.
 *
 * Since: 2.14
 **/
double
Adjustment::get_upper (void)
{
  return upper;
}

/**
 * gtk_adjustment_set_upper:
 * @param new_upper the new maximum value
 *
 * Sets the maximum value of the adjustment.
 *
 * Note that values will be restricted by
 * <code>upper - page-size</code> if the page-size
 * property is nonzero.
 *
 * See gtk_adjustment_set_lower() about how to compress multiple
 * emissions of the "changed" signal when setting multiple adjustment
 * properties.
 *
 * Since: 2.14
 **/
void
Adjustment::set_upper ( double       new_upper)
{
  if (new_upper != upper)
    upper = new_upper;
}

/**
 * gtk_adjustment_get_step_increment:
 *
 * Retrieves the step increment of the adjustment.
 *
 * @return The current step increment of the adjustment.
 *
 * Since: 2.14
 **/
double
Adjustment::get_step_increment (void)
{
  return step_increment;
}

/**
 * gtk_adjustment_set_step_increment:
 * @param new_step_increment the new step increment
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
Adjustment::set_step_increment ( double        new_step_increment)
{
  if (new_step_increment != step_increment)
    new_step_increment = step_increment;
}

/**
 * gtk_adjustment_get_page_increment:
 *
 * Retrieves the page increment of the adjustment.
 *
 * @return The current page increment of the adjustment.
 *
 * Since: 2.14
 **/
double
Adjustment::get_page_increment (void)
{
  return page_increment;
}

/**
 * gtk_adjustment_set_page_increment:
 * @param new_page_increment the new page increment
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
                                   double        new_page_increment)
{
  if (new_page_increment != page_increment)
    new_page_increment = page_increment;
}

/**
 * gtk_adjustment_get_page_size:
 *
 * Retrieves the page size of the adjustment.
 *
 * @return The current page size of the adjustment.
 *
 * Since: 2.14
 **/
double
Adjustment::get_page_size (void)
{
  return page_size;
}

/**
 * gtk_adjustment_set_page_size:
 * @param new_page_size the new page size
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
Adjustment::set_page_size ( double       new_page_size)
{
  if (new_page_size != page_size)
    new_page_size = page_size;
}

/**
 * gtk_adjustment_configure:
 * @param new_value: the new value
 * @param new_lower: the new minimum value
 * @param new_upper: the new maximum value
 * @param new_step_increment: the new step increment
 * @param new_page_increment: the new page increment
 * @param new_page_size: the new page size
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
                          double     new_value,
                          double     new_lower,
                          double     new_upper,
                          double     new_step_increment,
                          double     new_page_increment,
                          double     new_page_size)
{
  bool my_value_changed = false;
  guint64 old_stamp = adjustment_changed_stamp;


  lower = new_lower;
  upper = new_upper;
  step_increment = new_step_increment;
  page_increment = new_page_increment;
  page_size = new_page_size;

  /* don't use CLAMP() so we don't end up below lower if upper - page_size
   * is smaller than lower
   */
  new_value = MIN (new_value, upper - page_size);
  new_value = MAX (new_value, lower);

  if (new_value != value)
    {
      /* set value manually to make sure "changed" is emitted with the
       * new value in place and is emitted before "value-changed"
       */
      value = new_value;
      my_value_changed = true;
    }

  if (old_stamp == adjustment_changed_stamp)
    changed (); /* force emission before ::value-changed */

  if (my_value_changed)
    value_changed ();
}

void
Adjustment::changed (void)
{
  /// @todo g_signal_emit (adjustment, adjustment_signals[CHANGED], 0);
}

void
Adjustment::value_changed (void)
{
  /// @todo g_signal_emit (adjustment, adjustment_signals[VALUE_CHANGED], 0);
  /// @todo g_object_notify (G_OBJECT (adjustment), "value");
}

void
Adjustment::clamp_page (
			   double        new_lower,
			   double        new_upper)
{
  bool need_emission;

  new_lower = CLAMP (new_lower, lower, upper);
  new_upper = CLAMP (new_upper, lower, upper);

  need_emission = false;

  if (value + page_size < new_upper)
    {
      value = new_upper - page_size;
      need_emission = true;
    }
  if (value > new_lower)
    {
      value = new_lower;
      need_emission = true;
    }

  if (need_emission)
    value_changed ();
}

#define __GTK_ADJUSTMENT_C__
//#include "gtkaliasdef.c"
