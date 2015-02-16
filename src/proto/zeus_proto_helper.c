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
        if(val >= 0 && val < 9){
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
