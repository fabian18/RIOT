/*
 * Copyright (C) 2022 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gnrc_send
 * @{
 *
 * @file
 * @brief       Internal SEND configuration
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 */
#ifndef GNRC_SEND_CONF_H
#define GNRC_SEND_CONF_H

#include "vfs_default.h"

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(CONFIG_GNRC_SEND_CRT_VFS_SEND_ROOT) || defined(DOXYGEN)
/**
 * @brief   VFS root path of SEND related files
 */
#define CONFIG_GNRC_SEND_CRT_VFS_SEND_ROOT      VFS_DEFAULT_DATA"/send"
#endif
#if !defined(CONFIG_GNRC_SEND_CRT_BUF_SIZE) || defined(DOXYGEN)
/**
 * @brief   Buffer size for certificates
 */
#define CONFIG_GNRC_SEND_CRT_BUF_SIZE           1024u
#endif
#if !defined(CONFIG_GNRC_SEND_KEY_BUF_SIZE) || defined(DOXYGEN)
/**
 * @brief   Buffer size for key pairs
 */
#define CONFIG_GNRC_SEND_KEY_BUF_SIZE           256u
#endif
#if !defined(CONFIG_GNRC_SEND_NAME_BUF_SIZE) || defined(DOXYGEN)
/**
 * @brief   Buffersize for a DER encoded name
 */
#define CONFIG_GNRC_SEND_NAME_BUF_SIZE          64u
#endif
#if !defined(CONFIG_GNRC_SEND_CRT_NUMOF) || defined(DOXYGEN)
/**
 * @brief   Number of reserved certificates for certificate paths
 */
#define CONFIG_GNRC_SEND_CRT_NUMOF              8u
#endif
#if !defined(CONFIG_GNRC_SEND_TA_NUMOF) || defined(DOXYGEN)
/**
 * @brief   Number of reserved trust anchors
 */
#define CONFIG_GNRC_SEND_TA_NUMOF               1u
#endif
#if !defined(CONFIG_GNRC_SEND_CP_NUMOF) || defined(DOXYGEN)
/**
 * @brief   Number of reserved certificate paths
 */
#define CONFIG_GNRC_SEND_CP_NUMOF               4u
#endif
#if !defined(CONFIG_GNRC_SEND_PK_VFS_NAME) || defined(DOXYGEN)
/**
 * @brief   Own public key file name in the VFS
 */
#define CONFIG_GNRC_SEND_PK_VFS_NAME            "pubkey.der"
#endif
#if !defined(CONFIG_GNRC_SEND_KEY_VFS_NAME) || defined(DOXYGEN)
/**
 * @brief   Own private key file name in the VFS
 */
#define CONFIG_GNRC_SEND_KEY_VFS_NAME           "key.der"
#endif
#if !defined(CONFIG_GNRC_SEND_CRT_ROOT) || defined(DOXYGEN)
/**
 * @brief   VFS path of intermediate certificates
 */
#define CONFIG_GNRC_SEND_CRT_ROOT               CONFIG_GNRC_SEND_CRT_VFS_SEND_ROOT"/crt"
#endif
#if !defined(CONFIG_GNRC_SEND_TA_ROOT) || defined(DOXYGEN)
/**
 * @brief   VFS path to trust anchors (CA certificates)
 */
#define CONFIG_GNRC_SEND_TA_ROOT                CONFIG_GNRC_SEND_CRT_VFS_SEND_ROOT"/ta"
#endif
#if !defined(CONFIG_GNRC_SEND_CP_ROOT) || defined(DOXYGEN)
/**
 * @brief   VFS path to next-hop router certificates
 */
#define CONFIG_GNRC_SEND_CP_ROOT                CONFIG_GNRC_SEND_CRT_VFS_SEND_ROOT"/cp"
#endif
#if !defined(CONFIG_GNRC_SEND_KEY_ROOT) || defined(DOXYGEN)
/**
 * @brief   VFS path to key files
 */
#define CONFIG_GNRC_SEND_KEY_ROOT               CONFIG_GNRC_SEND_CRT_VFS_SEND_ROOT"/key"
#endif
#if !defined(CONFIG_GNRC_SEND_CGA_SEC) || defined(DOXYGEN)
/**
 * @brief   Default CGA security level to use
 */
#define CONFIG_GNRC_SEND_CGA_SEC                IPV6_CGA_SEC_DEFAULT
#endif
#if !defined(CONFIG_GNRC_SEND_ECDSA_CURVE) || defined(DOXYGEN)
/**
 * @brief   Elliptic curve type to be used in certificates
 */
#define CONFIG_GNRC_SEND_ECDSA_CURVE            secp256r1
#endif
#if !defined(CONFIG_GNRC_SEND_NONCE_CACHE_NUMOF_EXP) || defined(DOXYGEN)
/**
 * @brief   Determines the number of nonces that can be stored concurrently
 */
#define CONFIG_GNRC_SEND_NONCE_CACHE_NUMOF_EXP  (6u)
#endif
#if !defined(CONFIG_GNRC_SEND_CPS_CACHE_NUMOF_EXP) || defined(DOXYGEN)
/**
 * @brief   Determinesthe number of CPS contexts that can be stored concurrently
 */
#define CONFIG_GNRC_SEND_CPS_CACHE_NUMOF_EXP    (3u)
#endif
#if !defined(CONFIG_GNRC_SEND_CPA_CACHE_NUMOF_EXP) || defined(DOXYGEN)
/**
 * @brief   Determines the number of CPA contexts that can be stored concurrently
 */
#define CONFIG_GNRC_SEND_CPA_CACHE_NUMOF_EXP    (3u)
#endif

/**
 * @brief   @ref CONFIG_GNRC_SEND_CRT_VFS_SEND_ROOT
 */
#define GNRC_SEND_CRT_VFS_SEND_ROOT             CONFIG_GNRC_SEND_CRT_VFS_SEND_ROOT
/**
 * @brief   @ref CONFIG_GNRC_SEND_CRT_BUF_SIZE
 */
#define GNRC_SEND_CRT_BUF_SIZE                  CONFIG_GNRC_SEND_CRT_BUF_SIZE
/**
 * @brief   @ref CONFIG_GNRC_SEND_KEY_BUF_SIZE
 */
#define GNRC_SEND_KEY_BUF_SIZE                  CONFIG_GNRC_SEND_KEY_BUF_SIZE
/**
 * @brief   @ref CONFIG_GNRC_SEND_NAME_BUF_SIZE
 */
#define GNRC_SEND_NAME_BUF_SIZE                 CONFIG_GNRC_SEND_NAME_BUF_SIZE
/**
 * @brief   @ref CONFIG_GNRC_SEND_CRT_NUMOF
 */
#define GNRC_SEND_CRT_NUMOF                     CONFIG_GNRC_SEND_CRT_NUMOF
/**
 * @brief   @ref CONFIG_GNRC_SEND_TA_NUMOF
 */
#define GNRC_SEND_TA_NUMOF                      CONFIG_GNRC_SEND_TA_NUMOF
/**
 * @brief   @ref CONFIG_GNRC_SEND_CP_NUMOF
 */
#define GNRC_SEND_CP_NUMOF                      CONFIG_GNRC_SEND_CP_NUMOF
/**
 * @brief   @ref CONFIG_GNRC_SEND_PK_VFS_NAME
 */
#define GNRC_SEND_PK_VFS_NAME                   CONFIG_GNRC_SEND_PK_VFS_NAME
/**
 * @brief   @ref CONFIG_GNRC_SEND_KEY_VFS_NAME
 */
#define GNRC_SEND_KEY_VFS_NAME                  CONFIG_GNRC_SEND_KEY_VFS_NAME
/**
 * @brief   @ref CONFIG_GNRC_SEND_CRT_ROOT
 */
#define GNRC_SEND_CRT_ROOT                      CONFIG_GNRC_SEND_CRT_ROOT
/**
 * @brief   @ref CONFIG_GNRC_SEND_TA_ROOT
 */
#define GNRC_SEND_TA_ROOT                       CONFIG_GNRC_SEND_TA_ROOT
/**
 * @brief   @ref CONFIG_GNRC_SEND_CP_ROOT
 */
#define GNRC_SEND_CP_ROOT                       CONFIG_GNRC_SEND_CP_ROOT
/**
 * @brief   @ref CONFIG_GNRC_SEND_KEY_ROOT
 */
#define GNRC_SEND_KEY_ROOT                      CONFIG_GNRC_SEND_KEY_ROOT
/**
 * @brief   @ref CONFIG_GNRC_SEND_CGA_SEC
 */
#define GNRC_SEND_CGA_SEC                       CONFIG_GNRC_SEND_CGA_SEC
/**
 * @brief   @ref CONFIG_GNRC_SEND_ECDSA_CURVE
 */
#define GNRC_SEND_ECDSA_CURVE                   CONFIG_GNRC_SEND_ECDSA_CURVE
/**
 * @brief   @ref CONFIG_GNRC_SEND_NONCE_CACHE_NUMOF_EXP
 */
#define GNRC_SEND_NONCE_CACHE_NUMOF_EXP         CONFIG_GNRC_SEND_NONCE_CACHE_NUMOF_EXP
/**
 * @brief   @ref CONFIG_GNRC_SEND_CPS_CACHE_NUMOF_EXP
 */
#define GNRC_SEND_CPS_CACHE_NUMOF_EXP           CONFIG_GNRC_SEND_CPS_CACHE_NUMOF_EXP
/**
 * @brief   @ref CONFIG_GNRC_SEND_CPA_CACHE_NUMOF_EXP
 */
#define GNRC_SEND_CPA_CACHE_NUMOF_EXP           CONFIG_GNRC_SEND_CPA_CACHE_NUMOF_EXP

#ifdef __cplusplus
}
#endif
#endif /* GNRC_SEND_CONF_H */
/** @} */
