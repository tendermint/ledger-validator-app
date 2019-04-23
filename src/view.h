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

/// view_init
void view_init();

/// view_idle
void view_idle();

void view_set_data();

void view_set_pk(uint8_t public_key[32]);

void view_display_vote_init();

void view_display_vote_processing();
