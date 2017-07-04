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
#include "naming_utils.h"

/**
 * @param nm_info naming info
 * @param basename = "latency"
 * @param out out
 * @param len buffer lenght
 */
static void
	naming_csv_name(naming_info_t *nm_info, const char *basename, char *out, size_t len) {
	snprintf(out,
		len - 1,
		"%s-%s-%d-%d.csv.gz",
		nm_info->source,
		basename,
		nm_info->pid,
		nm_info->ppid);
}

static bool naming_initialize_csv_latency_writer(stats_writer_t *writer,
	naming_info_t *nm_info,
	gru_status_t *status) {
	csv_writer_latency_assign(&writer->latency);

	char lat_fname[64] = {0};

	naming_csv_name(nm_info, "latency", lat_fname, sizeof(lat_fname));

	stat_io_info_t lat_io_info = {0};
	lat_io_info.dest.name = lat_fname;
	lat_io_info.dest.location = (char *) nm_info->location;

	if (!writer->latency.initialize(&lat_io_info, status)) {
		return false;
	}

	return true;
}

static bool naming_initialize_csv_throughput_writer(stats_writer_t *writer,
	naming_info_t *nm_info,
	gru_status_t *status) {
	csv_writer_throughput_assign(&writer->throughput);

	char tp_fname[64] = {0};
	naming_csv_name(nm_info, "throughput", tp_fname, sizeof(tp_fname));

	stat_io_info_t tp_io_info = {0};
	tp_io_info.dest.name = tp_fname;
	tp_io_info.dest.location = (char *) nm_info->location;

	if (!writer->throughput.initialize(&tp_io_info, status)) {
		return false;
	}

	return true;
}

static bool naming_initialize_csv_tpr_writer(stats_writer_t *writer,
													naming_info_t *nm_info,
													gru_status_t *status) {
	csv_writer_tpr_assign(&writer->rate);

	char tp_fname[64] = {0};
	naming_csv_name(nm_info, "rate", tp_fname, sizeof(tp_fname));

	stat_io_info_t tp_io_info = {0};
	tp_io_info.dest.name = tp_fname;
	tp_io_info.dest.location = (char *) nm_info->location;

	if (!writer->rate.initialize(&tp_io_info, status)) {
		return false;
	}

	return true;
}

static bool naming_initialize_csv_writer(stats_writer_t *writer,
	naming_opt_t nm_opt,
	void *payload,
	gru_status_t *status) {
	if (nm_opt & NM_LATENCY) {
		if (!naming_initialize_csv_latency_writer(
				writer, (naming_info_t *) payload, status)) {
			return false;
		}
	}

	if (nm_opt & NM_THROUGHPUT) {
		if (!naming_initialize_csv_throughput_writer(
				writer, (naming_info_t *) payload, status)) {
			return false;
		}
	}

	if (nm_opt & NM_RATE) {
		if (!naming_initialize_csv_tpr_writer(
			writer, (naming_info_t *) payload, status)) {
			return false;
		}
	}

	return true;
}

bool naming_initialize_out_writer(stats_writer_t *writer, gru_status_t *status) {
	out_writer_latency_assign(&writer->latency);
	out_writer_throughput_assign(&writer->throughput);
	out_writer_tpr_assign(&writer->rate);

	return true;
}

static bool naming_initialize_nop_writer(stats_writer_t *writer, gru_status_t *status) {
	nop_writer_latency_assign(&writer->latency);
	nop_writer_throughput_assign(&writer->throughput);
	nop_writer_tpr_assign(&writer->rate);

	return true;
}

bool naming_initialize_writer(stats_writer_t *writer,
	report_format_t format,
	naming_opt_t nm_opt,
	void *payload,
	gru_status_t *status) {
	if (format == FORMAT_CSV) {
		return naming_initialize_csv_writer(writer, nm_opt, payload, status);
	}
	if (format == FORMAT_NOP) {
		return naming_initialize_nop_writer(writer, status);
	}
	if (format == FORMAT_OUT) {
		return naming_initialize_out_writer(writer, status);
	}

	return false;
}