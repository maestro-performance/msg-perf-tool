/**
 *   Copyright 2017 Otavio Rodolfo Piske
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#ifndef BMIC_PROBE_H
#define BMIC_PROBE_H

#include <common/gru_status.h>
#include <io/gru_ioutils.h>
#include <log/gru_logger.h>

#include <context/bmic_context.h>

#include "probes/probe.h"

#include "bmic/bmic_utils.h"
#include "bmic/bmic_writer.h"

#ifdef __cplusplus
extern "C" {
#endif

probe_entry_t *bmic_entry(gru_status_t *status);

bool bmic_init(const options_t *options, gru_status_t *status);
int bmic_collect(gru_status_t *status);
void bmic_stop();
const char *bmic_name();


#ifdef __cplusplus
}
#endif

#endif /* BMIC_PROBE_H */
