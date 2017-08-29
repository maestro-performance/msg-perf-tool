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

bool mpt_init_bmic_ctxt(gru_uri_t uri, bmic_context_t *ctxt, gru_status_t *status) {
	logger_t logger = gru_logger_get();

	logger(GRU_INFO, "Resolved host to %s", uri.host);

	bool ret = bmic_context_init_simple(ctxt, uri.host, "admin", "admin", status);

	if (!ret) {
		bmic_context_cleanup(ctxt);
		return false;
	}

	// Load the capabilities just so that it is cached
	logger(GRU_DEBUG, "Caching broker capabilities");
	const bmic_exchange_t *cap = ctxt->api->capabilities_load(ctxt->handle, status);
	if (!cap) {
		bmic_context_cleanup(ctxt);
		return false;
	}

	logger(GRU_DEBUG, "BMIC context and broker management API handle initialized successfully");
	return true;
}

void mpt_get_queue_stats(const bmic_context_t *ctxt,
	const char *name,
	bmic_queue_stat_t *stat,
	gru_status_t *status) {
	const bmic_exchange_t *cap = ctxt->api->capabilities_load(ctxt->handle, status);

	if (!cap) {
		logger_t logger = gru_logger_get();

		logger(GRU_INFO, "Unable to load capabilities");
		return;
	}

	*stat = ctxt->api->queue_stats(ctxt->handle, cap, name, status);
	if (gru_status_error(status)) {
		logger_t logger = gru_logger_get();

		logger(GRU_INFO, "Unable to read queue stats");

		return;
	}
}

void mpt_get_mem_info(const bmic_context_t *ctxt,
	bmic_java_memory_model_t memory_model,
	mpt_java_mem_t *out,
	gru_status_t *status) {
	bmic_api_interface_t *api = ctxt->api;

	out->eden = api->java.eden_info(ctxt->handle, status);
	out->survivor = api->java.survivor_info(ctxt->handle, status);
	out->tenured = api->java.tenured_info(ctxt->handle, status);

	if (memory_model == BMIC_JAVA_MODERN) {
		out->metaperm = api->java.metaspace_info(ctxt->handle, status);
	} else {
		out->metaperm = api->java.permgen_info(ctxt->handle, status);
	}
}

bool mpt_purge_queue(const bmic_context_t *ctxt, const char *name, gru_status_t *status) {
	logger_t logger = gru_logger_get();

	const bmic_exchange_t *cap = ctxt->api->capabilities_load(ctxt->handle, status);
	if (!cap) {
		logger(GRU_ERROR, "Unable to load capabilities: %s", status->message);
		return false;
	}

	bool ret = ctxt->api->queue_purge(ctxt->handle, cap, name, status);
	if (!ret && gru_status_error(status)) {
		logger(GRU_ERROR, "Unable to purge queue: %s", status->message);
	}

	ret = ctxt->api->queue_reset(ctxt->handle, cap, name, status);
	if (!ret && gru_status_error(status)) {
		logger(GRU_ERROR, "Unable to reset queue counters: %s", status->message);
	}

	return ret;
}