/*
 *	Email : ydzhao1992#gmail.com
 *	 Date : 2015-01-15
 */

#include "zeus_event.h"

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

}


zeus_status_t zeus_event_loop(zeus_process_t *p){
    

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
    }
    act.sa_flags = 0;

    for(idx = 0 ; idx < master_catch_signal_size ; ++ idx){
        if(sigaction(zeus_master_catch_signal[idx].signo,&act,NULL) == -1){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process change signal %s handler error : %s",\
                          (p->pidx)?"worker":"gateway",zeus_master_catch_signal[idx].signame,strerror(errno));
        }
    }
    
    act.sa_handler = zeus_signal_handler;

    for(idx = 0 ; idx < gateworker_catch_signal_size ; ++ idx){
        if(sigaction(zeus_gateworker_catch_signal[idx].signo,&act,NULL) == -1){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process re-change signal %s handler error : %s",\
                          (p->pidx)?"worker":"gateway",zeus_master_catch_signal[idx].signame,strerror(errno));
        }
    }

    if(sigemptyset(&lset) == -1){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process empty mask signal error : %s",\
                      (p->pidx)?"worker":"gateway",strerror(errno));
    }

    if(sigprocmask(SIG_SETMASK,&lset,NULL) == -1){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s process set signal mask error : %s",\
                      (p->pidx)?"worker":"gateway",strerror(errno));
    }

    pause();

}
