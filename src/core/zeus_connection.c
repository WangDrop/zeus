/*
 *	Email : ydzhao1992#gmail.com
 *	 Date : 2015-01-15
 */


#include "zeus_connection.h"


zeus_connection_t *zeus_create_connection_node(zeus_process_t *process){

	
	zeus_connection_t *alloc_connection;

	alloc_connection = (zeus_connection_t *)zeus_memory_alloc(process->pool,sizeof(zeus_connection_t));
	
	if(alloc_connection == NULL){
		zeus_write_log(process->log,ZEUS_LOG_ERROR,"create connection node error");
		return NULL;
	}

    alloc_connection->rd = NULL;
    alloc_connection->wr = NULL;
    alloc_connection->timeout = NULL;

    alloc_connection->rdstatus = ZEUS_EVENT_OFF;
    alloc_connection->wrstatus = ZEUS_EVENT_OFF;
    alloc_connection->timeoutstatus = ZEUS_EVENT_OFF;

	return alloc_connection;

}
