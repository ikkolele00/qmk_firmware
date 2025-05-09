#include QMK_KEYBOARD_H
#include "cb_anim.h"

// to compile: qmk compile -kb lily58 -km ikkoDefault

enum layer_number {
  _QWERTY = 0,
  _LOWER,
  _RAISE,
  _ADJUST,
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_QWERTY] = LAYOUT(KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_GRV, KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_MINS, KC_LCTL, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_F21, KC_DEL, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT, KC_LALT, KC_LGUI, MO(1), KC_SPC, KC_ENT, MO(2), KC_BSPC, KC_RCTL),
    [_LOWER] = LAYOUT(KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_NO, KC_NO, KC_NO, KC_LPRN, KC_RPRN, KC_NO, KC_TRNS, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_NO, KC_UNDS, KC_PLUS, KC_LCBR, KC_RCBR, KC_PIPE, KC_TRNS, KC_F11, KC_F12, KC_CUT, KC_COPY, KC_PSTE, KC_TRNS, KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, MO(3), KC_TRNS, KC_TRNS),
    [_RAISE] = LAYOUT(KC_TRNS, KC_NO, KC_7, KC_8, KC_9, KC_PSLS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_TRNS, KC_TRNS, KC_NO, KC_4, KC_5, KC_6, KC_PAST, KC_PGUP, KC_HOME, KC_UP,KC_END, KC_PGDN, KC_NO, KC_TRNS, KC_NO, KC_1, KC_2, KC_3, KC_PMNS, KC_NO, KC_LEFT, KC_DOWN, KC_RGHT, KC_NO, KC_NO, KC_TRNS, KC_NO, KC_0, KC_DOT, KC_EQL, KC_PPLS, KC_TRNS, KC_NO, KC_PLUS, KC_MINS, KC_EQL, KC_LBRC, KC_RBRC, KC_BSLS, KC_TRNS, KC_TRNS, MO(3), KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),
    [_ADJUST] = LAYOUT(KC_NO, KC_MUTE, KC_VOLD, KC_VOLU, KC_BRID, KC_BRIU, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_MPRV, KC_MPLY, KC_MNXT, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_TRNS, KC_CAPS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS)
};

layer_state_t layer_state_set_user(layer_state_t state) {
  return state;
}

//SSD1306 OLED update loop, make sure to enable OLED_ENABLE=yes in rules.mk
#ifdef OLED_ENABLE

// Animation timing
#define START_PAUSE 10000  // Wait 10s on frame 0
#define FRAME_DELAY 100    // 10fps (100ms per frame)
#define END_PAUSE   10000  // Wait 10s on last frame

// Animation state
uint32_t anim_timer = 0;
uint8_t current_frame = 0;

enum {
    STATE_PAUSE_START,  // Show frame 0 for 10 seconds
    STATE_ANIMATING,    // Animate all frames (including start and end) at 10fps
    STATE_PAUSE_END     // Hold the last frame for 10 seconds
} anim_state = STATE_PAUSE_START;

// render_animation function with new state machine
static void render_animation(void) {
    switch (anim_state) {
        case STATE_PAUSE_START:
            // Display frame 0 for 10 seconds
            if (timer_elapsed(anim_timer) > START_PAUSE) {
                anim_timer = timer_read();
                current_frame = 1;  // Start animating from frame 1
                anim_state = STATE_ANIMATING;
            }
            break;

        case STATE_ANIMATING:
            // Display each frame in cb_frames[] at 10fps (100ms per frame)
            if (timer_elapsed(anim_timer) > FRAME_DELAY) {
                anim_timer = timer_read();
                current_frame++;  // Increment to the next frame

                // If we have reached the last frame, stop and hold it
                if (current_frame >= sizeof(cb_frames) / sizeof(cb_frames[0])) {
                    current_frame = sizeof(cb_frames) / sizeof(cb_frames[0]) - 1; // Hold the last frame
                    anim_timer = timer_read();
                    anim_state = STATE_PAUSE_END;
                }
            }
            break;

        case STATE_PAUSE_END:
            // Hold the last frame for 10 seconds
            if (timer_elapsed(anim_timer) > END_PAUSE) {
                current_frame = 0;  // Reset to frame 0 to start the cycle again
                anim_timer = timer_read();
                anim_state = STATE_PAUSE_START;
            }
            break;
    }

    // Always render the current frame
    oled_write_raw_P(cb_frames[current_frame], frame_sizes[current_frame]);
}

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
  if (is_keyboard_master())
    return OLED_ROTATION_180;  // flips the display 180 degrees if offhand
  return rotation;
}

// When you add source files to SRC in rules.mk, you can use functions.
const char *read_layer_state(void);
const char *read_logo(void);
void set_keylog(uint16_t keycode, keyrecord_t *record);
const char *read_keylog(void);
const char *read_keylogs(void);

bool oled_task_user(void) {
  if (is_keyboard_master()) {
    oled_write_ln(read_layer_state(), false);
    oled_write_ln(read_keylog(), false);
    oled_write_ln(read_keylogs(), false);
  } else {
    render_animation();
  }
  return false;
}
#endif // OLED_ENABLE

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  if (record->event.pressed) {
#ifdef OLED_ENABLE
    set_keylog(keycode, record);
#endif
    // set_timelog();
  }
  return true;
}

bool encoder_update_user(uint8_t index, bool clockwise) { 
    if (index == 0) { 
        if (clockwise) { 
            tap_code(KC_VOLU); 
        } else { 
            tap_code(KC_VOLD); 
        } 
    } 
    return false;
}

