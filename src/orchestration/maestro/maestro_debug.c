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
#include "maestro_debug.h"

#if MPT_DEBUG >= 2
void maestro_trace_proto(char const *buf, ssize_t len) {
	size_t i = 0;

	fprintf(stderr, "[TRACE]");

	for(; i < len ; ++i) {
		fprintf(stderr, "%02x ", 0xff & buf[i]);
	}

	fprintf(stderr, "\n");
}
#endif // MPT_DEBUG >= 2