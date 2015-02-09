/*
 *  Email : ydzhao1992#gmail.com
 *   Date : 2015-02-08
 */

// This is the implemention of the timer-rb tree

#include "zeus_event_timer.h"

zeus_status_t zeus_event_timer_rbtree_construct(zeus_process_t *p){

    p->timer = (zeus_event_timer_rbtree_t *)zeus_memory_alloc(p->pool,\
                sizeof(zeus_event_timer_rbtree_t));

    if(p->timer == NULL){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"create timer rbtree error");
        return ZEUS_ERROR;
    }

    p->timer->nil = (zeus_event_timer_rbnode_t *)zeus_memory_alloc(p->pool,\
                     sizeof(zeus_event_timer_rbnode_t));

    if(p->timer->nil == NULL){
        zeus_write_log(p->log,ZEUS_LOG_ERROR,"create timer rbtree nil error");
        return ZEUS_ERROR;
    }

    p->timer->nil->lchild = p->timer->nil->rchild \
                          = p->timer->nil->parent \
                          = NULL;

    p->timer->nil->color = ZEUS_EVENT_TIMER_BLACK;
    p->timer->root = p->timer->nil;
    
    return ZEUS_OK;

}

zeus_int_t zeus_event_timer_rbtree_key_compare(zeus_timeval_t *u,zeus_timeval_t *v){
    
    if(u->tv_sec > v->tv_sec){

        return ZEUS_EVENT_TIMER_GT;

    }else if(u->tv_sec < v->tv_sec){

        return ZEUS_EVENT_TIMER_LT;

    }else{

        if(u->tv_usec > v->tv_usec){
            return ZEUS_EVENT_TIMER_GT;
        }else if(u->tv_usec < v->tv_usec){
            return ZEUS_EVENT_TIMER_LT;
        }else{
            return ZEUS_EVENT_TIMER_EQ;
        }

    }

}

zeus_status_t zeus_event_timer_rbtree_lrotate(zeus_event_timer_rbtree_t *t,\
                                              zeus_event_timer_rbnode_t *x){
    
    zeus_event_timer_rbnode_t *y = x->rchild;
    x->rchild = y->lchild;

    if(y->lchild != t->nil){
        y->lchild->parent = x;
    }

    y->parent = x->parent;

    if(x->parent == t->nil){
        t->root = y;
    }else if(x == x->parent->lchild){
        x->parent->lchild = y;
    }else{
        x->parent->rchild = y;
    }

    y->lchild = x;
    x->parent = y;
    
    return ZEUS_OK;

}

zeus_status_t zeus_event_timer_rbtree_rrotate(zeus_event_timer_rbtree_t *t,\
                                              zeus_event_timer_rbnode_t *x){

    zeus_event_timer_rbnode_t *y = x->lchild;
    x->lchild = y->rchild;

    if(y->rchild != t->nil){
        y->rchild->parent = x;
    }
    
    y->parent = x->parent;
    
    if(x->parent == t->nil){
        t->root = y;
    }else if(x == x->parent->lchild){
        x->parent->lchild = y;
    }else{
        x->parent->rchild = y;
    }

    y->rchild = x;
    x->parent = y;

    return ZEUS_OK;


}



zeus_status_t zeus_event_timer_rbtree_insert(zeus_event_timer_rbtree_t *t,\
                                             zeus_event_timer_rbnode_t *z){
    
    zeus_event_timer_rbnode_t *y = t->nil;
    zeus_event_timer_rbnode_t *x = t->root;

    while(x != t->nil){
        y = x;
        if(zeus_event_timer_rbtree_key_compare(&(z->t),&(x->t)) < 0){
            x = x->lchild;
        }else{
            x = x->rchild;
        }
    }

    z->parent = y;
    if(y == t->nil){
        t->root = z;
    }else if(zeus_event_timer_rbtree_key_compare(&(z->t),&(y->t)) < 0){
        y->lchild = z;
    }else{
        y->rchild = z;
    }

    z->lchild = z->rchild = t->nil;
    z->color = ZEUS_EVENT_TIMER_RED;
    
    return zeus_event_timer_rbtree_insert_fixup(t,z);

}

zeus_status_t zeus_event_timer_rbtree_insert_fixup(zeus_event_timer_rbtree_t *t,\
                                                   zeus_event_timer_rbnode_t *z){



    
    return ZEUS_OK;

}
