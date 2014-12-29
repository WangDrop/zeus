/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2014-12-24
 */

// This is the interface of file operation

#ifndef __ZEUS_FILE_H_H_H__
#define __ZEUS_FILE_H_H_H__

#include "zeus_common.h"

#define ZEUS_FILE_OPEN_ERROR -1
#define ZEUS_FILE_OPEN_SUCCESS 0

zeus_fd_t zeus_open_file(zeus_string_t *,zeus_int_t,zeus_mode_t);

#endif
