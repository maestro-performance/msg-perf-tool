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

#include <common/gru_status.h>
#include <common/gru_alloc.h>

#include "maestro_note.h"

typedef void *(*maestro_play_t)(maestro_note_t *note, gru_status_t *status);

typedef struct maestro_instrument_t_ {
	maestro_note_t tessitura; /** What it can play */
	maestro_play_t play;
} maestro_instrument_t; 

maestro_instrument_t *maestro_instrument_new(const char *note, maestro_play_t play, 
	gru_status_t *status);


#endif /* MAESTRO_INSTRUMENT_H */
