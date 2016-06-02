/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "proton-wrapper.h"
#include "proton-context.h"

static bool failed(pn_messenger_t *messenger)
{
    if (pn_messenger_errno(messenger)) {
        return true;
    }

    return false;
}

static inline proton_ctxt_t *proton_ctxt_cast(msg_ctxt_t *ctxt)
{
    return (proton_ctxt_t *) ctxt->api_context;
}

msg_ctxt_t *proton_init(void *data)
{
    logger_t logger = get_logger();

    logger(DEBUG, "Initializing proton wrapper");

    msg_ctxt_t *msg_ctxt = msg_ctxt_init();
    if (!msg_ctxt) {
        logger(FATAL, "Unable to initialize the messaging context");

        exit(1);
    }

    proton_ctxt_t *proton_ctxt = proton_context_init();

    if (!proton_ctxt) {
        logger(FATAL, "Unable to initialize the proton context");

        exit(1);
    }

    pn_messenger_t *messenger = pn_messenger(NULL);

    logger(DEBUG, "Initializing the proton messenger");
    int err = pn_messenger_start(messenger);
    if (err) {
        logger(FATAL, "Unable to start the proton messenger");

        exit(1);
    }

    proton_ctxt->messenger = messenger;
    msg_ctxt->api_context = proton_ctxt;

    return msg_ctxt;
}

static void proton_check_status(pn_messenger_t *messenger, pn_tracker_t tracker)
{
    logger_t logger = get_logger();

    pn_status_t status = pn_messenger_status(messenger, tracker);

    logger(TRACE, "Checking message status");
    switch (status) {
    case PN_STATUS_UNKNOWN:
    {
        logger(DEBUG, "Message status unknown");
        break;
    }
    case PN_STATUS_PENDING:
    {
        logger(DEBUG, "Message status pending");
        break;
    }
    case PN_STATUS_ACCEPTED:
    {
        logger(DEBUG, "Message status accepted");
        break;
    }
    case PN_STATUS_REJECTED:
    {
        logger(DEBUG, "Message status rejected");
        break;
    }
    case PN_STATUS_RELEASED:
    {
        logger(DEBUG, "Message status released");
        break;
    }
    case PN_STATUS_MODIFIED:
    {
        logger(DEBUG, "Message status modified");
        break;
    }
    case PN_STATUS_ABORTED:
    {
        logger(DEBUG, "Message status aborted");
        break;
    }
    case PN_STATUS_SETTLED:
    {
        logger(DEBUG, "Message status settled");
        break;
    }
    default:
    {
        logger(DEBUG, "Message status invalid");
        break;
    }
    }
}

static void proton_commit(pn_messenger_t *messenger)
{
    pn_tracker_t tracker = pn_messenger_outgoing_tracker(messenger);

    logger_t logger = get_logger();

    logger(DEBUG, "Committing the message delivery");

    proton_check_status(messenger, tracker);
    pn_messenger_settle(messenger, tracker, 0);
    proton_check_status(messenger, tracker);
}


static void proton_set_message_properties(pn_message_t *message) {
    logger_t logger = get_logger();
    const options_t *options = get_options_object();
    
    logger(DEBUG, "Setting message address to %s", options->url);
    pn_message_set_address(message, options->url);
    pn_message_set_durable(message, false);
    pn_message_set_ttl(message, 50000);

    pn_message_set_first_acquirer(message, true);
}


static void proton_set_message_data(pn_message_t *message) {
    logger_t logger = get_logger();
    
    logger(DEBUG, "Formatting message body");
    
    pn_data_t *body = pn_message_body(message);
    pn_data_put_string(body, pn_bytes(4, "1234"));
}

static void proton_set_outgoing_messenger_properties(proton_ctxt_t *proton_ctxt) {
    logger_t logger = get_logger();
    
    logger(DEBUG, "Setting outgoing window");
    pn_messenger_set_outgoing_window(proton_ctxt->messenger, 1);
}


static void proton_do_send(pn_messenger_t *messenger, pn_message_t *message) {
    logger_t logger = get_logger();
    
    logger(DEBUG, "Putting message");
    pn_messenger_put(messenger, message);
    if (failed(messenger)) {
        pn_error_t *error = pn_messenger_error(messenger);

        const char *protonErrorText = pn_error_text(error);
        logger(ERROR, protonErrorText);
    }


    if (failed(messenger)) {
        pn_error_t *error = pn_messenger_error(messenger);

        const char *protonErrorText = pn_error_text(error);
        logger(ERROR, protonErrorText);
    }

    pn_messenger_send(messenger, -1);
    if (failed(messenger)) {
        pn_error_t *error = pn_messenger_error(messenger);

        const char *protonErrorText = pn_error_text(error);
        logger(ERROR, protonErrorText);
    }

}

void proton_send(msg_ctxt_t *ctxt, void *data)
{
    logger_t logger = get_logger();

    logger(DEBUG, "Creating message object");
    pn_message_t *message = pn_message();


    proton_set_message_properties(message);
    proton_set_message_data(message);
    
    proton_ctxt_t *proton_ctxt = proton_ctxt_cast(ctxt);
    proton_set_outgoing_messenger_properties(proton_ctxt);
    
    proton_do_send(proton_ctxt->messenger, message);

    proton_commit(proton_ctxt->messenger);
}

static void proton_accept(pn_messenger_t *messenger) {
    pn_tracker_t tracker = pn_messenger_incoming_tracker(messenger);

    logger_t logger = get_logger();

    logger(DEBUG, "Accepting the message delivery");

    proton_check_status(messenger, tracker);
    pn_messenger_accept(messenger, tracker, 0);
    proton_check_status(messenger, tracker);
}

void proton_subscribe(msg_ctxt_t *ctxt, void *data) {
    logger_t logger = get_logger();
    const options_t *options = get_options_object();
    proton_ctxt_t *proton_ctxt = proton_ctxt_cast(ctxt);
    
    logger(DEBUG, "Subscribing to endpoint address at %s", options->url);
    pn_messenger_subscribe(proton_ctxt->messenger, options->url);
    if (failed(proton_ctxt->messenger)) {
        pn_error_t *error = pn_messenger_error(proton_ctxt->messenger);

        const char *protonErrorText = pn_error_text(error);
        logger(ERROR, protonErrorText);
    }
}



static void proton_set_incoming_messenger_properties(pn_messenger_t *messenger) {
    
    /*
     * By setting the incoming window to 1 it, basically, behaves as if 
     * it was working in an auto-accept mode
     */
    pn_messenger_set_incoming_window(messenger, 1);
}

static void proton_do_receive(pn_messenger_t *messenger, pn_message_t *message) {
    logger_t logger = get_logger();
    
    int limit = -1;

    logger(DEBUG, "Receiving at most %i messages", limit);
    pn_messenger_recv(messenger, limit);
    if (failed(messenger)) {
        pn_error_t *error = pn_messenger_error(messenger);

        const char *protonErrorText = pn_error_text(error);
        logger(ERROR, protonErrorText);
    }

    int incoming = pn_messenger_incoming(messenger);
    if (incoming == 0) {
        logger(WARNING, "There are 0 incoming messages");
        return 0;
    }

    logger(DEBUG, "Getting %i messages from proton buffer", incoming);
    
    pn_messenger_get(messenger, message);
    if (failed(messenger)) {
        pn_error_t *error = pn_messenger_error(messenger);

        const char *protonErrorText = pn_error_text(error);
        logger(ERROR, protonErrorText);
    }

    pn_data_t *body = pn_message_body(message);

    int buff_size = 1024;
    char *str = malloc(buff_size + 1);
    if (!str) {
        logger(DEBUG, "Unable to allocate memory for the buffer");

        
        exit(1);
    }
    bzero(str, buff_size);

    pn_data_format(body, str, &buff_size);
    if (failed(messenger)) {
        pn_error_t *error = pn_messenger_error(messenger);

        const char *protonErrorText = pn_error_text(error);
        logger(ERROR, protonErrorText);

        free(str);
        exit(1);
    }
    
    logger(DEBUG, "Received data (%d bytes): %s", buff_size, str);
    
    free(str);
}

void proton_receive(msg_ctxt_t *ctxt)
{
    proton_ctxt_t *proton_ctxt = proton_ctxt_cast(ctxt);

    proton_set_incoming_messenger_properties(proton_ctxt->messenger);
    
    pn_message_t *message = pn_message();
    proton_do_receive(proton_ctxt->messenger, message);
        
    proton_accept(proton_ctxt->messenger);

    
    

}