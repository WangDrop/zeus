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

    if(zeus_init_process(process) == ZEUS_ERROR){
        exit(-1);
    }


	zeus_write_log(process->log,ZEUS_LOG_NOTICE,"uid=%d",process->uid);
	zeus_write_log(process->log,ZEUS_LOG_NOTICE,"gid=%d",process->gid);
	zeus_write_log(process->log,ZEUS_LOG_NOTICE,"worker=%d",process->worker);
	zeus_write_log(process->log,ZEUS_LOG_NOTICE,"resolution=%d",process->resolution);
	zeus_write_log(process->log,ZEUS_LOG_NOTICE,"port=%hd",process->port);

    //zeus_log_config(process->config,process->log);
    
    
    return 0;

}
