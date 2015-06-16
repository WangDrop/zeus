/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2014-02-17
 */

// some macro of the protocol

#ifndef __ZEUS_PROTO_HEADER_H_H_H__
#define __ZEUS_PROTO_HEADER_H_H_H__

#define ZEUS_PROTO_NORMAL 0x00
#define ZEUS_PROTO_ADMIN 0x01

#define ZEUS_PROTO_UNCHECK 0x00
#define ZEUS_PROTO_CHECK 0x01

#define ZEUS_PROTO_OPCODE_SIZE (sizeof(zeus_uchar_t))
#define ZEUS_PROTO_DATA_LEN_SIZE (sizeof(zeus_uint_t))
#define ZEUS_PROTO_TRANS_SIZE (sizeof(zeus_idx_t))
#define ZEUS_PROTO_IDX_SIZE (sizeof(zeus_idx_t))
#define ZEUS_PROTO_CHILDREN_PROCESS_CNT_SIZE (sizeof(zeus_idx_t))
#define ZEUS_PROTO_NOP_SIZE (0)
#define ZEUS_PROTO_UINT_SIZE (sizeof(zeus_uint_t))

#define ZEUS_PROTO_CLIENT_CHECKOUT_STRING_SIZE_MAX 32

#define ZEUS_PROTO_INS_MAX (0x04 * sizeof(zeus_uint_t))
#define ZEUS_PROTO_ACK_CONNECTION_INS 0x00
#define ZEUS_PROTO_DATA_INS 0x01
#define ZEUS_PROTO_CLIENT_CHECKOUT_INS 0x02
#define ZEUS_PROTO_TRANS_SOCKET_ACK_INS 0x03
#define ZEUS_PROTO_RESET_LOAD_BALANCE_INS 0x04
#define ZEUS_PROTO_ACK_CLIENT_AFTER_TRANS 0x05
#define ZEUS_PROTO_UPDATE_LOAD_INS 0x06

#endif
