
#include "include/blink.h"
#include "include/blink_resource_handler.h"

size_t blink_messages_resource_size(const blink_msg_t *msg)
{
    size_t size = 0;
    const blink_msg_t *m;
    for (uint8_t i = 0; (m = &msg[i])->data; i++) {
        size += m->data_len + 1; /* ; */
    }
    return size;
}

ssize_t blink_messages_to_resource(const blink_msg_t *msg, uint8_t *buffer, size_t size)
{
    size_t s = 0;
    const blink_msg_t *m;
    if (blink_messages_resource_size(msg) > size) {
        return -ENOBUFS;
    }
    for (uint8_t i = 0; (m = &msg[i])->data; i++) {
        memcpy(buffer + s, m->data, m->data_len);
        s += m->data_len;
        buffer[s++] = ';';
    }
    buffer[--s] = '\0';
    return (ssize_t)s;
}


/*
    For now to keepit simple, the content of this resource os of type text.
    The content looks like: <Key1>=<Value1>;<Key2>=<Value2> ...
    If thinks work out well, then the cotent type could be changed to cbor.
*/

ssize_t blink_data_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *arg)
{
    blink_data_res_handle_t *blink_data = (blink_data_res_handle_t *)arg;

    unsigned method_flag = coap_method2flag(coap_get_code_detail(pdu));

    if (method_flag == COAP_GET) {
        gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
        /* TODO: append ETag */
        coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
        /* TODO: use some kind of timestamp and append MAX_AGE */

        memcpy(pdu->payload, blink_data->data, blink_data->size);
        size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);
        resp_len += blink_data->size;
        return resp_len;
    }
    else if (method_flag == COAP_PUT) {
        unsigned int format = coap_get_content_type(pdu);
        if (format != COAP_FORMAT_TEXT) {
            return gcoap_response(pdu, buf, len, COAP_CODE_UNSUPPORTED_CONTENT_FORMAT);
        }
        else  if (pdu->payload_len <= blink_data->capacity) {
            memcpy(blink_data->data, pdu->payload, pdu->payload_len);
            blink_data->size = pdu->payload_len;
            return gcoap_response(pdu, buf, len, COAP_CODE_CHANGED);
        }
    }
    return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
}

ssize_t blink_status_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *arg)
{
    blink_status_res_handle_t *blink_status = (blink_status_res_handle_t *)arg;

    unsigned method_flag = coap_method2flag(coap_get_code_detail(pdu));

    if (method_flag == COAP_GET) {
        gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
        /* TODO: append ETag */
        coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
        /* TODO: use some kind of timestamp and append MAX_AGE */

        if (*(blink_status->blinking)) {
            *((char *)pdu->payload) = '1';
        }
        else {
            *((char *)pdu->payload) = '0';
        }
        size_t resp_len = 1 + coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);
        return resp_len;
    }
    else if (method_flag == COAP_PUT) {
        unsigned int format = coap_get_content_type(pdu);
        if (format != COAP_FORMAT_TEXT) {
            return gcoap_response(pdu, buf, len, COAP_CODE_UNSUPPORTED_CONTENT_FORMAT);
        }
        if (pdu->payload_len != 1) {
            return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
        }
        else {
            char c = *((char *)pdu->payload);
            if (c == '1') {
                *blink_status->blinking = 1;
                return gcoap_response(pdu, buf, len, COAP_CODE_CHANGED);
            }
            else if (c == '0') {
                *blink_status->blinking = 0;
                return gcoap_response(pdu, buf, len, COAP_CODE_CHANGED);
            }
        }
    }
    return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
}