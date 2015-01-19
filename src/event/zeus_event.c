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

	for(idx = 0 ; idx < lset_sz ; ++ idx){

		if(sigaddset(&lset,zeus_master_catch_signal[idx].signo) == -1){
			
			zeus_write_log(p->log,ZEUS_ERROR,"master listen signal %s error : %s",\
						   zeus_master_catch_signal[idx].signame,strerror(errno));

		}
	}

	while(1){
	
		sigsuspend(&lset);

		if(zeus_segv == 1){
			
		}

		if(zeus_quit == 1){
		
		}

		if(zeus_refork == 1){
		
		}
	
	}

}


zeus_status_t zeus_event_loop(zeus_process_t *p){
	
	pause();

}
