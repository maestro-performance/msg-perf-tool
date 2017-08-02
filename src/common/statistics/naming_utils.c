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

typedef void (*naming_func_t)(naming_info_t *nm_info, const char *basename, char *out, size_t len);

/**
 * @param nm_info naming info
 * @param basename = "latency"
 * @param out out
 * @param len buffer lenght
 */
static void
naming_hdr_name(naming_info_t *nm_info, const char *basename, char *out, size_t len) {
	snprintf(out,
			 len - 1,
			 "%s-%s-%d-%d.hdr",
			 nm_info->source,
			 basename,
			 nm_info->pid,
			 nm_info->ppid);
}

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

static bool naming_initialize_latency_writer(latency_writer_t *writer,
											 naming_info_t *nm_info, naming_func_t naming_func,
											 gru_status_t *status) {

	char lat_fname[64] = {0};

	naming_func(nm_info, "latency", lat_fname, sizeof(lat_fname));

	stat_io_info_t lat_io_info = {0};
	lat_io_info.dest.name = lat_fname;
	lat_io_info.dest.location = (char *) nm_info->location;

	if (!writer->initialize(&lat_io_info, status)) {
		return false;
	}

	return true;
}

static bool naming_initialize_throughput_writer(throughput_writer_t *writer,
												naming_info_t *nm_info, naming_func_t naming_func,
												gru_status_t *status) {

	char tp_fname[64] = {0};
	naming_func(nm_info, "throughput", tp_fname, sizeof(tp_fname));

	stat_io_info_t tp_io_info = {0};
	tp_io_info.dest.name = tp_fname;
	tp_io_info.dest.location = (char *) nm_info->location;

	if (!writer->initialize(&tp_io_info, status)) {
		return false;
	}

	return true;
}

static bool naming_initialize_tpr_writer(tpr_writer_t *writer,
										 naming_info_t *nm_info, naming_func_t naming_func,
										 gru_status_t *status) {
	char tp_fname[64] = {0};
	naming_func(nm_info, "rate", tp_fname, sizeof(tp_fname));

	stat_io_info_t tp_io_info = {0};
	tp_io_info.dest.name = tp_fname;
	tp_io_info.dest.location = (char *) nm_info->location;

	if (!writer->initialize(&tp_io_info, status)) {
		return false;
	}

	return true;
}

static bool naming_initialize_hdr_writer(stats_writer_t *writer,
										 naming_opt_t nm_opt,
										 void *payload,
										 gru_status_t *status) {

	bool naming_ret;

	if (nm_opt & NM_LATENCY) {
		hdr_writer_latency_assign(&writer->latency);


		naming_ret = naming_initialize_latency_writer(
			&writer->latency, (naming_info_t *) payload, naming_hdr_name, status);

		if (!naming_ret) {
			return false;
		}
	}

	if (nm_opt & NM_THROUGHPUT) {
		hdr_writer_throughput_assign(&writer->throughput);

		naming_ret = naming_initialize_throughput_writer(
			&writer->throughput, (naming_info_t *) payload, naming_csv_name, status);

		if (!naming_ret) {
			return false;
		}
	}

	if (nm_opt & NM_RATE) {
		hdr_writer_tpr_assign(&writer->rate);

		naming_ret = naming_initialize_tpr_writer(
			&writer->rate, (naming_info_t *) payload, naming_csv_name, status);

		if (!naming_ret) {
			return false;
		}
	}

	return true;
}

static bool naming_initialize_csv_writer(stats_writer_t *writer,
	naming_opt_t nm_opt,
	void *payload,
	gru_status_t *status) {

	bool naming_ret;

	if (nm_opt & NM_LATENCY) {
		csv_writer_latency_assign(&writer->latency);


		naming_ret = naming_initialize_latency_writer(
			&writer->latency, (naming_info_t *) payload, naming_csv_name, status);

		if (!naming_ret) {
			return false;
		}
	}

	if (nm_opt & NM_THROUGHPUT) {
		csv_writer_throughput_assign(&writer->throughput);

		naming_ret = naming_initialize_throughput_writer(
			&writer->throughput, (naming_info_t *) payload, naming_csv_name, status);

		if (!naming_ret) {
			return false;
		}
	}

	if (nm_opt & NM_RATE) {
		csv_writer_tpr_assign(&writer->rate);

		naming_ret = naming_initialize_tpr_writer(
			&writer->rate, (naming_info_t *) payload, naming_csv_name, status);

		if (!naming_ret) {
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

	if (format == FORMAT_HDR) {
		return naming_initialize_hdr_writer(writer, nm_opt, payload, status);
	}
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