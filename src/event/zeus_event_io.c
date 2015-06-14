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

        if(current_read_cnt ++ == ZEUS_READ_MAX_TIME_CNT){
            break;
        }

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
        
        if((current_read_sz = read(ev->connection->fd,current_buf->last,current_left_sz)) == -1){
            terrno = errno;
            if(terrno == EAGAIN || terrno == EWOULDBLOCK){
                break;
            }else if(terrno == EINTR){
                continue;
            }else{
                zeus_write_log(p->log,ZEUS_LOG_ERROR,"read from the connection error : %s",strerror(terrno));
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
            break;
        }
    }

    if(ev->connection->quiting == 1){
        zeus_helper_close_connection(p,ev->connection);
    }else{
        /* set new deadline */
        if(ev->timeout == ZEUS_EVENT_ON && ev->timeout_rbnode->lchild != NULL){
            if(zeus_event_timer_rbtree_delete(p->timer,ev->timeout_rbnode) == ZEUS_ERROR){
                zeus_write_log(p->log,ZEUS_LOG_ERROR,"delete rbtree node to modify deadline error");
                return ZEUS_ERROR;
            }
            ev->timeout_rbnode->t.tv_sec = p->cache_time->tv_sec + ZEUS_EVENT_MAX_DELAY;
            ev->timeout_rbnode->t.tv_usec = p->cache_time->tv_usec;
            if(zeus_event_timer_rbtree_insert(p->timer,ev->timeout_rbnode) == ZEUS_ERROR){
                zeus_write_log(p->log,ZEUS_LOG_ERROR,"insert rbtree node to modify deadline error");
                return ZEUS_ERROR;
            }
        }
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
            zeus_delete_list(ev->buffer,lnode);
            zeus_recycle_buffer_list_node_to_pool(p,lnode);
        }
    }

    
    if(ev->buffer->head == NULL){
        tconn = ev->connection;

        if(p->pidx == ZEUS_DATA_GATEWAY_PROCESS_INDEX){

            tconn->wrstatus = ZEUS_EVENT_OFF;
            tconn->rdstatus = ZEUS_EVENT_ON;
            tconn->rd->handler = zeus_event_io_read;
           
            tconn->rd->timeout = ZEUS_EVENT_ON;
            if((tconn->rd->timeout_rbnode = zeus_event_timer_create_rbnode(p)) == NULL){
                zeus_write_log(p->log,ZEUS_LOG_ERROR,"create new connection rbnode error");
                return ZEUS_ERROR;
            }
            tconn->rd->timeout_rbnode->ev = tconn->rd;
            tconn->rd->timeout_rbnode->t.tv_sec = p->cache_time->tv_sec + ZEUS_EVENT_MAX_DELAY;
            tconn->rd->timeout_rbnode->t.tv_usec = p->cache_time->tv_usec;
            tconn->rd->timeout_handler = zeus_helper_timeout_handler;
            
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

        *(tconn->peerlen) = sizeof(zeus_sockaddr_in_t);
    }

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
    
    if(inet_ntop(AF_INET,(const void *)&tconn->peer->sin_addr,tconn->addr_string->data,ZEUS_IPV4_ADDRESS_STRING_SIZE) == NULL){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process convert the connection address error : %s",strerror(errno));
        return ZEUS_ERROR;
    }
    
    /* Log the client information */
    zeus_write_log(p->log,ZEUS_LOG_NOTICE,"client %s:%hu connect to the server",\
                                          tconn->addr_string->data,ntohs(tconn->peer->sin_port));
    /* Log the client information */


    if(fcntl(tconn->fd,F_SETFL,O_NONBLOCK) == -1){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process set new connection fd nonblock error : %s",strerror(errno));
        zeus_recycle_connection_list_node_to_pool(p,tnode);
        return ZEUS_ERROR;
    }
    
    p->worker_load[p->pidx] += 1;
    
    if(zeus_proto_helper_set_connection_privilege(tconn,ZEUS_PROTO_CLIENT_CHECKOUT_INS) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process set new accept connection client check privilege error");
        return ZEUS_ERROR;
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

    zeus_event_io_trans_socket_t trans_socket;
    struct msghdr msg;

    zeus_list_data_t *node;
    node = ev->buffer->head;

    struct iovec iov[1];
    zeus_char_t buf[1] = {0};
    iov[0].iov_base = (void *)buf;
    iov[0].iov_len = 1;

    if(node == NULL){
        zeus_write_log(p->log,ZEUS_LOG_NOTICE,"the sending connection list is empty");
        return ZEUS_OK;
    }

    ev->buflen -= 1;

    zeus_delete_list(ev->buffer,node);

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;
    
    trans_socket.cmsg.cmsg_level = SOL_SOCKET;
    trans_socket.cmsg.cmsg_type = SCM_RIGHTS;
    trans_socket.cmsg.cmsg_len = CMSG_LEN(sizeof(int));

    msg.msg_control = (void *)&trans_socket.cmsg;
    msg.msg_controllen = CMSG_LEN(sizeof(int));

    *(zeus_int_t *)CMSG_DATA(&trans_socket.cmsg) = ((zeus_connection_t *)(node->d))->fd;

    if(sendmsg(ev->connection->fd,&msg,0) == -1){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"send file descriptor error : %s",strerror(errno));
        return ZEUS_ERROR;
    }

    if(close(((zeus_connection_t *)node->d)->fd) == -1){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"close trans socket file descriptor in gateway process error");
        return ZEUS_ERROR;
    }

    zeus_recycle_connection_list_node_to_pool(p,node);
    
    if(ev->buflen == 0){
        ev->connection->wrstatus = ZEUS_EVENT_OFF;
        zeus_helper_mod_event(p,ev->connection);
    }

    zeus_write_log(p->log,ZEUS_LOG_NOTICE,"gateway process have send a socket file descriptor to worker");

    return ZEUS_OK;

}


zeus_status_t zeus_event_io_recv_socket(zeus_process_t *p,zeus_event_t *ev){
    
    zeus_event_io_trans_socket_t trans_socket;
    struct msghdr msg;

    zeus_list_data_t *node;
    zeus_connection_t *conn;
    
    struct iovec iov[1];
    zeus_char_t buf[1];
    iov[0].iov_base = (void *)buf;
    iov[0].iov_len = 1;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;
    
    msg.msg_control = (void *)&trans_socket.cmsg;
    msg.msg_controllen = CMSG_LEN(sizeof(int));

    if(recvmsg(ev->connection->fd,&msg,0) == -1){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"recv socket fd error : %s",strerror(errno));
        return ZEUS_ERROR;
    }

    if((node = zeus_create_connection_list_node(p)) == NULL){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"create new connection list node error");
        return ZEUS_ERROR;
    }
    
    conn = (zeus_connection_t *)(node->d);
    conn->fd = *(zeus_int_t *)CMSG_DATA(&trans_socket.cmsg);
    conn->quiting = 0;

    if(fcntl(conn->fd,F_SETFL,O_NONBLOCK) == -1){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"set new connection nonblock error : %s",strerror(errno));
        return ZEUS_ERROR;
    }

    conn->rdstatus = ZEUS_EVENT_ON;
    conn->rd->handler = zeus_event_io_read;
    conn->rd->connection = conn;

    if(zeus_proto_helper_set_connection_privilege(conn,ZEUS_PROTO_DATA_INS) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"worker process set new connection recv data privilege error");
        return ZEUS_ERROR;
    }

    if(zeus_proto_helper_set_connection_privilege(conn,ZEUS_PROTO_CLIENT_CHECKOUT_INS) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"worker process set new connection check client privilege error");
        return ZEUS_ERROR;
    }

    if(zeus_helper_add_event(p,conn) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"add new connection read event error");
        return ZEUS_ERROR;
    }

    zeus_insert_list(p->connection,node);

    if(zeus_proto_trans_socket_ack(p,ev->connection->wr) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"generate send back trans socket ack packet error");
        return ZEUS_ERROR;
    }

    ev->connection->wrstatus = ZEUS_EVENT_ON;

    if(zeus_helper_mod_event(p,ev->connection) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"set send back trans socket ack packet error");
        return ZEUS_ERROR;
    }

    p->worker_load[p->pidx] += 1;

    zeus_write_log(p->log,ZEUS_LOG_NOTICE,"worker process have recv a socket file descriptor from gateway");
    
    return ZEUS_OK;

}
