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
#include "hdr_writer.h"

static hdr_wrapper_t lat_wrapper;

bool hdr_lat_writer_initialize(const stat_io_info_t *io_info, gru_status_t *status) {
	return hdr_wrapper_initialize(io_info, &lat_wrapper, status);
}

bool hdr_lat_writer_write(const stat_latency_t *latency, gru_status_t *status) {
	int64_t milli_latency = gru_time_to_milli(&latency->elapsed);;

	hdr_interval_recorder_record_value(&lat_wrapper.recorder, milli_latency);
	return true;
}

bool hdr_lat_writer_flush(gru_status_t *status) {
	fflush(lat_wrapper.file);
	return true;
}

bool hdr_lat_writer_finalize(gru_status_t *status) {
	hdr_wrapper_finalize(&lat_wrapper);
	return true;
}

/**
 * Latency writer I/O assignment
 */
void hdr_writer_latency_assign(latency_writer_t *writer) {
	writer->initialize = hdr_lat_writer_initialize;
	writer->write = hdr_lat_writer_write;
	writer->flush = hdr_lat_writer_flush;
	writer->finalize = hdr_lat_writer_finalize;
}

/**
 * Throghput writer I/O assignment
 */
void hdr_writer_throughput_assign(throughput_writer_t *writer) {
	writer->initialize = csv_tp_writer_initialize;
	writer->write = csv_tp_writer_write;
	writer->flush = csv_tp_writer_flush;
	writer->finalize = csv_tp_writer_finalize;
}

/**
 * Throghput rate writer I/O assignment
 */
void hdr_writer_tpr_assign(tpr_writer_t *writer) {
	writer->initialize = csv_tpr_writer_initialize;
	writer->write = csv_tpr_writer_write;
	writer->flush = csv_tpr_writer_flush;
	writer->finalize = csv_tpr_writer_finalize;
}