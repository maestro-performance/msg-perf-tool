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
#ifndef TUNE_WORKER_H
#define TUNE_WORKER_H

#include <inttypes.h>
#include <signal.h>
#include <stdarg.h>
#include <unistd.h>

#include <common/gru_colors.h>
#include <common/gru_status.h>
#include <network/gru_uri.h>

#include <management/common/bmic_complements_java.h>
#include <management/common/bmic_queue_stat.h>

#include <context/bmic_context.h>

#include "bmic/bmic_utils.h"
#include "contrib/options.h"
#include "msg_content_data.h"
#include "msgctxt.h"
#include "vmsl.h"

#include "worker_options.h"
#include "worker_types.h"
#include "worker_utils.h"
#include "worker_manager.h"
#include "naive_sender.h"

#include "strategies/payload/pl_assign.h"

#ifdef __cplusplus
extern "C" {
#endif

int tune_worker_start(const vmsl_t *vmsl, const options_t *options);

#ifdef __cplusplus
}
#endif

#endif /* TUNE_WORKER_H */
