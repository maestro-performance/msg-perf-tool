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


#ifndef MPT_MAESTRO_TOPICS_H
#define MPT_MAESTRO_TOPICS_H


#define MAESTRO_DEFAULT_QOS 1
#define MAESTRO_DAEMON_SHEETS_COUNT 3

#define MAESTRO_ALL_DAEMONS "/mpt/daemon"
#define MAESTRO_NOTIFICATIONS "/mpt/notifications"
#define MAESTRO_TOPIC "/mpt/maestro"

#define MAESTRO_SENDER_DAEMON_SHEETS { MAESTRO_ALL_DAEMONS, MAESTRO_NOTIFICATIONS, "/mpt/daemon/sender" }
#define MAESTRO_RECEIVER_DAEMON_SHEETS { MAESTRO_ALL_DAEMONS, MAESTRO_NOTIFICATIONS, "/mpt/daemon/receiver" }
#define MAESTRO_BROKERI_DAEMON_SHEETS { MAESTRO_ALL_DAEMONS, MAESTRO_NOTIFICATIONS, "/mpt/daemon/brokerd" }

#define MAESTRO_CLI_COUNT 2
#define MAESTRO_CLI_TOPICS { MAESTRO_TOPIC , MAESTRO_NOTIFICATIONS }


#define MAESTRO_SENDER_DAEMONS "/mpt/daemon/sender"
#define MAESTRO_RECEIVER_DAEMONS "/mpt/daemon/receiver"
#define MAESTRO_BROKER_INSPECTOR_DAEMONS "/mpt/daemon/brokerd"




#endif //MPT_MAESTRO_TOPICS_H
