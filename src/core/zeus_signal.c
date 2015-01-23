/*
 *	Email : ydzhao1992#gmail.com
 *	 Date : 2015-01-16
 */

#include "zeus_signal.h"

extern zeus_process_t *process;

void zeus_master_signal_handler(int signo){
    
    zeus_int_t olderrno = errno;
    zeus_pid_t pid;
    zeus_size_t idx;

	switch(signo){

		case SIGINT:
		case SIGQUIT:
		case SIGTERM:
		case SIGHUP:
            
            zeus_write_log(process->log,ZEUS_LOG_NOTICE,"master process recieves quit signals");

			zeus_quit = 1;
			break;

		case SIGCHLD:
            
            zeus_write_log(process->log,ZEUS_LOG_NOTICE,"master process recieves SIGCHLD signal");

            while((pid = waitpid(-1,NULL,WNOHANG)) > 0){
                for(idx = 0 ; idx < (process->worker + 1) ; ++ idx){
                    if(process->child[idx] == pid){
                        process->child[idx] = -1;
                    }
                    if(!zeus_refork && !zeus_quit){
                        zeus_refork = 1;
                    }
                }
            }

			break;

		case SIGSEGV:

            zeus_write_log(process->log,ZEUS_LOG_NOTICE,"master process recieves SIGSEGV signal");

			zeus_segv = 1;
			break;

		default:
			break;
	}

    errno = olderrno;
	
	return ;
}


void zeus_signal_handler(int signo){
    
    zeus_int_t olderrno = errno;

	switch(signo){

		case SIGINT:
		case SIGQUIT:
		case SIGTERM:
            
            zeus_write_log(process->log,ZEUS_LOG_NOTICE,"%s process recieves quit signals",\
                          (process->pidx)?"worker":"gateway");

            exit(0);

            break;

		case SIGSEGV:
            
            zeus_write_log(process->log,ZEUS_LOG_NOTICE,"%s process recieves SIGSEGV signals",\
                          (process->pidx)?"worker":"gateway");
            
            exit(0);

			break;

		case SIGALRM:
			break;
		default:
			break;

	}

    
    errno = olderrno;

	return ;
	
}
