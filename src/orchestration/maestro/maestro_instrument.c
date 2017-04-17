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
#include "maestro_instrument.h"

maestro_instrument_t *maestro_instrument_new(const char *note, maestro_play_t play, 
	gru_status_t *status) 
{
	maestro_instrument_t *ret = gru_alloc(sizeof(maestro_instrument_t), status);
	gru_alloc_check(ret, NULL);

	ret->play = play;

	strlcpy(ret->tessitura.command, note, sizeof(ret->tessitura.command));
	ret->tessitura.payload = NULL;
	
	return ret;
}
