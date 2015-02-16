/*
 *	Email : ydzhao1992#gmail.com
 *	 Date : 2015-01-15
 */


#include "zeus_connection.h"

static zeus_list_data_t *zeus_get_connection_list_node_from_pool(zeus_process_t *);
static zeus_connection_t *zeus_create_connection_node(zeus_process_t *);

zeus_list_data_t *zeus_create_connection_list_node(zeus_process_t *process){
    
    zeus_list_data_t *alloc_node = NULL;
    zeus_connection_t *alloc_connection = NULL;

    if(process->connection_pool){
        alloc_node = zeus_get_connection_list_node_from_pool(process);
        return alloc_node;
    }

    if((alloc_node = zeus_create_list_data_node(process->pool,process->log)) == NULL){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"create connection list node error");
        return NULL;
    }

    if((alloc_connection = zeus_create_connection_node(process)) == NULL){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"create connection node error");
        return NULL;
    }

    alloc_node->next = NULL;
    alloc_node->d = (void *)alloc_connection;

    return alloc_node;

}

zeus_connection_t *zeus_create_connection_node(zeus_process_t *process){

	
    zeus_connection_t *alloc_connection;

    alloc_connection = (zeus_connection_t *)zeus_memory_alloc(process->pool,sizeof(zeus_connection_t));
	
    if(alloc_connection == NULL){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"create connection node error");
        return NULL;
    }

    alloc_connection->rd = NULL;
    alloc_connection->wr = NULL;

    alloc_connection->rdstatus = ZEUS_EVENT_OFF;
    alloc_connection->wrstatus = ZEUS_EVENT_OFF;

    alloc_connection->peer = NULL;
    alloc_connection->peerlen = NULL;

    alloc_connection->ukey = NULL;

    return alloc_connection;

}



zeus_list_data_t *zeus_get_connection_list_node_from_pool(zeus_process_t *process){
    
    zeus_list_data_t *z;

    z = process->connection_pool;
    process->connection_pool = z->next;
    z->next = NULL;

    return z;

}

zeus_status_t zeus_recycle_connection_list_node_to_pool(zeus_process_t *process,zeus_list_data_t *z){
    
    zeus_connection_t *conn;
    zeus_event_timer_rbnode_t *node;
    zeus_list_data_t *buf;

    conn = (zeus_connection_t *)z->d;
    conn->rdstatus = ZEUS_EVENT_OFF;
    conn->wrstatus = ZEUS_EVENT_OFF;
    conn->fd = -1;

    if(conn->rd){
        conn->rd->timeout = ZEUS_EVENT_OFF;
        conn->rd->timeout_handler = NULL;
        if(conn->rd->timeout_rbnode){

            node = conn->rd->timeout_rbnode;
            conn->rd->timeout_rbnode = NULL;
            zeus_recycle_event_timer_rbnode_to_pool(process->timer,node);
            
        }
        conn->rd->handler = NULL;
        if(conn->rd->buffer){
            while(conn->rd->buffer->head){
                buf = conn->rd->buffer->head;
                conn->rd->buffer->head = buf->next;
                zeus_recycle_buffer_list_node_to_pool(process,buf);
            }
            conn->rd->buffer->head = conn->rd->buffer->tail = NULL;
        }

        // conn->rd->buffer does not change
        // conn->rd->connection does not change
    }

    if(conn->wr){
        conn->wr->timeout = ZEUS_EVENT_OFF;
        conn->wr->timeout_handler = NULL;
        if(conn->wr->timeout_rbnode){
            
            node = conn->wr->timeout_rbnode;
            conn->wr->timeout_rbnode = NULL;
            zeus_recycle_event_timer_rbnode_to_pool(process->timer,node);

        }
        conn->wr->handler = NULL;

        if(conn->wr->buffer){
            while(conn->wr->buffer->head){
                buf = conn->wr->buffer->head;
                conn->wr->buffer->head = buf->next;
                zeus_recycle_buffer_list_node_to_pool(process,buf);
            }
            conn->rd->buffer->head = conn->wr->buffer->tail = NULL;
        }

        // conn->wr->buffer does not change
        // conn->wr->connection does not change
    }

    z->next = process->connection_pool;
    process->connection_pool = z;

    return ZEUS_OK;

}
