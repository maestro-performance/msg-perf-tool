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

#ifndef MPT_DEBUG_H
#define MPT_DEBUG_H

#include <log/gru_logger.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(MPT_DEBUG) && MPT_DEBUG >=2
 #define mpt_trace(...) { logger_t mptlogger = gru_logger_get(); mptlogger(TRACE, __VA_ARGS__); }
#else
 #define mpt_trace(...)
#endif


#ifdef __cplusplus
}
#endif

#endif /* MPT_DEBUG_H */

