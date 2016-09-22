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
#include "config.h"


static char *log_level_str[7] = {
    "trace", 
    "debug",
    "info",
    "stat",
    "warning",
    "error",
    "fatal",
};

void initialize_options(void *data) {
    options_t *options = (options_t *) data;
    
    options_set_defaults(options);
}

void save_options(FILE *file, void *data) {
    options_t *options = (options_t *) data;
   
    gru_config_write_string("broker.url", file, options->url);
    
    gru_config_write_ulong("message.count", file, options->count);
    gru_config_write_uint("message.throttle", file, options->throttle);
    gru_config_write_uint("message.size", file, options->message_size);
    
    gru_config_write_ushort("parallel.count", file, options->parallel_count);
    gru_config_write_ulong("test.duration", file, 
                           gru_duration_minutes(options->duration, NULL));
    
    gru_config_write_string("log.level", file, 
                            log_level_str[options->log_level]);
    gru_config_write_string("log.dir", file, options->logdir);
    
    fflush(file);
}

void read_options(FILE *file, void *data) {
    options_t *options = (options_t *) data;
    
    gru_config_read_string("broker_url", file, options->url);
    gru_config_read_ulong("message.count", file, &options->count);
     
    gru_config_read_ulong("message.count", file, &options->count);
    gru_config_read_uint("message.throttle", file, &options->throttle);
    gru_config_read_ulong("message.size", file, &options->message_size);
    
    gru_config_read_ushort("parallel.count", file, &options->parallel_count);
    
    uint64_t duration_minutes;
    gru_config_read_ulong("test.duration", file, &duration_minutes);
    options->duration = gru_duration_from_minutes(duration_minutes);
    
    char log_level_s[OPT_MAX_STR_SIZE] = {0};
    gru_config_read_string("log.level", file, log_level_s);
    options->log_level = gru_logger_get_level(log_level_s);
    
    gru_config_read_string("log.dir", file, options->logdir);
}

void config_init(options_t *options) {
    gru_status_t status = {0};
    gru_payload_t *payload = gru_payload_init(initialize_options, 
                                              save_options, read_options,
                                              &options,
                                              &status);
    
    if (!payload) {
        fprintf(stderr, "Unable to initialize the payload: %s\n", 
                status.message);
        
        gru_payload_destroy(&payload);
        return EXIT_FAILURE;
    }
    
    gru_config_t *config = gru_config_init(".", "test.cfg", payload,
                                           &status);

    if (!config) {
        fprintf(stderr, "Unable to initialize the configuration: %s\n",
                status.message);

        gru_payload_destroy(&payload);
        return EXIT_FAILURE;
    }
    
    if (!gru_path_fexists(fileno(config->file), &status)) {
        if (status.code != GRU_SUCCESS) {
            fprintf(stderr, "Unable to check if file exists: %s\n", 
                    status.message);
        }

        gru_payload_destroy(&payload);
        gru_config_destroy(&config);
        return EXIT_FAILURE;
    }

}