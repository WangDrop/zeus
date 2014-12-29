/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2014-12-24
 */

// This is the implemention of file operation

#include "zeus_file.h"

zeus_fd_t zeus_open_file(zeus_string_t *path,zeus_int_t flag,zeus_mode_t mode){
    return open(path->data,flag,mode);
}
