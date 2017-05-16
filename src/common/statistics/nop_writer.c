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
#include "nop_writer.h"

bool nop_lat_writer_initialize(const stat_io_info_t *io_info, gru_status_t *status) {
	return true;
}

bool nop_lat_writer_write(const stat_latency_t *latency, gru_status_t *status) {
	return true;
}

bool nop_lat_writer_flush(gru_status_t *status) {
	return true;
}

bool nop_lat_writer_finalize(gru_status_t *status) {
	return true;
}

bool nop_tp_writer_initialize(const stat_io_info_t *io_info, gru_status_t *status) {
	return true;
}

bool nop_tp_writer_write(const stat_throughput_t *tp, gru_status_t *status) {
	return true;
}

bool nop_tp_writer_flush(gru_status_t *status) {
	return true;
}

bool nop_tp_writer_finalize(gru_status_t *status) {
	return true;
}

/**
 * Latency writer I/O assignment
 */
void nop_writer_latency_assign(latency_writer_t *writer) {
	writer->initialize = nop_lat_writer_initialize;
	writer->write = nop_lat_writer_write;
	writer->flush = nop_lat_writer_flush;
	writer->finalize = nop_lat_writer_finalize;
}

/**
 * Throghput writer I/O assignment
 */
void nop_writer_throughput_assign(throughput_writer_t *writer) {
	writer->initialize = nop_tp_writer_initialize;
	writer->write = nop_tp_writer_write;
	writer->flush = nop_tp_writer_flush;
	writer->finalize = nop_tp_writer_finalize;
}