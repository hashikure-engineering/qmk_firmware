/* Copyright 2024 Hashikure-engineering
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

/* PMW3360 */
#define PMW33XX_CS_PIN SPI_SS_PIN
#define POINTING_DEVICE_INVERT_X

/* rotary encoder */
#define ENCODER_A_PINS { F0, F4 }
#define ENCODER_B_PINS { F1, F5 }
#define ENCODER_RESOLUTIONS { 4, 4 }

#define DYNAMIC_KEYMAP_LAYER_COUNT 5

#define TAPPING_TERM 200
#define TAPPING_TERM_PER_KEY

/* debug for Mouse / Scroll and METEORITE config */
#define DEBUG
