/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2014-12-29
 */

// This is the implemention.

#include "zeus_config.h"

#define ZEUS_CONFIG_PATH_MAX 4096
#define ZEUS_CONFIG_LINE_MAX 2048

static zeus_string_t zeus_config_uid = {"user",sizeof("user")};
static zeus_string_t zeus_config_gid = {"group",sizeof("group")};
static zeus_string_t zeus_config_prefix = {"prefix",sizeof("prefix")};
static zeus_string_t zeus_config_log = {"log",sizeof("log")};
static zeus_string_t zeus_config_pid = {"pid",sizeof("pid")};
static zeus_string_t zeus_config_port = {"port",sizeof("port")};
static zeus_string_t zeus_config_worker = {"worker",sizeof("worker")};
static zeus_string_t zeus_config_timer = {"timer",sizeof("timer")};
static zeus_string_t zeus_config_max_connection = {"connection",sizeof("connection")};

static zeus_string_t zeus_log_filename = {"zeus.log",sizeof("zeus.log")};

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

zeus_status_t zeus_config_check(zeus_config_t *config,zeus_log_t *log,\
								zeus_hash_data_t **d,zeus_string_t *s){
	
	if((*d = zeus_hash_lookup(config->conf,s)) == NULL){
		zeus_write_log(log,ZEUS_LOG_ERROR,"can not find the string %s in the mapping",s->data);
		return ZEUS_ERROR;
	}

	return ZEUS_OK;

}


zeus_status_t zeus_config_get_uid(zeus_config_t *config,zeus_log_t *log,zeus_uid_t *uid){
	
	zeus_hash_data_t *hd;
	zeus_int_t err;

	struct passwd *pwd;

	if(zeus_config_check(config,log,&hd,&zeus_config_uid) == ZEUS_ERROR){
		return ZEUS_ERROR;
	}

	errno = 0;
	if((pwd = getpwnam(hd->d->data)) == NULL){
		err = errno;
		if(err != 0){
			zeus_write_log(log,ZEUS_LOG_ERROR,"get user %s's uid fail : %s",hd->d->data,strerror(err));
		}else{
			zeus_write_log(log,ZEUS_LOG_ERROR,"can not find the user : %s",hd->d->data);
		}
		return ZEUS_ERROR;
	}

	
	*uid = pwd->pw_uid;

	return ZEUS_OK;
	
}


zeus_status_t zeus_config_get_gid(zeus_config_t *config,zeus_log_t *log,zeus_gid_t *gid){
	
	zeus_hash_data_t *hd;
	zeus_int_t err;

	struct passwd *pwd;

	if(zeus_config_check(config,log,&hd,&zeus_config_gid) == ZEUS_ERROR){
		return ZEUS_ERROR;
	}
	
	errno = 0;
	if((pwd = getpwnam(hd->d->data)) == NULL){
		err = errno;
		if(err != 0){
			zeus_write_log(log,ZEUS_LOG_ERROR,"get user %s's gid fail : %s",hd->d->data,strerror(err));
		}else{
			zeus_write_log(log,ZEUS_LOG_ERROR,"can not find the user : %s",hd->d->data);
		}
		return ZEUS_ERROR;
	}

	*gid = pwd->pw_gid;

	return ZEUS_OK;

}

zeus_status_t zeus_config_get_resolution(zeus_config_t *config,zeus_log_t *log,zeus_uint_t *val){
	
	zeus_hash_data_t *hd;
	zeus_char_t *beg;
	zeus_char_t *end;
	
	if(zeus_config_check(config,log,&hd,&zeus_config_timer) == ZEUS_ERROR){
		return ZEUS_ERROR;
	}
	
	beg = hd->d->data;
	end = hd->d->data + hd->d->size - 1;

	if(zeus_string_to_uint(beg,end,val) == ZEUS_ERROR){
		zeus_write_log(log,ZEUS_LOG_ERROR,"get timer resolution error");
		return ZEUS_ERROR;
	}

	return ZEUS_OK;

}

zeus_status_t zeus_config_get_worker(zeus_config_t *config,zeus_log_t *log,zeus_uint_t *val){

	zeus_hash_data_t *hd;
	zeus_char_t *beg;
	zeus_char_t *end;
	
	if(zeus_config_check(config,log,&hd,&zeus_config_worker) == ZEUS_ERROR){
		return ZEUS_ERROR;
	}
	
	beg = hd->d->data;
	end = hd->d->data + hd->d->size - 1;

	if(zeus_string_to_uint(beg,end,val) == ZEUS_ERROR){
		zeus_write_log(log,ZEUS_LOG_ERROR,"get worker number error");
		return ZEUS_ERROR;
	}

	return ZEUS_OK;

}

zeus_status_t zeus_config_get_max_connection(zeus_config_t *config,zeus_log_t *log,zeus_uint_t *connection){
	
	zeus_hash_data_t *hd;
	zeus_char_t *beg;
	zeus_char_t *end;
	
	if(zeus_config_check(config,log,&hd,&zeus_config_max_connection) == ZEUS_ERROR){
		return ZEUS_ERROR;
	}

	beg = hd->d->data;
	end = hd->d->data + hd->d->size - 1;

	if(zeus_string_to_uint(beg,end,connection) == ZEUS_ERROR){
		zeus_write_log(log,ZEUS_LOG_ERROR,"get max connection error");
		return ZEUS_ERROR;
	}

	return ZEUS_OK;

}

zeus_status_t zeus_config_get_port(zeus_config_t *config,zeus_log_t *log,zeus_ushort_t *val){

	zeus_hash_data_t *hd;
	zeus_char_t *beg;
	zeus_char_t *end;
	
	if(zeus_config_check(config,log,&hd,&zeus_config_port) == ZEUS_ERROR){
		return ZEUS_ERROR;
	}
	
	beg = hd->d->data;
	end = hd->d->data + hd->d->size - 1;

	if(zeus_string_to_ushort(beg,end,val) == ZEUS_ERROR){
		zeus_write_log(log,ZEUS_LOG_ERROR,"get worker number error");
		return ZEUS_ERROR;
	}

	return ZEUS_OK;

}



zeus_status_t zeus_config_get_log_path(zeus_process_t *process){
	
	zeus_log_t *alloc_log;

	zeus_hash_data_t *hd;

	zeus_size_t sz = 0;

	zeus_uint_t sep = 0;

	zeus_char_t *beg;
	
	if(zeus_config_check(process->config,process->log,&hd,&zeus_config_log) == ZEUS_ERROR){
		return ZEUS_ERROR;
	}
	
	if(zeus_string_equal(process->log->path,hd->d) == 0){
		process->old = NULL;
		process->log->level = ZEUS_DEFAULT_LOG_LEVEL;
		return ZEUS_OK;
	}
	
	alloc_log = (zeus_log_t *)zeus_memory_alloc(process->pool,sizeof(zeus_log_t));
	if(alloc_log == NULL){
		zeus_write_log(process->log,ZEUS_LOG_ERROR,"create log according to config file error");
		return ZEUS_ERROR;
	}

	alloc_log->level = ZEUS_DEFAULT_LOG_LEVEL;
	
	alloc_log->path = (zeus_string_t *)zeus_memory_alloc(process->pool,sizeof(zeus_string_t));
	if(alloc_log->path == NULL){
		zeus_write_log(process->log,ZEUS_LOG_ERROR,"create log path error");
		return ZEUS_ERROR;
	}

	if(hd->d->size <= 1){
		zeus_write_log(process->log,ZEUS_LOG_ERROR,"log config size error");
		return ZEUS_ERROR;
	}

	sz += (hd->d->size - 1);
	if(hd->d->data[hd->d->size - 2] == '/'){
		sep = 1;
	}else{
		sep = 0;
		sz += 1;
	}

	sz += zeus_strlen(ZEUS_LOG_FILENAME);
	sz += 1;

	alloc_log->path->data = (zeus_char_t *)zeus_memory_alloc(process->pool,sizeof(zeus_char_t) * sz);
	if(alloc_log == NULL){
		zeus_write_log(process->log,ZEUS_LOG_ERROR,"create log path error");
		return ZEUS_ERROR;
	}

	alloc_log->path->size = sz;
	
	beg = alloc_log->path->data;

	zeus_memcpy(beg,hd->d->data,hd->d->size - 1);

	beg += hd->d->size - 1;

	if(sep == 0){
		*beg++ = '/';
	}

	zeus_memcpy(beg,ZEUS_LOG_FILENAME,zeus_strlen(ZEUS_LOG_FILENAME));

	beg += zeus_strlen(ZEUS_LOG_FILENAME);

	*beg ++ = '\0';

	if((alloc_log->fd = zeus_open_file(alloc_log->path,O_CREAT | O_RDWR | O_APPEND ,\
					S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == ZEUS_FILE_OPEN_ERROR){
		zeus_write_log(process->log,ZEUS_LOG_ERROR,"create log file %s error : %s",
				alloc_log->path->data,strerror(errno));
		return ZEUS_ERROR;
	}
	
	if(zeus_chown_file(alloc_log->path,process) == ZEUS_ERROR){
		return ZEUS_ERROR;
	}

	alloc_log->status = ZEUS_LOG_OPEN;
	
	process->old = process->log;

	process->log = alloc_log;

	return ZEUS_OK;

}

zeus_status_t zeus_config_get_pid_path(zeus_process_t *process){
	
	zeus_hash_data_t *hd;

	zeus_char_t *beg;

	zeus_size_t sz = 0;

	zeus_uint_t sep = 0;

	if(zeus_config_check(process->config,process->log,&hd,&zeus_config_pid) == ZEUS_ERROR){
		return ZEUS_ERROR;
	}

	process->pid_run_flag_path = (zeus_string_t *)zeus_memory_alloc(process->pool,sizeof(zeus_string_t));
	if(process->pid_run_flag_path == NULL){
		zeus_write_log(process->log,ZEUS_LOG_ERROR,"create pid file string structure error");
		return ZEUS_ERROR;
	}

	if(hd->d->size <= 1){
		zeus_write_log(process->log,ZEUS_LOG_ERROR,"pid file configuration error");
		return ZEUS_ERROR;
	}

	sz += (hd->d->size - 1);

	if(hd->d->data[hd->d->size - 2] == '/'){
		sep = 1;
	}else{
		sep = 0;
		sz += 1;
	}

	sz += zeus_strlen(ZEUS_PID_FILENAME);
	sz += 1;


	process->pid_run_flag_path->data = (zeus_char_t *)zeus_memory_alloc(process->pool,sizeof(zeus_char_t) * sz);
	
	beg = process->pid_run_flag_path->data;

	zeus_memcpy(beg,hd->d->data,hd->d->size - 1);

	beg += (hd->d->size - 1);

	if(sep == 0){
		*beg++ = '/';
	}

	zeus_memcpy(beg,ZEUS_PID_FILENAME,zeus_strlen(ZEUS_PID_FILENAME));

	beg += zeus_strlen(ZEUS_PID_FILENAME);

	*beg++ = '\0';

	return ZEUS_OK;
}
