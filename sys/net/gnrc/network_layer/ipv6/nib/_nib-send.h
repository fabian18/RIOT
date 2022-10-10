/*
 * Copyright (C) 2022 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup net_gnrc_ipv6_nib
 * @brief
 * @{
 *
 * @file
 * @brief   Internal SEND NIB API
 * @internal
 *
 * @author  Fabian Hüßler <fabian.huessler@ovgu.de>
 */
#ifndef PRIV_NIB_SEND_H
#define PRIV_NIB_SEND_H

#include <stdint.h>

#include "kernel_defines.h"
#include "net/gnrc/netif.h"
#include "net/ipv6/addr.h"
#include "net/gnrc/send.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   True if @p sec evaluates as secured, else false
 */
#define GNRC_SEND_SECURED(sec) (IS_USED(MODULE_GNRC_SEND) && ((sec) == GNRC_SEND_STATUS_OK))

#if IS_USED(MODULE_GNRC_SEND) || defined(DOXYGEN)
/**
 * @brief This constitutes an extra argument if SEND is used
 */
#define GNRC_SEND_NIB_ARG(comma, a)    ,a
#else
#define GNRC_SEND_NIB_ARG(...)
#endif

/**
 * @brief   Initializer for an unrestricted IPv6 range
 */
#define GNRC_SEND_NIB_SEC_RTR_IP_UNRESTRICED { .range = {{ .lower = IPV6_ADDR_UNSPECIFIED,  \
                                                           .upper = IPV6_ADDR_MASK }},      \
                                               .numof = 1 }

/**
 * @brief   Initializer for an inherited IPv6 range
 */
#define GNRC_SEND_NIB_SEC_RTR_IP_INHERIT { .numof = 0 }

/**
 * @brief   IPv6 range restriction structure
 */
typedef struct gnrc_send_ip_restrict {
    struct {
        ipv6_addr_t lower;              /**< IPv6 lower bound */
        ipv6_addr_t upper;              /**< IPv6 upper bound */
    } range[GNRC_SEND_SEC_PFX_NUMOF];   /**< Array of restrictive IP ranges */
    unsigned numof;                     /**< Nunmber of restrictive IP ranges */
} gnrc_send_ip_restrict_t;

/**
 * @brief   Get IPv6 restrictions of certificate path @p cp
 *
 * @param[out]      ip_res      IPv6 restrictions
 * @param[in]       cp          Certificate path
 */
void gnrc_ipv6_nib_send_get_ip_restriction(gnrc_send_ip_restrict_t *ip_res,
                                           const gnrc_send_cp_t *cp);

/**
 * @brief   Check if the prefix @p pfx fulfills the restrctions @p ip_res
 *
 * @param[in]       pfx         Prefix to be checked
 * @param[in]       len         Bitlength of @p pfx
 * @param[in]       ip_res      IP restrictions
 *
 * @return  GNRC_SEND_STATUS_OK if restrictions are fulfilled
 *          else GNRC_SEND_STATUS_UNSECURED
 */
int gnrc_ipv6_nib_send_check_prefix(const ipv6_addr_t *pfx, uint8_t len,
                                    const gnrc_send_ip_restrict_t *ip_res);

/**
 * @brief   Process SEND related message components of a received neighbor solicitation
 *
 * @param[in]       netif       Network interface the NS was received on
 * @param[in]       ipv6        IPv6 header
 * @param[in]       nbr_sol     ICMPv6 neighbor solicitation
 * @param[in]       icmpv6_len  ICMPv6 length
 *
 * @retval  @ref GNRC_SEND_STATUS_OK if @p nbr_sol is secured
 * @retval  @ref GNRC_SEND_STATUS_UNSECURED if @p nbr_sol is not secured but SEND is running
 *          in compatibility mode @ref GNRC_NIB_IPV6_SEND_MODE_COMPAT
 * @retval  Negative SEND status code on error
 */
int gnrc_ipv6_nib_send_handle_nbr_sol(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                      const ndp_nbr_sol_t *nbr_sol, size_t icmpv6_len);

/**
 * @brief   Process SEND related message components of a received neighbor advertisement
 *
 * @param[in]       netif       Network interface the NA was received on
 * @param[in]       ipv6        IPv6 header
 * @param[in]       nbr_adv     ICMPv6 neighbor advertisement
 * @param[in]       icmpv6_len  ICMPv6 length
 *
 * @retval  @ref GNRC_SEND_STATUS_OK if @p nbr_adv is secured
 * @retval  @ref GNRC_SEND_STATUS_UNSECURED if @p nbr_adv is not secured but SEND is running
 *          in compatibility mode @ref GNRC_NIB_IPV6_SEND_MODE_COMPAT
 * @retval  Negative SEND status code on error
 */
int gnrc_ipv6_nib_send_handle_nbr_adv(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                      const ndp_nbr_adv_t *nbr_adv, size_t icmpv6_len);

/**
 * @brief   Process SEND related message components of a received router solicitation
 *
 * @param[in]       netif       Network interface the RS was received on
 * @param[in]       ipv6        IPv6 header
 * @param[in]       rtr_sol     ICMPv6 router solicitation
 * @param[in]       icmpv6_len  ICMPv6 length
 *
 * @retval  @ref GNRC_SEND_STATUS_OK if @p rtr_sol is secured
 * @retval  @ref GNRC_SEND_STATUS_UNSECURED if @p rtr_sol is not secured but SEND is running
 *          in compatibility mode @ref GNRC_NIB_IPV6_SEND_MODE_COMPAT
 * @retval  Negative SEND status code on error
 */
int gnrc_ipv6_nib_send_handle_rtr_sol(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                      const ndp_rtr_sol_t *rtr_sol, size_t icmpv6_len);

/**
 * @brief   Process SEND related message components of a received router advertisement
 *
 * @param[in]       netif       Network interface the RA was received on
 * @param[in]       ipv6        IPv6 header
 * @param[in]       rtr_adv     ICMPv6 router advertisement
 * @param[in]       icmpv6_len  ICMPv6 length
 * @param[out]      rtr_cp      Router certificate path
 *
 * @retval  @ref GNRC_SEND_STATUS_OK if @p rtr_adv is secured
 * @retval  @ref GNRC_SEND_STATUS_UNSECURED if @p rtr_adv is not secured but SEND is running
 *          in compatibility mode @ref GNRC_NIB_IPV6_SEND_MODE_COMPAT
 * @retval  Negative SEND status code on error
 */
int gnrc_ipv6_nib_send_handle_rtr_adv(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                      const ndp_rtr_adv_t *rtr_adv, size_t icmpv6_len,
                                      const gnrc_send_cp_t **rtr_cp);

/**
 * @brief   Process a certificate path solicitation
 *
 * @param[in]       netif       Network interface the CPS was received on
 * @param[in]       ipv6        IPv6 header
 * @param[in]       cp_sol      ICMPv6 certificate path solicitation
 * @param[in]       icmpv6_len  ICMPv6 length
 */
void gnrc_ipv6_nib_send_handle_cp_sol(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                      const ndp_cp_sol_t *cp_sol, size_t icmpv6_len);

/**
 * @brief   Process a certificate path advertisement
 *
 * @param[in]       netif       Network interface the CPA was received on
 * @param[in]       ipv6        IPv6 header
 * @param[in]       cp_adv      ICMPv6 certificate path advertisement
 * @param[in]       icmpv6_len  ICMPv6 length
 */
void gnrc_ipv6_nib_send_handle_cp_adv(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                      const ndp_cp_adv_t *cp_adv, size_t icmpv6_len);

/**
 * @brief   Get the nonce from a nonce option
 *
 * @param[in]       nonce_opt   Nonce option
 * @param[out]      nonce_size  Nonce size
 *
 * @return  The nonce
 */
void *gnrc_ipv6_nib_send_handle_nonce(const ndp_opt_nonce_t *nonce_opt, size_t *nonce_size);

/**
 * @brief   Handle sending of the next certificate path solicitation (retransmission)
 *
 * @param[in, out]  cps_ctx     CPS context
 */
void gnrc_ipv6_nib_send_handle_snd_cp_sol(gnrc_send_cache_cps_t *cps_ctx);

/**
 * @brief   Handle sending of the next certificate path advertisement
 *
 * @param[in, out]  cpa_ctx     CPA context
 */
void gnrc_ipv6_nib_send_handle_snd_cp_adv(gnrc_send_cache_cpa_t *cpa_ctx);

#ifdef __cplusplus
}
#endif
#endif /* PRIV_NIB_SEND_H */
/** @} */
