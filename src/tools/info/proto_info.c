/**
 * Copyright (C) Mellanox Technologies Ltd. 2001-2016.  ALL RIGHTS RESERVED.
 *
 * See file LICENSE for terms.
 */

#include "ucx_info.h"

#include <ucp/api/ucp.h>
#include <string.h>


void print_ucp_info(int print_opts, ucs_config_print_flags_t print_flags,
                    uint64_t features, size_t estimated_num_eps)
{
    ucp_config_t *config;
    ucs_status_t status;
    ucp_context_h context;
    ucp_worker_h worker;
    ucp_params_t params;
    ucp_worker_params_t worker_params;
    ucp_ep_params_t ep_params;
    ucp_address_t *address;
    size_t address_length;
    ucp_ep_h ep;

    status = ucp_config_read(NULL, NULL, &config);
    if (status != UCS_OK) {
        return;
    }

    memset(&params, 0, sizeof(params));
    params.field_mask        = UCP_PARAM_FIELD_FEATURES |
                               UCP_PARAM_FIELD_ESTIMATED_NUM_EPS;
    params.features          = features;
    params.estimated_num_eps = estimated_num_eps;

    status = ucp_init(&params, config, &context);
    if (status != UCS_OK) {
        printf("<Failed to create UCP context>\n");
        goto out_release_config;
    }

    if (print_opts & PRINT_UCP_CONTEXT) {
        ucp_context_print_info(context, stdout);
    }

    if (!(print_opts & (PRINT_UCP_WORKER|PRINT_UCP_EP))) {
        goto out_cleanup_context;
    }

    worker_params.field_mask  = UCP_WORKER_PARAM_FIELD_THREAD_MODE;
    worker_params.thread_mode = UCS_THREAD_MODE_MULTI;

    status = ucp_worker_create(context, &worker_params, &worker);
    if (status != UCS_OK) {
        printf("<Failed to create UCP worker>\n");
        goto out_cleanup_context;
    }

    if (print_opts & PRINT_UCP_WORKER) {
        ucp_worker_print_info(worker, stdout);
    }

    if (print_opts & PRINT_UCP_EP) {
        status = ucp_worker_get_address(worker, &address, &address_length);
        if (status != UCS_OK) {
            printf("<Failed to get UCP worker address>\n");
            goto out_destroy_worker;
        }

        ep_params.field_mask = UCP_EP_PARAM_FIELD_REMOTE_ADDRESS;
        ep_params.address    = address;

        status = ucp_ep_create(worker, &ep_params, &ep);
        ucp_worker_release_address(worker, address);
        if (status != UCS_OK) {
            printf("<Failed to get create UCP endpoint>\n");
            goto out_destroy_worker;
        }

        ucp_ep_print_info(ep, stdout);

        ucp_ep_destroy(ep);
    }

out_destroy_worker:
    ucp_worker_destroy(worker);
out_cleanup_context:
    ucp_cleanup(context);
out_release_config:
    ucp_config_release(config);
}
