/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2015-02-11
 */

// This is the implemention of event helper

#include "zeus_event_helper.h"

zeus_status_t zeus_helper_add_event(zeus_process_t *p,zeus_connection_t *conn){
    
    zeus_epoll_event_t epev;

    if(conn->rdstatus == ZEUS_EVENT_ON && conn->wrstatus == ZEUS_EVENT_ON){
        epev.events = EPOLLIN | EPOLLOUT;
    }else if(conn->rdstatus == ZEUS_EVENT_ON){
        epev.events = EPOLLIN;
    }else if(conn->wrstatus == ZEUS_EVENT_ON){
        epev.events = EPOLLOUT;
    }else{
        return ZEUS_OK;
    }
    epev.data.ptr = (void *)conn;
    
    if(epoll_ctl(p->epfd,EPOLL_CTL_ADD,conn->fd,&epev) == -1){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process add event error : %s",\
                      (p->pidx)?"worker":"gateway",strerror(errno));
        return ZEUS_ERROR;
    }

    if(conn->rdstatus == ZEUS_EVENT_ON && conn->rd->timeout == ZEUS_EVENT_ON){

        if(zeus_event_timer_rbtree_insert(p->timer,conn->rd->timeout_rbnode) == ZEUS_ERROR){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process add read timeout event error",\
                          (p->pidx)?"worker":"gateway",strerror(errno));
            return ZEUS_ERROR;
        }
    
    }

    if(conn->wrstatus == ZEUS_EVENT_ON && conn->wr->timeout == ZEUS_EVENT_ON){

        if(zeus_event_timer_rbtree_insert(p->timer,conn->wr->timeout_rbnode) == ZEUS_ERROR){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process add write timeout event error",\
                          (p->pidx)?"worker":"gateway",strerror(errno));
            return ZEUS_ERROR;
        }
    
    }

    return ZEUS_OK;

}

zeus_status_t zeus_helper_del_event(zeus_process_t *p,zeus_connection_t *conn){

    if(epoll_ctl(p->epfd,EPOLL_CTL_DEL,conn->fd,NULL) == -1){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process del event error in epoll_ctl : %s",
                      (p->pidx)?"worker":"gateway",strerror(errno));
        return ZEUS_ERROR;
    }

    return ZEUS_OK;

}

zeus_status_t zeus_helper_mod_event(zeus_process_t *p,zeus_connection_t *conn){
    
    zeus_event_timer_rbnode_t *rbnode;
    zeus_epoll_event_t epev;

    if(conn->rdstatus == ZEUS_EVENT_OFF){
        if(conn->rd){
            if(conn->rd->timeout == ZEUS_EVENT_ON){
                conn->rd->timeout = ZEUS_EVENT_OFF;
                rbnode = conn->rd->timeout_rbnode;
                conn->rd->timeout_rbnode = NULL;
                if(zeus_event_timer_rbtree_delete(p->timer,rbnode) == ZEUS_ERROR){
                    zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process delete rbnode in zeus_helper_mod_event error",\
                                  (p->pidx)?"worker":"gateway");
                    return ZEUS_ERROR;
                }
                zeus_recycle_event_timer_rbnode_to_pool(p->timer,rbnode);
            }
        }
    }

    if(conn->wrstatus == ZEUS_EVENT_OFF){
        if(conn->wr){
            if(conn->wr->timeout == ZEUS_EVENT_ON){
                conn->wr->timeout = ZEUS_EVENT_OFF;
                rbnode = conn->wr->timeout_rbnode;
                conn->wr->timeout_rbnode = NULL;
                if(zeus_event_timer_rbtree_delete(p->timer,rbnode) == ZEUS_ERROR){
                    zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process delete rbnode in zeus_helper_del_event error",\
                                  (p->pidx)?"worker":"gateway");
                    return ZEUS_ERROR;
                }
                zeus_recycle_event_timer_rbnode_to_pool(p->timer,rbnode);
            }
        }
    }

    if(conn->rdstatus == ZEUS_EVENT_OFF && conn->wrstatus == ZEUS_EVENT_OFF){
        return zeus_helper_del_event(p,conn);
    }else{
        if(conn->rdstatus == ZEUS_EVENT_ON && conn->wrstatus == ZEUS_EVENT_ON){
            epev.events = EPOLLIN | EPOLLOUT;
        }else if(conn->rdstatus == ZEUS_EVENT_ON){
            epev.events = EPOLLIN;
        }else{
            epev.events = EPOLLOUT;
        }

        if(conn->rdstatus == ZEUS_EVENT_ON && conn->rd->timeout == ZEUS_EVENT_ON && !conn->rd->timeout_rbnode->lchild){

            if(zeus_event_timer_rbtree_insert(p->timer,conn->rd->timeout_rbnode) == ZEUS_ERROR){
                zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process insert rbnode in mod event error",\
                              (p->pidx)?"worker":"gateway");
                return ZEUS_ERROR;
            }
        
        }

        if(conn->wrstatus == ZEUS_EVENT_ON && conn->wr->timeout == ZEUS_EVENT_ON && !conn->wr->timeout_rbnode->lchild){

            if(zeus_event_timer_rbtree_insert(p->timer,conn->wr->timeout_rbnode) == ZEUS_ERROR){
                zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process insert rbnode in mod event error",\
                              (p->pidx)?"worker":"gateway");
                return ZEUS_ERROR;
            }
        
        }
        
        epev.data.ptr = (void *)conn;

        if(epoll_ctl(p->epfd,EPOLL_CTL_MOD,conn->fd,&epev) == ZEUS_ERROR){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process call epoll_ctl mod event error : %s",\
                          (p->pidx)?"worker":"gateway",strerror(errno));
            return ZEUS_ERROR;
        }

    }

    return ZEUS_OK;

}

zeus_status_t zeus_helper_close_connection(zeus_process_t *p,zeus_connection_t* conn){

    zeus_list_data_t *node = conn->node;

    conn->rdstatus = ZEUS_EVENT_OFF;
    conn->wrstatus = ZEUS_EVENT_OFF;
    
    if(zeus_helper_mod_event(p,conn) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process close connection error when del event",\
                                             (p->pidx)?"worker":"gateway");
        return ZEUS_ERROR;
    }
    
    if(shutdown(conn->fd,SHUT_RDWR) == -1){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process close connection error when shutdown fd",\
                                             (p->pidx)?"worker":"gateway");
        return ZEUS_ERROR;
    }
    
    // TODO 
    // Check manage connection list
    
    zeus_delete_list(p->connection,node);
   
    
    if(zeus_recycle_connection_list_node_to_pool(p,conn->node) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process close connection error when recycle connection list node",\
                                             (p->pidx)?"worker":"gateway");
        return ZEUS_ERROR;
    }

    
    return ZEUS_OK;

}

zeus_idx_t zeus_helper_find_load_lowest(zeus_process_t *p){
    
    zeus_int_t idx;

    zeus_uint_t mval = p->worker_load[0];
    zeus_idx_t loc = 0;

    for(idx = 0 ; idx < p->worker ; ++ idx){

        if(mval > p->worker_load[idx]){
            mval = p->worker_load[idx];
            loc = idx;
        }
        
    }

    return loc;

}

zeus_status_t zeus_helper_trans_socket(zeus_process_t *p,zeus_connection_t *conn,zeus_idx_t idx){

    zeus_list_data_t *node = conn->node;
    zeus_list_data_t *r = p->connection->head;
    zeus_connection_t *c;
    
    if(!r){
        goto connection_error;
    }
    
    while(idx --){
        if(!r->next){
            goto connection_error;
        }
        r = r->next;
        if(!r){
            goto connection_error;
        }
    }
    
    zeus_delete_list(p->connection,node);

    conn->rdstatus = ZEUS_EVENT_OFF;
    conn->wrstatus = ZEUS_EVENT_OFF;

    if(zeus_helper_mod_event(p,conn) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"reset trans socket event error");
        return ZEUS_ERROR;
    }
    
    c = (zeus_connection_t *)r->d;
    c->wrstatus = ZEUS_EVENT_ON;
    
    zeus_insert_list(c->wr->buffer,node);
    c->wr->buflen += 1;
    
    if(zeus_helper_mod_event(p,c) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"set trans socket event error");
        return ZEUS_ERROR;
    }

    return ZEUS_OK;
    
connection_error:
    zeus_write_log(p->log,ZEUS_LOG_ERROR,"something wrong with connection list");
    return ZEUS_ERROR;

}

zeus_status_t zeus_helper_move_to_closing_connection_list(zeus_process_t *p,zeus_connection_t *conn,zeus_idx_t idx){

    zeus_list_data_t *node = conn->node;
    zeus_insert_list(p->closing_connection[idx],node);
    return ZEUS_OK;

}

zeus_status_t zeus_helper_unlink_connection_at_closing_list(zeus_process_t *p,zeus_idx_t idx){
    
    zeus_connection_t *conn;
    zeus_list_data_t *node = p->closing_connection[idx]->head;
    
    if(node == NULL){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"closing connection list at index %d is empty",idx);
        return ZEUS_ERROR;
    }

    zeus_delete_list(p->closing_connection[idx],node);
    
    conn = (zeus_connection_t *)node->d;
    if(close(conn->fd) == -1){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"closing the socket fd at closing list error : %s",strerror(errno));
        return ZEUS_ERROR;
    }

    zeus_recycle_connection_list_node_to_pool(p,node);
    
    return ZEUS_OK;
}
