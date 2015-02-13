/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2015-01-23
 */

// This is the event io handler

#include "zeus_event_io.h"

zeus_status_t zeus_event_io_read(zeus_process_t *p,zeus_event_t *ev){

    return ZEUS_OK;

}

zeus_status_t zeus_event_io_write(zeus_process_t *p,zeus_event_t *ev){

    return ZEUS_OK;

}

zeus_status_t zeus_event_io_accept(zeus_process_t *p,zeus_event_t *ev){


    zeus_connection_t *conn;
    
    zeus_list_data_t *tnode; // For new list node
    zeus_connection_t *tconn;// For new connection
    zeus_int_t terror;

    conn = ev->connection;

    if(!p->connection_pool){
        
        if((tnode = zeus_create_list_data_node(p->pool,p->log)) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process create accept fd list data node error");
            return ZEUS_ERROR;
        }

        if((tconn = zeus_create_connection_node(p)) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process create accept connection node error");
            return ZEUS_ERROR;
        }

        tnode->d = (void *)tconn;
        
    
    }else{
        // From object pool
    }

    if(!tconn->peer){
        if((tconn->peer = (zeus_sockaddr_in_t *)zeus_memory_alloc(p->pool,sizeof(zeus_sockaddr_in_t))) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process alloc sockaddr_in error");
            return ZEUS_ERROR;
        }

        if((tconn->peerlen = (zeus_socklen_t *)zeus_memory_alloc(p->pool,sizeof(zeus_socklen_t))) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process alloc socklen error");
            return ZEUS_ERROR;
        }
    }

    if((tconn->fd = accept(conn->fd,(zeus_sockaddr_t *)tconn->peer,tconn->peerlen)) == -1){
        terror = errno;
        if(terror == EAGAIN || terror == EWOULDBLOCK || terror == EINTR){
            return ZEUS_OK;
        }else{
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process accept new connection error : %s",strerror(terror));
            //recycle connection node here
            return ZEUS_ERROR;
        }
    }

    if(fcntl(tconn->fd,F_SETFL,O_NONBLOCK) == -1){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process set new connection fd nonblock error : %s",strerror(errno));
        //recycle connection node here
        return ZEUS_ERROR;
    }

    if(!tconn->wr){
        if((tconn->wr = zeus_create_event(p)) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process alloc write event error");
            //recycle connection node here
            return ZEUS_ERROR;
        }
    }

    tconn->wrstatus = ZEUS_EVENT_ON;
    tconn->wr->handler = zeus_proto_ack_connection;
    tconn->wr->connection = tconn;

    zeus_helper_add_event(p,tconn);

    zeus_insert_list(p->connection,tnode);

    return ZEUS_OK;

}
