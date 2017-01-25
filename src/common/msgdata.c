/*
 * Copyright 2017 Otavio Piske <angusyoung@gmail.com>.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "msgdata.h"

static char *data = NULL;
static size_t capacity;

const char *load_message_data(const options_t *options, gru_status_t *status) {
	data = gru_alloc(options->message_size, status);
	gru_alloc_check(data, NULL);

	logger_t logger = gru_logger_get();

	logger(INFO, "Loading %d bytes for message data", options->message_size);

	size_t i = options->message_size - 1;
	for (; 0 < i; i--) {
		data[i] = 'c';
	}
	data[0] = 'c';

	capacity = options->message_size;
	logger(DEBUG, "Loaded message data: %s", data);
	return data;
}

void unload_message_data() {
	free(data);
	capacity = 0;
}

void content_loader(msg_content_data_t *content_data) {
	content_data->capacity = capacity;
	content_data->size = capacity;
	content_data->data = data;
}