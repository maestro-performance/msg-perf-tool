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
#include <stdint.h>

#include "timestamp.h"

mpt_timestamp_t statistics_now() {
	struct timeval ret = {.tv_sec = 0, .tv_usec = 0};

	gettimeofday(&ret, NULL);
	return ret;
}

mpt_timestamp_t ts_from_milli(int64_t timestamp) {

	mpt_timestamp_t ret = {0};

	double ts = ((double) timestamp / 1000);
	double integral;

	ret.tv_usec = modf(ts, &integral) * 1000000;
	ret.tv_sec = integral;

	return ret;
}

mpt_timestamp_t ts_from_milli_char(const char *ts) {
	uint64_t ms = strtoull(ts, NULL, 10);

	return ts_from_milli(ms);
}
