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

#include "actions.h"
#include "vote.h"
#include "signature.h"
#include "lib/vote_buffer.h"
#include "apdu_codes.h"
#include <zxmacros.h>

const uint8_t bip32_depth = 5;
uint32_t bip32_path[5];

unsigned char public_key[32];
cx_ecfp_private_key_t cx_privateKey;
uint8_t keys_initialized = 0;

void action_reset() {
    vote_state_reset();
}

void action_reject() {
    set_code(G_io_apdu_buffer, 0, APDU_CODE_COMMAND_NOT_ALLOWED);
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
}

void action_accept() {
    vote_state.vote.Type = vote.Type;
    vote_state.vote.Height = vote.Height;
    vote_state.vote.Round = vote.Round;
    vote_state.isInitialized = 1;

    int tx = action_sign();

    set_code(G_io_apdu_buffer + tx, 0, APDU_CODE_OK);
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx + 2);
}

void format_pubkey(unsigned char *outputBuffer, cx_ecfp_public_key_t *pubKey) {
    for (int i = 0; i < 32; i++) {
        outputBuffer[i] = pubKey->W[64 - i];
    }

    if ((pubKey->W[32] & 1) != 0) {
        outputBuffer[31] |= 0x80;
    }
}

int action_sign() {
    uint8_t *signature = G_io_apdu_buffer;
    unsigned int signature_capacity = IO_APDU_BUFFER_SIZE - 2;
    unsigned int info = 0;

    const uint8_t *data = vote_get_buffer();
    const uint32_t data_len = vote_get_buffer_length();

    int ret = cx_eddsa_sign(&cx_privateKey,
                            CX_LAST,
                            CX_SHA512,
                            data,
                            data_len,
                            NULL,
                            0,
                            signature,
                            signature_capacity,
                            &info);
    return ret;
}

void actions_getkeys() {
    cx_ecfp_public_key_t cx_publicKey;
    uint8_t privateKeyData[32];

    bip32_path[0] = 0x80000000 | 44;
    bip32_path[1] = 0x80000000 | 118;
    bip32_path[2] = 0x80000000 | 0;
    bip32_path[3] = 0x00000000 | 0;
    bip32_path[4] = 0x00000000 | 0;

    // Generate keys
    os_perso_derive_node_bip32_seed_key(
            HDW_NORMAL,
            CX_CURVE_Ed25519,
            bip32_path,
            bip32_depth,
            privateKeyData,
            NULL,
            NULL,
            0);

    keys_ed25519(&cx_publicKey, &cx_privateKey, privateKeyData);
    explicit_bzero(privateKeyData, 32);

    format_pubkey(public_key, &cx_publicKey);
    keys_initialized = 1;
}
