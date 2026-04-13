/*
 * SPDX-FileCopyrightText: 2026 ML!PA Consulting GmbH.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @defgroup    sys_configuration_cfg_ieee802154_security_sid   IEEE 802.15.4 security configuration SID definitions
 * @ingroup     sys_configuration
 * @{
 *
 * @file
 * @brief       IEEE 802.15.4 security configuration SID definitions
 *
 * @author      Fabian Hüßler <fabian.huessler@mlpa.com>
 *
 * @}
 */

#include <stddef.h>

#include "auto_init_utils.h"
#include "macros/utils.h"
#include "configuration.h"

#include "cfg_ieee802154_security.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONFIG_IEEE802154_SECURITY_KEYS_NUMOF
#define CONFIG_IEEE802154_SECURITY_KEYS_NUMOF           CONFIG_IEEE802154_SEC_DEFAULT_KEYSTORE_SIZE
#endif
#ifndef CONFIG_IEEE802154_SECURITY_KEYLOOKUP_NUMOF
#define CONFIG_IEEE802154_SECURITY_KEYLOOKUP_NUMOF      CONFIG_IEEE802154_SEC_DEFAULT_KEYLOOKUP_SIZE
#endif
#ifndef CONFIG_IEEE802154_SECURITY_PEERS_NUMOF
#define CONFIG_IEEE802154_SECURITY_PEERS_NUMOF          CONFIG_IEEE802154_SEC_DEFAULT_DEVSTORE_SIZE
#endif
#ifndef CONFIG_IEEE802154_SECURITY_PEERLOOKUP_NUMOF
#define CONFIG_IEEE802154_SECURITY_PEERLOOKUP_NUMOF     CONFIG_IEEE802154_SEC_DEFAULT_DEVSTORE_SIZE
#endif
#ifndef CONFIG_IEEE802154_SECURITY_KEY_BUF_MAX
#define CONFIG_IEEE802154_SECURITY_KEY_BUF_MAX          80
#endif

#ifndef CONFIG_IEEE802154_SECURITY_LOWER_SID
#error "CONFIG_IEEE802154_SECURITY_LOWER_SID must be defined before including cfg_ieee802154_security.h"
#endif

/* keystore */
#ifdef CONFIG_IEEE802154_SECURITY_KEYSTORE_LOWER_SID
#undef CONFIG_IEEE802154_SECURITY_KEYSTORE_LOWER_SID
#endif
#define CONFIG_IEEE802154_SECURITY_KEYSTORE_LOWER_SID                                       (CONFIG_IEEE802154_SECURITY_LOWER_SID + 1)

#  ifdef CONFIG_IEEE802154_SECURITY_KEYSTORE_MASK_LOWER_SID
#  undef CONFIG_IEEE802154_SECURITY_KEYSTORE_MASK_LOWER_SID
#  endif
#  define CONFIG_IEEE802154_SECURITY_KEYSTORE_MASK_LOWER_SID                                (CONFIG_IEEE802154_SECURITY_KEYSTORE_LOWER_SID + 1)

#  ifdef CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_LOWER_SID
#  undef CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_LOWER_SID
#  endif
#  define CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_LOWER_SID                                (CONFIG_IEEE802154_SECURITY_KEYSTORE_MASK_LOWER_SID + 1)

#  define CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_INDEX_LOWER_SID                          1
#  define CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_INDEX_STRIDE                             2   /* reserve 2 SID per key: key */

#    ifdef CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_KEY_LOWER_SID
#    undef CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_KEY_LOWER_SID
#    endif
#    define CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_KEY_LOWER_SID                          (CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_LOWER_SID + \
                                                                                            CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_INDEX_LOWER_SID + 1)
#  ifdef CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_UPPER_SID
#  undef CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_UPPER_SID
#  endif
#  define CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_UPPER_SID                                (CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_LOWER_SID + \
                                                                                            CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_INDEX_LOWER_SID + 1 + \
                                                                                            CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_INDEX_STRIDE * CONFIG_IEEE802154_SECURITY_KEYS_NUMOF - 1)
#ifdef CONFIG_IEEE802154_SECURITY_KEYSTORE_UPPER_SID
#undef CONFIG_IEEE802154_SECURITY_KEYSTORE_UPPER_SID
#endif
#define CONFIG_IEEE802154_SECURITY_KEYSTORE_UPPER_SID                                       (CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_UPPER_SID)

/* key_lookup_table */
#ifdef CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_LOWER_SID
#undef CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_LOWER_SID
#endif
#define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_LOWER_SID                               (CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_UPPER_SID + 1)

#  ifdef CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_MASK_LOWER_SID
#  undef CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_MASK_LOWER_SID
#  endif
#  define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_MASK_LOWER_SID                        (CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_LOWER_SID + 1)

#  ifdef CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_LOWER_SID
#  undef CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_LOWER_SID
#  endif
#  define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_LOWER_SID                   (CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_MASK_LOWER_SID + 1)

#  define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_INDEX_LOWER_SID             1
#  define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_INDEX_STRIDE                5   /* reserve 5 SID per key lookup: key_mode, key_lookup, key, fc  */

#    ifdef CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_MODE_LOWER_SID
#    undef CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_MODE_LOWER_SID
#    endif
#    define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_MODE_LOWER_SID        (CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_LOWER_SID + \
                                                                                            CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_INDEX_LOWER_SID + 1)

#    ifdef CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_LOOKUP_LOWER_SID
#    undef CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_LOOKUP_LOWER_SID
#    endif
#    define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_LOOKUP_LOWER_SID      (CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_MODE_LOWER_SID + 1)

#    ifdef CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_LOWER_SID
#    undef CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_LOWER_SID
#    endif
#    define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_LOWER_SID             (CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_LOOKUP_LOWER_SID + 1)

#    ifdef CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_FC_LOWER_SID
#    undef CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_FC_LOWER_SID
#    endif
#    define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_FC_LOWER_SID              (CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_LOWER_SID + 1)

#  ifdef CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_UPPER_SID
#  undef CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_UPPER_SID
#  endif
#  define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_UPPER_SID                   (CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_LOWER_SID + \
                                                                                            CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_INDEX_LOWER_SID + 1 + \
                                                                                            CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_INDEX_STRIDE * CONFIG_IEEE802154_SECURITY_KEYLOOKUP_NUMOF - 1)
#ifdef CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_UPPER_SID
#undef CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_UPPER_SID
#endif
#define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_UPPER_SID                               (CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_UPPER_SID)

/* devstore */
#ifdef CONFIG_IEEE802154_SECURITY_DEVSTORE_LOWER_SID
#undef CONFIG_IEEE802154_SECURITY_DEVSTORE_LOWER_SID
#endif
#define CONFIG_IEEE802154_SECURITY_DEVSTORE_LOWER_SID                                       (CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_UPPER_SID + 1)

#  ifdef CONFIG_IEEE802154_SECURITY_DEVSTORE_MASK_LOWER_SID
#  undef CONFIG_IEEE802154_SECURITY_DEVSTORE_MASK_LOWER_SID
#  endif
#  define CONFIG_IEEE802154_SECURITY_DEVSTORE_MASK_LOWER_SID                                (CONFIG_IEEE802154_SECURITY_DEVSTORE_LOWER_SID + 1)

#  ifdef CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_LOWER_SID
#  undef CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_LOWER_SID
#  endif
#  define CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_LOWER_SID                               (CONFIG_IEEE802154_SECURITY_DEVSTORE_MASK_LOWER_SID + 1)

#  define CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_INDEX_LOWER_SID                         1
#  define CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_INDEX_STRIDE                            4   /* reserve 4 SID per peer: pan, short, long */

#    ifdef CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_PAN_ID_LOWER_SID
#    undef CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_PAN_ID_LOWER_SID
#    endif
#    define CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_PAN_ID_LOWER_SID                      (CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_LOWER_SID + \
                                                                                            CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_INDEX_LOWER_SID + 1)

#    ifdef CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_SHORT_ADDR_LOWER_SID
#    undef CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_SHORT_ADDR_LOWER_SID
#    endif
#    define CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_SHORT_ADDR_LOWER_SID                  (CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_PAN_ID_LOWER_SID + 1)

#    ifdef CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_LONG_ADDR_LOWER_SID
#    undef CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_LONG_ADDR_LOWER_SID
#    endif
#    define CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_LONG_ADDR_LOWER_SID                   (CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_SHORT_ADDR_LOWER_SID + 1)

#  ifdef CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_UPPER_SID
#  undef CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_UPPER_SID
#  endif
#  define CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_UPPER_SID                               (CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_LOWER_SID + \
                                                                                            CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_INDEX_LOWER_SID + 1 + \
                                                                                            (CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_INDEX_STRIDE * CONFIG_IEEE802154_SECURITY_PEERS_NUMOF) - 1)
#ifdef CONFIG_IEEE802154_SECURITY_DEVSTORE_UPPER_SID
#undef CONFIG_IEEE802154_SECURITY_DEVSTORE_UPPER_SID
#endif
#define CONFIG_IEEE802154_SECURITY_DEVSTORE_UPPER_SID                                       (CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_UPPER_SID)

/* peer_lookup_table */
#ifdef CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_LOWER_SID
#undef CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_LOWER_SID
#endif
#define CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_LOWER_SID                              (CONFIG_IEEE802154_SECURITY_DEVSTORE_UPPER_SID + 1)

#  ifdef CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_MASK_LOWER_SID
#  undef CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_MASK_LOWER_SID
#  endif
#  define CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_MASK_LOWER_SID                       (CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_LOWER_SID + 1)

#  ifdef CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_LOWER_SID
#  undef CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_LOWER_SID
#  endif
#  define CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_LOWER_SID                 (CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_MASK_LOWER_SID + 1)

#  define CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_INDEX_LOWER_SID           1
#  define CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_INDEX_STRIDE              4   /* reserve 4 SID per peer lookup: fc, peer, key */

#    ifdef CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_FC_LOWER_SID
#    undef CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_FC_LOWER_SID
#    endif
#    define CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_FC_LOWER_SID            (CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_LOWER_SID + \
                                                                                            CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_INDEX_LOWER_SID + 1)

#    ifdef CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_PEER_LOWER_SID
#    undef CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_PEER_LOWER_SID
#    endif
#    define CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_PEER_LOWER_SID          (CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_FC_LOWER_SID + 1)

#    ifdef CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_KEY_LOWER_SID
#    undef CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_KEY_LOWER_SID
#    endif
#    define CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_KEY_LOWER_SID           (CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_PEER_LOWER_SID + 1)

#  ifdef CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_UPPER_SID
#  undef CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_UPPER_SID
#  endif
#  define CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_UPPER_SID                 (CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_LOWER_SID + \
                                                                                            CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_INDEX_LOWER_SID + 1 + \
                                                                                            CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_INDEX_STRIDE * CONFIG_IEEE802154_SECURITY_PEERLOOKUP_NUMOF - 1)
#ifdef CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_UPPER_SID
#undef CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_UPPER_SID
#endif
#define CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_UPPER_SID                              (CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_UPPER_SID)

#ifdef CONFIG_IEEE802154_SECURITY_UPPER_SID
#undef CONFIG_IEEE802154_SECURITY_UPPER_SID
#endif
#define CONFIG_IEEE802154_SECURITY_UPPER_SID                                                (CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_UPPER_SID)

/* define this if no instance should be created */
#ifndef _CFG_NO_INSTANCE

#ifndef CONFIG_IEEE802154_SECURITY_NAME_PREFIX
#error "CONFIG_IEEE802154_SECURITY_NAME_PREFIX must be defined before including cfg_ieee802154_security.h"
#endif

#ifndef CONFIG_IEEE802154_SECURITY_DATA_LOC
#error "CONFIG_IEEE802154_SECURITY_DATA_LOC must be defined before including cfg_ieee802154_security.h"
#endif

/* security */

#define CONFIG_IEEE802154_SECURITY_INSTANCE_HANDLER_NODE_ID_NAME(prefix) \
    CONCAT(prefix, _security_handler_id)

static CONF_HANDLER_NODE_ID(CONFIG_IEEE802154_SECURITY_INSTANCE_HANDLER_NODE_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                            CONFIG_IEEE802154_SECURITY_LOWER_SID, CONFIG_IEEE802154_SECURITY_UPPER_SID,
                            "security");

#define CONFIG_IEEE802154_SECURITY_INSTANCE_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_handler)

static CONF_HANDLER(CONFIG_IEEE802154_SECURITY_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                    &CONFIG_IEEE802154_SECURITY_INSTANCE_HANDLER_NODE_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                    NULL,
                    &configuration_default_backend_ops,
                    &configuration_default_backend_data_ops,
                    0,
                    sizeof(cfg_ieee802154_security_t),
                    (CONFIG_IEEE802154_SECURITY_DATA_LOC));


/* security/keystore */

#define CONFIG_IEEE802154_SECURITY_KEYSTORE_INSTANCE_HANDLER_ID_NAME(prefix) \
    CONCAT(prefix, _security_keystore_handler_id)

static CONF_HANDLER_NODE_ID(CONFIG_IEEE802154_SECURITY_KEYSTORE_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                            CONFIG_IEEE802154_SECURITY_KEYSTORE_LOWER_SID, CONFIG_IEEE802154_SECURITY_KEYSTORE_UPPER_SID,
                            "keystore");

#define CONFIG_IEEE802154_SECURITY_KEYSTORE_INSTANCE_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_keystore_handler)

static CONF_HANDLER(CONFIG_IEEE802154_SECURITY_KEYSTORE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                    &CONFIG_IEEE802154_SECURITY_KEYSTORE_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                    NULL,
                    &configuration_default_backend_ops,
                    &configuration_default_backend_data_ops,
                    0,
                    sizeof(ieee802154_sec_key_table_t),
                    (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, keystore));

/* security/keystore/mask */

#define CONFIG_IEEE802154_SECURITY_KEYSTORE_MASK_INSTANCE_HANDLER_ID_NAME(prefix) \
    CONCAT(prefix, _security_keystore_mask_handler_id)

static CONF_HANDLER_ID(CONFIG_IEEE802154_SECURITY_KEYSTORE_MASK_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                       CONFIG_IEEE802154_SECURITY_KEYSTORE_MASK_LOWER_SID,
                       "mask");

#define CONFIG_IEEE802154_SECURITY_KEYSTORE_MASK_INSTANCE_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_keystore_mask_handler)

static CONF_PRIMITIVE_HANDLER(CONFIG_IEEE802154_SECURITY_KEYSTORE_MASK_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              &CONFIG_IEEE802154_SECURITY_KEYSTORE_MASK_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              NULL,
                              &configuration_default_backend_ops,
                              &configuration_default_backend_data_ops,
                              CONF_PRIM_TYPE_BSTR,
                              sizeof(((ieee802154_sec_key_table_t *)0)->mask),
                              (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, keystore.mask));

/* security/keystore/keys[] */

#define CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_INSTANCE_ARRAY_ID_NAME(prefix) \
    CONCAT(prefix, _security_keystore_keys_handler_id)

static CONF_HANDLER_ARRAY_ID(CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_INSTANCE_ARRAY_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                            CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_LOWER_SID, CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_UPPER_SID, \
                            CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_INDEX_STRIDE, "keys");

#define CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_INSTANCE_ARRAY_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_keystore_keys_handler)

static CONF_ARRAY_HANDLER(CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_INSTANCE_ARRAY_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                          &CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_INSTANCE_ARRAY_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                          NULL,
                          &configuration_default_backend_ops,
                          &configuration_default_backend_data_ops,
                          0,
                          sizeof(ieee802154_sec_key_t),
                          (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, keystore.keys),
                          CONFIG_IEEE802154_SECURITY_KEYS_NUMOF);

/* security/keystore/keys[i].key */

#define CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_KEY_INSTANCE_HANDLER_ID_NAME(prefix) \
    CONCAT(prefix, _security_keystore_keys_key_handler_id)

static CONF_HANDLER_ID(CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_KEY_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                       CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_KEY_LOWER_SID,
                       "key");

#define CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_KEY_INSTANCE_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_keystore_keys_key_handler)

static CONF_PRIMITIVE_HANDLER(CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_KEY_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              &CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_KEY_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              NULL,
                              &configuration_default_backend_ops,
                              &configuration_default_backend_data_ops,
                              CONF_PRIM_TYPE_BSTR,
                              sizeof(((ieee802154_sec_key_t *)0)->key),
                              (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, keystore.keys[0].key));

/* security/key_lookup_table */

#define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_INSTANCE_HANDLER_ID_NAME(prefix) \
    CONCAT(prefix, _security_key_lookup_table_handler_id)

static CONF_HANDLER_NODE_ID(CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                            CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_LOWER_SID, CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_UPPER_SID,
                            "key_lookup_table");

#define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_INSTANCE_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_key_lookup_table_handler)

static CONF_HANDLER(CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                    &CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                    NULL,
                    &configuration_default_backend_ops,
                    &configuration_default_backend_data_ops,
                    0,
                    sizeof(ieee802154_sec_key_lookup_table_t),
                    (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, key_lookup_table));

/* security/key_lookup_table/mask */

#define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_MASK_INSTANCE_HANDLER_ID_NAME(prefix) \
    CONCAT(prefix, _security_key_lookup_table_mask_handler_id)

static CONF_HANDLER_ID(CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_MASK_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                       CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_MASK_LOWER_SID,
                       "mask");

#define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_MASK_INSTANCE_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_key_lookup_table_mask_handler)

static CONF_PRIMITIVE_HANDLER(CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_MASK_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              &CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_MASK_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              NULL,
                              &configuration_default_backend_ops,
                              &configuration_default_backend_data_ops,
                              CONF_PRIM_TYPE_BSTR,
                              sizeof(((ieee802154_sec_key_lookup_table_t *)0)->mask),
                              (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, key_lookup_table.mask));

/* security/key_lookup_table/keylookup[] */

#define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_INSTANCE_ARRAY_ID_NAME(prefix) \
    CONCAT(prefix, _security_keylookup_handler_id)

static CONF_HANDLER_ARRAY_ID(CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_INSTANCE_ARRAY_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                             CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_LOWER_SID, CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_UPPER_SID, \
                             CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_INDEX_STRIDE, "keylookup");

#define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_INSTANCE_ARRAY_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_key_lookup_table_keylookup_handler)

static CONF_ARRAY_HANDLER(CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_INSTANCE_ARRAY_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                          &CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_INSTANCE_ARRAY_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                          NULL,
                          &configuration_default_backend_ops,
                          &configuration_default_backend_data_ops,
                          0,
                          sizeof(ieee802154_sec_key_lookup_t),
                          (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, key_lookup_table.key_lookup),
                          CONFIG_IEEE802154_SECURITY_KEYLOOKUP_NUMOF);

/* security/key_lookup_table/keylookup[i].key_mode */

#define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_MODE_INSTANCE_HANDLER_ID_NAME(prefix) \
    CONCAT(prefix, _security_key_lookup_table_keylookup_key_mode_handler_id)

static CONF_HANDLER_ID(CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_MODE_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                       CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_MODE_LOWER_SID,
                       "key_mode");

#define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_MODE_INSTANCE_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_key_lookup_table_keylookup_key_mode_handler)

static CONF_PRIMITIVE_HANDLER(CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_MODE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              &CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_MODE_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              NULL,
                              &configuration_default_backend_ops,
                              &configuration_default_backend_data_ops,
                              CONF_PRIM_TYPE_UINT,
                              sizeof(((ieee802154_sec_key_lookup_t *)0)->key_mode),
                              (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, key_lookup_table.key_lookup[0].key_mode));

/* security/key_lookup_table/key_lookup[i].key_lookup */

#define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_LOOKUP_INSTANCE_HANDLER_ID_NAME(prefix) \
    CONCAT(prefix, _security_key_lookup_table_keylookup_key_lookup_handler_id)

static CONF_HANDLER_ID(CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_LOOKUP_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                       CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_LOOKUP_LOWER_SID,
                       "key_lookup");

#define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_LOOKUP_INSTANCE_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_key_lookup_table_keylookup_key_lookup_handler)

static CONF_PRIMITIVE_HANDLER(CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_LOOKUP_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              &CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_LOOKUP_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              NULL,
                              &configuration_default_backend_ops,
                              &configuration_default_backend_data_ops,
                              CONF_PRIM_TYPE_BSTR,
                              sizeof(((ieee802154_sec_key_lookup_t *)0)->key_lookup),
                              (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, key_lookup_table.key_lookup[0].key_lookup));

/* security/key_lookup_table/key_lookup[i].key */

#define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_INSTANCE_HANDLER_ID_NAME(prefix) \
    CONCAT(prefix, _security_key_lookup_table_keylookup_key_handler_id)

static CONF_HANDLER_ID(CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                       CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_LOWER_SID,
                       "key");

#define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_INSTANCE_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_key_lookup_table_keylookup_key_handler)

static CONF_PRIMITIVE_HANDLER(CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              &CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              NULL,
                              &configuration_default_backend_ops,
                              &configuration_default_backend_data_ops,
                              CONF_PRIM_TYPE_UINT,
                              sizeof(((ieee802154_sec_key_lookup_t *)0)->key),
                              (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, key_lookup_table.key_lookup[0].key));

/* security/key_lookup_table/key_lookup[i].fc */

#define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_FC_INSTANCE_HANDLER_ID_NAME(prefix) \
    CONCAT(prefix, _security_key_lookup_table_keylookup_fc_handler_id)

static CONF_HANDLER_ID(CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_FC_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                       CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_FC_LOWER_SID,
                       "fc");

#define CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_FC_INSTANCE_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_key_lookup_table_keylookup_fc_handler)

static CONF_PRIMITIVE_HANDLER(CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_FC_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              &CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_FC_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              NULL,
                              &configuration_default_backend_ops,
                              &configuration_default_backend_data_ops,
                              CONF_PRIM_TYPE_UINT,
                              sizeof(((ieee802154_sec_key_lookup_t *)0)->fc),
                              (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, key_lookup_table.key_lookup[0].fc));

/* security/devstore */

#define CONFIG_IEEE802154_SECURITY_DEVSTORE_INSTANCE_HANDLER_ID_NAME(prefix) \
    CONCAT(prefix, _security_devstore_handler_id)

static CONF_HANDLER_NODE_ID(CONFIG_IEEE802154_SECURITY_DEVSTORE_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                            CONFIG_IEEE802154_SECURITY_DEVSTORE_LOWER_SID, CONFIG_IEEE802154_SECURITY_DEVSTORE_UPPER_SID,
                            "devstore");

#define CONFIG_IEEE802154_SECURITY_DEVSTORE_INSTANCE_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_devstore_handler)

static CONF_HANDLER(CONFIG_IEEE802154_SECURITY_DEVSTORE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                    &CONFIG_IEEE802154_SECURITY_DEVSTORE_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                    NULL,
                    &configuration_default_backend_ops,
                    &configuration_default_backend_data_ops,
                    0,
                    sizeof(ieee802154_sec_peer_table_t),
                    (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, devstore));

/* security/devstore/mask */

#define CONFIG_IEEE802154_SECURITY_DEVSTORE_MASK_INSTANCE_HANDLER_ID_NAME(prefix) \
    CONCAT(prefix, _security_devstore_mask_handler_id)

static CONF_HANDLER_ID(CONFIG_IEEE802154_SECURITY_DEVSTORE_MASK_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                       CONFIG_IEEE802154_SECURITY_DEVSTORE_MASK_LOWER_SID,
                       "mask");

#define CONFIG_IEEE802154_SECURITY_DEVSTORE_MASK_INSTANCE_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_devstore_mask_handler)

static CONF_PRIMITIVE_HANDLER(CONFIG_IEEE802154_SECURITY_DEVSTORE_MASK_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              &CONFIG_IEEE802154_SECURITY_DEVSTORE_MASK_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              NULL,
                              &configuration_default_backend_ops,
                              &configuration_default_backend_data_ops,
                              CONF_PRIM_TYPE_BSTR,
                              sizeof(((ieee802154_sec_peer_table_t *)0)->mask),
                              (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, devstore.mask));

/* security/devstore/peers[] */

#define CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_INSTANCE_ARRAY_ID_NAME(prefix) \
    CONCAT(prefix, _security_devstore_peers_handler_id)

static CONF_HANDLER_ARRAY_ID(CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_INSTANCE_ARRAY_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                            CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_LOWER_SID, CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_UPPER_SID, \
                            CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_INDEX_STRIDE, "peers");

#define CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_INSTANCE_ARRAY_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_devstore_peers_handler)

static CONF_ARRAY_HANDLER(CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_INSTANCE_ARRAY_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                          &CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_INSTANCE_ARRAY_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                          NULL,
                          &configuration_default_backend_ops,
                          &configuration_default_backend_data_ops,
                          0,
                          sizeof(ieee802154_sec_peer_t),
                          (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, devstore.peers),
                          CONFIG_IEEE802154_SECURITY_PEERS_NUMOF);

/* security/devstore/peers[i].pan_id */

#define CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_PAN_ID_INSTANCE_HANDLER_ID_NAME(prefix) \
    CONCAT(prefix, _security_devstore_peers_pan_id_handler_id)

static CONF_HANDLER_ID(CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_PAN_ID_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                       CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_PAN_ID_LOWER_SID,
                       "pan_id");

#define CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_PAN_ID_INSTANCE_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_devstore_peers_pan_id_handler)

static CONF_PRIMITIVE_HANDLER(CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_PAN_ID_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              &CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_PAN_ID_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              NULL,
                              &configuration_default_backend_ops,
                              &configuration_default_backend_data_ops,
                              CONF_PRIM_TYPE_BSTR,
                              sizeof(((ieee802154_sec_peer_t *)0)->pan_id),
                              (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, devstore.peers[0].pan_id));

/* security/devstore/peers[i].short_addr */

#define CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_SHORT_ADDR_INSTANCE_HANDLER_ID_NAME(prefix) \
    CONCAT(prefix, _security_devstore_peers_short_addr_handler_id)

static CONF_HANDLER_ID(CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_SHORT_ADDR_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                       CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_SHORT_ADDR_LOWER_SID,
                       "short_addr");

#define CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_SHORT_ADDR_INSTANCE_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_devstore_peers_short_addr_handler)

static CONF_PRIMITIVE_HANDLER(CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_SHORT_ADDR_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              &CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_SHORT_ADDR_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              NULL,
                              &configuration_default_backend_ops,
                              &configuration_default_backend_data_ops,
                              CONF_PRIM_TYPE_BSTR,
                              sizeof(((ieee802154_sec_peer_t *)0)->short_addr),
                              (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, devstore.peers[0].short_addr));

/* security/devstore/peers[i].long_addr */

#define CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_LONG_ADDR_INSTANCE_HANDLER_ID_NAME(prefix) \
    CONCAT(prefix, _security_devstore_peers_long_addr_handler_id)

static CONF_HANDLER_ID(CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_LONG_ADDR_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                       CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_LONG_ADDR_LOWER_SID,
                       "long_addr");

#define CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_LONG_ADDR_INSTANCE_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_devstore_peers_long_addr_handler)

static CONF_PRIMITIVE_HANDLER(CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_LONG_ADDR_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              &CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_LONG_ADDR_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              NULL,
                              &configuration_default_backend_ops,
                              &configuration_default_backend_data_ops,
                              CONF_PRIM_TYPE_BSTR,
                              sizeof(((ieee802154_sec_peer_t *)0)->long_addr),
                              (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, devstore.peers[0].long_addr));

/* security/peer_lookup_table */

#define CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_INSTANCE_HANDLER_ID_NAME(prefix) \
    CONCAT(prefix, _security_peer_lookup_table_handler_id)

static CONF_HANDLER_NODE_ID(CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                            CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_LOWER_SID, CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_UPPER_SID,
                            "peer_lookup_table");

#define CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_INSTANCE_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_peer_lookup_table_handler)

static CONF_HANDLER(CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                    &CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                    NULL,
                    &configuration_default_backend_ops,
                    &configuration_default_backend_data_ops,
                    0,
                    sizeof(ieee802154_sec_peer_lookup_table_t),
                    (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, peer_lookup_table));

/* security/peer_lookup_table/mask */

#define CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_MASK_INSTANCE_HANDLER_ID_NAME(prefix) \
    CONCAT(prefix, _security_peer_lookup_table_mask_handler_id)

static CONF_HANDLER_ID(CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_MASK_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                       CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_MASK_LOWER_SID,
                       "mask");

#define CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_MASK_INSTANCE_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_peer_lookup_table_mask_handler)

static CONF_PRIMITIVE_HANDLER(CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_MASK_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              &CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_MASK_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              NULL,
                              &configuration_default_backend_ops,
                              &configuration_default_backend_data_ops,
                              CONF_PRIM_TYPE_BSTR,
                              sizeof(((ieee802154_sec_peer_lookup_table_t *)0)->mask),
                              (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, peer_lookup_table.mask));

/* security/peer_lookup_table/peer_lookup[] */

#define CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_INSTANCE_ARRAY_ID_NAME(prefix) \
    CONCAT(prefix, _security_peer_lookup_table_peerlookup_handler_id)

static CONF_HANDLER_ARRAY_ID(CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_INSTANCE_ARRAY_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                            CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_LOWER_SID, CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_UPPER_SID, \
                            CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_INDEX_STRIDE, "peer_lookup");

#define CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_INSTANCE_ARRAY_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_peer_lookup_table_peerlookup_handler)

static CONF_ARRAY_HANDLER(CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_INSTANCE_ARRAY_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                          &CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_INSTANCE_ARRAY_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                          NULL,
                          &configuration_default_backend_ops,
                          &configuration_default_backend_data_ops,
                          0,
                          sizeof(ieee802154_sec_peer_lookup_t),
                          (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, peer_lookup_table.peer_lookup),
                          CONFIG_IEEE802154_SECURITY_PEERLOOKUP_NUMOF);


/* security/peer_lookup_table/peer_lookup[i].fc */

#define CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_FC_INSTANCE_HANDLER_ID_NAME(prefix) \
    CONCAT(prefix, _security_peer_lookup_table_peerlookup_fc_handler_id)

static CONF_HANDLER_ID(CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_FC_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                       CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_FC_LOWER_SID,
                       "fc");

#define CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_FC_INSTANCE_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_peer_lookup_table_peerlookup_fc_handler)

static CONF_PRIMITIVE_HANDLER(CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_FC_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              &CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_FC_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              NULL,
                              &configuration_default_backend_ops,
                              &configuration_default_backend_data_ops,
                              CONF_PRIM_TYPE_UINT,
                              sizeof(((ieee802154_sec_peer_lookup_t *)0)->fc),
                              (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, peer_lookup_table.peer_lookup[0].fc));


/* security/peer_lookup_table/peer_lookup[i].peer */

#define CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_PEER_INSTANCE_HANDLER_ID_NAME(prefix) \
    CONCAT(prefix, _security_peer_lookup_table_peerlookup_peer_handler_id)

static CONF_HANDLER_ID(CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_PEER_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                       CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_PEER_LOWER_SID,
                       "peer");

#define CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_PEER_INSTANCE_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_peer_lookup_table_peerlookup_peer_handler)

static CONF_PRIMITIVE_HANDLER(CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_PEER_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              &CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_PEER_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              NULL,
                              &configuration_default_backend_ops,
                              &configuration_default_backend_data_ops,
                              CONF_PRIM_TYPE_UINT,
                              sizeof(((ieee802154_sec_peer_lookup_t *)0)->peer),
                              (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, peer_lookup_table.peer_lookup[0].peer));

/* security/peer_lookup_table/peer_lookup[i].key */

#define CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_KEY_INSTANCE_HANDLER_ID_NAME(prefix) \
    CONCAT(prefix, _security_peer_lookup_table_peerlookup_key_handler_id)

static CONF_HANDLER_ID(CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_KEY_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                       CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_KEY_LOWER_SID,
                       "key");

#define CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_KEY_INSTANCE_HANDLER_NAME(prefix) \
    CONCAT(prefix, _security_peer_lookup_table_peerlookup_key_handler)

static CONF_PRIMITIVE_HANDLER(CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_KEY_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              &CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_KEY_INSTANCE_HANDLER_ID_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                              NULL,
                              &configuration_default_backend_ops,
                              &configuration_default_backend_data_ops,
                              CONF_PRIM_TYPE_UINT,
                              sizeof(((ieee802154_sec_peer_lookup_t *)0)->key),
                              (CONFIG_IEEE802154_SECURITY_DATA_LOC) + offsetof(cfg_ieee802154_security_t, peer_lookup_table.peer_lookup[0].key));

#define CONFIG_IEEE802154_SECURITY_REGISTER_FUNCTION_NAME(prefix) \
    CONCAT(prefix, _security_register)

static inline void CONFIG_IEEE802154_SECURITY_REGISTER_FUNCTION_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX)(conf_handler_t *parent)
{
    if (parent) {
        configuration_register(parent, &CONFIG_IEEE802154_SECURITY_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX));
    }
    configuration_register(&CONFIG_IEEE802154_SECURITY_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                           &CONFIG_IEEE802154_SECURITY_KEYSTORE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX));
    configuration_register(&CONFIG_IEEE802154_SECURITY_KEYSTORE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                           &CONFIG_IEEE802154_SECURITY_KEYSTORE_MASK_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX));
    configuration_register(&CONFIG_IEEE802154_SECURITY_KEYSTORE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                           &CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_INSTANCE_ARRAY_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX).handler);
    configuration_register(&CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_INSTANCE_ARRAY_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX).handler,
                           &CONFIG_IEEE802154_SECURITY_KEYSTORE_KEYS_KEY_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX));

    configuration_register(&CONFIG_IEEE802154_SECURITY_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                           &CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX));
    configuration_register(&CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                           &CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_MASK_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX));
    configuration_register(&CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                           &CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_INSTANCE_ARRAY_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX).handler);
    configuration_register(&CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_INSTANCE_ARRAY_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX).handler,
                           &CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_MODE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX));
    configuration_register(&CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_INSTANCE_ARRAY_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX).handler,
                           &CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_LOOKUP_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX));
    configuration_register(&CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_INSTANCE_ARRAY_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX).handler,
                           &CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_KEY_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX));
    configuration_register(&CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_INSTANCE_ARRAY_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX).handler,
                           &CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_KEYLOOKUP_FC_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX));

    configuration_register(&CONFIG_IEEE802154_SECURITY_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                           &CONFIG_IEEE802154_SECURITY_DEVSTORE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX));
    configuration_register(&CONFIG_IEEE802154_SECURITY_DEVSTORE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                           &CONFIG_IEEE802154_SECURITY_DEVSTORE_MASK_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX));
    configuration_register(&CONFIG_IEEE802154_SECURITY_DEVSTORE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                           &CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_INSTANCE_ARRAY_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX).handler);
    configuration_register(&CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_INSTANCE_ARRAY_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX).handler,
                           &CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_PAN_ID_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX));
    configuration_register(&CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_INSTANCE_ARRAY_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX).handler,
                           &CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_SHORT_ADDR_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX));
    configuration_register(&CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_INSTANCE_ARRAY_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX).handler,
                           &CONFIG_IEEE802154_SECURITY_DEVSTORE_PEERS_LONG_ADDR_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX));

    configuration_register(&CONFIG_IEEE802154_SECURITY_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                           &CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX));
    configuration_register(&CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                           &CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_MASK_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX));
    configuration_register(&CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                           &CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_INSTANCE_ARRAY_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX).handler);
    configuration_register(&CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_INSTANCE_ARRAY_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX).handler,
                           &CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_FC_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX));
    configuration_register(&CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_INSTANCE_ARRAY_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX).handler,
                           &CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_PEER_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX));
    configuration_register(&CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_INSTANCE_ARRAY_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX).handler,
                           &CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_PEERLOOKUP_KEY_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX));
}

#define CONFIG_IEEE802154_SECURITY_AUTO_INIT_FUNCTION_NAME(prefix) \
    CONCAT(prefix, _security_auto_init)

void CONFIG_IEEE802154_SECURITY_AUTO_INIT_FUNCTION_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX)(void)
{
    *configuration_get_src_backend(&CONFIG_IEEE802154_SECURITY_KEYSTORE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX))
        = &configuration_root_backend;
    *configuration_get_dst_backend(&CONFIG_IEEE802154_SECURITY_KEYSTORE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX))
        = &configuration_root_backend;

    *configuration_get_src_backend(&CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX))
        = &configuration_root_backend;
    *configuration_get_dst_backend(&CONFIG_IEEE802154_SECURITY_KEY_LOOKUP_TABLE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX))
        = &configuration_root_backend;

    *configuration_get_src_backend(&CONFIG_IEEE802154_SECURITY_DEVSTORE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX))
        = &configuration_root_backend;
    *configuration_get_dst_backend(&CONFIG_IEEE802154_SECURITY_DEVSTORE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX))
        = &configuration_root_backend;

    *configuration_get_src_backend(&CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX))
        = &configuration_root_backend;
    *configuration_get_dst_backend(&CONFIG_IEEE802154_SECURITY_PEER_LOOKUP_TABLE_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX))
        = &configuration_root_backend;
}

AUTO_INIT_CONFIGURATION(CONFIG_IEEE802154_SECURITY_AUTO_INIT_FUNCTION_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX),
                        CONFIG_AUTO_INIT_CONFIGURATION_BACKEND_PRIO);

#endif /* _CFG_NO_INSTANCE */

#ifdef __cplusplus
}
#endif
