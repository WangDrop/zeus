/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2014-12-29
 */

// Hash table implemention , just use link list array.

#include "zeus_hash.h"

zeus_hash_t *zeus_create_hash_table(zeus_memory_pool_t *pool){

    zeus_uint_t idx;
    zeus_hash_t *alloc_hash;

    alloc_hash = (zeus_hash_t *)zeus_memory_alloc(pool,sizeof(zeus_hash_t));

    if(alloc_hash == NULL){
        zeus_write_log(pool->log,ZEUS_LOG_ERROR,"alloc the hash structure error");
        return NULL;
    }

    alloc_hash->bucket_size = ZEUS_HASH_BUCKET_SIZE;

    alloc_hash->bucket = (zeus_hash_data_t **)zeus_memory_alloc(pool,sizeof(zeus_hash_data_t *)*ZEUS_HASH_BUCKET_SIZE);
    if(alloc_hash->bucket == NULL){
        zeus_write_log(pool->log,ZEUS_LOG_ERROR,"alloc the hash data structure error");
        return NULL;
    }

    for(idx = 0 ; idx < ZEUS_HASH_BUCKET_SIZE ; ++ idx){
        alloc_hash->bucket[idx] = NULL;
    }
    
    return alloc_hash;

}

zeus_hash_data_t *zeus_get_hash_data_node(zeus_memory_pool_t *pool){
    
    zeus_hash_data_t *alloc_hash_data;

    alloc_hash_data = (zeus_hash_data_t *)zeus_memory_alloc(pool,sizeof(zeus_hash_data_t));
    if(alloc_hash_data == NULL){
        zeus_write_log(pool->log,ZEUS_LOG_ERROR,"alloc hash data node error");
        return NULL;
    }

    alloc_hash_data->next = NULL;
    alloc_hash_data->s = NULL;
    alloc_hash_data->d = NULL;

    return alloc_hash_data;

}

zeus_uint_t zeus_get_hash_key(zeus_string_t *s){

    zeus_uint_t val = 0;
    zeus_uint_t idx;

    for(idx = 0 ; idx < s->size ; ++ idx){
        val = (val + (zeus_uint_t)(s->data[idx])) * ZEUS_HASH_MAGIC_NUM;
        val = val % ZEUS_HASH_BUCKET_SIZE;
    }

    return val;

}

void zeus_insert_hash_data_node(zeus_hash_t *ht,zeus_hash_data_t *hd){
    
    zeus_uint_t key = zeus_get_hash_key(hd->s);

    hd->next = ht->bucket[key];
    
    ht->bucket[key] = hd;

    return ;

}

void zeus_log_hash_table(zeus_hash_t *ht,zeus_log_t *log){
    
    zeus_uint_t idx;
    zeus_hash_data_t *node;

    for(idx = 0 ; idx < ZEUS_HASH_BUCKET_SIZE ; ++ idx){
        node = ht->bucket[idx];
        while(node != NULL){
            zeus_write_log(log,ZEUS_LOG_NOTICE,"%s = %s",node->s->data,node->d->data);
            node = node->next;
        }
    
    }

    return ;

}
