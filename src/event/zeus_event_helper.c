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
