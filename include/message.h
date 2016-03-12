/*
 * This file is part of intiLED.
 *
 * intiLED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * intiLED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with intiLED.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

#pragma once

#ifndef HOST_CTRL
#include <inttypes.h>
#include <descriptors.h>
#include <led.h>

#define NUMBER_OF_CHANNELS Led::TOTALCH
#else
/** Size in bytes of the Generic HID reporting endpoint. */
#define GENERIC_EPSIZE             8
#define NUMBER_OF_CHANNELS         7
#endif

#define GET_TIME    0
#define SET_TIME    1
#define GET_CURRENT 2
#define SET_CURRENT 3
#define GET_MAX     4
#define SET_MAX     5
#define GET_MODE    6
#define SET_MODE    7

