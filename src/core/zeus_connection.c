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
    
    alloc_node->prev = NULL;
    alloc_node->next = NULL;
    alloc_node->d = (void *)alloc_connection;
    alloc_connection->node = alloc_node;

    return alloc_node;

}

zeus_connection_t *zeus_create_connection_node(zeus_process_t *process){

	
    zeus_connection_t *alloc_connection;

    if((alloc_connection = (zeus_connection_t *)zeus_memory_alloc(process->pool,sizeof(zeus_connection_t))) == NULL){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"alloc connection memory error");
        return NULL;
    }
	
    if((alloc_connection->rd = zeus_create_event(process,alloc_connection)) == NULL){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"create connection read event error");
        return NULL;
    }

    if((alloc_connection->wr = zeus_create_event(process,alloc_connection)) == NULL){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"create connection write event error");
        return NULL;
    
    }

    alloc_connection->rdstatus = ZEUS_EVENT_OFF;
    alloc_connection->wrstatus = ZEUS_EVENT_OFF;

    alloc_connection->peer = NULL;
    alloc_connection->peerlen = NULL;
    alloc_connection->addr_string = NULL;
    alloc_connection->quiting = 0;
    alloc_connection->node = NULL;
    alloc_connection->admin = ZEUS_PROTO_NORMAL;
    alloc_connection->check = ZEUS_PROTO_UNCHECK;
    
    if((alloc_connection->privilege = (zeus_uint_t *)zeus_memory_alloc(process->pool,ZEUS_PROTO_INS_MAX)) == NULL){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"create connection privilege part error");
        return NULL;
    }

    zeus_memset(alloc_connection->privilege,0,ZEUS_PROTO_INS_MAX);

    return alloc_connection;

}

zeus_connection_t *zeus_create_connection_array(zeus_process_t *p,zeus_uint_t sz){

    zeus_uint_t idx;
    zeus_connection_t *alloc_connection_array;

    if((alloc_connection_array = (zeus_connection_t *)zeus_memory_alloc(p->pool,sz * sizeof(zeus_connection_t))) == NULL){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"alloc connection array memory error");
        return NULL;
    }

    for(idx = 0 ; idx < sz ; ++ idx){

        alloc_connection_array[idx].peer = NULL;
        alloc_connection_array[idx].peerlen = NULL;
        alloc_connection_array[idx].addr_string = NULL;
        alloc_connection_array[idx].quiting = 0;
        alloc_connection_array[idx].node = NULL;
        alloc_connection_array[idx].admin = ZEUS_PROTO_NORMAL;
        alloc_connection_array[idx].check = ZEUS_PROTO_UNCHECK;

        if((alloc_connection_array[idx].rd = zeus_create_event(p,&alloc_connection_array[idx])) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"alloc connection read event error");
            return NULL;
        }

        if((alloc_connection_array[idx].wr = zeus_create_event(p,&alloc_connection_array[idx])) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"alloc connection write event error");
            return NULL;
        }

        alloc_connection_array[idx].rdstatus = ZEUS_EVENT_OFF;
        alloc_connection_array[idx].wrstatus = ZEUS_EVENT_OFF;
        
        if((alloc_connection_array[idx].privilege = (zeus_uint_t *)zeus_memory_alloc(p->pool,ZEUS_PROTO_INS_MAX)) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"alloc connection privilege part error");
            return NULL;
        }

        zeus_memset(alloc_connection_array[idx].privilege,0,ZEUS_PROTO_INS_MAX);

    }
    

    return alloc_connection_array;
}



zeus_list_data_t *zeus_get_connection_list_node_from_pool(zeus_process_t *process){
    
    zeus_list_data_t *z;

    z = process->connection_pool;
    process->connection_pool = z->next;
    z->next = NULL;
    z->prev = NULL;

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
    conn->quiting = 0;

    // conn->node doesn't change

    if(conn->rd){

        if(conn->rd->timeout == ZEUS_EVENT_ON){
            conn->rd->timeout = ZEUS_EVENT_OFF;
            conn->rd->timeout_handler = NULL;
            node = conn->rd->timeout_rbnode;
            conn->rd->timeout_rbnode = NULL;
            if(zeus_event_timer_rbtree_delete(process->timer,node) == ZEUS_ERROR){
                zeus_write_log(process->log,ZEUS_LOG_ERROR,"recycle connection error when delete rbnode");
                return ZEUS_ERROR;
            }
            zeus_recycle_event_timer_rbnode_to_pool(process->timer,node);
        }
            
        conn->rd->handler = NULL;
        if(conn->rd->buffer){
            while(conn->rd->buffer->head){
                buf = conn->rd->buffer->head;
                zeus_delete_list(conn->rd->buffer,buf);
                zeus_recycle_buffer_list_node_to_pool(process,buf);
            }
        }
        conn->rd->buflen = 0;
        
        // conn->rd does not change
        // conn->rd->buffer does not change
        // conn->rd->connection does not change
    }

    if(conn->wr){

        if(conn->wr->timeout == ZEUS_EVENT_ON){
            conn->wr->timeout = ZEUS_EVENT_OFF;
            conn->wr->timeout_handler = NULL;
            node = conn->wr->timeout_rbnode;
            conn->wr->timeout_rbnode = NULL;
            if(zeus_event_timer_rbtree_delete(process->timer,node) == ZEUS_ERROR){
                zeus_write_log(process->log,ZEUS_LOG_ERROR,"recycle connection error when delete rbnode");
                return ZEUS_ERROR;
            }
            zeus_recycle_event_timer_rbnode_to_pool(process->timer,node);
        }
            
        conn->wr->handler = NULL;

        if(conn->wr->buffer){
            while(conn->wr->buffer->head){
                buf = conn->wr->buffer->head;
                zeus_delete_list(conn->wr->buffer,buf);
                zeus_recycle_buffer_list_node_to_pool(process,buf);
            }
        }
        conn->wr->buflen = 0;
        
        // conn->wr does not change
        // conn->wr->buffer does not change
        // conn->wr->connection does not change
    }

    zeus_memset(conn->privilege,0,ZEUS_PROTO_INS_MAX);
    conn->admin = ZEUS_PROTO_NORMAL;
    conn->check = ZEUS_PROTO_UNCHECK;
    
    z->prev = NULL;
    z->next = process->connection_pool;
    process->connection_pool = z;

    return ZEUS_OK;

}
