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

    zeus_write_log(process->log,ZEUS_LOG_NOTICE,"%d",process->uid);
    zeus_write_log(process->log,ZEUS_LOG_NOTICE,"%d",process->gid);
    zeus_write_log(process->log,ZEUS_LOG_NOTICE,"%hd",process->port);
    zeus_write_log(process->log,ZEUS_LOG_NOTICE,"%d",process->worker);
    zeus_write_log(process->log,ZEUS_LOG_NOTICE,"%d",process->resolution);
    zeus_write_log(process->log,ZEUS_LOG_NOTICE,"%s",process->log->path->data);
    zeus_write_log(process->log,ZEUS_LOG_NOTICE,"%s",process->pid_run_flag_path->data);
    
    return 0;

}
