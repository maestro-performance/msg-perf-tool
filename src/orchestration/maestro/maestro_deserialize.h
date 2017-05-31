/**
 *    Copyright 2017 Otavio Rodolfo Piske
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#ifndef MPT_MAESTRO_DESERIALIZE_H
#define MPT_MAESTRO_DESERIALIZE_H

#include <msgpack.h>

#include "maestro_note.h"
#include "msg_content_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Deserialize a note
 * @param in data to parse
 * @param note output note
 * @param status status structure in case of error
 * @return true if successfully parsed or false otherwise
 */
bool maestro_deserialize_note(const msg_content_data_t *in, maestro_note_t *note,
							  gru_status_t *status);

#ifdef __cplusplus
}
#endif


#endif //MPT_MAESTRO_DESERIALIZE_H
