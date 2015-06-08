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

    /* ZEUS_PROTO_CLIENT_CHECKOUT_INS */
    zeus_char_t client_check_data[ZEUS_PROTO_CLIENT_CHECKOUT_STRING_SIZE_MAX];
    zeus_idx_t worker_idx;
    /* ZEUS_PROTO_CLIENT_CHECKOUT_INS */

    zeus_proto_helper_get_opcode_and_pktlen(ev,&opcode,&len);

    if(ev->buflen - ZEUS_PROTO_OPCODE_SIZE - ZEUS_PROTO_DATA_LEN_SIZE >= len){

        switch(opcode){
            case ZEUS_PROTO_DATA_INS:
                // TODO
                // TRANS THE DATA TO THE ADMIN CLIENT
                // RECYCLE BUFFER
                break;

            case ZEUS_PROTO_CLIENT_CHECKOUT_INS:
                
                if(len != ZEUS_PROTO_CLIENT_CHECKOUT_STRING_SIZE_MAX + ZEUS_PROTO_TRANS_SIZE)
                    goto solve_error;

                zeus_proto_helper_cp_data_from_buf_to_carr(p,                \
                                                           ev,               \
                                                           client_check_data,\
                                                           ZEUS_PROTO_CLIENT_CHECKOUT_STRING_SIZE_MAX + ZEUS_PROTO_TRANS_SIZE);

                if(zeus_proto_helper_check_hash(p->manage_passwd,client_check_data,\
                                                ZEUS_PROTO_CLIENT_CHECKOUT_STRING_SIZE_MAX) == ZEUS_OK){
                    zeus_write_log(p->log,ZEUS_LOG_NOTICE,"%s:%hd is an admin client",\
                                   ev->connection->addr_string->data,ntohs(ev->connection->peer->sin_port));
                    worker_idx = *(zeus_idx_t *)&(client_check_data[ZEUS_PROTO_CLIENT_CHECKOUT_STRING_SIZE_MAX]);

                    if(worker_idx > p->worker || worker_idx < 1)
                        goto worker_idx_error;


                }else{
                    zeus_write_log(p->log,ZEUS_LOG_NOTICE,"%s:%hd is an ordinary client",\
                                   ev->connection->addr_string->data,ntohs(ev->connection->peer->sin_port));
                    
                    worker_idx = zeus_helper_find_load_lowest(p);
                    
                }
                
                zeus_helper_trans_socket(p,ev->connection,worker_idx);
                
                break;

            default:

                goto solve_error;

        }
    }

    return ZEUS_OK;

solve_error:
    zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s:%hu send a wrong opcode : %d",         \
                                         ev->connection->addr_string->data,         \
                                         ntohs(ev->connection->peer->sin_port),     \
                                         opcode);
    zeus_helper_close_connection(p,ev->connection);
    return ZEUS_ERROR;
worker_idx_error:
    zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s:%hu admin client send wrong worker index %d", \
                                         ev->connection->addr_string->data,                \
                                         ntohs(ev->connection->peer->sin_port),            \
                                         worker_idx);
    zeus_helper_close_connection(p,ev->connection);
    return ZEUS_ERROR;


}

zeus_status_t zeus_proto_timeout_handler(zeus_process_t *p,zeus_event_t *ev){

    return ZEUS_OK;

}
