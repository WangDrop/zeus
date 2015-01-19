/*
 *	Email : ydzhao1992#gmail.com
 *	 Date : 2015-01-16
 */

#include "zeus_signal.h"



void zeus_master_signal_handler(int signo){

	switch(signo){
		case SIGINT:
		case SIGQUIT:
		case SIGTERM:
		case SIGHUP:
			zeus_quit = 1;
			break;
		case SIGCHLD:
			zeus_refork = 1;
			break;
		case SIGSEGV:
			zeus_segv = 1;
			break;
		default:
			break;
	}
	
	return ;
}


void zeus_signal_handler(int signo){

	switch(signo){
		case SIGINT:
			break;
		case SIGQUIT:
			break;
		case SIGTERM:
			break;
		case SIGSEGV:
			break;
		case SIGALRM:
			break;
		default:
			break;
	}

	return ;
	
}
