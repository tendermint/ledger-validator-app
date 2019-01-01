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
#include "os.h"

#include "../view.h"
#include "apdu_codes.h"
#include "buffering.h"

// Ram
#define RAM_BUFFER_SIZE 512
uint8_t ram_buffer[RAM_BUFFER_SIZE];

// Flash
#define FLASH_BUFFER_SIZE 10000
typedef struct {
    uint8_t buffer[FLASH_BUFFER_SIZE];
} storage_t;

storage_t N_appdata_impl __attribute__ ((aligned(64)));
#define N_appdata (*(storage_t *)PIC(&N_appdata_impl))

vote_state_t vote_state;
vote_t vote;

void update_ram(buffer_state_t *buffer, uint8_t *data, int size) {
    os_memmove(buffer->data + buffer->pos, data, size);
}

void update_flash(buffer_state_t *buffer, uint8_t *data, int size) {
    nvm_write((void *) buffer->data + buffer->pos, data, size);
}

void vote_state_reset() {
    os_memset(&vote_state, 0, sizeof(vote_state_t));
}

vote_state_t *vote_state_get() {
    return &vote_state;
}

void vote_initialize() {
    append_buffer_delegate update_ram_delegate = &update_ram;
    append_buffer_delegate update_flash_delegate = &update_flash;

    buffering_init(ram_buffer,
                   sizeof(ram_buffer),
                   update_ram_delegate,
                   N_appdata.buffer,
                   sizeof(N_appdata.buffer),
                   update_flash_delegate
    );
}

void vote_reset() {
    buffering_reset();
}

uint32_t vote_append(unsigned char *buffer, uint32_t length) {
    return buffering_append(buffer, length);
}

uint32_t vote_get_buffer_length() {
    return buffering_get_buffer()->pos;
}

const uint8_t *vote_get_buffer() {
    return buffering_get_buffer()->data;
}

parse_error_t vote_parse() {
    const uint8_t *vote_buffer = vote_get_buffer();
    const uint32_t vote_buffer_length = vote_get_buffer_length();

    parse_error_t error = vote_amino_parse(vote_buffer, vote_buffer_length, &vote);

    return error;
}

vote_t *vote_get() {
    return &vote;
}
