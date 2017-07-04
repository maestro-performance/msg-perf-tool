/**
 Copyright 2016 Otavio Rodolfo Piske

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */
#ifndef RECEIVER_WORKER_H
#define RECEIVER_WORKER_H

#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/wait.h>

#include <common/gru_status.h>

#include "contrib/options.h"
#include "msg_content_data.h"
#include "msgctxt.h"
#include "process_utils.h"
#include "vmsl.h"

#include "maestro/maestro_player.h"
#include "maestro/maestro_sheet.h"
#include "maestro/maestro_topics.h"
#include "statistics/calculator.h"
#include "statistics/csv_writer.h"
#include "statistics/nop_writer.h"
#include "statistics/out_writer.h"
#include "statistics/stats_types.h"
#include "statistics/stats_writer.h"

#include "abstract_worker.h"
#include "worker_options.h"
#include "worker_types.h"
#include "worker_utils.h"
#include "worker_wait.h"
#include "worker_info.h"
#include "worker_manager.h"
#include "naive_receiver.h"

#ifdef __cplusplus
extern "C" {
#endif

int receiver_start(const vmsl_t *vmsl, const options_t *options);

#ifdef __cplusplus
}
#endif

#endif /* RECEIVER_WORKER_H */
