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
#include "hdr_wrapper.h"

static FILE *hdr_open_file(const char *dir, const char *name, gru_status_t *status) {
	char *fullpath = gru_path_format(dir, name, status);

	if (!gru_path_rename(fullpath, status)) {
		gru_dealloc_string(&fullpath);
		return NULL;
	}

	return gru_io_open_file_path(fullpath, status);
}

static bool hdr_initialize(const stat_io_info_t *io_info, struct hdr_interval_recorder *recorder,
						   gru_status_t *status) {
	int hdr_ret = 0;

	hdr_ret = hdr_interval_recorder_init_all(recorder, 1, INT64_C(24) * 60 * 60 * 1000000, 3);

	if (hdr_ret != 0) {
		gru_status_set(status, GRU_FAILURE, "Unable to initialize HDR histogram library");

		return false;
	}

	return true;
}

bool hdr_wrapper_initialize(const stat_io_info_t *io_info, hdr_wrapper_t *wrapper, gru_status_t *status) {
	if (!hdr_initialize(io_info, &wrapper->recorder, status)) {
		return false;
	}


	wrapper->file = hdr_open_file(io_info->dest.location, io_info->dest.name, status);
	if (!wrapper->file) {
		return false;
	}

	hdr_gettime(&wrapper->start_timestamp);

	hdr_log_writer_init(&wrapper->writer);

	struct timespec timestamp;
	hdr_getnow(&timestamp);

	hdr_log_write_header(&wrapper->writer, wrapper->file, "mpt", &timestamp);
	return true;
}

void hdr_wrapper_finalize(hdr_wrapper_t *wrapper) {
	struct timespec end_timestamp;
	hdr_gettime(&end_timestamp);

	struct hdr_histogram* h = hdr_interval_recorder_sample(&wrapper->recorder);

	hdr_log_write(&wrapper->writer, wrapper->file, &wrapper->start_timestamp, &end_timestamp, h);
	fflush(wrapper->file);
	fclose(wrapper->file);

}