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

#include "vote_fsm.h"

uint8_t validate_state_transition() {

    if (!vote_state.isInitialized) {
        return 0;
    }

    // Based on https://github.com/tendermint/tendermint/pull/3061/files#diff-31241946f48e67f759076b664f3d2745

    vote_t *s = &vote_state.vote;
    vote_t *v = &vote;

    // This applies to all messages. If monotonically increasing, then accept
    if (v->Height > s->Height ||
        (v->Height == s->Height && v->Round > s->Round)) {
        return 1;
    }

    // If exactly same height/round, then need to be in type sequence
    // (TYPE_PROPOSAL, TYPE_PREVOTE, TYPE_PRECOMMIT)
    if (v->Height == s->Height && v->Round == s->Round) {
        switch (v->Type) {
            case TYPE_PREVOTE: {
                return (uint8_t) (s->Type == TYPE_PROPOSAL);
            }

            case TYPE_PRECOMMIT: {
                return (uint8_t) (s->Type != TYPE_PRECOMMIT);
            }
            default:{
                break;
            }
        }

        return 0;
    }

    return 0;
}

uint8_t try_state_transition() {
    if (!validate_state_transition()) {
        return 0;
    }

    vote_state.vote.Type = vote.Type;
    vote_state.vote.Round = vote.Round;
    vote_state.vote.Height = vote.Height;

    return 1;
}
