/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2015-01-01
 */

// this is the link list data structure.
// When we insert or delete from the list , we may just use it like queue

#ifndef __ZEUS_LIST_H_H_H__
#define __ZEUS_LIST_H_H_H__

#include "zeus_common.h"


struct zeus_list_data_s{
    
    void *d;

    zeus_list_data_t *next;

};

struct zeus_list_s{

    zeus_list_data_t *head;

    zeus_list_data_t *tail;

};

zeus_list_t *zeus_create_list(zeus_memory_pool_t *pool,zeus_log_t *log);

zeus_list_data_t *zeus_create_list_data_node(zeus_memory_pool_t *pool,zeus_log_t *log);

void zeus_insert_list(zeus_list_t *,zeus_list_data_t *);

#endif
