/*******************************************************************************
*   (c) 2016 Ledger
*   (c) 2018, 2019 ZondaX GmbH
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
#include "actions.h"
#include "vote.h"

#include "glyphs.h"
#include "bagl.h"

#include <zxmacros.h>
#include "view_templates.h"
#include "common.h"

#include <os_io_seproxyhal.h>
#include <os.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

struct {
    char height[MAX_SCREEN_LINE_WIDTH];
    char round[MAX_SCREEN_LINE_WIDTH];
    char type_round[MAX_SCREEN_LINE_WIDTH];
    char public_key[MAX_SCREEN_LINE_WIDTH];
#if defined(TARGET_NANOX)
    char public_key1[MAX_SCREEN_LINE_WIDTH];
    char public_key2[MAX_SCREEN_LINE_WIDTH];
#endif
} view;

void reject(unsigned int _) {
    action_reject();
    view_idle();
}

void accept(unsigned int _) {
    action_accept();
    view_display_vote_processing();
}

#ifdef TARGET_NANOX

#include "ux.h"
ux_state_t G_ux;
bolos_ux_params_t G_ux_params;

#ifdef TESTING_ENABLED
UX_FLOW_DEF_NOCB(ux_idle_flow_1_step, pbb, { &C_icon_app, "Tendermint", "Validator (TEST)", });
#else
UX_FLOW_DEF_NOCB(ux_idle_flow_1_step, pbb, { &C_icon_app, "Tendermint", "Validator", });
#endif
UX_FLOW_DEF_NOCB(ux_idle_flow_2_step, bnnn_paging, { "Public Key", view.public_key });
UX_FLOW_DEF_NOCB(ux_idle_flow_3_step, bn, { "Version", APPVERSION, });
UX_FLOW_DEF_VALID(ux_idle_flow_4_step, pb, os_sched_exit(-1), { &C_icon_dashboard, "Quit",});
const ux_flow_step_t *const ux_idle_flow [] = {
  &ux_idle_flow_1_step,
  &ux_idle_flow_2_step,
  &ux_idle_flow_3_step,
  &ux_idle_flow_4_step,
  FLOW_END_STEP,
};

UX_FLOW_DEF_NOCB(ux_init_flow_1_step, bnn, { "Init Validator", view.height, view.type_round});
UX_FLOW_DEF_NOCB(ux_init_flow_2_step, bnnn_paging, { "Public Key", view.public_key });
UX_FLOW_DEF_VALID(ux_init_flow_3_step, pb, accept(0), { &C_icon_validate_14, "Reply", });
UX_FLOW_DEF_VALID(ux_init_flow_4_step, pb, reject(0), { &C_icon_crossmark, "Reject", });
const ux_flow_step_t *const ux_init_flow [] = {
  &ux_init_flow_1_step,
  &ux_init_flow_2_step,
  &ux_init_flow_3_step,
  &ux_init_flow_4_step,
  FLOW_END_STEP,
};

UX_FLOW_DEF_NOCB(ux_update_flow_1_step, nnnn, { view.height, view.type_round, view.public_key1, view.public_key2});
const ux_flow_step_t *const ux_update_flow [] = {
  &ux_update_flow_1_step,
  FLOW_END_STEP,
};

#else

ux_state_t ux;

//------ View elements
const ux_menu_entry_t menu_main[];
const ux_menu_entry_t menu_about[];

const ux_menu_entry_t menu_main[] = {
#ifdef TESTING_ENABLED
        {NULL, NULL, 0, &C_icon_app, "Tendermint TEST!", "Validator", 0, 0},
#else
        {NULL, NULL, 0, &C_icon_app, "Tendermint", "Validator", 0, 0},
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
        UI_FillRectangle(0, 0, 0, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, UI_BLACK, UI_WHITE),
        UI_Icon(0, 0, 0, 7, 7, BAGL_GLYPH_ICON_CROSS),
        UI_Icon(0, 128 - 7, 0, 7, 7, BAGL_GLYPH_ICON_CHECK),
        UI_LabelLine(1, 0, 8, UI_SCREEN_WIDTH, UI_11PX, UI_WHITE, UI_BLACK, "Init validation"),
        UI_LabelLine(1, 0, 19, UI_SCREEN_WIDTH, UI_11PX, UI_WHITE, UI_BLACK, (const char *) view.height),
        UI_LabelLine(1, 0, 30, UI_SCREEN_WIDTH, UI_11PX, UI_WHITE, UI_BLACK, (const char *) view.round),
};

static const bagl_element_t bagl_ui_validating_transaction[] = {
        UI_FillRectangle(0, 0, 0, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, UI_BLACK, UI_WHITE),
        // Type
        // "Height:Round"
        // "PK"      [PK]
        UI_LabelLine(1, 0, 8, UI_SCREEN_WIDTH, UI_11PX, UI_WHITE, UI_BLACK, (const char *) view.height),
        UI_LabelLine(1, 0, 19, UI_SCREEN_WIDTH, UI_11PX, UI_WHITE, UI_BLACK, (const char *) view.type_round),
        UI_LabelLine(1, 0, 30, UI_SCREEN_WIDTH, UI_11PX, UI_WHITE, UI_BLACK, (const char *) view.public_key),
};

static unsigned int bagl_ui_initialize_transaction_button(
        unsigned int button_mask,
        unsigned int button_mask_counter) {

    switch (button_mask) {

        // Press left to progress to the previous element
        case BUTTON_EVT_RELEASED | BUTTON_LEFT: {
            reject(0);
            break;
        }

            // Press right to progress to the next element
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT: {
            accept(0);
            break;
        }
    }
    return 0;
}

static unsigned int bagl_ui_validating_transaction_button(
        unsigned int button_mask,
        unsigned int button_mask_counter) {

    switch (button_mask) {
        // We dont allow people to exit this mode. ONLY TEST MODE
#ifdef TESTING_ENABLED
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: {
            reset_vote();
            view_idle();
            break;
        }
#endif
    }
    return 0;
}

#endif

void io_seproxyhal_display(const bagl_element_t *element) {
    io_seproxyhal_display_default((bagl_element_t *) element);
}

void view_init() {
    UX_INIT();
}

void view_idle() {
#if defined(TARGET_NANOS)
    UX_MENU_DISPLAY(0, menu_main, NULL);
#elif defined(TARGET_NANOX)
    if(G_ux.stack_count == 0) {
        ux_stack_push();
    }
    ux_flow_init(0, ux_idle_flow, NULL);
#endif
}

void view_display_vote_init() {
#if defined(TARGET_NANOS)
    UX_DISPLAY(bagl_ui_initialize_transaction, NULL);
#elif defined(TARGET_NANOX)
    if(G_ux.stack_count == 0) {
        ux_stack_push();
    }
    ux_flow_init(0, ux_init_flow, NULL);
#endif
}

void view_display_vote_processing() {
#if defined(TARGET_NANOS)
    UX_DISPLAY(bagl_ui_validating_transaction, NULL);
#elif defined(TARGET_NANOX)
    if(G_ux.stack_count == 0) {
        ux_stack_push();
    }
    ux_flow_init(0, ux_update_flow, NULL);
#endif
}

void view_set_data() {
    switch (vote.Type) {
        case TYPE_PREVOTE:
#ifdef TESTING_ENABLED
            snprintf((char *) view.type_round, MAX_SCREEN_LINE_WIDTH, "TEST Prevote   :R%03d", vote.Round);
#else
            snprintf((char *) view.type_round, MAX_SCREEN_LINE_WIDTH, "Prevote   :R%03d", vote.Round);
#endif
            break;
        case TYPE_PRECOMMIT:
#ifdef TESTING_ENABLED
            snprintf((char *) view.type_round, MAX_SCREEN_LINE_WIDTH, "TEST Precommit :R%03d", vote.Round);
#else
            snprintf((char *) view.type_round, MAX_SCREEN_LINE_WIDTH, "Precommit :R%03d", vote.Round);
#endif
            break;
        case TYPE_PROPOSAL:
#ifdef TESTING_ENABLED
            snprintf((char *) view.type_round, MAX_SCREEN_LINE_WIDTH, "TEST Proposal  :R%03d", vote.Round);
#else
            snprintf((char *) view.type_round, MAX_SCREEN_LINE_WIDTH, "Proposal  :R%03d", vote.Round);
#endif
            break;
        default:
#ifdef TESTING_ENABLED
            snprintf((char *) view.type_round, MAX_SCREEN_LINE_WIDTH, "TEST Unknown   :R%03d", vote.Round);
#else
            snprintf((char *) view.type_round, MAX_SCREEN_LINE_WIDTH, "Unknown   :R%03d", vote.Round);
#endif
            break;
    }

    char tmp[MAX_SCREEN_LINE_WIDTH];
    int64_to_str((char *) tmp, MAX_SCREEN_LINE_WIDTH, vote.Height);

    snprintf((char *) view.height, MAX_SCREEN_LINE_WIDTH, "Height: %s", tmp);
    snprintf((char *) view.round, MAX_SCREEN_LINE_WIDTH, "Round: %03d", vote.Round);
}

void view_set_pk(uint8_t public_key[32]) {
    // split pubkey
#if defined(TARGET_NANOS)
    array_to_hexstr((char *) view.public_key, public_key, 4);
    view.public_key[8] = '.';
    view.public_key[9] = '.';
    array_to_hexstr((char *) view.public_key + 10, public_key + 28, 4);
#elif defined(TARGET_NANOX)
    array_to_hexstr((char *) view.public_key, public_key, 32);
    array_to_hexstr((char *) view.public_key1, public_key, 16);
    array_to_hexstr((char *) view.public_key2, public_key+16, 16);
#endif
}
