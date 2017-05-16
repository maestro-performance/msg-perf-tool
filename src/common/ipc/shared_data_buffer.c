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

#include "shared_data_buffer.h"

static int pagesize = 0;

static bool shr_data_buff_create_sem(shr_data_buff_t *buff,
	const char *name,
	int initial,
	gru_status_t *status) {
	char *rname = NULL;

	if (asprintf(&rname, "/%s-read", name) == -1) {
		gru_status_set(status, GRU_FAILURE, "Unable to format read semaphore name");

		return false;
	}

	buff->sem_read =
		sem_open(rname, O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, initial);
	if (buff->sem_read == SEM_FAILED) {
		gru_status_set(
			status, GRU_FAILURE, "Unable to open read semaphore: %s\n", strerror(errno));

		goto err_exit_1;
	}

	gru_dealloc_string(&rname);
	return true;

err_exit_1:
	gru_dealloc_string(&rname);

	return false;
}

static shr_data_buff_t *
	shr_buff_new_reader(size_t len, const char *name, gru_status_t *status) {
	const int sem_initial_value = 0; // The initial semaphore vaue for read

	shr_data_buff_t *ret = gru_alloc(sizeof(shr_data_buff_t), status);
	if (!ret) {
		gru_status_set(
			status, GRU_FAILURE, "Not enough memory for the shared data buffer");

		return NULL;
	}

	if (asprintf(&ret->name, "/%s", name) == -1) {
		gru_status_set(status, GRU_FAILURE, "Unable to format shared data buffer name");

		goto err_exit_1;
	}

	ret->fd = shm_open(ret->name, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (ret->fd < 0) {
		gru_status_set(
			status, GRU_FAILURE, "Unable to open shared memory: %s\n", strerror(errno));

		goto err_exit_2;
	}

#if defined __linux__
	ret->ptr = mmap((caddr_t) 0,
		pagesize,
		PROT_READ,
		MAP_SHARED | MAP_LOCKED | MAP_POPULATE,
		ret->fd,
		0);
#else
	ret->ptr =
		mmap((caddr_t) 0, pagesize, PROT_READ, MAP_SHARED | MAP_HASSEMAPHORE, ret->fd, 0);
#endif

	if (ret->ptr == (caddr_t)(-1)) {
		gru_status_set(status,
			GRU_FAILURE,
			"Unable to open memory mapped file: %s\n",
			strerror(errno));

		goto err_exit_3;
	}

	if (!shr_data_buff_create_sem(ret, name, sem_initial_value, status)) {
		goto err_exit_4;
	}

	ret->perm = BUFF_READ;
	return ret;

err_exit_4:
	munmap(ret->ptr, pagesize);

err_exit_3:
	shm_unlink(ret->name);
	close(ret->fd);

err_exit_2:
	gru_dealloc_string(&ret->name);

err_exit_1:
	gru_dealloc((void **) &ret);

	return NULL;
}

static shr_data_buff_t *
	shr_buff_new_writer(size_t len, const char *name, gru_status_t *status) {
	const int sem_initial_value = 1; // The initial semaphore vaue for write

	shr_data_buff_t *ret = gru_alloc(sizeof(shr_data_buff_t), status);
	if (!ret) {
		gru_status_set(
			status, GRU_FAILURE, "Not enough memory for the shared data buffer");

		return NULL;
	}

	if (asprintf(&ret->name, "/%s", name) == -1) {
		gru_status_set(status, GRU_FAILURE, "Unable to format shared data buffer name");

		goto err_exit_1;
	}

	ret->fd = shm_open(
		ret->name, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (ret->fd < 0) {
		gru_status_set(
			status, GRU_FAILURE, "Unable to open shared memory: %s\n", strerror(errno));

		goto err_exit_2;
	}

	if (ftruncate(ret->fd, pagesize) != 0) {
		gru_status_set(status,
			GRU_FAILURE,
			"Unable to truncate memory mapped file: %s",
			strerror(errno));

		goto err_exit_2;
	}

#if defined __linux__
	ret->ptr = mmap((caddr_t) 0,
		pagesize,
		PROT_WRITE,
		MAP_SHARED | MAP_LOCKED | MAP_POPULATE,
		ret->fd,
		0);
#else
	ret->ptr = mmap(
		(caddr_t) 0, pagesize, PROT_WRITE, MAP_SHARED | MAP_HASSEMAPHORE, ret->fd, 0);
#endif

	if (ret->ptr == (caddr_t)(-1)) {
		gru_status_set(status,
			GRU_FAILURE,
			"Unable to open memory mapped file: %s\n",
			strerror(errno));

		goto err_exit_3;
	}

	if (!shr_data_buff_create_sem(ret, name, sem_initial_value, status)) {
		goto err_exit_4;
	}

	ret->perm = BUFF_WRITE;
	return ret;

err_exit_4:
	munmap(ret->ptr, pagesize);

err_exit_3:
	close(ret->fd);

err_exit_2:
	gru_dealloc_string(&ret->name);

err_exit_1:
	gru_dealloc((void **) &ret);

	return NULL;
}

volatile shr_data_buff_t *shr_buff_new(shr_buff_perm_t perm,
	size_t len,
	const char *name,
	gru_status_t *status) {
	logger_t logger = gru_logger_get();

	if (pagesize == 0) {
		pagesize = getpagesize();
		logger(DEBUG, "System page size: %d. Requested size: %d", pagesize, len);
		if (len > pagesize) {
			gru_status_set(
				status, GRU_FAILURE, "The requested size is greater than the page size");

			return NULL;
		}
	}

	if (perm == BUFF_READ) {
		return shr_buff_new_reader(len, name, status);
	}

	return shr_buff_new_writer(len, name, status);
}

void shr_buff_detroy(volatile shr_data_buff_t **ptr) {
	volatile shr_data_buff_t *buff = *ptr;

	if (!buff) {
		return;
	}

	munmap(buff->ptr, pagesize);
	close(buff->fd);
	if (buff->perm == BUFF_READ) {
		shm_unlink(buff->name);
	}

	sem_close(buff->sem_read);

	gru_dealloc_string(&buff->name);
	gru_dealloc((void **) ptr);
}

bool shr_buff_read(const volatile shr_data_buff_t *src, void *dest, size_t len) {
#ifdef __linux__
	struct timespec ts;

	if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
		sem_trywait(src->sem_read);

		if (errno == EAGAIN) {
			return false;
		}
	} else {
		ts.tv_sec = ts.tv_sec + 2;
		sem_timedwait(src->sem_read, &ts);

		if (errno == ETIMEDOUT || errno == EAGAIN) {
			return false;
		}
	}
#else
	sem_trywait(src->sem_read);

	if (errno == EAGAIN) {
		return false;
	}

#endif // __linux__

	memcpy(dest, src->ptr, len);
	sem_post(src->sem_read);
	return true;
}

bool shr_buff_write(volatile shr_data_buff_t *dest, void *src, size_t len) {
	sem_trywait(dest->sem_read);
	if (errno == EAGAIN) {
		return false;
	}

	memcpy(dest->ptr, src, len);
	sem_post(dest->sem_read);

	return true;
}