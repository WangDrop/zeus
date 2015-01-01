/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2014-12-24
 */

// log manipulation.

#ifndef __ZEUS_LOG_H_H_H__
#define __ZEUS_LOG_H_H_H__

#include "zeus_common.h"

#define ZEUS_LOG_DEBUG 0x0
#define ZEUS_LOG_NOTICE 0x1
#define ZEUS_LOG_WARNING 0x2
#define ZEUS_LOG_ERROR 0x3

#define ZEUS_LOG_OK 0x0
#define ZEUS_LOG_FAIL 0x1
#define ZEUS_LOG_IGNO 0x2

#define ZEUS_LOG_OPEN 0x0
#define ZEUS_LOG_CLOSE 0x1

#define ZEUS_LOG_MAX_LENGTH 4096

typedef unsigned int zeus_log_level_t;
typedef unsigned int zeus_log_status_t;
typedef unsigned int zeus_log_res_t;

struct zeus_log_s{
    
    zeus_log_level_t level;  

    zeus_fd_t fd;

    zeus_log_status_t status;
    
    zeus_string_t *path;

    zeus_list_t *buf;

};

zeus_log_t *zeus_create_log(zeus_string_t *path,zeus_log_level_t level);
zeus_log_res_t zeus_write_log(zeus_log_t *,zeus_log_level_t,const zeus_char_t *,...);
zeus_log_res_t zeus_write_log_to_stderr(zeus_log_level_t,const zeus_char_t *,...);
zeus_log_res_t zeus_write_log_in_loop(zeus_log_t *,zeus_log_level_t,const zeus_char_t *,...);

#endif
