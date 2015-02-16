/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2015-02-16
 */

// some logic here．

#ifndef __ZEUS_PROTO_HELPER_H_H_H__
#define __ZEUS_PROTO_HELPER_H_H_H__

#include "../core/zeus_common.h"

#define ZEUS_CLIENT_USER_KEY_SIZE 32
#define ZEUS_CLIENT_USER_KEY_MAX 62

zeus_status_t zeus_proto_helper_generate_ukey(zeus_process_t *,zeus_connection_t *);

#endif
