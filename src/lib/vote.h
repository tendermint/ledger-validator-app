/*******************************************************************************
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

#include <inttypes.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TYPE_PREVOTE        0x01
#define TYPE_PRECOMMIT      0x02
#define TYPE_PROPOSAL       0x20

typedef struct {
    uint8_t Type;
    int64_t Height;
    int8_t Round;
} vote_t;

typedef struct {
  int8_t isInitialized;
  vote_t vote;
} vote_state_t;

vote_state_t *vote_state_get();

vote_t *vote_get();

#ifdef __cplusplus
}
#endif
