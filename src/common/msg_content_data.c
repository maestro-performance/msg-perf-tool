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
#include "msg_content_data.h"

msg_content_data_t *msg_content_data_new(size_t size, gru_status_t *status) {
	msg_content_data_t *ret = gru_alloc(sizeof(msg_content_data_t), status);

	if (!ret) {
		return NULL;
	}

	ret->data = gru_alloc(size, status);
	if (!ret->data) {
		gru_dealloc((void **) &ret);

		return NULL;
	}

	ret->capacity = size;

	return ret;
}

void msg_content_data_init(msg_content_data_t *mdata, size_t size, gru_status_t *status) {
	msg_content_data_release(mdata);

	mdata->data = gru_alloc(size, status);

	if (!mdata->data) {
		mdata->data = NULL;

		return;
	}

	mdata->capacity = size;
}

void msg_content_data_reset(msg_content_data_t *mdata) {
	if (!mdata) {
		return;
	}

	bzero(mdata->data, mdata->capacity);
	mdata->size = 0;
}

void msg_content_data_release(msg_content_data_t *mdata) {
	if (!mdata) {
		return;
	}

	gru_dealloc(&mdata->data);
	mdata->capacity = 0;
	mdata->size = 0;
}

void msg_content_data_destroy(msg_content_data_t **data) {
	msg_content_data_t *ptr = *data;
	if (!ptr) {
		return;
	}

	msg_content_data_release(ptr);
	gru_dealloc((void **) data);
}

void msg_content_data_fill(msg_content_data_t *content, char v) {
	size_t i = content->capacity - 1;

	for (; 0 < i; i--) {
		((char *) content->data)[i] = v;
	}

	((char *) content->data)[0] = v;
	content->size = content->capacity;
}

void msg_content_data_rfill(msg_content_data_t *content) {
	static const char dict[] = "0123456789"
							   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
							   "abcdefghijklmnopqrstuvwxyz";

	size_t i = content->capacity - 1;

	for (; 0 < i; i--) {
		((char *) content->data)[i] = dict[rand() % (sizeof(dict) - 1)];
	}

	((char *) content->data)[0] = dict[rand() % (sizeof(dict) - 1)];
	content->size = content->capacity;
}

bool msg_content_data_copy(msg_content_data_t *cont, const void *data, size_t size) {
	msg_content_data_release(cont);

	gru_status_t status = gru_status_new();

	cont->data = gru_alloc(size, &status);
	gru_alloc_check(cont->data, false);

	cont->capacity = size;
	cont->size = size;

	memcpy(cont->data, data, size);

	return true;
}
