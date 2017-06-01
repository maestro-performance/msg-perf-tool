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
#ifndef MPT_MAESTRO_MSGPACK_UTIL_H_
#define MPT_MAESTRO_MSGPACK_UTIL_H_

#include <msgpack.h>

#ifdef __cplusplus
extern "C" {
#endif

#if MSGPACK_VERSION_MAJOR == 0

 #define msgpack_pack_char(pk, d) msgpack_pack_int(pk, d)
 #define msgpack_pack_str(pk, d) msgpack_pack_raw(pk, d)
 #define msgpack_pack_str_body(pk, v, d) msgpack_pack_raw_body(pk, v, d)

 #define MSGPACK_OBJECT_STR MSGPACK_OBJECT_RAW
 #define MSGPACK_OBJECT_FLOAT MSGPACK_OBJECT_DOUBLE

#endif // HAVE_MSGPACK_STR_BODY

#ifdef __cplusplus
}
#endif


#endif //MPT_MAESTRO_MSGPACK_UTIL_H_
