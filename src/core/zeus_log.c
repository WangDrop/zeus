/*
 *  Email : ydzhao1992#gmail.com    
 *   Date : 2014-12-24
 */

// This is the implemention of the log manipulation

#include "zeus_log.h"

static zeus_char_t *zeus_log_time(zeus_char_t *beg,zeus_char_t *end);
static zeus_char_t *zeus_log_level(zeus_uint_t level,zeus_char_t *beg,zeus_char_t *end);
static zeus_char_t *zeus_log_pid(zeus_char_t *beg,zeus_char_t *end);

static const zeus_char_t *zeus_log_level_string[] = \
                    {                         \
                        "debug",              \
                        "notice",             \
                        "warning",            \
                        "error"               \
                    };


#define zeus_log_newline(a) ((a)='\n')

zeus_log_t *zeus_create_log(zeus_string_t *path,zeus_log_level_t level){
    
    zeus_log_t *alloc;

    alloc = (zeus_log_t *)malloc(sizeof(zeus_log_t));

    if(alloc == NULL){
        zeus_write_log_to_stderr(ZEUS_LOG_ERROR,"malloc zeus_log_t error");
        return NULL;
    }

    alloc->level = level;

    alloc->path = (zeus_string_t *)malloc(sizeof(zeus_string_t));
    if(alloc->path == NULL){
        zeus_write_log_to_stderr(ZEUS_LOG_ERROR,"malloc zeus_string_t error");
        free(alloc);
        return NULL;
    }
       
    if(path == NULL){
        alloc->path->data = (zeus_char_t *)malloc(sizeof("/dev/stderr"));
        if(alloc->path->data == NULL){
            zeus_write_log_to_stderr(ZEUS_LOG_ERROR,"malloc zeus_char_t array error");
            free(alloc->path);
            free(alloc);
            return NULL;
        }
        alloc->path->size = sizeof("/dev/stderr");
        zeus_memcpy(alloc->path->data,"/dev/stderr",alloc->path->size);
        alloc->fd = STDERR_FILENO;
    }else{
        alloc->path->data = (zeus_char_t *)malloc(path->size);
        if(alloc->path->data == NULL){
            zeus_write_log_to_stderr(ZEUS_LOG_ERROR,"malloc zeus_char_t array error");
            free(alloc->path);
            free(alloc);
            return NULL;
        }
        alloc->path->size = path->size;
        zeus_memcpy(alloc->path->data,path->data,alloc->path->size);
        if((alloc->fd = zeus_open_file(alloc->path,O_CREAT | O_RDWR | O_APPEND,\
                         S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)) == ZEUS_FILE_OPEN_ERROR){
            zeus_write_log_to_stderr(ZEUS_LOG_ERROR,"open log file %s error : %s",alloc->path,strerror(errno));
            free(alloc->path->data);
            free(alloc->path);
            free(alloc);
            return NULL;
        }
    }

    alloc->status = ZEUS_LOG_OPEN;

    return alloc;
}

zeus_log_res_t zeus_write_log_to_stderr(zeus_log_level_t level,const zeus_char_t *fmt,...){
    
    zeus_char_t data[ZEUS_LOG_MAX_LENGTH];
    zeus_char_t *beg = data;
    zeus_char_t *end = data + ZEUS_LOG_MAX_LENGTH - 1;
    va_list ap;

    beg = zeus_log_time(beg,end);
    if(beg == end){
        zeus_log_newline(*beg++);
        write(STDERR_FILENO,(const void *)data,beg-data);
    }
    
    beg = zeus_log_level(level,beg,end);
    if(beg == end){
        zeus_log_newline(*beg++);
        write(STDERR_FILENO,(const void *)data,beg-data);
    }

    beg = zeus_log_pid(beg,end);
    if(beg == end){
        zeus_log_newline(*beg++);
        write(STDERR_FILENO,(const void *)data,beg-data);
    }

    va_start(ap,fmt);
    beg = zeus_vsnprintf(beg,end,fmt,ap);
    va_end(ap);
    
    zeus_log_newline(*beg++);
    write(STDERR_FILENO,(const void *)data,beg-data);

    return ZEUS_LOG_OK;
}

zeus_log_res_t zeus_write_log(zeus_log_t *log,zeus_log_level_t level,const zeus_char_t *fmt,...){
    
    zeus_char_t data[ZEUS_LOG_MAX_LENGTH];
    zeus_char_t *beg = data;
    zeus_char_t *end = data + ZEUS_LOG_MAX_LENGTH - 1;
    va_list ap;

    if(level < log->level){
        return ZEUS_LOG_IGNO;
    }
    
    beg = zeus_log_time(beg,end);
    if(beg == end){
        zeus_log_newline(*beg++);
        write(log->fd,(const void *)data,beg-data);
    }
    
    beg = zeus_log_level(level,beg,end);
    if(beg == end){
        zeus_log_newline(*beg++);
        write(log->fd,(const void *)data,beg-data);
    }

    beg = zeus_log_pid(beg,end);
    if(beg == end){
        zeus_log_newline(*beg++);
        write(log->fd,(const void *)data,beg-data);
    }

    va_start(ap,fmt);
    beg = zeus_vsnprintf(beg,end,fmt,ap);
    va_end(ap);

    zeus_log_newline(*beg++);
    write(log->fd,(const void *)data,beg-data);

    return ZEUS_LOG_OK;
}

zeus_char_t *zeus_log_time(zeus_char_t *beg,zeus_char_t *end){
    
    zeus_int_t nwrite;

    nwrite = snprintf(beg,(zeus_size_t)(end - beg),"[%s]",zeus_get_time());
    
    return (beg + nwrite < end) ? (beg + nwrite) : end;

}

zeus_char_t *zeus_log_level(zeus_uint_t level,zeus_char_t *beg,zeus_char_t *end){

    zeus_int_t nwrite;

    nwrite = snprintf(beg,(zeus_size_t)(end - beg),"[%s]",zeus_log_level_string[level]);

    return (beg + nwrite < end) ? (beg + nwrite) : end;

}

zeus_char_t *zeus_log_pid(zeus_char_t *beg,zeus_char_t *end){
    
    zeus_int_t nwrite;

    nwrite = snprintf(beg,(zeus_size_t)(end - beg),"[%d]",getpid()); // we will replace getpid() with cache data later
    
    return (beg + nwrite < end) ? (beg + nwrite) : end;
}
