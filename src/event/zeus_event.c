/*
 *	Email : ydzhao1992#gmail.com
 *	 Date : 2015-01-15
 */

#include "zeus_event.h"

zeus_event_t *zeus_create_event(zeus_process_t *p,zeus_connection_t *conn){
    
    zeus_event_t *alloc_event;
    
    alloc_event = (zeus_event_t*)zeus_memory_alloc(p->pool,sizeof(zeus_event_t));

    if(alloc_event == NULL){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"create event node error");
        return NULL;
    }

    alloc_event->connection = conn;
    
    alloc_event->buffer = zeus_create_list(p->pool,p->log);
    if(alloc_event->buffer == NULL){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"create event buffer list error");
        return NULL;
    }

    alloc_event->handler = NULL;
    alloc_event->timeout_rbnode = NULL;
    alloc_event->timeout_handler = NULL;
    alloc_event->timeout = ZEUS_EVENT_OFF;
    alloc_event->buflen = 0;

    return alloc_event;

}

zeus_status_t zeus_master_event_loop(zeus_process_t *p){
	
    sigset_t lset;

    if(sigemptyset(&lset) == -1){
        zeus_write_log(p->log,ZEUS_ERROR,"master empty listening set error : %s",strerror(errno));
    }

    while(1){
	
        sigsuspend(&lset);
		
        if(zeus_refork == 1){
            
            zeus_refork = 0;

            zeus_write_log(p->log,ZEUS_LOG_NOTICE,"restart the gateway/child process");

            zeus_respawn(p);
            
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
    
    zeus_event_timer_rbnode_t *tnode;
    zeus_itimerval_t itimer;


    zeus_epoll_event_t *loopev;
    zeus_int_t nev;
    zeus_int_t terrno;
    zeus_int_t tidx;
    zeus_connection_t *tconn;

    
    zeus_uint_t old_worker_load;

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

    if((loopev = (zeus_epoll_event_t *)zeus_memory_alloc(p->pool,\
                 sizeof(zeus_epoll_event_t) * ZEUS_EVENT_CNT)) == NULL){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process alloc loop event error",\
                      (p->pidx)?"worker":"gateway");
    }

    itimer.it_interval.tv_sec = 0;
    itimer.it_interval.tv_usec = 0;
    p->cache_time = zeus_get_cache_timeval();

    for(;;){ 

        if(p->pidx != ZEUS_DATA_GATEWAY_PROCESS_INDEX){
            old_worker_load = p->worker_load[p->pidx];
        }
        
        if(zeus_quit == 1){
            zeus_write_log(p->log,ZEUS_LOG_NOTICE,"%s process will quit",(p->pidx)?"worker":"gateway");
            exit(0);
        }

        if(zeus_segv == 1){
            zeus_write_log(p->log,ZEUS_LOG_NOTICE,"%s process recv SIGSEGV",(p->pidx)?"worker":"gateway");
            exit(0);
        }
        
        itimer.it_value.tv_sec = p->resolution / ZEUS_EVENT_TIMER_S_TO_MS;
        itimer.it_value.tv_usec = (p->resolution % ZEUS_EVENT_TIMER_S_TO_MS) * ZEUS_EVENT_TIMER_MS_TO_US;

        if(setitimer(ITIMER_REAL,&itimer,NULL) == -1){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process setitimer error : %s",\
                          (p->pidx)?"worker":"gateway",strerror(errno));
            continue;
        }


        if((nev = epoll_wait(p->epfd,loopev,ZEUS_EVENT_CNT,-1)) >= 0){

            zeus_update_time();
            
            for(tidx = 0 ; tidx < nev ; ++ tidx){
                
                tconn = (zeus_connection_t *)(loopev[tidx].data.ptr);
                
                if(loopev[tidx].events & EPOLLIN){
                    tconn->rd->handler(p,tconn->rd);
                }

                if(loopev[tidx].events & EPOLLOUT){
                    tconn->wr->handler(p,tconn->wr);
                }
            
            }
        
        }else{

            terrno = errno;

            if(terrno == EINTR){
                //
            }else{
                zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process epoll_wait error : %s",\
                              (p->pidx)?"worker":"gateway",strerror(terrno));
            }
        
        }

        zeus_update_time();

        while((tnode = zeus_event_timer_rbtree_find_next(p->timer,p->timer->root)) != p->timer->nil){
            if(zeus_event_timer_rbtree_key_compare(p->cache_time,&tnode->t) == ZEUS_EVENT_TIMER_GT){
                    tnode->ev->timeout_handler(p,tnode->ev);
            }else{
                break;
            }
        }

        if(p->pidx != ZEUS_DATA_GATEWAY_PROCESS_INDEX && old_worker_load != p->worker_load[p->pidx]){
            if(zeus_proto_send_update_workload_packet(p,p->ipc_connection->wr) == ZEUS_ERROR){
                zeus_write_log(p->log,ZEUS_LOG_ERROR,"worker process send back update workload error");
                return ZEUS_ERROR;
            }
            if(p->ipc_connection->wrstatus == ZEUS_EVENT_OFF){
                p->ipc_connection->wrstatus = ZEUS_EVENT_ON;
                if(zeus_helper_mod_event(p,p->ipc_connection) == ZEUS_ERROR){
                    zeus_write_log(p->log,ZEUS_LOG_ERROR,"worker process set send back update workload event error");
                    return ZEUS_ERROR;
                }
            }
        }

    }
    
    return ZEUS_OK;

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
    zeus_idx_t idx;

    if((p->admin_connection = zeus_create_list(p->pool,p->log)) == NULL){

        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process create admin connection list error",\
                       (p->pidx)?"worker":"gateway");
        return ZEUS_ERROR;
    
    }
    
    if((p->client_connection = zeus_create_list(p->pool,p->log)) == NULL){

        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process create client connection list error",\
                       (p->pidx)?"worker":"gateway");
        return ZEUS_ERROR;
    
    }


    if(p->pidx == ZEUS_DATA_GATEWAY_PROCESS_INDEX){

        if((node = zeus_create_connection_list_node(p)) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process create listen connection error");
            return ZEUS_ERROR;
        }

        conn = (zeus_connection_t *)(node->d);
        conn->fd = p->listenfd;
        conn->rd->handler = zeus_event_io_accept;
        conn->rdstatus = ZEUS_EVENT_ON;
        conn->quiting = 0;
        
        p->listen_connection_node = node;
        
        if((p->ipc_connection = zeus_create_connection_array(p,p->worker)) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process create ipc connection array error",\
                           (p->pidx)?"worker":"gateway");
            return ZEUS_ERROR;
        }

        for(idx = 0 ; idx < p->worker ; ++ idx){
            
            conn = &(p->ipc_connection[idx]);
            conn->fd = p->channel[idx][0];
            conn->rd->handler = zeus_event_io_read;
            conn->rdstatus = ZEUS_EVENT_ON;
            
            if(zeus_proto_helper_set_connection_privilege(conn,ZEUS_PROTO_TRANS_SOCKET_ACK_INS) == ZEUS_ERROR){
                zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway open recv trans socket ack privilege error");
                return ZEUS_ERROR;
            }

            if(zeus_proto_helper_set_connection_privilege(conn,ZEUS_PROTO_RESET_LOAD_BALANCE_INS) == ZEUS_ERROR){
                zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway open recv reset load balance privilege error");
                return ZEUS_ERROR;
            }

            if(zeus_proto_helper_set_connection_privilege(conn,ZEUS_PROTO_UPDATE_LOAD_INS) == ZEUS_ERROR){
                zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway open update work load privilege error");
                return ZEUS_ERROR;
            }

            conn->wr->handler = zeus_event_io_send_socket;
            conn->wrstatus = ZEUS_EVENT_OFF;
            conn->quiting = 0;

        }

    }else{

        if((p->ipc_connection = zeus_create_connection_array(p,1)) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process create ipc connection array error",\
                           (p->pidx)?"worker":"gateway");
            return ZEUS_ERROR;
        }
        
        conn = p->ipc_connection;
        conn->fd = p->channel[p->pidx - 1][1];
        conn->rd->handler = zeus_event_io_recv_socket;
        conn->rdstatus = ZEUS_EVENT_ON;
    
        zeus_proto_send_reset_load_balance_packet(p,conn->wr);
        conn->wr->handler = zeus_event_io_write;
        conn->wrstatus = ZEUS_EVENT_ON;
        conn->quiting = 0;

    }

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
    
    zeus_uint_t idx;

    if(p->pidx == ZEUS_DATA_GATEWAY_PROCESS_INDEX){
        
        for(idx = 0 ; idx < p->worker ; ++ idx){
            if(zeus_helper_add_event(p,&p->ipc_connection[idx]) == ZEUS_ERROR){
                zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process init epoll add ipc connection event error");
                return ZEUS_ERROR;
            }
        }

        if(zeus_helper_add_event(p,(zeus_connection_t *)(p->listen_connection_node->d)) == ZEUS_ERROR){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process init epoll add listen socket event error");
            return ZEUS_ERROR;
        }

    }else{
    
        if(zeus_helper_add_event(p,p->ipc_connection) == ZEUS_ERROR){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"worker process init epoll add ipc connection event error");
            return ZEUS_ERROR;
        }
        
    }

    return ZEUS_OK;

}
