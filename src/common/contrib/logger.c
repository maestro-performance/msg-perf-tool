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

#include "logger.h"

static logger_t msg = NULL;

bool can_log(log_level_t current, log_level_t minimum)
{
    if (current >= minimum) {
        return true;
    }

    return false;
}



log_level_t get_log_level(const char *str) {
    if (strncasecmp("TRACE", str, strlen(str)) == 0) {
        return TRACE;
    }
    
    if (strncasecmp("DEBUG", str, strlen(str)) == 0) {
        return DEBUG;
    }
    
    if (strncasecmp("INFO", str, strlen(str)) == 0) {
        return INFO;
    }
        
    if (strncasecmp("WARNING", str, strlen(str)) == 0) {
        return WARNING;
    }
    
    if (strncasecmp("ERROR", str, strlen(str)) == 0) {
        return ERROR;
    }
    
    if (strncasecmp("FATAL", str, strlen(str)) == 0) {
        return FATAL;
    }
    
    if (strncasecmp("STAT", str, strlen(str)) == 0) {
        return STAT;
    }
    
    
    fprintf(stderr, "Invalid log level %s\n. Using INFO as default", str);
    return INFO;
}

void set_logger(logger_t new_msg) {
	msg = new_msg;
}


logger_t get_logger(void) {
	return msg;
}
