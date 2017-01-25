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
#ifndef VMSL_INIT_H
#define VMSL_INIT_H

#include <stdbool.h>

#include <network/gru_uri.h>

#include "vmsl.h"

#ifdef __cplusplus
extern "C" {
#endif

#define vmsl_assign_none(name, proto)                                                    \
	static bool name(vmsl_t *vmsl) {                                                     \
		logger_t logger = gru_logger_get();                                              \
		logger(ERROR, "" proto " protocol support was not enabled");                     \
		return false;                                                                    \
	}

/**
 * Initializes the virtual messaging system layer
 * @param uri
 * @param vmsl
 * @return
 */
bool vmsl_assign_by_url(gru_uri_t *uri, vmsl_t *vmsl);

#ifdef __cplusplus
}
#endif

#endif /* VMSL_INIT_H */
