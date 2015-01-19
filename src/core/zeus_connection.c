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


	return alloc_connection;

}
