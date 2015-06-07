/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2015-02-17
 */

// this is for buffer handler

#include "zeus_proto_io.h"

zeus_status_t zeus_proto_buffer_write_byte(zeus_process_t *p,zeus_event_t *ev,zeus_char_t c){

    zeus_list_data_t *tbuf;
    zeus_buffer_t *b;
    zeus_char_t *twrite;

    if(!ev->buffer->tail){
        if((tbuf = zeus_create_buffer_list_node(p)) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"create buffer node error in zeus_proto_buffer_write_char");
            return ZEUS_ERROR;
        }
        zeus_insert_list(ev->buffer,tbuf);
    }

    tbuf = ev->buffer->tail;
    b = (zeus_buffer_t *)(tbuf->d);
    
    if(zeus_addr_delta(b->end,b->last) < sizeof(zeus_char_t)){
        if((tbuf = zeus_create_buffer_list_node(p)) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"create buffer node error in zeus_proto_buffer_write_char");
            return ZEUS_ERROR;
        }
        zeus_insert_list(ev->buffer,tbuf);
        tbuf = ev->buffer->tail;
        b = (zeus_buffer_t *)(tbuf->d);
    }

    twrite = (zeus_char_t *)(b->last);
    *twrite = c;
    b->last= zeus_addr_add(b->last,sizeof(zeus_char_t));
    ev->buflen += sizeof(zeus_char_t);

    return ZEUS_OK;

}

zeus_status_t zeus_proto_buffer_write_uint(zeus_process_t *p,zeus_event_t *ev,zeus_uint_t i){
    
    // TODO
    // UPDATE TO BYTE WRITE ONE BY ONE

    zeus_list_data_t *tbuf;
    zeus_buffer_t *b;
    zeus_uint_t *iwrite;

    if(!ev->buffer->tail){
        if((tbuf = zeus_create_buffer_list_node(p)) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"create buffer node error in zeus_proto_buffer_write_int");
            return ZEUS_ERROR;
        }
        zeus_insert_list(ev->buffer,tbuf);
    }

    tbuf = ev->buffer->tail;
    b = (zeus_buffer_t *)(tbuf->d);

    if(zeus_addr_delta(b->end,b->last) < sizeof(zeus_uint_t)){
        if((tbuf = zeus_create_buffer_list_node(p)) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"create buffer node error in zeus_proto_buffer_write_int");
            return ZEUS_ERROR;
        }
        zeus_insert_list(ev->buffer,tbuf);
        tbuf = ev->buffer->tail;
        b = (zeus_buffer_t *)(tbuf->d);
    }

    i = htonl(i);
    iwrite = (zeus_uint_t *)(b->last);
    *iwrite = i;
    b->last= zeus_addr_add(b->last,sizeof(zeus_uint_t));
    ev->buflen += sizeof(zeus_uint_t);

    return ZEUS_OK;

}

zeus_status_t zeus_proto_buffer_write_byte_array(zeus_process_t *p,zeus_event_t *ev,zeus_char_t *seq,zeus_size_t sz){

    zeus_list_data_t *tbuf;
    zeus_buffer_t *b;
    zeus_char_t *start = seq;
    zeus_char_t *end = seq + sz;
    zeus_size_t leftsz;
    zeus_size_t leftwr;

    if(!ev->buffer->tail){
        if((tbuf = zeus_create_buffer_list_node(p)) == NULL){
            zeus_write_log(p->log,ZEUS_LOG_ERROR,"create buffer node error in zeus_proto_buffer_write_byte_array");
            return ZEUS_ERROR;
        }
        zeus_insert_list(ev->buffer,tbuf);
    }

    tbuf = ev->buffer->tail;
    b = (zeus_buffer_t *)(tbuf->d);

    while(start != end){

        leftsz = zeus_addr_delta(b->end,b->last);
        leftwr = zeus_addr_delta(end,start);
        if(leftsz >= leftwr){
            zeus_memcpy(b->last,start,leftwr);
            start += leftwr;
            b->last= zeus_addr_add(b->last,leftwr);
            ev->buflen += leftwr;
        }else{
            zeus_memcpy(b->last,start,leftsz);
            start += leftsz;
            ev->buflen += leftsz;
            b->last= zeus_addr_add(b->last,leftsz);
            if((tbuf = zeus_create_buffer_list_node(p)) == NULL){
                zeus_write_log(p->log,ZEUS_LOG_ERROR,"create buffer node error in zeus_proto_buffer_write_byte_array");
                return ZEUS_ERROR;
            }
            zeus_insert_list(ev->buffer,tbuf);
            tbuf = ev->buffer->tail;
            b = (zeus_buffer_t *)(tbuf->d);
        }
    
    }
    
    return ZEUS_OK;

}
