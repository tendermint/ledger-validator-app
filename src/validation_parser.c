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

#include <jsmn.h>
#include <stdio.h>
#include "validation_parser.h"
#include "json_parser.h"
#include <limits.h>
#include <zxmacros.h>

//---------------------------------------------

const char whitespaces[] = {
        0x20,// space ' '
        0x0c, // form_feed '\f'
        0x0a, // line_feed, '\n'
        0x0d, // carriage_return, '\r'
        0x09, // horizontal_tab, '\t'
        0x0b // vertical_tab, '\v'
};

//---------------------------------------------

int msgs_total_pages = 0;
int msgs_array_elements = 0;

//---------------------------------------------

copy_delegate copy_fct = NULL;
parsing_context_t parsing_context;

void set_copy_delegate(copy_delegate delegate) {
    copy_fct = delegate;
}

void set_parsing_context(parsing_context_t context) {
    parsing_context = context;
}

int is_space(char c)
{
    for (unsigned int i = 0;i < sizeof(whitespaces); i++) {
        if (whitespaces[i] == c) {
            return 1;
        }
    }
    return 0;
}

int contains_whitespace(parsed_json_t* parsed_transaction,
                        const char *transaction) {

    int start = 0;
    int last_element_index = parsed_transaction->Tokens[0].end;

    // Starting at token 1 because token 0 contains full transaction
    for (int i = 1; i < parsed_transaction->NumberOfTokens; i++) {
        if (parsed_transaction->Tokens[i].type != JSMN_UNDEFINED) {
            int end = parsed_transaction->Tokens[i].start;
            for (int j = start; j < end; j++) {
                if (is_space(transaction[j]) == 1) {
                    return 1;
                }
            }
            start = parsed_transaction->Tokens[i].end + 1;
        }
        else {
            return 0;
        }
    }
    while (start <= last_element_index && transaction[start] != '\0') {
        if (is_space(transaction[start]) == 1) {
            return 1;
        }
        start++;
    }
    return 0;
}

int is_sorted(int first_index,
              int second_index,
              parsed_json_t* parsed_transaction,
              const char* transaction)
{
#if DEBUG_SORTING
    char first[256];
    char second[256];

    int size =  parsed_transaction->Tokens[first_index].end - parsed_transaction->Tokens[first_index].start;
    strncpy(first, transaction + parsed_transaction->Tokens[first_index].start, size);
    first[size] = '\0';
    size =  parsed_transaction->Tokens[second_index].end - parsed_transaction->Tokens[second_index].start;
    strncpy(second, transaction + parsed_transaction->Tokens[second_index].start, size);
    second[size] = '\0';
#endif

    if (strcmp(
            transaction + parsed_transaction->Tokens[first_index].start,
            transaction + parsed_transaction->Tokens[second_index].start) <= 0) {
        return 1;
    }
    return 0;
}

int dictionaries_sorted(parsed_json_t* parsed_transaction,
                        const char* transaction)
{
    for (int i = 0; i < parsed_transaction->NumberOfTokens; i++) {
        if (parsed_transaction->Tokens[i].type == JSMN_OBJECT) {

            int count = object_get_element_count(i, parsed_transaction);
            if (count > 1) {
                int prev_token_index = object_get_nth_key(i, 0, parsed_transaction);
                for (int j = 1; j < count; j++) {
                    int next_token_index = object_get_nth_key(i, j, parsed_transaction);
                    if (!is_sorted(prev_token_index, next_token_index, parsed_transaction, transaction)) {
                        return 0;
                    }
                    prev_token_index = next_token_index;
                }
            }
        }
    }
    return 1;
}

const char* json_validate(parsed_json_t* parsed_json,
                          const char* raw_json) {

    if (contains_whitespace(parsed_json, raw_json) == 1) {
        return "Contains whitespace in the corpus";
    }

    if (dictionaries_sorted(parsed_json, raw_json) != 1) {
        return "Dictionaries are not sorted";
    }

    int round_token_index = object_get_value(0,
                         "round",
                         parsed_json,
                         raw_json);
    if (round_token_index == -1) {
        return "Missing round";
    }

    int height_token_index = object_get_value(0,
                         "height",
                         parsed_json,
                         raw_json);
    if (height_token_index == -1) {
        return "Missing height";
    }

    if (object_get_value(0,
                         "other",
                         parsed_json,
                         raw_json) == -1) {
        return "Missing other";
    }

    char result = 0;
    validation_parser_get_msg_height(parsed_json, raw_json, &result);
    if (result != 0) {
        return "Could not parse height";
    }

    validation_parser_get_msg_round(parsed_json, raw_json, &result);
    if (result != 0) {
        return "Could not parse round";
    }

    return NULL;
}

int64_t validation_parser_get_msg_height(
    parsed_json_t *parsed_json,
    const char *raw_json,
    char *result) {
    int token_index = object_get_value(0, "height", parsed_json, raw_json);

    return str_to_int64(
            raw_json + parsed_json->Tokens[token_index].start,
            raw_json + parsed_json->Tokens[token_index].end,
            result);
}

int8_t validation_parser_get_msg_round(
    parsed_json_t *parsed_json,
    const char *raw_json,
    char *result)
{
    int token_index = object_get_value(0, "round", parsed_json, raw_json);

    return str_to_int8(
            raw_json + parsed_json->Tokens[token_index].start,
            raw_json + parsed_json->Tokens[token_index].end,
            result);
}