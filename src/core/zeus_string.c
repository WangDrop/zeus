/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2014-12-24
 */

// This is the implemention of the string operation

#include "zeus_string.h"

zeus_char_t *zeus_vsnprintf(zeus_char_t *beg,zeus_char_t *end,const zeus_char_t *fmt,va_list ap){
    
    zeus_int_t nwrite;

    nwrite = vsnprintf(beg,(zeus_size_t)(end - beg),fmt,ap);

    return (beg + nwrite < end) ? (beg + nwrite) : end;

}
