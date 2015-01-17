/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2014-12-24
 */

#include "zeus.h"

zeus_process_t *process;

int main(int argc,char *argv[]){
    
    if((process = zeus_create_process()) == NULL){
        exit(-1);
    }

	if(zeus_record_command_and_env(process,argc,argv) == ZEUS_ERROR){
		exit(-1);
	}
    
	if(zeus_init_process(process) == ZEUS_ERROR){
        exit(-1);
    }

	if(zeus_init_daemon(process) == ZEUS_ERROR){
		exit(-1);
	}

	if(zeus_daemon(process) == ZEUS_ERROR){
		exit(-1);
	}

    return 0;

}

zeus_status_t zeus_check_already_run(zeus_process_t *p){
	
	struct flock fl;

	if((p->pid_run_flag_fd = open(process->pid_run_flag_path->data,O_RDWR | O_CREAT | O_APPEND)) == -1){
		zeus_write_log(p->log,ZEUS_LOG_ERROR,"open pid file error : %s",strerror(errno));
		return ZEUS_ERROR;
	}
	
	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;
	fl.l_pid = getpid();

	if(fcntl(p->pid_run_flag_fd,F_SETLK,&fl) == -1){
		if(errno == EAGAIN || errno == EACCES){
			zeus_write_log(p->log,ZEUS_LOG_ERROR,"zeus process is already running");
			return ZEUS_ERROR;
		}else{
			zeus_write_log(p->log,ZEUS_LOG_ERROR,"lock pid file error : %s",strerror(errno));
			return ZEUS_ERROR;
		}
	}


	return ZEUS_OK;

}

zeus_status_t zeus_init_daemon(zeus_process_t *p){
	
	sigset_t mask;

	zeus_size_t idx;
	zeus_size_t ign_signal_size = sizeof(zeus_master_ignore_signal)/sizeof(zeus_master_ignore_signal[0]);
	zeus_size_t cat_signal_size = sizeof(zeus_master_catch_signal)/sizeof(zeus_master_catch_signal[0]);

	struct sigaction act;

	umask(0);

	act.sa_handler = SIG_IGN;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	
	for(idx = 0 ; idx < ign_signal_size ; ++ idx){
		
		if(sigaction(zeus_master_ignore_signal[idx].signo,&act,NULL) == -1){
			zeus_write_log(p->log,ZEUS_LOG_ERROR,"ignore signal %s error : %s",\
						   zeus_master_ignore_signal[idx].signame,strerror(errno));
			return ZEUS_ERROR;
		}

	}
	
	act.sa_handler = zeus_master_signal_handler;

	for(idx = 0 ; idx < cat_signal_size ; ++ idx){
		
		if(sigaction(zeus_master_catch_signal[idx].signo,&act,NULL) == -1){
			zeus_write_log(p->log,ZEUS_LOG_ERROR,"catch signal %s error : %s",\
						   zeus_master_catch_signal[idx].signame,strerror(errno));
			return ZEUS_ERROR;
		}

	}

	if(sigemptyset(&mask) == -1){
		zeus_write_log(p->log,ZEUS_LOG_ERROR,"empty mask signal set error : %s",strerror(errno));
		return ZEUS_ERROR;
	}

	for(idx = 0 ; idx < cat_signal_size ; ++ idx){
		
		if(sigaddset(&mask,zeus_master_catch_signal[idx].signo) == -1){
			zeus_write_log(p->log,ZEUS_LOG_ERROR,"add signal to mask set error : %s",strerror(errno));
			return ZEUS_ERROR;
		}

	}

	if(sigprocmask(SIG_BLOCK,&mask,NULL) == -1){
		
		zeus_write_log(p->log,ZEUS_LOG_ERROR,"set signal mask error : %s",strerror(errno));
		return ZEUS_ERROR;

	}

	return ZEUS_OK;
	
}

zeus_status_t zeus_daemon(zeus_process_t *p){
	
	zeus_char_t tmp[ZEUS_PID_MAX_SIZE];
	zeus_int_t nwrite;

	zeus_char_t *beg = tmp;
	zeus_char_t *end = tmp + ZEUS_PID_MAX_SIZE;

	zeus_fd_t fd;
	zeus_pid_t pid;

	switch((pid = fork())){
		case -1:
			zeus_write_log(p->log,ZEUS_LOG_ERROR,"create child process error : %s",strerror(errno));
			return ZEUS_ERROR;
			break;
		case 0:
			break;
		default:
			exit(0);
			break;
	}

	if(setsid() == -1){
		zeus_write_log(p->log,ZEUS_LOG_ERROR,"create new session error : %s",strerror(errno));
		return ZEUS_ERROR;
	}

	switch((pid = fork())){
		case -1:
			zeus_write_log(p->log,ZEUS_LOG_ERROR,"create child process again error : %s",strerror(errno));
			return ZEUS_ERROR;
			break;
		case 0:
			break;
		default:
			exit(0);
			break;
	}
	
	if(close(STDIN_FILENO) == -1){
		zeus_write_log(p->log,ZEUS_LOG_ERROR,"close STDIN_FILENO error : %s",strerror(errno));
		return ZEUS_ERROR;
	}
	
	if((fd = open("/dev/null",O_RDWR)) == -1 || fd != STDIN_FILENO){
		zeus_write_log(p->log,ZEUS_LOG_ERROR,"open /dev/null at STDIN_FILENO error : %s",strerror(errno));
		return ZEUS_ERROR;
	}

	if(dup2(fd,STDOUT_FILENO) == -1){
		zeus_write_log(p->log,ZEUS_LOG_ERROR,"redirect stdout to /dev/null error : %s",strerror(errno));
		return ZEUS_ERROR;

	}

	if(dup2(fd,STDERR_FILENO) == -1){
		zeus_write_log(p->log,ZEUS_LOG_ERROR,"redirect stderr to /dev/null error : %s",strerror(errno));
		return ZEUS_ERROR;
	}
	
	p->pid = getpid();

	if(zeus_check_already_run(p) == ZEUS_ERROR){
		return ZEUS_ERROR;
	}

	if(ftruncate(p->pid_run_flag_fd,0) == -1){
		zeus_write_log(p->log,ZEUS_LOG_ERROR,"truncate pid file error : %s",strerror(errno));
		return ZEUS_ERROR;
	}
	
	nwrite = snprintf(beg,ZEUS_PID_MAX_SIZE,"%d",p->pid);

	if(beg + nwrite > end){
		zeus_write_log(p->log,ZEUS_LOG_ERROR,"pid is too large");
		return ZEUS_ERROR;
	}else{
		beg = beg + nwrite;
	}

	if(write(p->pid_run_flag_fd,(const void *)tmp,zeus_addr_delta(beg,tmp)) == -1){
		zeus_write_log(p->log,ZEUS_LOG_ERROR,"write pid file error");
		return ZEUS_ERROR;
	}
		
	return ZEUS_OK;

}
