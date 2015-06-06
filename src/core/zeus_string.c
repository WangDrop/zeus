/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2014-12-24
 */

// This is the implemention of the string operation

#include "zeus_string.h"

zeus_string_t *zeus_create_string(zeus_process_t *p,zeus_size_t sz){
    
    zeus_string_t *alloc_string;

    alloc_string = (zeus_string_t *)zeus_memory_alloc(p->pool,sizeof(zeus_string_t));
    if(!alloc_string){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"zeus create string memory error");
        return NULL;
    }

    alloc_string->size = sz;
    alloc_string->data = (zeus_char_t *)zeus_memory_alloc(p->pool,sizeof(zeus_char_t) * sz);
    if(!alloc_string->data){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"zeus create string data error");
        return NULL;
    }

    return alloc_string;

}


zeus_char_t *zeus_vsnprintf(zeus_char_t *beg,zeus_char_t *end,const zeus_char_t *fmt,va_list ap){
    
    zeus_int_t nwrite;

    nwrite = vsnprintf(beg,(zeus_size_t)(end - beg),fmt,ap);

    return (beg + nwrite < end) ? (beg + nwrite) : end;

}


zeus_status_t zeus_string_to_uint(zeus_char_t *beg,zeus_char_t *end,zeus_uint_t *val){
	
    zeus_uint_t result = 0;

    while(beg != end){
        if(*beg < '0' || *beg > '9'){
            return ZEUS_ERROR;
        }
        result *= 10;
        result += (*beg++ - '0');
    }
		
    *val = result;
    return ZEUS_OK;

}

zeus_status_t zeus_string_to_ushort(zeus_char_t *beg,zeus_char_t *end,zeus_ushort_t *val){

    zeus_ushort_t result = 0;

    while(beg != end){
        if(*beg < '0' || *beg > '9'){
            return ZEUS_ERROR;
        }
        result *= 10;
        result += (*beg++ - '0');
    }
	
    *val = result;
    return ZEUS_OK;

}

zeus_int_t zeus_string_equal(zeus_string_t *s,zeus_string_t *d){
	
    zeus_size_t idx;

    if(s->size != d->size){
        return -1;
    }

    for(idx = 0 ; idx != d->size ; ++ idx){
        if(s->data[idx] != d->data[idx]){
            return -1;
        }
    }

    return 0;

}

zeus_int_t zeus_string_and_carr_equal(zeus_string_t *s,zeus_char_t *carr,zeus_size_t sz){

    zeus_size_t idx;
    
    if(s->size != sz + 1){
        return -1;
    }

    for(idx = 0 ; idx != s->size ; ++ idx){
        if(s->data[idx] != carr[idx]){
            return -1;
        }
    }

    return 0;

}

