/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2015-01-23
 */

// This is the event io handler

#include "zeus_event_io.h"

zeus_status_t zeus_event_io_read(zeus_process_t *p,zeus_event_t *ev){

    zeus_int_t terrno;
    zeus_list_data_t *lnode;
    zeus_buffer_t *current_buf;
    zeus_size_t current_left_sz;
    zeus_size_t current_read_sz;
    zeus_size_t current_read_cnt = 0;

    if(ev->buffer->tail == NULL){
        if((lnode = zeus_create_buffer_list_node(p)) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"create buffer error in zeus_event_io_read");
            return ZEUS_ERROR;
        }
        zeus_insert_list(ev->buffer,lnode);
    }

    lnode = ev->buffer->tail;
    current_buf = (zeus_buffer_t *)(lnode->d);

    for(;;){

        current_left_sz = zeus_addr_delta(current_buf->end,current_buf->last);

        if(current_left_sz == 0){

            if((lnode = zeus_create_buffer_list_node(p)) == NULL){
                zeus_write_log(p->log,ZEUS_LOG_ERROR,"create buffer error in zeus_event_io_read");
                return ZEUS_ERROR;
            }

            zeus_insert_list(ev->buffer,lnode);
            lnode = ev->buffer->tail;
            current_buf = (zeus_buffer_t *)(lnode->d);
            current_left_sz = zeus_addr_delta(current_buf->end,current_buf->last);
        
        }
        
        if(current_read_cnt ++ == ZEUS_READ_MAX_TIME_CNT){
            break;
        }

        if((current_read_sz = read(ev->connection->fd,current_buf->last,current_left_sz)) == -1){
            terrno = errno;
            if(terrno == EAGAIN || terrno == EWOULDBLOCK || terrno == EINTR){
                return ZEUS_OK;
            }else{
                zeus_write_log(p->log,ZEUS_LOG_ERROR,"read from the connection error");
                return ZEUS_ERROR;
            }
        }else if(current_read_sz == 0){

            ev->connection->quiting = 1;

            break;    
        
        }else if(current_read_sz < current_left_sz){

            ev->buflen += current_read_sz;
            current_buf->last = zeus_addr_add(current_buf->last,current_read_sz);
            break;

        }else{
            
            ev->buflen += current_read_sz;
            current_buf->last = zeus_addr_add(current_buf->last,current_read_sz);
            if((lnode = zeus_create_buffer_list_node(p)) == NULL){
                zeus_write_log(p->log,ZEUS_LOG_ERROR,"create buffer error in zeus_event_io_read");
                return ZEUS_ERROR;
            }

            zeus_insert_list(ev->buffer,lnode);
            lnode = ev->buffer->tail;
            current_buf = (zeus_buffer_t *)(lnode->d);

        }
    
    }
    
    while(ev->buflen > ZEUS_PROTO_OPCODE_SIZE + ZEUS_PROTO_DATA_LEN_SIZE){
        if(zeus_proto_solve_read_buf(p,ev) == ZEUS_ERROR){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process solve read buffer and do operation error", \
                                                 p->pidx?"worker":"gateway");
            // TODO
            // recyle things
            return ZEUS_ERROR;
        }
    }

    if(ev->connection->quiting == 1){
        zeus_helper_close_connection(p,ev->connection);
    }
    
    return ZEUS_OK;

}

zeus_status_t zeus_event_io_write(zeus_process_t *p,zeus_event_t *ev){
    
    zeus_int_t terrno;
    zeus_list_data_t *lnode;
    zeus_buffer_t *current_buf;
    zeus_connection_t *tconn;
    zeus_size_t current_left_sz;
    zeus_size_t current_write_sz;
    zeus_size_t current_write_cnt = 0;

    while(ev->buffer->head){
        lnode = ev->buffer->head;
        current_buf = (zeus_buffer_t *)(lnode->d);
        current_left_sz = zeus_addr_delta(current_buf->last,current_buf->current);
        if(current_write_cnt ++ == ZEUS_WRITE_MAX_TIME_CNT){
            break;
        }
        if((current_write_sz = write(ev->connection->fd,(const void *)current_buf->current,current_left_sz)) == -1){
            terrno = errno;
            if(terrno == EAGAIN || terrno == EWOULDBLOCK || terrno == EINTR){
                return ZEUS_OK;
            }else{
                zeus_write_log(p->log,ZEUS_LOG_ERROR,"write to connection error : %s",strerror(errno));
                return ZEUS_ERROR;
            }
        }else if(current_write_sz < current_left_sz){
            current_buf->current = zeus_addr_add(current_buf->current,current_write_sz);
            ev->buflen -= current_write_sz;
            break;
        }else{
            current_buf->current = zeus_addr_add(current_buf->current,current_write_sz);
            ev->buflen -= current_write_sz;
            if(lnode->next){
                lnode->next->prev = NULL;
            }
            ev->buffer->head = lnode->next;
            if(!ev->buffer->head){
                ev->buffer->tail = NULL;
            }
            zeus_recycle_buffer_list_node_to_pool(p,lnode);
        }
    }

    
    if(ev->buffer->head == NULL){
        tconn = ev->connection;
        if(p->pidx == ZEUS_DATA_GATEWAY_PROCESS_INDEX){

            tconn->wrstatus = ZEUS_EVENT_OFF;
            tconn->rdstatus = ZEUS_EVENT_ON;
            if(!tconn->rd){

                if((tconn->rd = zeus_create_event(p)) == NULL){
                    zeus_write_log(p->log,ZEUS_LOG_ERROR,"create read event node error");
                    return ZEUS_ERROR;
                }
                tconn->rd->connection = tconn;
            }
            tconn->rd->handler = zeus_event_io_read;

        }else{
            tconn->wrstatus = ZEUS_EVENT_OFF;
        }
        if(zeus_helper_mod_event(p,tconn) == ZEUS_ERROR){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"modify event in zeus_event_io_write error");
            return ZEUS_ERROR;
        }
    }

    return ZEUS_OK;

}

zeus_status_t zeus_event_io_accept(zeus_process_t *p,zeus_event_t *ev){


    zeus_connection_t *conn;
    
    zeus_list_data_t *tnode; // For new list node
    zeus_connection_t *tconn;// For new connection
    zeus_int_t terror;

    //zeus_idx_t lowest;

    conn = ev->connection;

    if((tnode = zeus_create_connection_list_node(p)) == NULL){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process create accept fd connection node error");
        return ZEUS_ERROR;
    }

    tconn = (zeus_connection_t *)(tnode->d);

    if(!tconn->peer){
        if((tconn->peer = (zeus_sockaddr_in_t *)zeus_memory_alloc(p->pool,sizeof(zeus_sockaddr_in_t))) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process alloc sockaddr_in error");
            return ZEUS_ERROR;
        }

        if((tconn->peerlen = (zeus_socklen_t *)zeus_memory_alloc(p->pool,sizeof(zeus_socklen_t))) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process alloc socklen error");
            return ZEUS_ERROR;
        }

        if((tconn->addr_string = zeus_create_string(p,ZEUS_IPV4_ADDRESS_STRING_SIZE)) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process alloc address string error");
            return ZEUS_ERROR;
        }
    }

    if(inet_ntop(AF_INET,(const void *)&tconn->peer->sin_addr,tconn->addr_string->data,ZEUS_IPV4_ADDRESS_STRING_SIZE) == NULL){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process convert the connection address error : %s",strerror(errno));
        return ZEUS_ERROR;
    }
    
    /* Log the client information */
    zeus_write_log(p->log,ZEUS_LOG_NOTICE,"client %s:%hu connect to the server",\
                                          tconn->addr_string->data,ntohs(tconn->peer->sin_port));
    /* Log the client information */

    if((tconn->fd = accept(conn->fd,(zeus_sockaddr_t *)tconn->peer,tconn->peerlen)) == -1){
        terror = errno;
        zeus_recycle_connection_list_node_to_pool(p,tnode);
        if(terror == EAGAIN || terror == EWOULDBLOCK || terror == EINTR){
            return ZEUS_OK;
        }else{
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process accept new connection error : %s",strerror(terror));
            return ZEUS_ERROR;
        }
    }


    if(fcntl(tconn->fd,F_SETFL,O_NONBLOCK) == -1){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process set new connection fd nonblock error : %s",strerror(errno));
        zeus_recycle_connection_list_node_to_pool(p,tnode);
        return ZEUS_ERROR;
    }
    
/*
    p->worker_load[p->pidx] += 1;

    if(p->pidx == ZEUS_DATA_GATEWAY_PROCESS_INDEX){
        
        lowest = zeus_helper_find_lowest(p);
    
    }
*/

    if(!tconn->wr){
        if((tconn->wr = zeus_create_event(p)) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process alloc write event error");
            zeus_recycle_connection_list_node_to_pool(p,tnode);
            return ZEUS_ERROR;
        }
        tconn->wr->connection = tconn;
    }
    
    if(zeus_proto_ack_connection(p,tconn->wr) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process,generate ack connection package error");
        zeus_recycle_connection_list_node_to_pool(p,tnode);
        return ZEUS_ERROR;
    }

    tconn->wrstatus = ZEUS_EVENT_ON;
    tconn->wr->handler = zeus_event_io_write;

    zeus_helper_add_event(p,tconn);
    
    zeus_insert_list(p->connection,tnode);


    return ZEUS_OK;

}


zeus_status_t zeus_event_io_send_socket(zeus_process_t *p,zeus_event_t *ev){

    return ZEUS_OK;

}


zeus_status_t zeus_event_io_recv_socket(zeus_process_t *p,zeus_event_t *ev){

    return ZEUS_OK;

}
