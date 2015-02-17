/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2015-01-01
 */

//This is the implemention of buffer.

#include "zeus_buffer.h"

//get from buffer pool.
static zeus_list_data_t *zeus_get_buffer_list_node_from_pool(zeus_process_t *process);

zeus_list_data_t *zeus_create_buffer_list_node(zeus_process_t *process){

    zeus_list_data_t *alloc_node = NULL;
    zeus_buffer_t *alloc_buffer_head = NULL;
    void *alloc_b = NULL;

    if(process->buffer_pool){
        alloc_node = zeus_get_buffer_list_node_from_pool(process);
        return alloc_node;
    }

    alloc_node = zeus_create_list_data_node(process->pool,process->log);
    if(alloc_node == NULL){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"create buffer error");
        return NULL;
    }

    alloc_buffer_head = (zeus_buffer_t *)zeus_memory_alloc(process->pool,sizeof(zeus_buffer_t));
    if(alloc_buffer_head == NULL){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"create buffer error");
        return NULL;
    }

    alloc_b = (void *)zeus_memory_alloc(process->pool,ZEUS_BUFFER_MAX_SIZE);
    if(alloc_b == NULL){
        zeus_write_log(process->log,ZEUS_LOG_ERROR,"create buffer error");
        return NULL;
    }

    alloc_buffer_head->start = alloc_b;
    alloc_buffer_head->last = alloc_b;
    alloc_buffer_head->current = alloc_b;
    alloc_buffer_head->end = zeus_addr_add(alloc_b,ZEUS_BUFFER_MAX_SIZE);

    alloc_node->d = alloc_buffer_head;
    alloc_node->next = NULL;
    
    return alloc_node;

}

zeus_list_data_t *zeus_get_buffer_list_node_from_pool(zeus_process_t *process){
    
    zeus_list_data_t *p = NULL;

    p = process->buffer_pool;
    process->buffer_pool = p->next;
    p->next = NULL;

    return p;

}

zeus_status_t zeus_recycle_buffer_list_node_to_pool(zeus_process_t *process,zeus_list_data_t *z){
    
    zeus_buffer_t *p;

    p = (zeus_buffer_t *)(z->d);
    p->current = p->start;
    p->last = p->start;
    
    z->next = process->buffer_pool;
    process->buffer_pool = z;
    
    return ZEUS_OK;

}
