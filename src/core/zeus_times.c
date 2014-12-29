/*
 *  Email : ydzhao1992#gmail.com    
 *   date : 2014-12-21
 */

// This is the implemention of the cache time lib.
// We use this just in the multiple processes and single thread environment,
// so we do not take thread-safe into consideration.

#include "zeus_times.h"

static const zeus_char_t zeus_time_sample[] = "2014-12-21 16:18:20 Sun";
#define ZEUS_CACHE_TIME_STRING_LENGTH (sizeof(zeus_time_sample)/sizeof(zeus_time_sample[0]))

static const zeus_char_t *zeus_time_weekday[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
static zeus_timeval_t zeus_cache_time;
static zeus_char_t zeus_cache_time_string[ZEUS_CACHE_TIME_STRING_LENGTH];

// Update or get cached time , we need take async-signal-safe into consideration.
// When we are in a signal handler , we will not use the cache time , we may use a local object in the signal handler.

void zeus_update_time(){
    
    gettimeofday(&zeus_cache_time,NULL);
    
    zeus_trans_timeval_to_string();
    
    return ;

}


const zeus_char_t *zeus_get_time(){

    static zeus_uint_t zeus_time_flag = 0;

    if(zeus_time_flag == 0){
        ++ zeus_time_flag;
        zeus_update_time();
    }

    return zeus_cache_time_string;

}


void zeus_trans_timeval_to_string(){
    
    time_t t = zeus_cache_time.tv_sec;
    struct tm tmval;

    if(localtime_r(&t,&tmval) == NULL){
        zeus_memcpy(zeus_cache_time_string,"",1);
    }else{
        snprintf(zeus_cache_time_string,ZEUS_CACHE_TIME_STRING_LENGTH,\
                 "%04d-%02d-%02d %02d:%02d:%02d %s",\
                 tmval.tm_year+1900,tmval.tm_mon+1,tmval.tm_mday,\
                 tmval.tm_hour,tmval.tm_min,tmval.tm_sec,zeus_time_weekday[tmval.tm_wday]);
    }
    
    return ;

}
