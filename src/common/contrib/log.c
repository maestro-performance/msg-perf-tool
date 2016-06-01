/**
 Copyright 2015 Otavio Rodolfo Piske
 
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
#include "log.h"

/**
 * Checks whether a given file exists
 * @param filename the filename
 * @return
 */
bool exists(const char *filename)
{
	int ret = 0;
	struct stat info;

	ret = stat(filename, &info);
	if (ret == 0) {
		return true;
	}

	return false;
}

/**
 * Checks whether can read/write a given file
 * @param filename the filename
 * @return
 */
static bool can_read_write(const char *filename)
{
	int ret = 0;


	ret = access(filename, R_OK | W_OK);
	if (ret < 0) {
		logger_t logger = get_logger();
		switch (errno) {
		case ENOENT:
		{
			logger(ERROR, "No such file or directory %s: %s",
				filename, strerror(errno));
			return false;
		}
		case EACCES:
		{
			logger(ERROR, "Access denied (no read/write permission) %s: %s",
				filename, strerror(errno));
			return false;
		}
		default:
		{
			logger(ERROR, "Unhandled IO error trying to access %s: %s",
				filename, strerror(errno));
			return false;
		}
		}
	}

	return true;
}

static bool rename_if_exists(const char *filename)
{
	logger_t logger = get_logger();

	if (!exists(filename)) {
		return true;
	}

	if (!can_read_write(filename)) {
		return false;
	}

	int size = strlen(filename) + 16;
	char *new_file = (char *) malloc(size);

	if (!new_file) {
		logger(ERROR,
			"Not enough memory to allocate for renaming the existing log file");
		return false;
	}

	int i = 0;
	do {
		bzero(new_file, size);
		snprintf(new_file, size, "%s.%03i", filename, i);

		if (!exists(new_file)) {
			int ret = 0;

			ret = rename(filename, new_file);
			if (ret != 0) {
				logger(ERROR, "Unable to rename log file: %s",
					strerror(errno));

				free(new_file);
				return false;
			}

			break;
		}
		i++;
	} while (true);

	free(new_file);
	return true;
}

static bool remap_io(const char *dir, const char *name, FILE *fd)
{
	char *fullpath;
	size_t size = strlen(dir) + APPEND_SIZE_REMAP;
	logger_t logger = get_logger();

	fullpath = (char *) malloc(size);
	if (fullpath == NULL) {
		logger(ERROR, "Unable to remap IO: not enough free memory");

		return false;
	}

	bzero(fullpath, size);
	snprintf(fullpath, size - 1, "%s/%s", dir, name);

	if (!rename_if_exists(fullpath)) {
		return false;
	}

	FILE *f = freopen(fullpath, "a", fd);
	if (f == NULL) {
		logger(ERROR, "Unable to remap error IO: %s (%s)", strerror(errno),
			fullpath);

		return false;
	}

	logger(INFO, "Log file successfully opened");

	free(fullpath);
	return true;
}

bool remap_log(const char *dir, const char *base_name, pid_t pid, FILE *fd)
{
	char name[32];

	bzero(name, sizeof(name));
	snprintf(name, sizeof(name) - 1, "%s-%d.log", base_name, pid);

	return remap_io(dir, name, fd);
}
