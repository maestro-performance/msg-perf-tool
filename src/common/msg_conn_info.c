/**
 *   Copyright 2017 Otavio Rodolfo Piske
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#include "msg_conn_info.h"


void msg_conn_info_gen_id(msg_conn_info_t *conn_info) {
	uuid_t id;
	const size_t uuid_size = 37;

	uuid_generate(id);

	conn_info->id = gru_alloc(uuid_size, NULL);
	uuid_unparse_lower(id, conn_info->id);
}


void msg_conn_info_gen_id_char(char **out) {
	uuid_t id;
	const size_t uuid_size = 37;

	uuid_generate(id);

	*out = gru_alloc(uuid_size, NULL);
	uuid_unparse_lower(id, *out);
}