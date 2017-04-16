/**
 Copyright 2017 Otavio Rodolfo Piske

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
#ifndef RECEIVERD_MAIN_H
#define RECEIVERD_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/wait.h>

#include <cli/gru_cli_opt.h>
#include <config/gru_config.h>
#include <log/gru_logger.h>
#include <time/gru_time_utils.h>

#include "contrib/options.h"
#include "process_utils.h"

#include "receiverd_worker.h"
#include "statistics.h"
#include "vmsl.h"
#include "vmsl_assign.h"

#include "process_utils.h"

#ifdef __cplusplus
}
#endif

#endif /* RECEIVERD_MAIN_H */