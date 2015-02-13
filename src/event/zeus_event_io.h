/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2015-01-23
 */

// This is the interface of io handler

#ifndef __ZEUS_EVENT_IO_H_H_H__
#define __ZEUS_EVENT_IO_H_H_H__


#include "../core/zeus_common.h"

zeus_status_t zeus_event_io_read(zeus_process_t *,zeus_event_t *);

zeus_status_t zeus_event_io_write(zeus_process_t *,zeus_event_t *);

zeus_status_t zeus_event_io_accept(zeus_process_t *,zeus_event_t *);

#endif
