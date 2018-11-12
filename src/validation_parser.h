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

#ifndef CI_TEST_VALIDATIONPARSER_H
#define CI_TEST_VALIDATIONPARSER_H

#include "json_parser.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Extract height number from the json
/// \param parsed_json, parsed json with validation message
/// \param raw_json, raw json with validation message
/// \return height number
int64_t  validation_parser_get_msg_height(
    parsed_json_t *parsed_json,
    const char *raw_json,
    char *result);

/// Extract round number from the json
/// \param parsed_json, parsed json with validation message
/// \param raw_json, raw json with validation message
/// \return round number
int8_t validation_parser_get_msg_round(
    parsed_json_t *parsed_json,
    const char *raw_json,
    char *result);

/// Validate validation json
/// \param parsed_json, parsed json with validation message
/// \param raw_json, raw json with validation message
/// \return error code
const char* json_validate(
        parsed_json_t* parsed_json,
        const char* raw_json);

typedef struct {
    int64_t CurrentHeight;
    int8_t  IsInitialized;
    int8_t  CurrentMsgRound;
} validation_reference_t;

//---------------------------------------------
// Delegates

typedef void(*copy_delegate)(void *dst, const void *source, size_t size);
void set_copy_delegate(copy_delegate delegate);
void set_parsing_context(parsing_context_t context);

//---------------------------------------------


#ifdef __cplusplus
}
#endif
#endif //CI_TEST_VALIDATIONPARSER_H
