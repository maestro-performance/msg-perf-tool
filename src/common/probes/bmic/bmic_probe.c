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
#include "bmic_probe.h"

static const char *name = "bmic";
static bmic_context_t ctxt = {0};

probe_entry_t *bmic_entry(gru_status_t *status) {
	probe_entry_t *ret = gru_alloc(sizeof(probe_entry_t), status);
	gru_alloc_check(ret, NULL);

	ret->init = bmic_init;
	ret->collect = bmic_collect;
	ret->stop = bmic_stop;
	ret->name = bmic_name;

	ret->cancel = false;

	return ret;
}

bool bmic_init(const options_t *options, gru_status_t *status) {
	logger_t logger = gru_logger_get();

	logger(DEBUG, "Creating report file");

	if (!mpt_init_bmic_ctxt(options->uri, &ctxt, status)) {
		return false;
	}

	char filename[64] = {0};

	snprintf(filename, sizeof(filename) - 1, "broker-jvm-statistics-%d.csv.gz", getpid());

	if (!bmic_writer_initialize(options->logdir, filename, status)) {
		return false;
	}

	return true;
}

int bmic_collect(gru_status_t *status) {
	const options_t *options = get_options_object();

	bmic_api_interface_t *api = ctxt.api;
	bmic_java_info_t jinfo = api->java.java_info(ctxt.handle, status);
	while (true) {
		if (!bmic_writer_current_time(status)) {
			return 1;
		}

		bmic_java_os_info_t osinfo = api->java.os_info(ctxt.handle, status);
		bmic_writer_osinfo(&osinfo);

		mpt_java_mem_t java_mem = {0};
		mpt_get_mem_info(&ctxt, jinfo.memory_model, &java_mem, status);
		if (gru_status_error(status)) {
			return 1;
		}

		bmic_writer_java_mem(&java_mem, jinfo.memory_model);

		bmic_queue_stat_t qstats = {0};
		mpt_get_queue_stats(&ctxt, &options->uri.path[1], &qstats, status);

		if (gru_status_error(status)) {
			return 1;
		}
		bmic_writer_queue_stat(&qstats);

		sleep(10);
	}

	return 0;
}

void bmic_stop() {
	gru_status_t status = gru_status_new();

	if (!bmic_writer_finalize(&status)) {
		logger_t logger = gru_logger_get();

		logger(WARNING, "Failed to finalize BMIC writer: %s", status.message);
	}

	bmic_context_cleanup(&ctxt);
}

const char *bmic_name() {
	return name;
}