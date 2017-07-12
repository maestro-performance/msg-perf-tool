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

#ifndef MAESTRO_SHEET_H
#define MAESTRO_SHEET_H

#include <collection/gru_list.h>
#include <common/gru_status.h>
#include <log/gru_logger.h>
#include <vmsl.h>

#include "mpt-debug.h"
#include "msg_content_data.h"

#include "maestro_debug.h"
#include "maestro_instrument.h"
#include "maestro_note.h"
#include "maestro_serialize.h"
#include "maestro_deserialize.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct maestro_sheet_t_ {
  	vmsl_mtopic_spec_t location;
	gru_list_t *instruments; /** A list of instruments */
} maestro_sheet_t;

maestro_sheet_t *maestro_sheet_new(gru_status_t *status);
void maestro_sheet_destroy(maestro_sheet_t **ptr);

void maestro_sheet_set_location(maestro_sheet_t *sheet, int count, char **topics, int qos);

void maestro_sheet_add_instrument(maestro_sheet_t *sheet,
	maestro_instrument_t *instrument);

void maestro_sheet_play(const maestro_sheet_t *sheet,
	const maestro_player_info_t *pinfo,
	const msg_content_data_t *cont,
	msg_content_data_t *resp,
	gru_status_t *status);

#ifdef __cplusplus
}
#endif

#endif /* MAESTRO_SHEET_H */
