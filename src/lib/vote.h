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
#include "vote_parser.h"

typedef struct {
  int8_t isInitialized;
  vote_t vote;
} vote_reference_t;

void vote_reference_reset();

vote_reference_t* vote_reference_get();

// Initializes vote context
void vote_initialize();

/// Clears the vote buffer
void vote_reset();

/// Appends buffer to the end of the current transaction buffer
/// Transaction buffer will grow until it reaches the maximum allowed size
/// \param buffer
/// \param length
/// \return number of appended bytes
uint32_t vote_append(unsigned char *buffer, uint32_t length);

/// Returns size of the raw buffer
/// \return
uint32_t vote_get_buffer_length();

/// Returns the raw buffer
/// \return
const uint8_t* vote_get_buffer();

/// Parse vote in buffer
/// This function should be called as soon as full buffer data is loaded.
/// \return It an error core or PARSE_OK
parse_error_t vote_parse();

/// Returns parsed vote
/// \return
vote_t* vote_get();
