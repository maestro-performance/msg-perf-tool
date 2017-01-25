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
#include "net_probe.h"

static FILE *report;
static const char *name = "net";
static char device[4096] = {0};

probe_entry_t *net_entry(gru_status_t *status) {
	probe_entry_t *ret = gru_alloc(sizeof(probe_entry_t), status);
	gru_alloc_check(ret, NULL);

	ret->init = net_init;
	ret->collect = net_collect;
	ret->stop = net_stop;
	ret->name = net_name;

	ret->cancel = false;

	return ret;
}

static char *net_get_filename(const char *device, const char *what, gru_status_t *status) {
	char *ret = NULL;
	// logger_t logger = gru_logger_get();

	if (asprintf(&ret,  "/sys/class/net/%s/statistics/%s", device, what) == -1) {
		gru_status_set(status, GRU_FAILURE, "Not enough memory");

		return NULL;

	}

	// logger(INFO, "Openning %s", ret);

	return ret;
}

static FILE *net_open_tx_file(const char *device, gru_status_t *status) {
	char *path = net_get_filename(device, "tx_bytes", status); 
	
	if (gru_status_error(status)) {
		return NULL;
	}

	int tx_fd = open(path, O_RDONLY);
	FILE *tx_file = fdopen(tx_fd, "r");
	if (!tx_file) {
		gru_dealloc_string(&path);
		return NULL;
	}

	return tx_file;
}

static FILE *net_open_rx_file(const char *device, gru_status_t *status) {
	char *path = net_get_filename(device, "rx_bytes", status);

	if (gru_status_error(status)) {
		return NULL;
	}
	
	int rx_fd = open(path, O_RDONLY);
	FILE *rx_file = fdopen(rx_fd, "r");
	if (!rx_file) {
		gru_dealloc_string(&path);
		return NULL;
	}

	return rx_file;
}


bool net_init(const options_t *options, gru_status_t *status) {
	logger_t logger = gru_logger_get();

	gru_config_read_string("probes.net.device", options->config->file, device);

	logger(INFO, "Reading device %s", device);

	report = gru_io_open_file(options->logdir, "net.csv", status);

	if (!report) {
		return false;
	}

	fprintf(report, "TX;RX\n");

	return true;
}

int net_collect(gru_status_t *status) {
	char tx_data[1024] = {0};
	char rx_data[1024] = {0};
	logger_t logger = gru_logger_get();

	uint64_t last_tx_data = 0;
	uint64_t last_rx_data = 0;
	while (true) { 
		FILE *tx_file = net_open_tx_file(device, status);
		if (!tx_file) {
			logger(ERROR, "Unable to open device TX file");
			return 1;
		}

		FILE *rx_file = net_open_rx_file(device, status);
		if (!rx_file) {
			logger(ERROR, "Unable to open device RX file");
			return 1;
		}

		fgets(tx_data, sizeof(tx_data) - 1, tx_file);
		fgets(rx_data, sizeof(rx_data) - 1, rx_file);

		fclose(rx_file);
		fclose(tx_file);

		// char *filtered_tx_data = gru_rtrim(tx_data, sizeof(tx_data));
		// char *filtered_rx_data = gru_rtrim(rx_data, sizeof(rx_data));
		uint64_t curr_tx_data = atoll(tx_data);
		uint64_t curr_rx_data = atoll(rx_data);

		if (last_tx_data == 0 || last_rx_data == 0) {
			last_tx_data = curr_tx_data;
			last_rx_data = curr_rx_data;
			
			sleep(1);
			continue;
		}

		uint64_t tx_rate = curr_tx_data - last_tx_data;
		uint64_t rx_rate = curr_rx_data - last_rx_data;

		fprintf(report, "%ld;%ld\n", tx_rate, rx_rate);
		fflush(report);
		sleep(1);
	}

	return 0;
}

void net_stop() {
	fclose(report);
}

const char *net_name() {
	return name;
}
