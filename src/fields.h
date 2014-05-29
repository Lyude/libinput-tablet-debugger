/*
 * Copyright ©2014 Lyude
 *
 * This file is free software: you may copy it, redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 2 of this License or (at your option) any
 * later version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FIELD_LOCATIONS_H
#define FIELD_LOCATIONS_H

/* The position of each field */
#define TABLET_SYSTEM_NAME_ROW     0
#define TABLET_STYLUS_TOUCHING_ROW 1

#define TABLET_TOOL_NAME_ROW       3

#define TABLET_X_AND_Y_ROW         5

#define TABLET_TILT_VERTICAL_ROW   7
#define TABLET_TILT_HORIZONTAL_ROW 8

#define TABLET_DISTANCE_ROW        10
#define TABLET_PRESSURE_ROW        11

#define TABLET_STYLUS_BUTTONS_ROW  13

/* The text in each field */
#define TABLET_SYSTEM_NAME_FIELD     "System name: %s"
#define TABLET_STYLUS_TOUCHING_FIELD "Stylus is touching tablet? %s"

#define TABLET_TOOL_NAME_FIELD       "Current tool: %s"

#define TABLET_X_AND_Y_FIELD         "X: %7.3f Y: %7.3f"

#define TABLET_TILT_VERTICAL_FIELD   "Vertical tilt: %.3f"
#define TABLET_TILT_HORIZONTAL_FIELD "Horizontal tilt: %.3f"

#define TABLET_DISTANCE_FIELD        "Distance: %.3f"
#define TABLET_PRESSURE_FIELD        "Pressure: %.3f"

#define TABLET_STYLUS_BUTTONS_FIELD  "Stylus Buttons: #1: %-5s  #2: %s"

#endif /* FIELD_LOCATIONS_H */
