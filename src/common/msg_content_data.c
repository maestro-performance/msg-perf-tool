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

msg_content_data_t msg_content_data_new(size_t size, gru_status_t *status) {
	msg_content_data_t content_storage;

	content_storage.data = gru_alloc(size, status);
	if (!content_storage.data) {
		content_storage.data = NULL;

		return content_storage;
	}

	content_storage.capacity = size;
	content_storage.count = 0;
	content_storage.errors = 0;
}
