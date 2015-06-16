/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2015-02-13
 */

// This is the implemention of the application protocol

#include "zeus_proto.h"

zeus_status_t zeus_proto_ack_connection(zeus_process_t *p,zeus_event_t *ev){

    return zeus_proto_helper_generate_ack_ok_packet(p,ev);

}

zeus_status_t zeus_proto_trans_socket_ack(zeus_process_t *p,zeus_event_t *ev){

    return zeus_proto_helper_generate_trans_socket_ack_packet(p,ev);

}

zeus_status_t zeus_proto_send_reset_load_balance_packet(zeus_process_t *p,zeus_event_t *ev){
    
    return zeus_proto_helper_generate_reset_load_balance_packet(p,ev);

}

zeus_status_t zeus_proto_ack_client_after_trans(zeus_process_t *p,zeus_event_t *ev){

    return zeus_proto_helper_generate_ack_client_after_trans_packet(p,ev);

}

zeus_status_t zeus_proto_send_update_workload_packet(zeus_process_t *p,zeus_event_t *ev){

    return zeus_proto_helper_generate_update_workload_packet(p,ev);

}

zeus_status_t zeus_proto_solve_read_buf(zeus_process_t *p,zeus_event_t *ev){

    zeus_uchar_t opcode;
    zeus_uint_t len;

    /* ZEUS_PROTO_CLIENT_CHECKOUT_INS */
    zeus_char_t client_check_data[ZEUS_PROTO_CLIENT_CHECKOUT_STRING_SIZE_MAX];
    zeus_idx_t worker_idx;
    /* ZEUS_PROTO_CLIENT_CHECKOUT_INS */

    /* ZEUS_PROTO_TRANS_SOCKET_ACK_INS */
    zeus_idx_t idx;
    /* ZEUS_PROTO_TRANS_SOCKET_ACK_INS */

    /* ZEUS_PROTO_UPDATE_LOAD_INS */
    zeus_uint_t load;
    /* ZEUS_PROTO_UPDATE_LOAD_INS */

    zeus_proto_helper_get_opcode_and_pktlen(ev,&opcode,&len);
    
    if(zeus_proto_helper_check_opcode_privilege_of_connection(ev->connection,opcode) == ZEUS_ERROR){
        goto solve_error;
    }

    if(ev->buflen - ZEUS_PROTO_OPCODE_SIZE - ZEUS_PROTO_DATA_LEN_SIZE >= len){

        switch(opcode){
            case ZEUS_PROTO_DATA_INS:
                // TODO
                // TRANS THE DATA TO THE ADMIN CLIENT
                // RECYCLE BUFFER
                break;

            case ZEUS_PROTO_CLIENT_CHECKOUT_INS:

                if(len != ZEUS_PROTO_CLIENT_CHECKOUT_STRING_SIZE_MAX + ZEUS_PROTO_TRANS_SIZE)
                    goto len_error;

                zeus_proto_helper_cp_data_from_buf_to_carr(p,                \
                                                           ev,               \
                                                           client_check_data,\
                                                           ZEUS_PROTO_CLIENT_CHECKOUT_STRING_SIZE_MAX + ZEUS_PROTO_TRANS_SIZE);

                if(zeus_proto_helper_check_hash(p->manage_passwd,client_check_data,\
                                                ZEUS_PROTO_CLIENT_CHECKOUT_STRING_SIZE_MAX) == ZEUS_OK){
                    
                    if(p->pidx == ZEUS_DATA_GATEWAY_PROCESS_INDEX){

                        if(ev->connection->check == ZEUS_PROTO_CHECK){
                            goto solve_error;
                        }

                        worker_idx = *(zeus_idx_t *)&(client_check_data[ZEUS_PROTO_CLIENT_CHECKOUT_STRING_SIZE_MAX]);
                        worker_idx = ntohl(worker_idx);

                        zeus_write_log(p->log,ZEUS_LOG_NOTICE,"%s:%hu is an admin client : send to %d",\
                                       ev->connection->addr_string->data,ntohs(ev->connection->peer->sin_port),worker_idx);
                    
                        if(worker_idx >= p->worker || worker_idx < 0)
                            goto worker_idx_error;

                        ev->connection->check = ZEUS_PROTO_CHECK;
                        ev->connection->admin = ZEUS_PROTO_ADMIN;

                        zeus_delete_list(p->client_connection,ev->connection->node);
                        zeus_insert_list(p->admin_connection,ev->connection->node);

                        if(zeus_helper_trans_socket(p,ev->connection,worker_idx) == ZEUS_ERROR){
                            zeus_write_log(p->log,ZEUS_LOG_ERROR,"set trans socket file descriptor event error");
                            return ZEUS_ERROR;
                        }

                    }else{

                        
                        if(ev->connection->admin == ZEUS_PROTO_NORMAL){
                            
                            zeus_write_log(p->log,ZEUS_LOG_NOTICE,"%s:%hu is an admin client",\
                                           ev->connection->addr_string->data,ntohs(ev->connection->peer->sin_port));

                            ev->connection->check = ZEUS_PROTO_CHECK;
                            ev->connection->admin = ZEUS_PROTO_ADMIN;
                            zeus_delete_list(p->client_connection,ev->connection->node);
                            zeus_insert_list(p->admin_connection,ev->connection->node);

                        }

                    }


                }else{
                    

                    if(p->pidx == ZEUS_DATA_GATEWAY_PROCESS_INDEX){

                        if(ev->connection->check == ZEUS_PROTO_CHECK){
                            goto solve_error;
                        }

                        worker_idx = zeus_helper_find_load_lowest(p);

                        ev->connection->check = ZEUS_PROTO_CHECK;
                        ev->connection->admin = ZEUS_PROTO_NORMAL;
                    
                        zeus_write_log(p->log,ZEUS_LOG_NOTICE,"%s:%hu is an ordinary client : send to %d",\
                                       ev->connection->addr_string->data,ntohs(ev->connection->peer->sin_port),worker_idx);
                        
                        if(zeus_helper_trans_socket(p,ev->connection,worker_idx) == ZEUS_ERROR){
                            zeus_write_log(p->log,ZEUS_LOG_ERROR,"set trans socket file descriptor event error");
                            return ZEUS_ERROR;
                        }

                    }else{

                        if(ev->connection->admin == ZEUS_PROTO_ADMIN){

                            zeus_write_log(p->log,ZEUS_LOG_NOTICE,"%s:%hu is an ordinary client",\
                                           ev->connection->addr_string->data,ntohs(ev->connection->peer->sin_port));

                            ev->connection->check = ZEUS_PROTO_CHECK;
                            ev->connection->admin = ZEUS_PROTO_NORMAL;

                            zeus_delete_list(p->admin_connection,ev->connection->node);
                            zeus_insert_list(p->client_connection,ev->connection->node);

                        }
                    
                    }
                    
                }
                
                
                break;

            case ZEUS_PROTO_TRANS_SOCKET_ACK_INS:


                if(len != ZEUS_PROTO_IDX_SIZE){
                    zeus_write_log(p->log,ZEUS_LOG_ERROR,"wrong channel index (ZEUS_PROTO_TRANS_SOCKET_ACK_INS)");
                    return ZEUS_ERROR;
                }

                if(zeus_proto_helper_get_channel_index(p,ev,&idx) == ZEUS_ERROR){
                    zeus_write_log(p->log,ZEUS_LOG_ERROR,"get channel index error (ZEUS_PROTO_TRANS_SOCKET_ACK_INS)");
                    return ZEUS_ERROR;
                }
                
                idx = ntohl(idx);

                zeus_write_log(p->log,ZEUS_LOG_NOTICE,"recv socket ack from channel %d",idx);

                p->worker_load[p->pidx] -= 1;
                p->worker_load[idx] += 1;

                break;

            case ZEUS_PROTO_RESET_LOAD_BALANCE_INS:

                if(len != ZEUS_PROTO_IDX_SIZE){
                    zeus_write_log(p->log,ZEUS_LOG_ERROR,"wrong channel index (ZEUS_PROTO_RESET_LOAD_BALANCE_INS)");
                    return ZEUS_ERROR;
                }

                if(zeus_proto_helper_get_channel_index(p,ev,&idx) == ZEUS_ERROR){
                    zeus_write_log(p->log,ZEUS_LOG_ERROR,"get channel index error (ZEUS_PROTO_RESET_LOAD_BALANCE_INS)");
                    return ZEUS_ERROR;
                }

                idx = ntohl(idx);

                zeus_write_log(p->log,ZEUS_LOG_NOTICE,"recv reset load balance from channel %d",idx);

                p->worker_load[idx] = 0;

                break;

            case ZEUS_PROTO_UPDATE_LOAD_INS:

                if(len != ZEUS_PROTO_IDX_SIZE + ZEUS_PROTO_UINT_SIZE){
                    zeus_write_log(p->log,ZEUS_LOG_ERROR,"wrong channel index or work load (ZEUS_PROTO_UPDATE_LOAD_INS)");
                    return ZEUS_ERROR;
                }

                if(zeus_proto_helper_get_channel_index_and_load(p,ev,&idx,&load) == ZEUS_ERROR){
                    zeus_write_log(p->log,ZEUS_LOG_ERROR,"get channel index and load error (ZEUS_PROTO_UPDATE_LOAD_INS)");
                    return ZEUS_ERROR;
                }

                idx = ntohl(idx);
                load = ntohl(load);

                zeus_write_log(p->log,ZEUS_LOG_NOTICE,"set channel %d's load to %u",idx,load);

                p->worker_load[idx] = load;

                break;

            default:

                goto solve_error;

        }

    }else{

        return ZEUS_ERROR;

    }

    return ZEUS_OK;

solve_error:
    zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s:%hu send a wrong opcode : %d",         \
                                         ev->connection->addr_string->data,         \
                                         ntohs(ev->connection->peer->sin_port),     \
                                         opcode);
    zeus_helper_close_connection(p,ev->connection);
    return ZEUS_ERROR;
len_error:
    zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s:%hu send a wrong len : %d",            \
                                         ev->connection->addr_string->data,         \
                                         ntohs(ev->connection->peer->sin_port),     \
                                         len);
    return ZEUS_ERROR;
worker_idx_error:
    zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s:%hu admin client send wrong worker index %d", \
                                         ev->connection->addr_string->data,                \
                                         ntohs(ev->connection->peer->sin_port),            \
                                         worker_idx);
    zeus_helper_close_connection(p,ev->connection);
    return ZEUS_ERROR;


}
