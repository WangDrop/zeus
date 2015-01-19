/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2014-12-24
 */

#include "zeus.h"

volatile zeus_process_t *process;
volatile zeus_atomic_t zeus_quit;
volatile zeus_atomic_t zeus_refork;
volatile zeus_atomic_t zeus_segv;

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

	if(zeus_prepare_spawn(process) == ZEUS_ERROR){
		exit(-1);
	}

	zeus_spawn(process);

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

	struct rlimit rl;

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
	
	rl.rlim_cur = p->max_connection;
	rl.rlim_max = p->max_connection;

	if(setrlimit(RLIMIT_NOFILE,&rl) == -1){
		zeus_write_log(p->log,ZEUS_LOG_ERROR,"change max connection error : %s",strerror(errno));
		return ZEUS_ERROR;
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

zeus_status_t zeus_prepare_spawn(zeus_process_t *p){

	zeus_size_t idx = 0;

	p->channel = (int **)zeus_memory_alloc(p->pool,sizeof(int *) * (p->worker + 1));

	if(p->channel == NULL){
		zeus_write_log(p->log,ZEUS_LOG_ERROR,"create channel error");
		return ZEUS_ERROR;
	}

	for(idx = 0 ; idx < (p->worker + 1) ; ++ idx){
		
		p->channel[idx] = (int *)zeus_memory_alloc(p->pool,sizeof(int) * 2);
		if(p->channel[idx] == NULL){
			zeus_write_log(p->log,ZEUS_LOG_ERROR,"create channel element error : %d",idx);
			return ZEUS_ERROR;
		}

	}

	for(idx = 0 ; idx < (p->worker + 1) ; ++ idx){
		if(socketpair(AF_UNIX,SOCK_STREAM,0,p->channel[idx]) == -1){
			zeus_write_log(p->log,ZEUS_LOG_ERROR,"create channel error when call socketpair : %s",strerror(errno));
			return ZEUS_ERROR;
		}
	}

	for(idx = 0 ; idx < (p->worker + 1) ; ++ idx){

		if(fcntl(p->channel[idx][0],F_SETFL,O_NONBLOCK) == -1){
			zeus_write_log(p->log,ZEUS_LOG_ERROR,"set nonblocking channel %d:0 error : %s",strerror(errno));
			return ZEUS_ERROR;
		}
	
		if(fcntl(p->channel[idx][1],F_SETFL,O_NONBLOCK) == -1){
			zeus_write_log(p->log,ZEUS_LOG_ERROR,"set nonblocking channel %d:1 error : %s",strerror(errno));
			return ZEUS_ERROR;
		}

	}

	p->child = (zeus_pid_t *)zeus_memory_alloc(p->pool,sizeof(zeus_pid_t) * (p->worker + 1));
	if(p->child == NULL){
		zeus_write_log(p->log,ZEUS_LOG_ERROR,"alloc children process pid memory space error");
		return ZEUS_ERROR;
	}
	
	for(idx = 0 ; idx < (p->worker + 1) ; ++ idx){
		p->child[idx] = 0;
	}

	return ZEUS_OK;

}

zeus_status_t zeus_spawn(zeus_process_t *p){
	
	zeus_size_t idx;

	zeus_pid_t pid;

	// create gateway process;
	
	switch(pid = fork()){
		case -1:
			zeus_write_log(p->log,ZEUS_LOG_ERROR,"create gateway process error");
			return ZEUS_ERROR;
			break;
		case 0:
			zeus_prepare_loop(p,0);
			break;
		default:
			p->child[0] = pid;
			break;
	}


	for(idx = 0 ; idx < p->worker ; ++ idx){
	
		switch(pid = fork()){
			case -1:
				zeus_write_log(p->log,ZEUS_LOG_ERROR,"create worker process error");
				return ZEUS_ERROR;
				break;
			case 0:
				zeus_prepare_loop(p,idx + 1);
				break;
			default:
				p->child[idx + 1] = pid;
				break;
		}
	
	}

	zeus_master_prepare_loop(p);
		
	return ZEUS_OK;

}

zeus_status_t zeus_master_prepare_loop(zeus_process_t *p){
	
	zeus_size_t i;

	for(i = 0 ; i < p->worker + 1 ; ++ i){
		if(close(p->channel[i][0]) == -1){
			zeus_write_log(p->log,ZEUS_LOG_ERROR,"close read channel errro : %s",strerror(errno));
		}
		if(close(p->channel[i][1]) == -1){
			zeus_write_log(p->log,ZEUS_LOG_ERROR,"close write channel error : %s",strerror(errno));
		}
	}


	zeus_master_event_loop(p);

}

zeus_status_t zeus_prepare_loop(zeus_process_t *p,zeus_idx_t idx){
	
	zeus_size_t i;

	p->pid = getpid();
	p->pidx = idx;
	p->child = NULL;
	
	for(i = 0 ; i < p->worker + 1 ; ++ i){
		if(i == p->pidx){
			if(close(p->channel[i][1]) == -1){
				zeus_write_log(p->log,ZEUS_LOG_ERROR,"close own write channel error : %s",strerror(errno));
			}
		}else{
			if(close(p->channel[i][0]) == -1){
				zeus_write_log(p->log,ZEUS_LOG_ERROR,"close other read channel error : %s",strerror(errno));
			}
		}
	}

	if(p->pidx == 0){
		if(zeus_gateway_prepare_listen(p) == ZEUS_ERROR){
			zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway process prepare listen error");
		}
	}
	
	if(zeus_update_process_flag(p) == ZEUS_ERROR){
		zeus_write_log(p->log,ZEUS_LOG_ERROR,"%s : change process flag error",(p->pidx)?"gateway":"worker");
	}

	zeus_event_loop(p);

}


zeus_status_t zeus_gateway_prepare_listen(zeus_process_t *p){
	
	int reuse_flag = 1;

	struct sockaddr_in addr;

	if((p->listenfd = socket(AF_INET,SOCK_STREAM,0)) == -1){
		zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway create listen file descriptor error : %s",strerror(errno));
		return ZEUS_ERROR;
	}

	if(setsockopt(p->listenfd,SOL_SOCKET,SO_REUSEADDR,(const void *)&reuse_flag,sizeof(reuse_flag)) == -1){
		zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway set reuse address error : %s",strerror(errno));
		return ZEUS_ERROR;
	}

	if(fcntl(p->listenfd,F_SETFL,O_NONBLOCK) == -1){
		zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway set listen fd nonblock error : %s",strerror(errno));
		return ZEUS_ERROR;
	}
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(p->port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(p->listenfd,(struct sockaddr *)&addr,sizeof(addr)) == -1){
		zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway bind error : %s",strerror(errno));
		return ZEUS_ERROR;
	}

	if(listen(p->listenfd,SOMAXCONN) == -1){
		zeus_write_log(p->log,ZEUS_LOG_ERROR,"gateway listen error : %s",strerror(errno));
		return ZEUS_ERROR;
	}

	return ZEUS_OK;

}
