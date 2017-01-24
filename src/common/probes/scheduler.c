/*
 * Copyright 2017 otavio.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <common/gru_alloc.h>

#include "scheduler.h"
#include "contrib/options.h"

gru_list_t *list;

static void *probe_scheduler_run(void *en) {
	logger_t logger = gru_logger_get();
    probe_entry_t *entry = (probe_entry_t *) en;
	gru_status_t status = gru_status_new();

    while (!entry->cancel) {
        logger(DEBUG, "Running probe %s", entry->name());
		entry->collect(&status);
		if (status.code != GRU_SUCCESS) {
			logger(ERROR, "Probe %s error: %s", entry->name(), status.message);

			break;
		}

        sleep(1);
    }

    logger(DEBUG, "Finished running probe %s", entry->name());
    return NULL;
}

static void probe_scheduler_launch_probe(const void *nodedata, void *payload) {
	probe_entry_t *entry = (probe_entry_t *) nodedata;
	options_t *options = get_options_object();
	logger_t logger = gru_logger_get();

	gru_status_t status = gru_status_new();
	bool pret = entry->init(options, &status);
	if (!pret) {
		logger(ERROR, "Unable to initialize probe %s: %s", entry->name(), status.message);

		return;
	}

	int sys_ret = pthread_create(&entry->thread, NULL, probe_scheduler_run, entry);
    if (sys_ret != 0) {
        logger(ERROR, "Unable to create probe thread");

        return;
    }

    logger(INFO, "Probe %s created", entry->name());
}

static probe_entry_t *probe_scheduler_load_probe(const char *lib, const char *name) {
	void *handle = NULL;
	char *error = NULL;

	handle = dlopen(lib, RTLD_NOW);
	if (!handle) {
		fprintf(stderr, "Unable to open handle: %s\n", dlerror());

		return NULL;
	}

	dlerror();

	probe_entry_t *(*new_entry)(gru_status_t *);

	new_entry = (probe_entry_t *(*)(gru_status_t *)) dlsym(handle, name);
	error = dlerror();
	if (error) {
		fprintf(stderr, "Unable to open handle: %s\n", error);

		return NULL;
	}

	gru_status_t status = gru_status_new();
	probe_entry_t *ret = (*new_entry)(&status);

	if (status.code != GRU_SUCCESS) {
		fprintf(stderr, "%s\n", status.message);
	}
	ret->handle = handle;

	return ret;
}

bool probe_scheduler_start(gru_status_t *status) {
	list = gru_list_new(status);
	if (!list) {
		return false;
	}

	probe_entry_t *net = probe_scheduler_load_probe("libmpt-probe-net.so", "net_entry");
	gru_list_append(list, net);

	gru_list_for_each(list, probe_scheduler_launch_probe, NULL);
}

static void probe_scheduler_stop_probe(const void *nodedata, void *payload) {
	probe_entry_t *entry = (probe_entry_t *) nodedata;

	entry->stop();
	entry->cancel = true;
	dlclose(entry->handle);
}

void probe_scheduler_stop() {
	gru_list_for_each(list, probe_scheduler_stop_probe, NULL);
}