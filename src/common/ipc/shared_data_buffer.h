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
#ifndef SHARED_DATA_BUFFER_H
#define SHARED_DATA_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>

#include <common/gru_alloc.h>
#include <common/gru_status.h>
#include <log/gru_logger.h>

typedef enum shr_buff_perm_t_ {
	BUFF_READ,
	BUFF_WRITE,
} shr_buff_perm_t;

typedef struct shr_data_buff_t_ {
	char *name;
	void *ptr;
	sem_t *sem_read;
	int fd;
	shr_buff_perm_t perm;
} shr_data_buff_t;

/**
 * Creates a new shared data buffer
 * @param perm Permission (read / write)
 * @param len lenght, in bytes
 * @param name buffer name
 * @param status status structure in case of error
 * @return A pointer to a new shared data buffer or false otherwise
 */
volatile shr_data_buff_t *shr_buff_new(shr_buff_perm_t perm,
	size_t len,
	const char *name,
	gru_status_t *status);

/**
 * Destroys a shared data buffer
 * @param ptr a pointer to a pointer of a shared ptr
 */
void shr_buff_detroy(volatile shr_data_buff_t **ptr);

/**
 * Atomic read of a data buffer
 * @param src Source buffer
 * @param dest destination pointer
 * @param len number of bytes to read
 */
bool shr_buff_read(const volatile shr_data_buff_t *src, void *dest, size_t len);

/**
 * Atomic write of a data buffer
 * @param dest destination buffer
 * @param src source pointer
 * @param len number of bytes to write
 */
bool shr_buff_write(volatile shr_data_buff_t *dest, void *src, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* SHARED_DATA_BUFFER_H */
