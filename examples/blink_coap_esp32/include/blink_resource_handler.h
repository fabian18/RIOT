
#include <stdint.h>
#include <stdbool.h>

#include "net/gcoap.h"
#include "blink.h"

#define BLINK_DATA_BUFFER_SIZE      (64)

typedef struct {
    uint8_t *data;
    uint8_t size;
    uint8_t capacity;
} blink_data_res_handle_t;

typedef struct {
    bool *blinking;
} blink_status_res_handle_t;

size_t blink_messages_resource_size(const blink_msg_t *msg);
ssize_t blink_messages_to_resource(const blink_msg_t *msg, uint8_t *buffer, size_t size);
ssize_t blink_data_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *arg);
ssize_t blink_status_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *arg);
