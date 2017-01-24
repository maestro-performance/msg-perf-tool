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
#ifndef NET_PROBE_H
#define NET_PROBE_H

#include <common/gru_status.h>
#include <io/gru_ioutils.h>
#include <log/gru_logger.h>

#include "../probe.h"

#ifdef __cplusplus
extern "C" {
#endif

probe_entry_t *net_entry(gru_status_t *status);

bool net_init(const options_t *options, gru_status_t *status);
int net_collect(gru_status_t *status);
void net_stop();
const char *net_name();

#ifdef __cplusplus
}
#endif

#endif /* NET_PROBE_H */

