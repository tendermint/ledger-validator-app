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

#ifdef __cplusplus
extern "C" {
#endif

#include "vote.h"

#include <stdint.h>

typedef enum {
    parse_ok = 0,
    parse_unexpected_buffer_end = 1,
    parse_unexpected_field = 2,
    parse_unexpected_wire_type = 3,
    parse_unexpected_type_value = 4,
    parse_unexpected_height_value = 5,
    parse_unexpected_round_value = 6,
    parse_unexpected_buffer_size = 7,
    parse_unexpected_varint_len = 8,
} parse_error_t;

parse_error_t get_varint(const uint8_t *buffer, size_t buf_size, size_t *value, uint32_t start, uint32_t *pos_end);

parse_error_t vote_amino_parse(const uint8_t *buffer, size_t size, vote_t *vote);

/// Parse vote in buffer
/// This function should be called as soon as full buffer data is loaded.
/// \return It an error core or PARSE_OK
parse_error_t vote_parse();

#ifdef __cplusplus
}
#endif
