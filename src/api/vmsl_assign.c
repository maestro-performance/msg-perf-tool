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
#include "vmsl_assign.h"

/**
 * Contains the ugly code to assign the VMSL functions based on the available
 * protocols.
 */

static bool vmsl_assign_proton(vmsl_t *vmsl) {
	logger_t logger = gru_logger_get();

#ifdef __AMQP_SUPPORT__
	logger(INFO, "Initializing AMQP protocol");

	vmsl->init = proton_init;
	vmsl->receive = proton_receive;
	vmsl->subscribe = proton_subscribe;
	vmsl->send = proton_send;
	vmsl->stop = proton_stop;
	vmsl->destroy = proton_destroy;

	return true;
#else
	logger(ERROR, "AMQP protocol support was not enabled");
	return false;
#endif // __AMQP_SUPPORT__
}

static bool vmsl_assign_stomp(vmsl_t *vmsl) {
	logger_t logger = gru_logger_get();

#ifdef __STOMP_SUPPORT__
	logger(INFO, "Initializing STOMP protocol");

	vmsl->init = litestomp_init;
	vmsl->receive = litestomp_receive;
	vmsl->subscribe = litestomp_subscribe;
	vmsl->send = litestomp_send;
	vmsl->stop = litestomp_stop;
	vmsl->destroy = litestomp_destroy;

	return true;
#else
	logger(ERROR, "STOMP protocol support was not enabled");
	return false;
#endif // __STOMP_SUPPORT__
}


static bool vmsl_assign_mqtt(vmsl_t *vmsl) {
	logger_t logger = gru_logger_get();

#ifdef __AMQP_SUPPORT__
	logger(INFO, "Initializing MQTT protocol");

	vmsl->init = paho_init;
	vmsl->receive = paho_receive;
	vmsl->subscribe = paho_subscribe;
	vmsl->send = paho_send;
	vmsl->stop = paho_stop;
	vmsl->destroy = paho_destroy;

	return true;
#else
	logger(ERROR, "MQTT protocol support was not enabled");
	return false;
#endif // __AMQP_SUPPORT__
}

bool vmsl_assign_by_url(const char *url, vmsl_t *vmsl) {
	logger_t logger = gru_logger_get();

	if (strncmp(url, "amqp://", 7) == 0) {
		return vmsl_assign_proton(vmsl);
	} else {
		if (strncmp(url, "stomp://", 8) == 0) {
			return vmsl_assign_stomp(vmsl);
		}
		else {
			if (strncmp(url, "mqtt://", 7) == 0) {
				return vmsl_assign_mqtt(vmsl);
			}
		}
	}

	logger(ERROR, "Unsupported protocol or invalid URL");
	return false;
}