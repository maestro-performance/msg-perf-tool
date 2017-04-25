/**
 *   Copyright 2017 Otavio Rodolfo Piske
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
 #include "worker_utils.h"

bool worker_check(const worker_options_t *options, const worker_snapshot_t *snapshot) {
	if (is_interrupted()) {
		return false;
	}

	if (likely(options->duration_type == TEST_TIME)) {
		if (likely(snapshot->now.tv_sec <= options->duration.time.end.tv_sec)) {
			return true;
		}
	} else {
		if (likely(snapshot->count < options->duration.count)) {
			return true;
		}
	}

	return false;
}