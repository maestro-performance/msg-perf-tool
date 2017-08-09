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
#ifndef MPT_WORKER_QUEUE_STAT_H
#define MPT_WORKER_QUEUE_STAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum worker_queue_stat_t_ {
  MPT_WQ_ERROR = 0,
  MPT_WQ_SUCCESS = 1,
  MPT_WQ_NO_DATA = 2,
} worker_queue_stat_t;

static inline bool wq_stat_success(worker_queue_stat_t stat) {
	return (stat & MPT_WQ_SUCCESS) ? true : false;
}

static inline bool wq_stat_error(worker_queue_stat_t stat) {
	return (stat & MPT_WQ_SUCCESS) ? false : true;
}

static inline bool wq_has_data(worker_queue_stat_t stat) {
	return (stat & MPT_WQ_NO_DATA) ? false : true;
}

#ifdef __cplusplus
}
#endif

#endif //MPT_WORKER_QUEUE_STAT_H
