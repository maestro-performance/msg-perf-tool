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

#ifdef __AMQP_SUPPORT__
extern bool proton_vmsl_assign(vmsl_t *vmsl);
#else
 vmsl_assign_none(proton_vmsl_assign, "AMQP")
#endif


#ifdef __MQTT_SUPPORT__
extern bool paho_vmsl_assign(vmsl_t *vmsl);
#else
vmsl_assign_none(paho_vmsl_assign, "MQTT")
#endif

#ifdef __STOMP_SUPPORT__
extern bool litestomp_vmsl_assign(vmsl_t *vmsl);
#else
vmsl_assign_none(litestomp_vmsl_assign, "STOMP")
#endif


bool vmsl_assign_by_url(const char *url, vmsl_t *vmsl) {
	logger_t logger = gru_logger_get();

	if (strncmp(url, "amqp://", 7) == 0) {
		return proton_vmsl_assign(vmsl);
	} else {
		if (strncmp(url, "stomp://", 8) == 0) {
			return litestomp_vmsl_assign(vmsl);
		}
		else {
			if (strncmp(url, "mqtt://", 7) == 0) {
				return paho_vmsl_assign(vmsl);
			}
		}
	}

	logger(ERROR, "Unsupported protocol or invalid URL");
	return false;
}