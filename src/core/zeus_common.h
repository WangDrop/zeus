/*
 *  email : ydzhao1992#gmail.com
 *   date : 2014-12-12
 */

// This is the common header-include file

#ifndef __ZEUS_COMMON_H_H_H__
#define __ZEUS_COMMON_H_H_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <inttypes.h>
#include <pwd.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

typedef unsigned long zeus_ulong_t;
typedef unsigned char zeus_uchar_t;
typedef unsigned int zeus_uint_t;
typedef sig_atomic_t zeus_sig_atomic_t;
typedef size_t zeus_size_t;
typedef struct timeval zeus_timeval_t;
typedef char zeus_char_t;
typedef int zeus_int_t;
typedef int zeus_fd_t;
typedef mode_t zeus_mode_t;
typedef unsigned int zeus_status_t;
typedef uid_t zeus_uid_t;
typedef gid_t zeus_gid_t;
typedef uint16_t zeus_port_t;
typedef uint16_t zeus_ushort_t;
typedef pid_t zeus_pid_t;
typedef int zeus_idx_t;
typedef sig_atomic_t zeus_atomic_t;
typedef unsigned long zeus_event_status_t;
typedef struct epoll_event zeus_epoll_event_t;

#define ZEUS_OK 0x0
#define ZEUS_ERROR 0x1


typedef struct zeus_memory_pool_s zeus_memory_pool_t;
typedef struct zeus_string_s zeus_string_t;
typedef struct zeus_log_s zeus_log_t;
typedef struct zeus_hash_s zeus_hash_t;
typedef struct zeus_hash_data_s zeus_hash_data_t;
typedef struct zeus_process_s zeus_process_t;
typedef struct zeus_config_s zeus_config_t;
typedef struct zeus_buffer_s zeus_buffer_t;
typedef struct zeus_list_s zeus_list_t;
typedef struct zeus_list_data_s  zeus_list_data_t;
typedef struct zeus_connection_s zeus_connection_t;
typedef struct zeus_signal_s zeus_signal_t;

typedef struct zeus_event_s zeus_event_t;


#include "../../build_/zeus_build.h"

#include "zeus_alloc.h"
#include "zeus_times.h"
#include "zeus_string.h"
#include "zeus_log.h"
#include "zeus_file.h"
#include "zeus_process.h"
#include "zeus_hash.h"
#include "zeus_config.h"
#include "zeus_buffer.h"
#include "zeus_list.h"
#include "zeus_connection.h"
#include "zeus_signal.h"

#include "../event/zeus_event.h"
#include "../event/zeus_event_io.h"

#define ZEUS_DEFAULT_LOG_LEVEL ZEUS_LOG_NOTICE
#define ZEUS_LOG_FILENAME "zeus.log"
#define ZEUS_PID_FILENAME "zeus.pid"

extern char **environ;

#endif
