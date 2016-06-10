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
#ifndef MESSAGE_RECEIVER_H
#define MESSAGE_RECEIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmsl.h"
#include "msgctxt.h"
#include "contrib/options.h"

#include <signal.h>
#include <unistd.h>
#include <inttypes.h>
    
void receiver_start(const vmsl_t *vmsl, const options_t *options);

#ifdef __cplusplus
}
#endif

#endif /* MESSAGE_RECEIVER_H */

