/*
 *	Email : ydzhao1992#gmail.com
 *	 Date : 2015-01-15
 */

#include "zeus_event.h"

zeus_event_t *zeus_create_event(zeus_process_t *p){
    
    zeus_event_t *alloc_event;
    
    alloc_event = (zeus_event_t*)zeus_memory_alloc(p->pool,sizeof(zeus_event_t));

    if(alloc_event == NULL){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"create event node error");
        return NULL;
    }

    alloc_event->connection = NULL;
    alloc_event->handler = NULL;

    alloc_event->buffer = zeus_create_list(p->pool,p->log);
    if(alloc_event->buffer == NULL){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"create event node buffer link list error");
        return NULL;
    }

    alloc_event->timeout_rbnode = NULL;
    alloc_event->timeout = ZEUS_EVENT_OFF;

    return alloc_event;

}

zeus_status_t zeus_master_event_loop(zeus_process_t *p){
	
    zeus_size_t idx;

    zeus_size_t lset_sz = sizeof(zeus_master_catch_signal)/sizeof(zeus_master_catch_signal[0]);

    sigset_t lset;

    if(sigemptyset(&lset) == -1){
        zeus_write_log(p->log,ZEUS_ERROR,"master empty listening set error : %s",strerror(errno));
    }

    while(1){
	
        sigsuspend(&lset);
		
        if(zeus_refork == 1){
	        
            zeus_write_log(p->log,ZEUS_LOG_NOTICE,"restart the gateway/child process");

            zeus_respawn(p);
            
            zeus_refork = 0;

        }

        if(zeus_quit == 1){

            zeus_write_log(p->log,ZEUS_LOG_NOTICE,"the process will quit.");

            zeus_inform_exit(p);

            exit(0);

        }

        if(zeus_segv == 1){

            zeus_write_log(p->log,ZEUS_LOG_ERROR,"master process recv SIGSEGV.");

            exit(-1);

        }
	
    }

    return ZEUS_OK;

}


zeus_status_t zeus_event_loop(zeus_process_t *p){
    
    zeus_timeval_t nt;
    zeus_event_timer_rbnode_t *tnode;

    if(zeus_event_loop_init_signal(p) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process init loop signal error",\
                      (p->pidx)?"worker":"gateway");
    }

    if(zeus_event_loop_init_connection(p) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process init loop connection error",\
                      (p->pidx)?"worker":"gateway");
    }
        
    if(zeus_event_create_epfd(p) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process create epoll fd error",\
                      (p->pidx)?"worker":"gateway");
    }

    if(zeus_event_timer_rbtree_construct(p) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process create timer rbtree error",\
                      (p->pidx)?"worker":"gateway");
    }

    if(zeus_event_init_epoll(p) == ZEUS_ERROR){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process add init fd error,",\
                      (p->pidx)?"worker":"gateway");
    }

    for(;;){ 
        
        if(zeus_quit == 1 || zeus_segv == 1){
            exit(0);
        }

        if(gettimeofday(&nt,NULL) == -1){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"get current time error");
            continue;
        }

        while((tnode = zeus_event_timer_rbtree_find_next(p->timer,p->timer->root)) != p->timer->nil){
            if(zeus_event_timer_rbtree_key_compare(&nt,&tnode->t) == ZEUS_EVENT_TIMER_GT){
                if(zeus_event_timer_rbtree_delete(p->timer,tnode) != ZEUS_OK){
                    zeus_write_log(p->log,ZEUS_LOG_ERROR,"delete timer tree node error");
                    continue;
                }else{
                    tnode->ev->timeout = ZEUS_EVENT_OFF;
                    tnode->ev->timeout_rbnode = NULL;
                    tnode->ev->handler(tnode->ev);
                    zeus_event_timer_rbnode_recycle(p->timer,tnode);
                }
            } 
        }



    }

}

zeus_status_t zeus_event_loop_init_signal(zeus_process_t *p){
    
    sigset_t lset;
    struct sigaction act;

    zeus_size_t idx;
    zeus_size_t master_catch_signal_size = sizeof(zeus_master_catch_signal)/\
                                           sizeof(zeus_master_catch_signal[0]);
    zeus_size_t gateworker_catch_signal_size = sizeof(zeus_gateworker_catch_signal)/\
                                               sizeof(zeus_gateworker_catch_signal[0]);

    act.sa_handler = SIG_DFL;
    if(sigemptyset(&act.sa_mask) == -1){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process empty handler mask error : %s",\
                      (p->pidx)?"worker":"gateway",strerror(errno));
        return ZEUS_ERROR;
    }
    act.sa_flags = 0;

    for(idx = 0 ; idx < master_catch_signal_size ; ++ idx){
        if(sigaction(zeus_master_catch_signal[idx].signo,&act,NULL) == -1){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process change signal %s handler error : %s",\
                          (p->pidx)?"worker":"gateway",zeus_master_catch_signal[idx].signame,strerror(errno));
            return ZEUS_ERROR;
        }
    }
    
    act.sa_handler = zeus_signal_handler;

    for(idx = 0 ; idx < gateworker_catch_signal_size ; ++ idx){
        if(sigaction(zeus_gateworker_catch_signal[idx].signo,&act,NULL) == -1){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process re-change signal %s handler error : %s",\
                          (p->pidx)?"worker":"gateway",zeus_master_catch_signal[idx].signame,strerror(errno));
            return ZEUS_ERROR;
        }
    }

    if(sigemptyset(&lset) == -1){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process empty mask signal error : %s",\
                      (p->pidx)?"worker":"gateway",strerror(errno));
        return ZEUS_ERROR;
    }

    if(sigprocmask(SIG_SETMASK,&lset,NULL) == -1){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process set signal mask error : %s",\
                      (p->pidx)?"worker":"gateway",strerror(errno));
        return ZEUS_ERROR;
    }
    
    return ZEUS_OK;

}

zeus_status_t zeus_event_loop_init_connection(zeus_process_t *p){
    
    zeus_list_data_t *node;
    zeus_connection_t *conn;
    zeus_event_t *ev;

    if((p->connection = zeus_create_list(p->pool,p->log)) == NULL){
        
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process create connection list error",\
                      (p->pidx)?"worker":"gateway");

        return ZEUS_ERROR;
    }

    if(p->pidx == 0){

        if((node = zeus_create_list_data_node(p->pool,p->log)) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process create listen connection link node error");
            return ZEUS_ERROR;
        }

        if((conn = zeus_create_connection_node(p)) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process create listen connection node error");
            return ZEUS_ERROR;
        }
        
        if((ev = zeus_create_event(p)) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process create listen connection event error");
            return ZEUS_ERROR;
        }

        ev->connection = conn;
        
        conn->fd = p->listenfd;
        conn->rd = ev;
        conn->rdstatus = ZEUS_EVENT_ON;
        ev->handler = zeus_event_io_accept;
        ev->timeout = ZEUS_EVENT_OFF;

        node->d = (void *)conn;
        
        zeus_insert_list(p->connection,node);

    }

    if((node = zeus_create_list_data_node(p->pool,p->log)) == NULL){

        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process create channel connection link node error",\
                      (p->pidx)?"worker":"gateway");
        return ZEUS_ERROR;

    }

    if((conn = zeus_create_connection_node(p)) == NULL){

        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process create channel connection node error",\
                      (p->pidx)?"worker":"gateway");
        return ZEUS_ERROR;

    }

    if((ev = zeus_create_event(p)) == NULL){
            
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process create channel connection event error",\
                      (p->pidx)?"worker":"gateway");

        return ZEUS_ERROR;

    }

    conn->fd = p->channel[p->pidx][0];
    conn->rd = ev;
    conn->rdstatus = ZEUS_EVENT_ON;
    ev->connection = conn;
    ev->handler = zeus_event_io_read;
    ev->timeout = ZEUS_EVENT_OFF;

    node->d = (void *)conn;

    zeus_insert_list(p->connection,node);
    
    return ZEUS_OK;
}

zeus_status_t zeus_event_create_epfd(zeus_process_t *p){
 
    if((p->epfd = epoll_create(p->max_connection)) == -1){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process create epoll fd error : %s",\
                      (p->pidx)?"worker":"gateway",strerror(errno));
        return ZEUS_ERROR;
    }

    return ZEUS_OK;

}

zeus_status_t zeus_event_init_epoll(zeus_process_t *p){
    
    zeus_list_data_t *cnode = p->connection->head; 
    zeus_connection_t *conn;

    while(cnode){

        conn = (zeus_connection_t *)(cnode->d);

        if(zeus_helper_add_event(p,conn) == ZEUS_ERROR){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process init epoll add fd event error",\
                          (p->pidx)?"worker":"gateway");
            cnode = cnode->next;
            continue;
        }

        cnode = cnode->next;

    }


    return ZEUS_OK;

}
