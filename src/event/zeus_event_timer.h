/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2015-02-08
 */

// This is the interface of the timer rb-tree

#ifndef __ZEUS_EVENT_TIMER_H_H_H__
#define __ZEUS_EVENT_TIMER_H_H_H__

#include "../core/zeus_common.h"

#define ZEUS_EVENT_TIMER_RED 0
#define ZEUS_EVENT_TIMER_BLACK 1

#define ZEUS_EVENT_TIMER_EQ 0
#define ZEUS_EVENT_TIMER_GT 1
#define ZEUS_EVENT_TIMER_LT -1

struct zeus_event_timer_rbnode_s{

    zeus_event_timer_rbnode_t *lchild;
    zeus_event_timer_rbnode_t *rchild;
    zeus_event_timer_rbnode_t *parent;
    zeus_event_timer_color color;
    zeus_timeval_t t;

};

struct zeus_event_timer_rbtree_s{
    
    zeus_event_timer_rbnode_t *nil;
    zeus_event_timer_rbnode_t *root;

};

zeus_status_t zeus_event_timer_rbtree_construct(zeus_process_t *);

zeus_int_t zeus_event_timer_rbtree_key_compare(zeus_timeval_t *,zeus_timeval_t *);
zeus_status_t zeus_event_timer_rbtree_lrotate(zeus_event_timer_rbtree_t *,zeus_event_timer_rbnode_t *);
zeus_status_t zeus_event_timer_rbtree_rrotate(zeus_event_timer_rbtree_t *,zeus_event_timer_rbnode_t *);
zeus_status_t zeus_event_timer_rbtree_insert(zeus_event_timer_rbtree_t *,zeus_event_timer_rbnode_t *);
zeus_status_t zeus_event_timer_rbtree_insert_fixup(zeus_event_timer_rbtree_t *,zeus_event_timer_rbnode_t *);


zeus_event_timer_rbnode_t *zeus_event_timer_rbtree_find_next(zeus_event_timer_rbtree_t *,
                                                             zeus_event_timer_rbnode_t *);
zeus_status_t zeus_event_timer_rbtree_transplant(zeus_event_timer_rbtree_t *,\
                                                 zeus_event_timer_rbnode_t *,\
                                                 zeus_event_timer_rbnode_t *);
zeus_status_t zeus_event_timer_rbtree_delete(zeus_event_timer_rbtree_t *,zeus_event_timer_rbnode_t *);
zeus_status_t zeus_event_timer_rbtree_delete_fixup(zeus_event_timer_rbtree_t *,zeus_event_timer_rbnode_t *);

#endif
