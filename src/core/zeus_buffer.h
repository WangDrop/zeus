/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2015-01-01
 */

// This is the buffer interface , the size of each buffer is fixed , they can be link with pointer.


#ifndef __ZEUS_BUFFER_H_H_H__
#define __ZEUS_BUFFER_H_H_H__

#include "zeus_common.h"

#define ZEUS_BUFFER_MAX_SIZE 8192

struct zeus_buffer_s{
    
    void *start;
    void *last;
    void *current;
    void *end;

};

zeus_list_data_t *zeus_create_buffer_list_node(zeus_process_t *);
zeus_status_t zeus_recycle_buffer_list_node_to_pool(zeus_process_t *,zeus_list_data_t *);


#endif
