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
#pragma once

#include "os.h"
#include "cx.h"

#define MAX_CHARS_PER_KEY_LINE      64
#define MAX_CHARS_PER_VALUE_LINE    128
#define MAX_SCREEN_LINE_WIDTH       60

enum UI_STATE {
    UI_MAIN_MENU,
    UI_VALIDAITON_INIT,
    UI_VALIDATION_PROCESSING
};

extern enum UI_STATE view_uiState;

//------ Delegates definitions
typedef void (*delegate_accept_reference_signature)(int8_t msg_round, int64_t height);
typedef void (*delegate_reject_reference_signature)();
typedef void (*delegate_validation_reset)();

//------ Event handlers
/// Set validation reset event handler
void view_set_validation_reset_eh(delegate_validation_reset delegate);

/// Set accept reference signature event handler
void view_set_accept_eh(delegate_accept_reference_signature delegate);

/// Set reject reference signature event handler
void view_set_reject_eh(delegate_reject_reference_signature delegate);

//------ Common functions
void view_init();
void view_display_main_menu();

void view_set_state(int8_t msg_round, int64_t msg_height);
void view_set_msg_height(int64_t height);
void view_set_msg_round(int8_t msg_round);
void view_set_public_key(const char* publicKey);

void view_display_validation_init();
void view_display_validation_processing();
