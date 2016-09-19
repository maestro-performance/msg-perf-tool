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
#include "vmsl.h"


vmsl_t *vmsl_init() {
    logger_t logger = gru_logger_get();
    
    logger(DEBUG, "Initializing virtual messaging system layer");
    
    vmsl_t *ret = malloc(sizeof(vmsl_t));
    if (!ret) {
        logger(FATAL, "Unable to initialize the virtual messaging system layer");
        
        exit(1);
    }
    
    return ret;
}


void vmsl_destroy(vmsl_t **vmsl) {
    free(*vmsl);
    *vmsl = NULL;
}