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

#include "vote_parser.h"
#include "vote_buffer.h"

#define WIRE_TYPE_VARINT   0
#define WIRE_TYPE_64BIT    1
#define WIRE_TYPE_LEN      2
#define WIRE_TYPE_32BIT    5

#define FIELD_ZERO     0
#define FIELD_TYPE     1
#define FIELD_HEIGHT   2
#define FIELD_ROUND    3
#define FIELD_UNKNOWN  4

#define FIELD_NUM(x) ((uint8_t)((x) >> 3u))
#define WIRE_TYPE(x) ((uint8_t)((x) & 0x7u))

int64_t decode_amino_64bits(const uint8_t *p) {
    int64_t v = 0;
    p += 7;
    for (int8_t i = 0; i < 8; i++, p--) {
        v <<= 8;
        v += *p;
    }
    return v;
}

parse_error_t get_varint(const uint8_t *buffer,
                         size_t buf_size,
                         size_t *value,
                         uint32_t pos_start,
                         uint32_t *pos_end) {
    size_t shift = 0;
    *value = 0;
    *pos_end = pos_start;

    while (*pos_end < buf_size) {
        uint8_t b = buffer[*pos_end];
        *value += (b & 0x7Fu) << shift;
        (*pos_end)++;
        if (!(b & 0x80u))
            break;
        shift += 7;
    }

    if (*pos_end < buf_size) {
        return parse_ok;
    }

    return parse_unexpected_buffer_end;
}

parse_error_t vote_amino_parse(const uint8_t *buffer, size_t size, vote_t *vote) {
    uint32_t pos = 0;
    uint8_t expected_field = FIELD_TYPE;

    // Initialize vote values
    vote->Type = 0;
    vote->Round = 0;
    vote->Height = 0;

    if (size < 2) {
        return parse_unexpected_buffer_end;
    }

    size_t message_length;
    parse_error_t err = get_varint(buffer, size, &message_length, pos, &pos);
    if (err != parse_ok) {
        return err;
    }

    if (message_length+1 != size) {
        return parse_unexpected_buffer_size;
    }

    while (pos < size) {
        const uint8_t field_num = FIELD_NUM(buffer[pos]);
        const uint8_t wire_type = WIRE_TYPE(buffer[pos]);

        switch (field_num) {
            case FIELD_ZERO: {
                return parse_unexpected_field;
            }

            case FIELD_TYPE: {
                if (expected_field < FIELD_TYPE) {
                    return parse_unexpected_field;
                }
                if (wire_type != WIRE_TYPE_VARINT) {
                    return parse_unexpected_wire_type;
                }
                if (size < pos + 2) {
                    return parse_unexpected_buffer_end;
                }
                pos++;
                vote->Type = buffer[pos];
                pos += 1;
                expected_field = FIELD_HEIGHT;
                break;
            }

            case FIELD_HEIGHT: {
                if (expected_field < FIELD_TYPE) {
                    return parse_unexpected_field;
                }
                if (wire_type != WIRE_TYPE_64BIT) {
                    return parse_unexpected_wire_type;
                }
                if (size < pos + 9) {
                    return parse_unexpected_buffer_end;
                }
                pos++;
                vote->Height = decode_amino_64bits(buffer + pos);
                pos += 8;
                expected_field = FIELD_ROUND;
                break;
            }

            case FIELD_ROUND: {
                if (expected_field < FIELD_TYPE) {
                    return parse_unexpected_field;
                }
                if (wire_type != WIRE_TYPE_64BIT) {
                    return parse_unexpected_wire_type;
                }
                if (size < pos + 9) {
                    return parse_unexpected_buffer_end;
                }
                pos++;
                int64_t tmp = decode_amino_64bits(buffer + pos);
                if (tmp < 0) {
                    return parse_unexpected_round_value;
                }
                if (tmp > 255) {
                    return parse_unexpected_round_value;
                }
                vote->Round = (uint8_t) tmp;
                pos += 8;
                expected_field = FIELD_UNKNOWN;
                break;

                default: {
                    if (size > 10000) {
                        return parse_unexpected_buffer_size;
                    }
                    pos = (uint32_t) size;
                    break;
                }
            }
        }
        // NOTE: Other fields are not parsed. In particular BlockID
    }

    // Validate values
    switch (vote->Type) {
        case TYPE_PREVOTE:
        case TYPE_PRECOMMIT:
        case TYPE_PROPOSAL:
            break;
        default:
            return parse_unexpected_type_value;
    }
    if (vote->Height < 0) {
        return parse_unexpected_height_value;
    }
    if (vote->Round < 0) {
        return parse_unexpected_height_value;
    }

    // NOTE: for proposal POLRound is not parsed or verified

    return parse_ok;
}

parse_error_t vote_parse() {
    const uint8_t *vote_buffer = vote_get_buffer();
    const uint32_t vote_buffer_length = vote_get_buffer_length();

    parse_error_t error = vote_amino_parse(vote_buffer, vote_buffer_length, &vote);

    return error;
}
