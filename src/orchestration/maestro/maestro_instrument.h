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
#ifndef MAESTRO_INSTRUMENT_H
#define MAESTRO_INSTRUMENT_H

#include <common/gru_alloc.h>
#include <common/gru_status.h>
#include <string/gru_alt.h>

#include "maestro_note.h"
#include "maestro_player_info.h"

typedef void *(*maestro_play_t)(const maestro_note_t *request,
	maestro_note_t *response,
	const maestro_player_info_t *pinfo);

typedef struct maestro_instrument_t_ {
	maestro_note_t tessitura; /** What it can play */
	maestro_play_t play;
} maestro_instrument_t;

maestro_instrument_t *
	maestro_instrument_new(maestro_command_t cmd, maestro_play_t play, gru_status_t *status);
void maestro_instrument_destroy(maestro_instrument_t **ptr);
bool maestro_instrument_can_play(const maestro_instrument_t *instrument,
	const maestro_note_t *note);

static inline void maestro_instrument_destroy_wrapper(void **ptr) {
	maestro_instrument_destroy((maestro_instrument_t **) ptr);
}

#endif /* MAESTRO_INSTRUMENT_H */
