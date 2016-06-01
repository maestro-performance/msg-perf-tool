#include "vmsl.h"


vmsl_t *vmsl_init() {
    logger_t logger = get_logger();
    
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
}