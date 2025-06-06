/* Copyright 2024 Hashikure-engneering
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

#include "meteorite40.h"
#include "quantum.h"
#include "print.h"
#include <math.h>

// Default configuration values for mouse settings
#define METEORITE_CPI_DEFAULT 5
#define METEORITE_CPI_MAX 32
#define METEORITE_SCROLL_DIV_DEFAULT 5
#define METEORITE_SCROLL_DIV_MAX 16
#define METEORITE_SCROLL_LAYER_DEFAULT 2

#define METEORITE_ROTATION_DEFAULT 7
#define METEORITE_ROTATION_ANGLE { -70, -60, -50, -40, -30, -20, -10, 0, 10, 20, 30, 40, 50, 60, 70 }

#define SCROLL_LAYER_TAPPING_TERM 120

uint16_t angle_array[] = METEORITE_ROTATION_ANGLE;
#define ANGLE_SIZE (sizeof(angle_array) / sizeof(uint16_t))

meteorite_config_t meteorite_config;

static bool scroll_mode = false; // Scroll mode flag
static bool scroll_mode_layer = false; // Scroll mode by layer flag
static float scroll_h_acm = 0.0f; // Horizontal scroll accumulator
static float scroll_v_acm = 0.0f; // Vertical scroll accumulator

// Mouse position accumulators for precise tracking
static float mouse_x_acm = 0.0f;
static float mouse_y_acm = 0.0f;

//static int x_prev = 0;
//static int y_prev = 0;

// Handle mouse movement and scroll modes
report_mouse_t pointing_device_task_kb(report_mouse_t mouse_report) {

    if(mouse_report.x != 0 || mouse_report.y != 0){
        /*
        if((abs(mouse_report.x) + abs(mouse_report.y)) > (abs(x_prev) + abs(y_prev) + 1) * 4){
            uprintf("### x:%3d, y:%3d, x_prev: %3d, y_prev: %3d ###\n", mouse_report.x, mouse_report.y, x_prev, y_prev);
            mouse_report.x = x_prev;
            mouse_report.y = y_prev;
        } else {
            x_prev = mouse_report.x;
            y_prev = mouse_report.y;
        };
        */

        int16_t custom_x = 0;
        int16_t custom_y = 0;
        int16_t custom_h = 0;
        int16_t custom_v = 0;

        // Rotate mouse movement based on configured angle
        float rad = angle_array[meteorite_config.rotation_ang] * (M_PI / 180.0f) * -1.0f;
        //float cos_rad = cosf(angle_array[meteorite_config.rotation_ang]);
        //float sin_rad = sinf(angle_array[meteorite_config.rotation_ang]);

        float rotated_x =  + mouse_report.x * cosf(rad) - mouse_report.y * sinf(rad);
        float rotated_y =  + mouse_report.x * sinf(rad) + mouse_report.y * cosf(rad);

        if (!scroll_mode && !scroll_mode_layer) {  // Regular mode: apply scaling if enabled
            //Calculate the mouse movement delta for each polling interval
            float delta = sqrt(rotated_x * rotated_x + rotated_y * rotated_y);

            if(meteorite_config.scaling_mode == 1){
                //Calculate a coefficient that ranges from 0.1 to 10 when delta is between 1 and 100.
                //float scaling_factor =  0.1f * delta; //(0.099f * delta) + 0.2f;
                rotated_x *= 0.1f * delta;
                rotated_y *= 0.1f * delta;
            }

            // Clip the rotated values within -127 to 127 range
            rotated_x = fmaxf(-127.0f, fminf(127.0f, rotated_x));
            rotated_y = fmaxf(-127.0f, fminf(127.0f, rotated_y));

            // Accumulate mouse position and extract integer values
            mouse_x_acm += rotated_x;
            mouse_y_acm += rotated_y;
            custom_x = (int16_t) mouse_x_acm;
            custom_y = (int16_t) mouse_y_acm;

            // Retain decimal portion for smoother movement
            mouse_x_acm -= custom_x;
            mouse_y_acm -= custom_y;

            #ifdef DEBUG
            uprintf("x:%3d, y:%3d,  ", mouse_report.x, mouse_report.y);
            uprintf("x_rot:%4d.%02d, ", (int)(rotated_x), abs((int)(rotated_x * 1000) % 100));
            uprintf("y_rot:%4d.%02d,  ", (int)(rotated_y), abs((int)(rotated_y * 1000) % 100));
            uprintf("x_acm:%5d, y_acm:%5d,  ", (int)(mouse_x_acm * 1000), (int)(mouse_y_acm * 1000));
            uprintf("x_cus:%4d, y_cus:%4d,  ", custom_x, custom_y);
            uprintf("delta:%3d.%02d\n", (int)delta, abs((int)(delta * 100) % 100));
            #endif

            mouse_report.x = custom_x;
            mouse_report.y = custom_y;

        } else { // Scroll mode: accumulate scroll values

            //Get Scroll Division Factor
            int16_t scroll_div = meteorite_get_scroll_div(meteorite_config.scroll_div);

            if (abs(rotated_x) > abs(rotated_y) * 2) { // Horizontal scroll
                // Accumulate horizontal scroll movement
                scroll_h_acm += rotated_x;

                if(abs(scroll_h_acm) > scroll_div){
                    custom_h = scroll_h_acm / scroll_div;
                    scroll_h_acm -= custom_h * scroll_div;
                }
            } else { // Vertical scroll
                // Accumulate vertical scroll movement
                scroll_v_acm += rotated_y;

                if(abs(scroll_v_acm) > scroll_div){
                    custom_v = scroll_v_acm / scroll_div;
                    scroll_v_acm -= custom_v * scroll_div;
                }
            }

            mouse_report.x = 0;
            mouse_report.y = 0;
            mouse_report.h = custom_h * (meteorite_config.scroll_h_rev ? 1 : -1);
            mouse_report.v = custom_v * (meteorite_config.scroll_v_rev ? 1 : -1);

            #ifdef DEBUG
            uprintf("h: %d, v: %d\n", mouse_report.h, mouse_report.v);
            #endif
        }
    }
    return pointing_device_task_user(mouse_report);
}

// Handle custom keycodes for adjusting configuration
bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case SCR_MO: // Toggle scroll mode
            if (record->event.pressed) {
                scroll_mode = true;
                //mouse_x_acm = 0;
                //mouse_y_acm = 0;
            } else {
                scroll_mode = false;
                //scroll_h_acm = 0;
                //scroll_v_acm = 0;
            }
            return false;
        case CPI_DEC:
            if(record->event.pressed){
                meteorite_config.cpi_val = (meteorite_config.cpi_val -1 + METEORITE_CPI_MAX) % (METEORITE_CPI_MAX);
                pointing_device_set_cpi(meteorite_get_cpi(meteorite_config.cpi_val));
                #ifdef DEBUG
                debug_meteorite_config(meteorite_config);
                #endif
            }
            return false;
        case CPI_INC:
            if(record->event.pressed){
                meteorite_config.cpi_val = (meteorite_config.cpi_val + 1) % METEORITE_CPI_MAX;
                pointing_device_set_cpi(meteorite_get_cpi(meteorite_config.cpi_val));
                #ifdef DEBUG
                debug_meteorite_config(meteorite_config);
                #endif
            }
            return false;
        case SCAL_MODE: // Toggle scaling mode
            if (record->event.pressed) {
                meteorite_config.scaling_mode ^= 1;
                #ifdef DEBUG
                debug_meteorite_config(meteorite_config);
                #endif
            }
            return false;
        case SCR_DEC:
            if(record->event.pressed){
                meteorite_config.scroll_div = (meteorite_config.scroll_div + 1) % METEORITE_SCROLL_DIV_MAX;
                #ifdef DEBUG
                debug_meteorite_config(meteorite_config);
                #endif
            }
            return false;
        case SCR_INC:
            if(record->event.pressed){
                meteorite_config.scroll_div = (meteorite_config.scroll_div -1 + METEORITE_SCROLL_DIV_MAX) % (METEORITE_SCROLL_DIV_MAX);
                #ifdef DEBUG
                debug_meteorite_config(meteorite_config);
                #endif
            }
            return false;
        case ROT_DEC:
            if(record->event.pressed){
                meteorite_config.rotation_ang = (meteorite_config.rotation_ang - 1 + ANGLE_SIZE) % ANGLE_SIZE;
                #ifdef DEBUG
                debug_meteorite_config(meteorite_config);
                #endif
            }
            return false;
        case ROT_INC:
            if(record->event.pressed){
                meteorite_config.rotation_ang = (meteorite_config.rotation_ang + 1) % ANGLE_SIZE;
                #ifdef DEBUG
                debug_meteorite_config(meteorite_config);
                #endif
            }
            return false;
        case SCR_H_REV:
            if (record->event.pressed) {
                meteorite_config.scroll_h_rev ^= 1;
                #ifdef DEBUG
                debug_meteorite_config(meteorite_config);
                #endif
            }
            return false;
        case SCR_V_REV:
            if (record->event.pressed) {
                meteorite_config.scroll_v_rev ^= 1;
                #ifdef DEBUG
                debug_meteorite_config(meteorite_config);
                #endif
            }
            return false;
        case SCR_LYR_DEC:
            if(record->event.pressed){
                meteorite_config.scroll_layer = (meteorite_config.scroll_layer - 2 + (DYNAMIC_KEYMAP_LAYER_COUNT - 1)) % (DYNAMIC_KEYMAP_LAYER_COUNT - 1) + 1;
                #ifdef DEBUG
                debug_meteorite_config(meteorite_config);
                #endif
            }
            return false;
        case SCR_LYR_INC:
            if(record->event.pressed){
                meteorite_config.scroll_layer = (meteorite_config.scroll_layer % (DYNAMIC_KEYMAP_LAYER_COUNT - 1)) + 1;
                #ifdef DEBUG
                debug_meteorite_config(meteorite_config);
                #endif
            }
            return false;
        case KBC_RST:
            if(record->event.pressed){
                meteorite_set_default_config();
                pointing_device_set_cpi(meteorite_get_cpi(meteorite_config.cpi_val));
                #ifdef DEBUG
                uprintf("\n### Reset Meteorite default config ###\n");
                debug_meteorite_config(meteorite_config);
                #endif
            }
            return false;
        case KBC_SAV:
            if(record->event.pressed){
                eeconfig_update_kb(meteorite_config.raw);
                #ifdef DEBUG
                uprintf("\n### Save Meteorite config ###\n");
                debug_meteorite_config(meteorite_config);
                #endif
            }
            return false;
        default:
            #ifdef DEBUG
            if (record->event.pressed) {
                uprintf("Key pressed: %u\n", keycode);
            }
            #endif
        return process_record_user(keycode, record);
    }
}

void matrix_scan_kb(void) {
    // Check if scroll layer is active
    if (layer_state_is(meteorite_config.scroll_layer)) {
        scroll_mode_layer = true;
        //mouse_x_acm = 0;
        //mouse_y_acm = 0;
    } else {
        scroll_mode_layer = false;
        //scroll_h_acm = 0;
        //scroll_v_acm = 0;
    }
    matrix_scan_user();
}

uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record) {
    // If the keycode is an LT key and the layer number
    if ((keycode & QK_LAYER_TAP) == QK_LAYER_TAP && ((keycode >> 8) & 0x0F) == meteorite_config.scroll_layer) {
        /*
        #ifdef DEBUG
        uprintf("change tapping term of scroll layer: %d \n", meteorite_config.scroll_layer);
        #endif
        */

        return SCROLL_LAYER_TAPPING_TERM;
    }
    return TAPPING_TERM;
}

void eeconfig_init_kb(void) {
    meteorite_set_default_config();
    eeconfig_update_kb(meteorite_config.raw);
    meteorite_config.raw = eeconfig_read_kb();

    eeconfig_init_user();
}

void matrix_init_kb(void) {
    meteorite_config.raw = eeconfig_read_kb();
    matrix_init_user();
}

void pointing_device_init_kb(void) {
    pointing_device_set_cpi(meteorite_get_cpi(meteorite_config.cpi_val));
}

uint16_t meteorite_get_cpi(uint8_t cpi) {
    // Convert CPI configuration to hardware-specific value
    return (cpi + 1) * 100;
}

uint16_t meteorite_get_scroll_div(uint8_t div) {
    // Calculate scroll division factor
    return div * div * 2 + 10;
}

void meteorite_set_default_config(void){
    meteorite_config.cpi_val = METEORITE_CPI_DEFAULT;
    meteorite_config.scroll_div = METEORITE_SCROLL_DIV_DEFAULT;
    meteorite_config.rotation_ang = METEORITE_ROTATION_DEFAULT;
    meteorite_config.scroll_layer = METEORITE_SCROLL_LAYER_DEFAULT;
    meteorite_config.scroll_v_rev = 0;
    meteorite_config.scroll_h_rev = 0;
    meteorite_config.scaling_mode = 0;
}

#ifdef DEBUG
void debug_meteorite_config(meteorite_config_t config) {
    uprintf("Meteorite Config:\n");
    uprintf("  RAW          : %d\n", config.raw);
    uprintf("  CPI          : %d (%d)\n", config.cpi_val, meteorite_get_cpi(config.cpi_val));
    uprintf("  Scroll Div   : %d (%d)\n", config.scroll_div, meteorite_get_scroll_div(config.scroll_div));
    uprintf("  Scroll layer : %d / %d\n", config.scroll_layer, DYNAMIC_KEYMAP_LAYER_COUNT - 1);
    uprintf("  Rot Angle    : %d (%d)\n", config.rotation_ang, angle_array[config.rotation_ang]);
    //uprintf("  Rot Angle    : %d\n", config.rotation_ang);
    uprintf("  Scroll H Rev : %d\n", config.scroll_h_rev);
    uprintf("  Scroll V Rev : %d\n", config.scroll_v_rev);
    uprintf("  Scaling Mode : %d\n", config.scaling_mode);
}
#endif
