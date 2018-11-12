/*******************************************************************************
*   (c) 2016 Ledger
*   (c) 2018 ZondaX GmbH
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/

#include "view.h"
#include "view_templates.h"
#include "common.h"

#include "glyphs.h"
#include "bagl.h"

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <zxmacros.h>

#define TRUE  1
#define FALSE 0

enum UI_DISPLAY_MODE {
    VALUE_SCROLLING,
    KEY_SCROLLING_NO_VALUE,
    KEY_SCROLLING_SHORT_VALUE,
    PENDING
};

ux_state_t ux;
enum UI_STATE view_uiState;
enum UI_DISPLAY_MODE scrolling_mode;

volatile char view_data_height[MAX_SCREEN_LINE_WIDTH];
volatile char view_data_round[MAX_SCREEN_LINE_WIDTH];
volatile char view_data_state[MAX_SCREEN_LINE_WIDTH];
volatile char view_data_publicKey[MAX_SCREEN_LINE_WIDTH];

int8_t data_msg_round;
int64_t data_msg_height;

//------ Event handlers
delegate_accept_reference_signature eh_accept = NULL;
delegate_reject_reference_signature eh_reject = NULL;
delegate_validation_reset           eh_validation_reset = NULL;

void view_set_validation_reset_eh(delegate_validation_reset delegate)
{
    eh_validation_reset = delegate;
}

void view_set_accept_eh(delegate_accept_reference_signature delegate)
{
    eh_accept = delegate;
}

void view_set_reject_eh(delegate_reject_reference_signature delegate)
{
    eh_reject = delegate;
}

//------ View elements
const ux_menu_entry_t menu_main[];
const ux_menu_entry_t menu_about[];

const ux_menu_entry_t menu_main[] = {
#ifdef TESTING_ENABLED
    {NULL, NULL, 0, NULL, "Cosmos TEST!", "Validator", 0, 0},
#else
    {NULL, NULL, 0, &C_icon_dashboard, "Cosmos", "Validator", 0, 0},
#endif
    {menu_about, NULL, 0, NULL, "About", NULL, 0, 0},
    {NULL, os_sched_exit, 0, &C_icon_dashboard, "Quit app", NULL, 50, 29},
    UX_MENU_END
};

const ux_menu_entry_t menu_about[] = {
    {NULL, NULL, 0, NULL, "Version", APPVERSION, 0, 0},
    {menu_main, NULL, 2, &C_icon_back, "Back", NULL, 61, 40},
    UX_MENU_END
};

static const bagl_element_t bagl_ui_initialize_transaction[] = {
    UI_FillRectangle(0, 0, 0, 128, 32, 0x000000, 0xFFFFFF),
    UI_Icon(0, 0, 0, 7, 7, BAGL_GLYPH_ICON_CROSS),
    UI_Icon(0, 128 - 7, 0, 7, 7, BAGL_GLYPH_ICON_CHECK),
    UI_LabelLine(1, 0, 8, 128, 11, 0xFFFFFF, 0x000000, "Init validation"),
    UI_LabelLine(1, 0, 19, 128, 11, 0xFFFFFF, 0x000000, (const char *)view_data_height),
    UI_LabelLine(1, 0, 30, 128, 11, 0xFFFFFF, 0x000000, (const char *)view_data_round),
};

static const bagl_element_t bagl_ui_validating_transaction[] = {
    UI_FillRectangle(0, 0, 0, 128, 32, 0x000000, 0xFFFFFF),
    UI_LabelLine(1, 0, 8, 128, 11, 0xFFFFFF, 0x000000, "Validating"),

    // 4 labels..
    // "Height:" [value]
    // "PK"      [PK]
    UI_LabelLine(1, 0, 19, 128, 11, 0xFFFFFF, 0x000000, (const char *)view_data_state),
    UI_LabelLineScrolling(1, 0, 30, 128, 11, 0xFFFFFF, 0x000000, (const char *)view_data_publicKey),
};

static unsigned int bagl_ui_initialize_transaction_button(
        unsigned int button_mask,
        unsigned int button_mask_counter) {

    switch (button_mask) {

        // Press left to progress to the previous element
        case BUTTON_EVT_RELEASED | BUTTON_LEFT: {
            if (eh_reject != NULL) {
                eh_reject();
            }
            break;
        }

        // Press right to progress to the next element
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT: {
            if (eh_accept != NULL) {
                eh_accept(data_msg_round, data_msg_height);
            }
            break;
        }
    }
    return 0;
}

static unsigned int bagl_ui_validating_transaction_button(
        unsigned int button_mask,
        unsigned int button_mask_counter) {

    switch (button_mask) {
        // We dont allow people to exit this mode
//        // Press both left and right to switch to value scrolling
//        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: {
//            if (eh_validation_reset != NULL) {
//                eh_validation_reset();
//            }
//            view_display_main_menu();
//            break;
//        }
    }
    return 0;
}

void io_seproxyhal_display(const bagl_element_t *element) {
    io_seproxyhal_display_default((bagl_element_t *) element);
}

void view_init() {
    UX_INIT();
    view_uiState = UI_MAIN_MENU;
}

void view_display_main_menu() {
    view_uiState = UI_MAIN_MENU;
    UX_MENU_DISPLAY(0, menu_main, NULL);
}

const bagl_element_t *ui_validation_processing_prepro(const bagl_element_t *element) {

    UX_CALLBACK_SET_INTERVAL(50);
    return element;
}

void view_display_validation_init() {

    view_uiState = UI_VALIDAITON_INIT;
    UX_DISPLAY(bagl_ui_initialize_transaction, NULL);
}

void view_display_validation_processing() {

    view_uiState = UI_VALIDATION_PROCESSING;
    UX_DISPLAY(bagl_ui_validating_transaction, ui_validation_processing_prepro);
}

void view_set_state(int8_t msg_round, int64_t msg_height)
{
    char int64str[] = "-9223372036854775808";
    int64_to_str(int64str, sizeof(int64str), msg_height);
    snprintf((char*)view_data_state, MAX_SCREEN_LINE_WIDTH, "%s-%03d\n", int64str, msg_round);
}

void view_set_msg_height(int64_t height)
{
    char int64str[] = "-9223372036854775808";
    int64_to_str(int64str, sizeof(int64str), height);
    data_msg_height = height;
    snprintf((char*)view_data_height, MAX_SCREEN_LINE_WIDTH, "Height: %s\n", int64str);
}

void view_set_msg_round(int8_t msg_round)
{
    data_msg_round = msg_round;
    snprintf((char*)view_data_round, MAX_SCREEN_LINE_WIDTH, "Round: %03d\n", data_msg_round);
}

void view_set_public_key(const char* publicKey)
{
    snprintf((char*)view_data_publicKey, MAX_SCREEN_LINE_WIDTH, "PK: %s\n", publicKey);
}
