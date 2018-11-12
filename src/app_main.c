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

#include "app_main.h"
#include "view.h"
#include "validation.h"
#include "signature.h"

#include <os_io_seproxyhal.h>
#include <os.h>

#include <string.h>
#include "validation_parser.h"

uint8_t bip32_depth;
uint32_t bip32_path[10];
unsigned char public_key[32];

unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

unsigned char io_event(unsigned char channel) {
    switch (G_io_seproxyhal_spi_buffer[0]) {
    case SEPROXYHAL_TAG_FINGER_EVENT: //
        UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
        break;

    case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT: // for Nano S
        UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
        break;

    case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
        if (!UX_DISPLAYED())
            UX_DISPLAYED_EVENT();
        break;

    case SEPROXYHAL_TAG_TICKER_EVENT: { //
        UX_TICKER_EVENT(G_io_seproxyhal_spi_buffer, {
            if (UX_ALLOWED) {
                UX_REDISPLAY();
            }
        });
        break;
    }

        // unknown events are acknowledged
    default:UX_DEFAULT_EVENT();
        break;
    }
    if (!io_seproxyhal_spi_is_status_sent()) {
        io_seproxyhal_general_status();
    }
    return 1; // DO NOT reset the current APDU transport
}

unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len) {
    switch (channel & ~(IO_FLAGS)) {
    case CHANNEL_KEYBOARD:break;

        // multiplexed io exchange over a SPI channel and TLV encapsulated protocol
    case CHANNEL_SPI:
        if (tx_len) {
            io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);

            if (channel & IO_RESET_AFTER_REPLIED) {
                reset();
            }
            return 0; // nothing received from the master so far (it's a tx
            // transaction)
        } else {
            return io_seproxyhal_spi_recv(G_io_apdu_buffer,
                                          sizeof(G_io_apdu_buffer), 0);
        }

    default:THROW(INVALID_PARAMETER);
    }
    return 0;
}

void app_init() {
    io_seproxyhal_init();
    USB_power(0);
    USB_power(1);
    view_display_main_menu();
}

bool extractBip32(uint8_t *depth, uint32_t path[10], uint32_t rx, uint32_t offset) {
    if (rx < offset + 1) {
        return 0;
    }

    *depth = G_io_apdu_buffer[offset];
    const uint16_t req_offset = 4 * *depth + 1 + offset;

    if (rx < req_offset || *depth > 10) {
        return 0;
    }
    memcpy(path, G_io_apdu_buffer + offset + 1, *depth * 4);
    return 1;
}

void extractPubKey(unsigned char *outputBuffer, cx_ecfp_public_key_t *pubKey) {
    for (int i = 0; i < 32; i++) {
        outputBuffer[i] = pubKey->W[64 - i];
    }
    if ((pubKey->W[32] & 1) != 0) {
        outputBuffer[31] |= 0x80;
    }
}

bool process_chunk(volatile uint32_t *tx, uint32_t rx, bool getBip32) {
    int packageIndex = G_io_apdu_buffer[OFFSET_PCK_INDEX];
    int packageCount = G_io_apdu_buffer[OFFSET_PCK_COUNT];

    uint16_t offset = OFFSET_DATA;
    if (rx < offset) {
        THROW(APDU_CODE_DATA_INVALID);
    }

    if (packageIndex == 1) {
        validation_initialize();
        validation_reset();
        if (getBip32) {
            if (!extractBip32(&bip32_depth, bip32_path, rx, OFFSET_DATA)) {
                THROW(APDU_CODE_DATA_INVALID);
            }
            return packageIndex == packageCount;
        }
    }

//    if (validation_append(&(G_io_apdu_buffer[offset]), rx - offset) != NULL) {
//	    THROW(APDU_CODE_OUTPUT_BUFFER_TOO_SMALL);
//    }
    validation_append(&(G_io_apdu_buffer[offset]), rx - offset);

    return packageIndex == packageCount;
}

void extract_public_key()
{
    cx_ecfp_public_key_t publicKey;
    cx_ecfp_private_key_t privateKey;
    uint8_t privateKeyData[32];

    // Generate keys
    os_perso_derive_node_bip32(
            CX_CURVE_Ed25519,
            bip32_path,
            bip32_depth,
            privateKeyData,
            NULL);

    keys_ed25519(&publicKey, &privateKey, privateKeyData);
    memset(privateKeyData, 0, 32);

    extractPubKey(public_key, &publicKey);
}

void handleApdu(volatile uint32_t *flags, volatile uint32_t *tx, uint32_t rx) {
    uint16_t sw = 0;

    BEGIN_TRY
    {
        TRY
        {
            if (G_io_apdu_buffer[OFFSET_CLA] != CLA) {
                THROW(APDU_CODE_CLA_NOT_SUPPORTED);
            }

            if (rx < 5) {
                THROW(APDU_CODE_WRONG_LENGTH);
            }

            switch (G_io_apdu_buffer[OFFSET_INS]) {
            case INS_GET_VERSION: {
#ifdef TESTING_ENABLED
                G_io_apdu_buffer[0] = 0xFF;
#else
                G_io_apdu_buffer[0] = 0x55;
#endif
                G_io_apdu_buffer[1] = LEDGER_MAJOR_VERSION;
                G_io_apdu_buffer[2] = LEDGER_MINOR_VERSION;
                G_io_apdu_buffer[3] = LEDGER_PATCH_VERSION;
                *tx += 4;
                THROW(APDU_CODE_OK);
                break;
            }

            case INS_PUBLIC_KEY_ED25519: {
                if (!extractBip32(&bip32_depth, bip32_path, rx, OFFSET_DATA)) {
                    THROW(APDU_CODE_DATA_INVALID);
                }

                extract_public_key();

                os_memmove(G_io_apdu_buffer, public_key, sizeof(public_key));
                *tx += sizeof(public_key);

                THROW(APDU_CODE_OK);
            }

            case INS_SIGN_ED25519: {
                if (!process_chunk(tx, rx, true)) {
                    THROW(APDU_CODE_OK);
                }
                const char *error_msg = validation_parse();
                if (error_msg != NULL) {
                    int error_msg_length = strlen(error_msg);
                    os_memmove(G_io_apdu_buffer, error_msg, error_msg_length);
                    *tx += sizeof(error_msg_length);
                    THROW(APDU_CODE_DATA_INVALID);
                }

                char result = 0;
                int8_t msg_round = validation_parser_get_msg_round(
                        validation_get_parsed(),
                        (const char *) validation_get_buffer(),
                        &result);

                if (result != 0) {
                    THROW(APDU_CODE_DATA_INVALID);
                }

                int64_t height = validation_parser_get_msg_height(
                        validation_get_parsed(),
                        (const char *) validation_get_buffer(),
                        &result);

                if (result != 0) {
                    THROW(APDU_CODE_DATA_INVALID);
                }

                if (!validation_reference_get()->IsInitialized) {
                    view_set_msg_round(msg_round);
                    view_set_msg_height(height);
                    view_display_validation_init();
                    *flags |= IO_ASYNCH_REPLY;
                    THROW(APDU_CODE_DATA_INVALID);
                    break;
                }

                if ((msg_round > validation_reference_get()->CurrentMsgRound)
                    ||
                    ((msg_round == validation_reference_get()->CurrentMsgRound)
                     && height > validation_reference_get()->CurrentHeight)) {

                    validation_reference_get()->CurrentMsgRound = msg_round;
                    validation_reference_get()->CurrentHeight = height;
                    view_set_state(msg_round, height);

                    sign(tx);
                } else {
                    THROW(APDU_CODE_DATA_INVALID);
                }
            }
                break;

            default:THROW(APDU_CODE_INS_NOT_SUPPORTED);
            }
        }
        CATCH(EXCEPTION_IO_RESET)
        {
            THROW(EXCEPTION_IO_RESET);
        }
        CATCH_OTHER(e)
        {
            switch (e & 0xF000) {
            case 0x6000:
            case APDU_CODE_OK:sw = e;
                break;
            default:sw = 0x6800 | (e & 0x7FF);
                break;
            }
            G_io_apdu_buffer[*tx] = sw >> 8;
            G_io_apdu_buffer[*tx + 1] = sw;
            *tx += 2;
        }
        FINALLY
        {
        }
    }
    END_TRY;
}

void reject_reference() {
    set_code(G_io_apdu_buffer, 0, APDU_CODE_COMMAND_NOT_ALLOWED);
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
    view_display_main_menu();
}

void accept_reference(int8_t msg_round, int64_t height) {
    validation_reference_get()->CurrentHeight = height;
    validation_reference_get()->CurrentMsgRound = msg_round;
    validation_reference_get()->IsInitialized = 1;
    view_set_public_key("050b52687662f8ba73ed3f618a4d91c0d19d4a9ca5b966aa71f9523bc7d21f04");

    set_code(G_io_apdu_buffer, 0, APDU_CODE_OK);
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
    view_display_validation_processing();
}

void sign(volatile uint32_t *tx)
{
    // Generate keys
    cx_ecfp_public_key_t publicKey;
    cx_ecfp_private_key_t privateKey;
    uint8_t privateKeyData[32];

    unsigned int length = 0;

    os_perso_derive_node_bip32(
            CX_CURVE_Ed25519,
            bip32_path, bip32_depth,
            privateKeyData, NULL);

    keys_ed25519(&publicKey, &privateKey, privateKeyData);
    memset(privateKeyData, 0, 32);

    sign_ed25519(
            validation_get_buffer(),
            validation_get_buffer_length(),
            G_io_apdu_buffer,
            IO_APDU_BUFFER_SIZE,
            &length,
            &privateKey);

    *tx += length;
    THROW(APDU_CODE_OK);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
void app_main() {
    volatile uint32_t rx = 0, tx = 0, flags = 0;

    validation_reference_reset();
    view_set_validation_reset_eh(&validation_reference_reset);
    view_set_accept_eh(&accept_reference);
    view_set_reject_eh(&reject_reference);

    for (;;) {
        volatile uint16_t sw = 0;

        BEGIN_TRY;
        {
            TRY;
            {
                rx = tx;
                tx = 0;
                rx = io_exchange(CHANNEL_APDU | flags, rx);
                flags = 0;

                if (rx == 0)
                    THROW(APDU_CODE_EMPTY_BUFFER);

                handleApdu(&flags, &tx, rx);
            }
            CATCH_OTHER(e);
            {
                switch (e & 0xF000) {
                case 0x6000:
                case 0x9000:sw = e;
                    break;
                default:sw = 0x6800 | (e & 0x7FF);
                    break;
                }
                G_io_apdu_buffer[tx] = sw >> 8;
                G_io_apdu_buffer[tx + 1] = sw;
                tx += 2;
            }
            FINALLY;
            {}
        }
        END_TRY;
    }
}
#pragma clang diagnostic pop

