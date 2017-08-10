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
#include "bmic_writer.h"

static gzFile report = NULL;

static gzFile
	bmic_writer_open_file(const char *dir, const char *name, gru_status_t *status) {

	char *fullpath = gru_path_format(dir, name, status);

	if (!gru_path_rename(fullpath, status)) {
		gru_dealloc_string(&fullpath);
		return NULL;
	}

	gzFile f = gzopen(fullpath, "wb");
	gru_dealloc_string(&fullpath);

	if (!f) {
		gru_status_strerror(status, GRU_FAILURE, errno);
		return NULL;
	}

	return f;
}

static void bmic_writer_add_header_column(const char *name, bool more) {
	if (more) {
		gzprintf(report, "\"%s\",");
	}
	else {
		gzprintf(report, "\"%s\"\n");
	}
}

bool bmic_writer_initialize(const char *dir, const char *name, gru_status_t *status) {
	logger_t logger = gru_logger_get();

	logger(GRU_INFO, "Creating broker report file: %s/%s", dir, name);

	report = bmic_writer_open_file(dir, name, status);
	if (!report) {
		return false;
	}

	bmic_writer_add_header_column("timestamp", true);
	bmic_writer_add_header_column("load", true);
	bmic_writer_add_header_column("open fds", true);
	bmic_writer_add_header_column("free fds", true);
	bmic_writer_add_header_column("free mem", true);
	bmic_writer_add_header_column("swap free", true);
	bmic_writer_add_header_column("swap committed", true);
	bmic_writer_add_header_column("eden initial", true);
	bmic_writer_add_header_column("eden committed", true);
	bmic_writer_add_header_column("eden max", true);
	bmic_writer_add_header_column("eden used", true);
	bmic_writer_add_header_column("survivor initial", true);
	bmic_writer_add_header_column("survivor committed", true);
	bmic_writer_add_header_column("survivor max", true);
	bmic_writer_add_header_column("survivor used", true);
	bmic_writer_add_header_column("tenured initial", true);
	bmic_writer_add_header_column("tenured committed", true);
	bmic_writer_add_header_column("tenured max", true);
	bmic_writer_add_header_column("tenured used", true);
	bmic_writer_add_header_column("pm initial", true);
	bmic_writer_add_header_column("pm committed", true);
	bmic_writer_add_header_column("pm max", true);
	bmic_writer_add_header_column("pm used", true);
	bmic_writer_add_header_column("queue size", true);
	bmic_writer_add_header_column("consumers", true);
	bmic_writer_add_header_column("ack", true);
	bmic_writer_add_header_column("exp", false);


	return true;
}

bool bmic_writer_flush(gru_status_t *status) {
	if (gzflush(report, Z_SYNC_FLUSH) != 0) {
		gru_status_strerror(status, GRU_FAILURE, errno);

		return false;
	}

	return true;
}

bool bmic_writer_current_time(gru_status_t *status) {
	gru_timestamp_t now = gru_time_now();
	char *curr_time_str = gru_time_write_format(&now, "%Y-%m-%d %H:%M:%S", status);

	if (!curr_time_str) {
		return false;
	}

	gzprintf(report, "\"%s\",", curr_time_str);
	gru_dealloc_string(&curr_time_str);
	return true;
}

static inline uint64_t bmic_writer_osinfo_free_fd(const bmic_java_os_info_t *osinfo) {
	return (osinfo->max_fd - osinfo->open_fd);
}

void bmic_writer_osinfo(const bmic_java_os_info_t *osinfo) {
	gzprintf(report, "%.1f,", osinfo->load_average);
	gzprintf(report,
		"%" PRId64 ",%" PRId64 ",",
		osinfo->open_fd,
		bmic_writer_osinfo_free_fd(osinfo));

	gzprintf(report, "%" PRId64 ",", gru_unit_mb(osinfo->mem_free));
	gzprintf(report,
		"%" PRId64 ",%" PRId64 ",",
		gru_unit_mb(osinfo->swap_free),
		gru_unit_mb(osinfo->swap_committed));
}

static void bmic_write_mem(const bmic_java_mem_info_t *mem) {
	gzprintf(report,
		"%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",",
		gru_unit_mb(mem->init),
		gru_unit_mb(mem->committed),
		gru_unit_mb(mem->max),
		gru_unit_mb(mem->used));
}

void bmic_writer_java_mem(const mpt_java_mem_t *java_mem,
	bmic_java_memory_model_t memory_model) {
	bmic_write_mem(&java_mem->eden);
	bmic_write_mem(&java_mem->survivor);
	bmic_write_mem(&java_mem->tenured);

	if (memory_model == BMIC_JAVA_MODERN) {
		bmic_write_mem(&java_mem->metaperm);
	}
}

void bmic_writer_queue_stat(const bmic_queue_stat_t *stat) {
	gzprintf(report,
		"%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 "\n",
		stat->queue_size,
		stat->consumer_count,
		stat->msg_ack_count,
		stat->msg_exp_count);
}

bool bmic_writer_finalize(gru_status_t *status) {
	if (gzclose(report) != 0) {
		gru_status_strerror(status, GRU_FAILURE, errno);
		return false;
	}

	return true;
}