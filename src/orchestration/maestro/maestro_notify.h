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
#ifndef MPT_MAESTRO_NOTIFY_H
#define MPT_MAESTRO_NOTIFY_H

#include <common/gru_status.h>

#include "maestro_topics.h"
#include "maestro_player_info.h"
#include "maestro_note.h"
#include "maestro_player.h"

#ifdef __cplusplus
extern "C" {
#endif

void maestro_notify_test_failed(gru_status_t *status);

#ifdef __cplusplus
}
#endif

#endif //MPT_MAESTRO_NOTIFY_H
