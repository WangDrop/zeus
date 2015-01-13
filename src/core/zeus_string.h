/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2014-12-21
 */

// This file is about string manipulation.


#ifndef __ZEUS_STRING_H_H_H__
#define __ZEUS_STRING_H_H_H__

#include "zeus_common.h"

struct zeus_string_s{
    zeus_char_t *data;
    zeus_size_t size;
};

#define zeus_memcpy(d,s,n) (memcpy(d,s,n))
#define zeus_strlen(s) (strlen(s))

zeus_char_t *zeus_vsnprintf(zeus_char_t *,zeus_char_t *,const zeus_char_t *,va_list);
zeus_status_t zeus_string_to_uint(zeus_char_t *,zeus_char_t *,zeus_uint_t *);
zeus_status_t zeus_string_to_ushort(zeus_char_t *,zeus_char_t *,zeus_ushort_t *);
zeus_int_t zeus_string_equal(zeus_string_t *,zeus_string_t *);

#endif
