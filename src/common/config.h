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

#ifndef CONFIG_H
#define CONFIG_H

#include "contrib/options.h"

#include <common/gru_status.h>
#include <config/gru_payload.h>
#include <config/gru_config.h>

#ifdef __cplusplus
extern "C" {
#endif

void config_init(options_t *options, const char *dir, const char *filename);


#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H */

