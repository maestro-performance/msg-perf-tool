/**
 Copyright 2017 Otavio Rodolfo Piske

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
#include "proton-handlers.h"

void proton_set_properties(void *ctxt, void *payload) {
	pn_message_t *message = (pn_message_t*) payload;
	pn_data_t *properties = pn_message_properties(message);

	pn_data_enter(properties);
	pn_data_put_string(properties, pn_bytes(sizeof("key")-1, "key"));
	pn_data_put_string(properties, pn_bytes(sizeof("pair")-1, "pair"));
	pn_data_exit(properties);
}

// Probably only for debugging
void proton_log_body_type(void *ctxt, void *payload) {
	pn_message_t *message = (pn_message_t*) payload;
	pn_data_t *body = pn_message_body(message);

	if (!body) {
		return;
	}

	logger_t logger = gru_logger_get();

	switch (pn_data_type(body)) {
	case PN_STRING:
		logger(INFO, "Message body type is string");
		break;
	case PN_INT:
		logger(INFO, "Message body type is int");
		break;
	case PN_MAP:
		logger(INFO, "Message body type is map");
		break;
	case PN_LIST:
		logger(INFO, "Message body type is list");
		break;
	case PN_ARRAY:
		logger(INFO, "Message body type is array");
		break;
	case PN_NULL:
		logger(INFO, "Message body type is null");
		break;
	case PN_BOOL:
		logger(INFO, "Message body type is boolean");
		break;
	case PN_UBYTE:
		logger(INFO, "Message body type is unsigned byte");
		break;
	case PN_USHORT:
		logger(INFO, "Message body type is unsigned short");
		break;
	case PN_UINT:
		logger(INFO, "Message body type is unsigned int");
		break;
	case PN_ULONG:
		logger(INFO, "Message body type is unsigned long");
		break;
	case PN_LONG:
		logger(INFO, "Message body type is long");
		break;
	case PN_FLOAT:
		logger(INFO, "Message body type is float");
		break;
	case PN_BINARY:
		logger(INFO, "Message body type is binary");
		break;
	case PN_SYMBOL:
		logger(INFO, "Message body type is symbol");
		break;
	default:
		logger(INFO, "Message body type is undefined");
		break;
	}
}

void proton_set_content_type(void *ctxt, void *payload) {
	pn_message_t *message = (pn_message_t*) payload;

	pn_message_set_content_type(message, "text/plain");
}