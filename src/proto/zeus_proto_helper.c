/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2015-02-16
 */

#include "zeus_proto_helper.h"


zeus_status_t zeus_proto_helper_generate_ukey(zeus_process_t *p,zeus_connection_t *conn){

    zeus_size_t idx;
    zeus_int_t val;

    if(!conn->ukey){
        
        if((conn->ukey = zeus_create_string(p,ZEUS_CLIENT_USER_KEY_SIZE + 1)) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"alloc ukey memory error");
            return ZEUS_ERROR;
        }

    }
    
    srand(time(NULL));

    for(idx = 0 ; idx < ZEUS_CLIENT_USER_KEY_SIZE ; ++ idx){
        val = rand() % ZEUS_CLIENT_USER_KEY_MAX;
        if(val >= 0 && val < 10){
            conn->ukey->data[idx] = '0' + val;
        }else if(val >= 10 && val < 36){
            conn->ukey->data[idx] = 'a' + val - 10;
        }else{
            conn->ukey->data[idx] = 'A' + val - 36;
        }
    }

    conn->ukey->data[ZEUS_CLIENT_USER_KEY_SIZE] = '\0';

    return ZEUS_OK;

}

zeus_status_t zeus_proto_helper_generate_ack_connection_package(zeus_process_t *p,zeus_event_t *ev){
    

    if(zeus_proto_buffer_write_byte(p,ev,ZEUS_PROTO_ACK_CONNECTION_INS) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"generate ins error");
        return ZEUS_ERROR;
    }

    if(zeus_proto_buffer_write_uint(p,ev,ZEUS_CLIENT_USER_KEY_SIZE) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"generate package size error");
        return ZEUS_ERROR;
    }
    
    if(zeus_proto_buffer_write_byte_array(p,ev,ev->connection->ukey->data,ZEUS_CLIENT_USER_KEY_SIZE) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"generate package ukey buffer error");
        return ZEUS_ERROR;
    }

    return ZEUS_OK;

}
