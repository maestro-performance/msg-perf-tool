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
#ifndef MSG_CONTENT_DATA_H
#define MSG_CONTENT_DATA_H

#include <stdint.h>
#include <stdlib.h>

#include <common/gru_alloc.h>
#include <common/gru_status.h>
#include <log/gru_logger.h>
#include <time/gru_time_utils.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct msg_content_data_t_ {
	void *data;
	size_t capacity;
	size_t size;
	gru_timestamp_t created;
} msg_content_data_t;

msg_content_data_t *msg_content_data_new(size_t size, gru_status_t *status);

void msg_content_data_init(msg_content_data_t *ptr, size_t size, gru_status_t *status);
void msg_content_data_reset(msg_content_data_t *mdata);
void msg_content_data_release(msg_content_data_t *data);
void msg_content_data_destroy(msg_content_data_t **data);
void msg_content_data_fill(msg_content_data_t *data, char v);
void msg_content_data_rfill(msg_content_data_t *content);

bool msg_content_data_copy(msg_content_data_t *cont, const void *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* MSG_CONTENT_DATA_H */
