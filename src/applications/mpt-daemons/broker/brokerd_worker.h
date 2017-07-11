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
#ifndef BROKERD_WORKER_H
#define BROKERD_WORKER_H

#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include <common/gru_status.h>

#include <context/bmic_context.h>

#include "bmic/bmic_utils.h"
#include "bmic/bmic_writer.h"
#include "contrib/options.h"
#include "maestro/maestro_player.h"
#include "maestro/maestro_player.h"
#include "maestro/maestro_sheet.h"
#include "maestro/maestro_topics.h"
#include "msg_content_data.h"
#include "msgctxt.h"
#include "process_utils.h"
#include "statistics/calculator.h"
#include "statistics/csv_writer.h"
#include "statistics/stats_types.h"
#include "statistics/stats_writer.h"
#include "vmsl.h"

#include "worker_options.h"
#include "worker_types.h"
#include "worker_utils.h"

#include "daemon_common.h"

#define MPT_BROKERD_COLLECT_WAIT_TIME 10

#ifdef __cplusplus
extern "C" {
#endif

int brokerd_worker_start(const options_t *options);

#ifdef __cplusplus
}
#endif

#endif /* BROKERD_WORKER_H */
