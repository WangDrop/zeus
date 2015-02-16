/*
 *  email : ydzhao1992#gmail.com
 *   date : 2014-12-21
 */

// This is the implemention of the time library.
// In order to avoid calling gettimeofday so many times,we cache the time.
// Only for multiple processes and single thread.

#ifndef __ZEUS_TIMES_H_H_H__
#define __ZEUS_TIMES_H_H_H__

#include "zeus_common.h"

void zeus_update_time();
void zeus_trans_timeval_to_string();
const zeus_char_t *zeus_get_time();
zeus_timeval_t *zeus_get_cache_timeval();

#endif
