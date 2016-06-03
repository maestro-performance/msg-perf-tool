#ifndef VMSL_H
#define VMSL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "contrib/logger.h"

#include "msgctxt.h"
#include "statistics.h"

  
typedef msg_ctxt_t *(*msg_init)(void *data);
typedef void(*msg_send)(msg_ctxt_t *ctxt, msg_content_loader content_loader);
typedef void(*msg_subscribe)(msg_ctxt_t *ctxt, void *data);
typedef void(*msg_receive)(msg_ctxt_t *ctxt, msg_content_data_t *content);
typedef void(*msg_stop)(msg_ctxt_t *ctxt);
typedef void(*msg_commit)(msg_ctxt_t *ctxt, void *data);
typedef void(*msg_destroy)(msg_ctxt_t *);

typedef struct vmsl_t_ {
    msg_init init;
    msg_send send;
    msg_subscribe subscribe;
    msg_receive receive;
    msg_commit commit;
    msg_stop stop;
    msg_destroy destroy;
} vmsl_t;

vmsl_t *vmsl_init();
void vmsl_destroy(vmsl_t **vmsl);


#ifdef __cplusplus
}
#endif

#endif /* VMSL_H */

