/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2015-01-01
 */

// This is the implemention of the link list.

#include "zeus_list.h"

zeus_list_t *zeus_create_list(zeus_memory_pool_t *pool,zeus_log_t *log){

    zeus_list_t *alloc_list;

    alloc_list = (zeus_list_t *)zeus_memory_alloc(pool,sizeof(zeus_list_t));

    if(alloc_list == NULL){
        zeus_write_log(log,ZEUS_LOG_ERROR,"create link list error");
        return NULL;
    }

    alloc_list->head = NULL;
    alloc_list->tail = NULL;

    return alloc_list;

}

zeus_list_data_t *zeus_create_list_data_node(zeus_memory_pool_t *pool,zeus_log_t *log){
    
    zeus_list_data_t *alloc_data = NULL;

    alloc_data = (zeus_list_data_t *)zeus_memory_alloc(pool,sizeof(zeus_list_data_t));

    if(alloc_data == NULL){
        zeus_write_log(log,ZEUS_LOG_ERROR,"create link list data error");
        return NULL;
    }

    alloc_data->next = NULL;
    alloc_data->prev = NULL;
    alloc_data->d = NULL;

    return alloc_data;

}

void zeus_insert_list(zeus_list_t *l,zeus_list_data_t *ld){

    if(l->head == l->tail && l->head == NULL){
        l->head = l->tail = ld;
        ld->prev = NULL;
        ld->next = NULL;
    }else{
        ld->next = l->tail->next;
        l->tail->next = ld;
        ld->prev = l->tail;
        l->tail = ld;
    }

    return ;

}
