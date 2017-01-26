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
#include "bmic_utils.h"

bool mpt_init_bmic_ctxt(const options_t *options, bmic_context_t *ctxt, 
	gru_status_t *status)
{
	logger_t logger = gru_logger_get();

	logger(INFO, "Resolved host to %s", options->uri.host);

	bool ret =
		bmic_context_init_simple(ctxt, options->uri.host, "admin", "admin", status);

	if (!ret) {
		bmic_context_cleanup(ctxt);
		return false;
	}

	// Load the capabilities just so that it is cached
	const bmic_exchange_t *cap = ctxt->api->capabilities_load(ctxt->handle, status);
	if (!cap) {
		bmic_context_cleanup(ctxt);
		return false;
	}

	return true;
}


void mpt_get_queue_stats(const bmic_context_t *ctxt, const char *name, 
	bmic_queue_stat_t *stat, gru_status_t *status) 
{
	const bmic_exchange_t *cap = ctxt->api->capabilities_load(ctxt->handle, status);

	if (!cap) {
		logger_t logger = gru_logger_get();

		logger(INFO, "Unable to load capabilities");
		return;
	}

	*stat = ctxt->api->queue_stats(ctxt->handle, cap, name, status);
	if (gru_status_error(status)) {
		logger_t logger = gru_logger_get();

		logger(INFO, "Unable to read queue stats");

		return;
	}
	
}