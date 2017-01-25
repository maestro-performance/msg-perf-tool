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
#ifndef SENDER_TUNE_MAIN_H
#define SENDER_TUNE_MAIN_H

#include <sys/wait.h>
#include <unistd.h>

#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/wait.h>

#include <cli/gru_cli_opt.h>
#include <time/gru_time_utils.h>

#include "contrib/options.h"

#include <common/gru_base.h>
#include <config/gru_config.h>
#include <log/gru_logger.h>

#include "config.h"
#include "process_utils.h"
#include "vmsl.h"
#include "vmsl_assign.h"

#include "tune_sender.h"

#ifdef __cplusplus
extern "C" {
#endif

int tune_main(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif /* SENDER_TUNE_MAIN_H */
