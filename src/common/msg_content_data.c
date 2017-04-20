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
}

bool msg_content_data_vserialize(msg_content_data_t *cont, const char *fmt, va_list ap) {
	cont->size = vasprintf((char **) &cont->data, fmt, ap); 
	
	if (cont->size == -1) {
		return false;
	}
	cont->capacity = cont->size;
	
	
	return true;
}

bool msg_content_data_serialize(msg_content_data_t *cont, const char *fmt, ...) {	
	va_list ap;

	va_start(ap, fmt);
	bool ret = msg_content_data_vserialize(cont, fmt, ap);
	va_end(ap);
	
	return ret;
}
