/**
 *    Copyright 2017 Otavio Rodolfo Piske
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef WORKER_OPTIONS_H
#define WORKER_OPTIONS_H

#include <stdbool.h>
#include <stdint.h>

#include <log/gru_logger.h>
#include <network/gru_uri.h>
#include <time/gru_duration.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum test_duration_type_t_ {
	MESSAGE_COUNT,
	TEST_TIME,
} test_duration_type_t;

typedef enum test_condition_type_t_ {
  MPT_COND_NONE = 0,
  MPT_COND_RATE = 1,
  MPT_COND_LATENCY = 2,
} test_condition_type_t;

typedef struct worker_options_t_ {
	gru_uri_t uri;

	test_duration_type_t duration_type;
	union {
		uint64_t count;
		gru_duration_t time;
	} duration;

  	test_condition_type_t condition_type;

  	union {
	  double rate;
	  int64_t latency;
	} condition;

	log_level_t log_level;
	uint16_t parallel_count;
	size_t message_size;
	bool variable_size;
	uint32_t throttle;
  	uint32_t rate;
} worker_options_t;

#ifdef __cplusplus
}
#endif

#endif /* WORKER_OPTIONS_H */
