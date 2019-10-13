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

#include "vote.h"
#include "buffering.h"
#include "vote_buffer.h"
#include "vote_parser.h"
#include <zxmacros.h>

#include <string.h>

vote_state_t vote_state;
vote_t vote;

void vote_state_reset() {
    explicit_bzero(&vote_state, sizeof(vote_state_t));
}

void vote_reset() {
    buffering_reset();
}
