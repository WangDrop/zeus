/*
 *	Email : ydzhao1992#gmail.com
 *	 Date : 2015-01-17
 */

// This is the interface of signal handler

#ifndef __ZEUS_SIGNAL_H_H_H__
#define __ZEUS_SIGNAL_H_H_H__

#include "zeus_common.h"


void zeus_master_signal_handler(int signo); // for master process
void zeus_signal_handler(int signo); // for gateway and worker process

struct zeus_signal_s{
	
	int signo;
	zeus_char_t *signame;

};


static zeus_signal_t zeus_master_ignore_signal[] = 	\
			  {										\
			  	{SIGTTIN,"SIGTTIN"},				\
				{SIGTTOU,"SIGTTOU"} 				\
			  };

static zeus_signal_t zeus_master_catch_signal[] = 	\
			  {										\
			  	{SIGINT,"SIGINT"}, 					\
				{SIGQUIT,"SIGQUIT"},				\
				{SIGTERM,"SIGTERM"},				\
				{SIGHUP,"SIGHUP"},					\
				{SIGCHLD,"SIGCHLD"},				\
				{SIGSEGV,"SIGSEGV"}					\
			  };
#endif
