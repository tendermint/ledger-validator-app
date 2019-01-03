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
#include "vote.h"


#define MAX_CHARS_PER_KEY_LINE      64
#define MAX_CHARS_PER_VALUE_LINE    128
#define MAX_SCREEN_LINE_WIDTH       60

enum UI_STATE {
    UI_MAIN_MENU,
    UI_VOTE_INIT,
    UI_VOTE_PROCESSING
};

extern enum UI_STATE view_uiState;

//------ Delegates definitions
typedef void (*delegate_accept_vote_state_signature)(vote_t *vote);

typedef void (*delegate_reject_vote_state_signature)();

typedef void (*delegate_vote_reset)();

//------ Event handlers
/// Set validation reset event handler
void view_set_vote_reset_eh(delegate_vote_reset delegate);

/// Set accept reference signature event handler
void view_set_accept_eh(delegate_accept_vote_state_signature delegate);

/// Set reject reference signature event handler
void view_set_reject_eh(delegate_reject_vote_state_signature delegate);

//------ Common functions
/// view_init
void view_init();

/// view_mainmenu
void view_display_main_menu();

void view_set_state(vote_state_t *s, uint8_t public_key[32]);

void view_set_msg(vote_t *v);

void view_display_vote_init();

void view_display_vote_processing();
