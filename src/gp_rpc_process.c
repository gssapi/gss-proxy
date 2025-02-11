/* Copyright (C) 2011 the GSS-PROXY contributors, see COPYING for license */

#include "gp_proxy.h"
#include "gp_rpc_process.h"

typedef int (*gp_exec_fn)(gp_exec_std_args);

struct gp_rpc_fn_set {
    uint32_t proc;
    const char *proc_name;
    xdrproc_t arg_fn;
    xdrproc_t res_fn;
    gp_exec_fn exec_fn;
} gp_xdr_set[] = {
    {
        0,
        "NULLPROC",
        (xdrproc_t)xdr_void,
        (xdrproc_t)xdr_void,
        NULL
    },
    {
        GSSX_INDICATE_MECHS,
        "GSSX_INDICATE_MECHS",
        (xdrproc_t)xdr_gssx_arg_indicate_mechs,
        (xdrproc_t)xdr_gssx_res_indicate_mechs,
        gp_indicate_mechs
    },
    {
        GSSX_GET_CALL_CONTEXT,
        "GSSX_GET_CALL_CONTEXT",
        (xdrproc_t)xdr_gssx_arg_get_call_context,
        (xdrproc_t)xdr_gssx_res_get_call_context,
        gp_get_call_context
    },
    {
        GSSX_IMPORT_AND_CANON_NAME,
        "GSSX_IMPORT_AND_CANON_NAME",
        (xdrproc_t)xdr_gssx_arg_import_and_canon_name,
        (xdrproc_t)xdr_gssx_res_import_and_canon_name,
        gp_import_and_canon_name
    },
    {
        GSSX_EXPORT_CRED,
        "GSSX_EXPORT_CRED",
        (xdrproc_t)xdr_gssx_arg_export_cred,
        (xdrproc_t)xdr_gssx_res_export_cred,
        gp_export_cred
    },
    {
        GSSX_IMPORT_CRED,
        "GSSX_IMPORT_CRED",
        (xdrproc_t)xdr_gssx_arg_import_cred,
        (xdrproc_t)xdr_gssx_res_import_cred,
        gp_import_cred
    },
    {
        GSSX_ACQUIRE_CRED,
        "GSSX_ACQUIRE_CRED",
        (xdrproc_t)xdr_gssx_arg_acquire_cred,
        (xdrproc_t)xdr_gssx_res_acquire_cred,
        gp_acquire_cred
    },
    {
        GSSX_STORE_CRED,
        "GSSX_STORE_CRED",
        (xdrproc_t)xdr_gssx_arg_store_cred,
        (xdrproc_t)xdr_gssx_res_store_cred,
        gp_store_cred
    },
    {
        GSSX_INIT_SEC_CONTEXT,
        "GSSX_INIT_SEC_CONTEXT",
        (xdrproc_t)xdr_gssx_arg_init_sec_context,
        (xdrproc_t)xdr_gssx_res_init_sec_context,
        gp_init_sec_context
    },
    {
        GSSX_ACCEPT_SEC_CONTEXT,
        "GSSX_ACCEPT_SEC_CONTEXT",
        (xdrproc_t)xdr_gssx_arg_accept_sec_context,
        (xdrproc_t)xdr_gssx_res_accept_sec_context,
        gp_accept_sec_context
    },
    {
        GSSX_RELEASE_HANDLE,
        "GSSX_RELEASE_HANDLE",
        (xdrproc_t)xdr_gssx_arg_release_handle,
        (xdrproc_t)xdr_gssx_res_release_handle,
        gp_release_handle
    },
    {
        GSSX_GET_MIC,
        "GSSX_GET_MIC",
        (xdrproc_t)xdr_gssx_arg_get_mic,
        (xdrproc_t)xdr_gssx_res_get_mic,
        gp_get_mic
    },
    {
        GSSX_VERIFY,
        "GSSX_VERIFY",
        (xdrproc_t)xdr_gssx_arg_verify_mic,
        (xdrproc_t)xdr_gssx_res_verify_mic,
        gp_verify_mic
    },
    {
        GSSX_WRAP,
        "GSSX_WRAP",
        (xdrproc_t)xdr_gssx_arg_wrap,
        (xdrproc_t)xdr_gssx_res_wrap,
        gp_wrap
    },
    {
        GSSX_UNWRAP,
        "GSSX_UNWRAP",
        (xdrproc_t)xdr_gssx_arg_unwrap,
        (xdrproc_t)xdr_gssx_res_unwrap,
        gp_unwrap
    },
    {
        GSSX_WRAP_SIZE_LIMIT,
        "GSSX_WRAP_SIZE_LIMIT",
        (xdrproc_t)xdr_gssx_arg_wrap_size_limit,
        (xdrproc_t)xdr_gssx_res_wrap_size_limit,
        gp_wrap_size_limit
    }
};

static int gp_rpc_decode_call_header(XDR *xdr_call_ctx,
                                     uint32_t *xid,
                                     uint32_t *proc,
                                     gp_rpc_accept_status *acc,
                                     gp_rpc_reject_status *rej)
{
    struct gp_rpc_call_header *chdr;
    gp_rpc_msg msg;
    bool decoded;
    int ret;

    memset(&msg, 0, sizeof(gp_rpc_msg));

    decoded = xdr_gp_rpc_msg(xdr_call_ctx, &msg);
    if (!decoded) {
        return EFAULT;
    }

    *xid = msg.xid;

    if (msg.header.type != GP_RPC_CALL) {
        *acc = GP_RPC_GARBAGE_ARGS;
        ret = EINVAL;
        goto done;
    }

    chdr = &msg.header.gp_rpc_msg_union_u.chdr;

    if (chdr->rpcvers != 2) {
        *rej = GP_RPC_RPC_MISMATCH;
        ret = EACCES;
        goto done;
    }
    if (chdr->prog != GSSPROXY) {
        *acc = GP_RPC_PROG_UNAVAIL;
        ret = EINVAL;
        goto done;
    }
    if (chdr->vers != GSSPROXYVERS) {
        *acc = GP_RPC_PROG_MISMATCH;
        ret = EINVAL;
        goto done;
    }
    if (chdr->proc < GSSX_PROC_MIN || chdr->proc > GSSX_PROC_MAX) {
        *acc = GP_RPC_PROC_UNAVAIL;
        ret = EINVAL;
        goto done;
    }
    if (chdr->cred.flavor != GP_RPC_AUTH_NONE) {
        *rej = GP_RPC_AUTH_ERROR;
        ret = EACCES;
        goto done;
    }

    *proc = chdr->proc;
    *acc = GP_RPC_SUCCESS;
    ret = 0;

done:
    xdr_free((xdrproc_t)xdr_gp_rpc_msg, (char *)&msg);
    return ret;
}

static int gp_rpc_decode_call(XDR *xdr_call_ctx,
                              uint32_t *xid,
                              uint32_t *proc,
                              union gp_rpc_arg *arg,
                              gp_rpc_accept_status *acc,
                              gp_rpc_reject_status *rej)
{
    xdrfn *arg_fn;
    bool xdrok;
    int ret;

    ret = gp_rpc_decode_call_header(xdr_call_ctx, xid, proc, acc, rej);
    if (ret) {
        return ret;
    }

    arg_fn = gp_xdr_set[*proc].arg_fn;
    xdrok = arg_fn(xdr_call_ctx, arg);
    if (!xdrok) {
        *acc = GP_RPC_GARBAGE_ARGS;
        return EINVAL;
    }

    return 0;
}

static int gp_rpc_encode_reply_header(XDR *xdr_reply_ctx,
                                      uint32_t xid, int err,
                                      gp_rpc_accept_status acc,
                                      gp_rpc_reject_status rej)
{
    gp_rpc_msg msg;
    gp_rpc_reply_header *rhdr;
    gp_rpc_accepted_reply *accepted;
    gp_rpc_rejected_reply *rejected;
    bool encoded;

    memset(&msg, 0, sizeof(gp_rpc_msg));

    msg.xid = xid;
    msg.header.type = GP_RPC_REPLY;

    rhdr = &msg.header.gp_rpc_msg_union_u.rhdr;
    accepted = &rhdr->gp_rpc_reply_header_u.accepted;
    rejected = &rhdr->gp_rpc_reply_header_u.rejected;

    switch (err) {
    case EFAULT:
        return EFAULT;
    case EACCES:
        rhdr->status = GP_RPC_MSG_DENIED;
        rejected->status = rej;
        if (rej == GP_RPC_RPC_MISMATCH) {
            rejected->gp_rpc_rejected_reply_u.mismatch_info.high = 2;
            rejected->gp_rpc_rejected_reply_u.mismatch_info.low = 2;
        } else {
            rejected->gp_rpc_rejected_reply_u.status = GP_RPC_AUTH_FAILED;
        }
        break;
    case EINVAL:
        rhdr->status = GP_RPC_MSG_ACCEPTED;
        accepted->reply_data.status = acc;
        if (acc == GP_RPC_PROG_MISMATCH) {
            accepted->reply_data.gp_rpc_reply_union_u.mismatch_info.high = GSSPROXYVERS;
            accepted->reply_data.gp_rpc_reply_union_u.mismatch_info.low = GSSPROXYVERS;
        }
        break;
    case 0:
        rhdr->status = GP_RPC_MSG_ACCEPTED;
        accepted->reply_data.status = GP_RPC_SUCCESS;
        break;
    default:
        rhdr->status = GP_RPC_MSG_ACCEPTED;
        accepted->reply_data.status = GP_RPC_SYSTEM_ERR;
        break;
    }

    /* always reset xdr_ctx position, as this function may be called
     * multiple times in case errors occurred after the initial header
     * was created */
    xdr_setpos(xdr_reply_ctx, 0);

    encoded = xdr_gp_rpc_msg(xdr_reply_ctx, &msg);
    if (!encoded) {
        return EFAULT;
    }

    return 0;
}

static int gp_rpc_encode_reply(XDR *xdr_reply_ctx,
                               uint32_t xid, uint32_t proc,
                               union gp_rpc_res *res, int err,
                               gp_rpc_accept_status acc,
                               gp_rpc_reject_status rej)
{
    xdrfn *res_fn;
    bool xdrok;
    int ret;

    ret = gp_rpc_encode_reply_header(xdr_reply_ctx, xid, err, acc, rej);
    if (ret != 0  || err != 0) {
        return ret;
    }

    res_fn = gp_xdr_set[proc].res_fn;
    xdrok = res_fn(xdr_reply_ctx, res);

    if (!xdrok) {
        return gp_rpc_encode_reply_header(xdr_reply_ctx, xid, EINVAL,
                                          GP_RPC_SYSTEM_ERR, GP_RPC_RPC_MISMATCH);
    }

    return 0;
}

static const char *gp_rpc_procname(uint32_t proc)
{
    if (proc > GSSX_PROC_MAX) {
        return NULL;
    }

    return gp_xdr_set[proc].proc_name;
}

static int gp_rpc_execute(struct gp_call_ctx *gpcall, uint32_t proc,
                          union gp_rpc_arg *arg, union gp_rpc_res *res)
{
    GPDEBUG("gp_rpc_execute: executing %d (%s) for service \"%s\", euid: %d,"
            "socket: %s\n", proc, gp_rpc_procname(proc),
            gpcall->service->name, gp_conn_get_uid(gpcall->connection),
            gpcall->service->socket);

    return gp_xdr_set[proc].exec_fn(gpcall, arg, res);
}

static int gp_rpc_return_buffer(XDR *xdr_reply_ctx, char *reply_buffer,
                                uint8_t **outbuf, size_t *outlen)
{
    unsigned int length;
    uint8_t *buffer;

    length = xdr_getpos(xdr_reply_ctx);

    buffer = malloc(length);
    if (!buffer) {
        return ENOMEM;
    }
    memcpy(buffer, reply_buffer, length);

    *outbuf = buffer;
    *outlen = length;
    return 0;
}

static void gp_rpc_free_xdrs(int proc,
                             union gp_rpc_arg *arg,
                             union gp_rpc_res *res)
{

    xdr_free(gp_xdr_set[proc].arg_fn, (char *)arg);
    xdr_free(gp_xdr_set[proc].res_fn, (char *)res);
}

int gp_rpc_process_call(struct gp_call_ctx *gpcall,
                        uint8_t *inbuf, size_t inlen,
                        uint8_t **outbuf, size_t *outlen)
{
    XDR xdr_call_ctx;
    XDR xdr_reply_ctx;
    gp_rpc_accept_status acc = GP_RPC_SUCCESS;
    gp_rpc_reject_status rej = GP_RPC_RPC_MISMATCH;
    char reply_buffer[MAX_RPC_SIZE];
    union gp_rpc_arg arg;
    union gp_rpc_res res;
    uint32_t xid = 0;
    uint32_t proc;
    int ret;

    memset(&arg, 0, sizeof(union gp_rpc_arg));
    memset(&res, 0, sizeof(union gp_rpc_res));
    proc = 0;

    xdrmem_create(&xdr_call_ctx, (caddr_t)inbuf, inlen, XDR_DECODE);
    xdrmem_create(&xdr_reply_ctx, reply_buffer, MAX_RPC_SIZE, XDR_ENCODE);

    /* decode request */
    GPDEBUGN(3, "[status] Processing request [%p (%zu)]\n", inbuf, inlen);
    ret = gp_rpc_decode_call(&xdr_call_ctx, &xid, &proc, &arg, &acc, &rej);
    if (!ret) {
        /* execute request */
        GPDEBUGN(3, "[status] Executing request %d (%s) from [%p (%zu)]\n",
                 proc, gp_rpc_procname(proc), inbuf, inlen);
        ret = gp_rpc_execute(gpcall, proc, &arg, &res);
        if (ret) {
            acc = GP_RPC_SYSTEM_ERR;
            ret = EINVAL;
        }
    }

    /* encode reply */
    ret = gp_rpc_encode_reply(&xdr_reply_ctx, xid, proc, &res, ret, acc, rej);
    if (ret == 0) {
        /* return encoded buffer */
        ret = gp_rpc_return_buffer(&xdr_reply_ctx,
                                   reply_buffer, outbuf, outlen);
        GPDEBUGN(3, "[status] Returned buffer %d (%s) from [%p (%zu)]: "
                 "[%p (%zu)]\n", proc, gp_rpc_procname(proc), inbuf, inlen,
                 *outbuf, *outlen);
    }
    /* free resources */
    gp_rpc_free_xdrs(proc, &arg, &res);
    xdr_destroy(&xdr_call_ctx);
    xdr_destroy(&xdr_reply_ctx);
    return ret;
}

GP_EXEC_UNUSED_FUNC(gp_get_call_context);
GP_EXEC_UNUSED_FUNC(gp_export_cred);
GP_EXEC_UNUSED_FUNC(gp_import_cred);
GP_EXEC_UNUSED_FUNC(gp_store_cred);
