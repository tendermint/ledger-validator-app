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

#include <stdint.h>
#include <os_io_seproxyhal.h>
#include <os.h>
#include <cx.h>

extern const uint8_t bip32_depth;
extern uint32_t bip32_path[5];

extern unsigned char public_key[32];
extern cx_ecfp_private_key_t cx_privateKey;
extern uint8_t keys_initialized;


void action_reset();
void action_reject();
void action_accept();
int action_sign();
void actions_getkeys();
