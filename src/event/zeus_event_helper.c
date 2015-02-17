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
    zeus_list_data_t *tbuf;
    zeus_epoll_event_t epev;

    if(conn->rdstatus == ZEUS_EVENT_OFF){
        if(conn->rd){
            if(conn->rd->timeout == ZEUS_EVENT_ON){
                conn->rd->timeout = ZEUS_EVENT_OFF;
                rbnode = conn->rd->timeout_rbnode;
                conn->rd->timeout_rbnode = NULL;
                if(zeus_event_timer_rbtree_delete(p->timer,rbnode) == ZEUS_ERROR){
                    zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process delete rbnode in zeus_helper_del_event error",\
                                  (p->pidx)?"worker":"gateway");
                    return ZEUS_ERROR;
                }
                zeus_recycle_event_timer_rbnode_to_pool(p->timer,rbnode);
            }
            while(conn->rd->buffer->head){
                tbuf = conn->rd->buffer->head;
                conn->rd->buffer->head = tbuf->next;
                if(conn->rd->buffer->head == NULL){
                    conn->rd->buffer->tail = NULL;
                }
                zeus_recycle_buffer_list_node_to_pool(p,tbuf);
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
            while(conn->wr->buffer->head){
                tbuf = conn->wr->buffer->head;
                conn->wr->buffer->head = tbuf->next;
                if(conn->wr->buffer->head == NULL){
                    conn->wr->buffer->tail = NULL;
                }
                zeus_recycle_buffer_list_node_to_pool(p,tbuf);
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
