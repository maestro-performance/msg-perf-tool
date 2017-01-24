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

bool net_init(const options_t *options, gru_status_t *status) {
    report = gru_io_open_file(options->logdir, "net.csv", status);

    if (!report) {
        return false;
    }

    return true;
}


int net_collect(gru_status_t *status) {
    fprintf(report, "test\n");
	fflush(report);
    return 0;
}


void net_stop() {
    fclose(report);
}

const char *net_name() {
	return name;
}
