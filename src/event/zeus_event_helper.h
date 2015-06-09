/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2015-02-11
 */

// This is the event helper interface

#ifndef __ZEUS_EVENT_HELPER_H_H_H__
#define __ZEUS_EVENT_HELPER_H_H_H__


#include "../core/zeus_common.h"

zeus_status_t zeus_helper_add_event(zeus_process_t *,zeus_connection_t *);
zeus_status_t zeus_helper_del_event(zeus_process_t *,zeus_connection_t *);
zeus_status_t zeus_helper_mod_event(zeus_process_t *,zeus_connection_t *);
zeus_status_t zeus_helper_close_connection(zeus_process_t *,zeus_connection_t *);
zeus_idx_t zeus_helper_find_load_lowest(zeus_process_t *);
zeus_status_t zeus_helper_trans_socket(zeus_process_t *,zeus_connection_t *,zeus_idx_t);
zeus_status_t zeus_helper_move_to_closing_connection_list(zeus_process_t *,zeus_connection_t *,zeus_idx_t);
zeus_status_t zeus_helper_unlink_connection_at_closing_list(zeus_process_t *,zeus_idx_t);

#endif
