/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2015-02-13
 */

// This is the implemention of the application protocol

#include "zeus_proto.h"

zeus_status_t zeus_proto_ack_connection(zeus_process_t *p,zeus_event_t *ev){

    return zeus_proto_helper_generate_ack_ok_packet(p,ev);

}

zeus_status_t zeus_proto_solve_read_buf(zeus_process_t *p,zeus_event_t *ev){

    zeus_uchar_t opcode;
    zeus_uint_t len;
    zeus_connection_t *conn;
    zeus_size_t leftlen;

    if(ev->buflen > 0){

        zeus_proto_helper_get_opcode_and_pktlen(ev,&opcode,&len);

        switch(opcode){
            case ZEUS_PROTO_DATA_INS:

                break;
            default:
                conn = ev->connection;
                zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s:%hu send a wrong opcode : %d",\
                                                     conn->addr_string->data,          \
                                                     ntohs(conn->peer->sin_port),      \
                                                     opcode);
                return ZEUS_ERROR;
        }

    }else{
        return ZEUS_OK;
    }

}

zeus_status_t zeus_proto_timeout_handler(zeus_process_t *p,zeus_event_t *ev){

    return ZEUS_OK;

}
