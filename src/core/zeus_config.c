/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2014-12-29
 */

// This is the implemention.

#include "zeus_config.h"

#define ZEUS_CONFIG_PATH_MAX 4096
#define ZEUS_CONFIG_LINE_MAX 2048

zeus_config_t *zeus_create_config(zeus_memory_pool_t *pool){
    
    zeus_size_t nwrite;
    zeus_config_t *alloc_config;

    zeus_char_t tmp_config_path[ZEUS_CONFIG_PATH_MAX];
    zeus_char_t *beg = tmp_config_path;
    zeus_char_t *end = tmp_config_path + ZEUS_CONFIG_PATH_MAX - 1;

    alloc_config = (zeus_config_t *)zeus_memory_alloc(pool,sizeof(zeus_config_t));

    if(alloc_config == NULL){
        zeus_write_log(pool->log,ZEUS_LOG_ERROR,"alloc config structure error");
        return NULL;
    }

    nwrite = snprintf(beg,(zeus_size_t)(end - beg),"%s",ZEUS_CONFIGURATION_FILE);
    beg = (beg + nwrite < end) ? (beg + nwrite) : end;
    ++ beg;

    alloc_config->path = (zeus_string_t *)zeus_memory_alloc(pool,sizeof(zeus_string_t));
    if(alloc_config->path == NULL){
        zeus_write_log(pool->log,ZEUS_LOG_ERROR,"alloc configuration file path string structure error");
        return NULL;
    }

    alloc_config->path->size = (zeus_size_t)(beg - tmp_config_path);

    alloc_config->path->data = (zeus_char_t *)zeus_memory_alloc(pool,alloc_config->path->size);
    if(alloc_config->path->data == NULL){
        zeus_write_log(pool->log,ZEUS_LOG_ERROR,"alloc configuration file path string data error");
        return NULL;
    }

    zeus_memcpy(alloc_config->path->data,tmp_config_path,alloc_config->path->size);
    
    alloc_config->conf = zeus_create_hash_table(pool);
    if(alloc_config->conf == NULL){
        zeus_write_log(pool->log,ZEUS_LOG_ERROR,"get config hash table error");
        return NULL;
    }

    return alloc_config;

}

zeus_status_t zeus_init_config(zeus_config_t *config,zeus_memory_pool_t *pool){
    
    FILE *fp;
    zeus_size_t sz;
    zeus_char_t line[ZEUS_CONFIG_LINE_MAX];

    if((fp = fopen(config->path->data,"r")) == NULL){
        zeus_write_log(pool->log,ZEUS_LOG_ERROR,"open configuration file %s error : %s",config->path->data,strerror(errno));
        return ZEUS_ERROR;
    }

    while(fgets(line,ZEUS_CONFIG_LINE_MAX,fp) != NULL){
        
        sz = strlen(line);
        
        if(line[sz - 1] != '\n' && !feof(fp)){
            return ZEUS_ERROR;
        }
        
        //last line
        if(line[sz - 1] != '\n'){
            if(zeus_parse_config_line(line,sz,config,pool) == ZEUS_ERROR){
                return ZEUS_ERROR;
            }
        }else{
            if(zeus_parse_config_line(line,sz - 1,config,pool) == ZEUS_ERROR){
                return ZEUS_ERROR;
            }
        }

    }

    if(feof(fp)){
        return ZEUS_OK;
    }else{
        return ZEUS_ERROR;
    }

}

zeus_status_t zeus_parse_config_line(zeus_char_t *line,zeus_int_t nread,zeus_config_t *config,zeus_memory_pool_t *pool){
    
    zeus_int_t idx;
    zeus_string_t *s;
    zeus_string_t *d;
    zeus_hash_data_t *node;
    
    if(nread <= 0){
        return ZEUS_OK;
    }

    if(line[0] == ' ' || line[0] == '#'){
        return ZEUS_OK;
    }

    for(idx = 0 ; idx < nread ; ++ idx){
        if(line[idx] == '='){
            break;
        }
    }

    if(idx == 0){
        zeus_write_log(pool->log,ZEUS_LOG_ERROR,"config item has no name");
        return ZEUS_ERROR;
    }else if(idx == nread - 1){
        zeus_write_log(pool->log,ZEUS_LOG_ERROR,"config item has no value");
        return ZEUS_ERROR;
    }else if(idx == nread){
        zeus_write_log(pool->log,ZEUS_LOG_ERROR,"config item has no =");
        return ZEUS_ERROR;
    }
    
    s = (zeus_string_t *)zeus_memory_alloc(pool,sizeof(zeus_string_t));
    if(s == NULL){
        zeus_write_log(pool->log,ZEUS_LOG_ERROR,"alloc config item error");
        return ZEUS_ERROR;
    }
    s->size = idx + 1;
    s->data = (zeus_char_t *)zeus_memory_alloc(pool,s->size);
    if(s->data == NULL){
        zeus_write_log(pool->log,ZEUS_LOG_ERROR,"alloc config item error");
        return ZEUS_ERROR;
    }
    zeus_memcpy(s->data,line,idx);
    s->data[s->size - 1] = '\0';

    
    d = (zeus_string_t *)zeus_memory_alloc(pool,sizeof(zeus_string_t));
    if(d == NULL){
        zeus_write_log(pool->log,ZEUS_LOG_ERROR,"alloc config item error");
        return ZEUS_ERROR;
    }
    d->size = nread - idx;
    d->data = (zeus_char_t *)zeus_memory_alloc(pool,d->size);
    if(d->data == NULL){
        zeus_write_log(pool->log,ZEUS_LOG_ERROR,"alloc config item error");
        return ZEUS_ERROR;
    }
    zeus_memcpy(d->data,&line[idx + 1],d->size - 1);
    d->data[d->size - 1] = '\0';

    node = zeus_get_hash_data_node(pool);
    if(node == NULL){
        zeus_write_log(pool->log,ZEUS_LOG_ERROR,"get config data node error");
        return ZEUS_ERROR;
    }

    node->s = s;
    node->d = d;

    zeus_insert_hash_data_node(config->conf,node);
    
    return ZEUS_OK;
}

void zeus_log_config(zeus_config_t *config,zeus_log_t *log){

    zeus_log_hash_table(config->conf,log);

}
