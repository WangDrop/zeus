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
    
    zeus_event_timer_rbnode_t *y;

    while(z->parent->color == ZEUS_EVENT_TIMER_RED){

        if(z->parent == z->parent->parent->lchild){

            y = z->parent->parent->rchild;

            if(y->color == ZEUS_EVENT_TIMER_RED){

                z->parent->color = ZEUS_EVENT_TIMER_BLACK;
                y->color = ZEUS_EVENT_TIMER_BLACK;
                z->parent->parent->color = ZEUS_EVENT_TIMER_RED;
                z = z->parent->parent;

            }else if(z == z->parent->rchild){

                z = z->parent;
                zeus_event_timer_rbtree_lrotate(t,z);

            }else{

                z->parent->color = ZEUS_EVENT_TIMER_BLACK;
                z->parent->parent->color = ZEUS_EVENT_TIMER_RED;
                zeus_event_timer_rbtree_rrotate(t,z->parent->parent);

            }
        }else{

            y = z->parent->parent->lchild;

            if(y->color == ZEUS_EVENT_TIMER_RED){

                z->parent->color = ZEUS_EVENT_TIMER_BLACK;
                y->color = ZEUS_EVENT_TIMER_BLACK;
                z->parent->parent->color = ZEUS_EVENT_TIMER_RED;
                z = z->parent->parent;

            }else if(z == z->parent->lchild){

                z = z->parent;
                zeus_event_timer_rbtree_rrotate(t,z);

            }else{

                z->parent->color = ZEUS_EVENT_TIMER_BLACK;
                z->parent->parent->color = ZEUS_EVENT_TIMER_RED;
                zeus_event_timer_rbtree_lrotate(t,z->parent->parent);

            }
        }
    }

    t->root->color = ZEUS_EVENT_TIMER_BLACK;
    
    return ZEUS_OK;

}


zeus_status_t zeus_event_timer_rbtree_transplant(zeus_event_timer_rbtree_t *t,\
                                                 zeus_event_timer_rbnode_t *u,\
                                                 zeus_event_timer_rbnode_t *v){


    if(u->parent == t->nil){
        t->root = v;
    }else if(u == u->parent->lchild){
        u->parent->lchild = v;
    }else{
        u->parent->rchild = v;
    }

    v->parent = u->parent;

    return ZEUS_OK;


}


zeus_status_t zeus_event_timer_rbtree_delete(zeus_event_timer_rbtree_t *t,\
                                             zeus_event_timer_rbnode_t *z){

    zeus_event_timer_rbnode_t *x;
    zeus_event_timer_rbnode_t *y = z;
    zeus_uint_t y_origin_color = y->color;
    
    if(z->lchild == t->nil){

        x = z->rchild;
        zeus_event_timer_rbtree_transplant(t,z,z->rchild);

    }else if(z->rchild == t->nil){
    
        x = z->lchild;
        zeus_event_timer_rbtree_transplant(t,z,z->lchild);

    }else{
        
        y = zeus_event_timer_rbtree_find_next(t,z->rchild);
        y_origin_color = y->color;
        x = y->rchild;
        if(y->parent == z){
            x->parent = y;
        }else{
            zeus_event_timer_rbtree_transplant(t,y,y->rchild);
            y->rchild = z->rchild;
            y->rchild->parent = y;
        }
        zeus_event_timer_rbtree_transplant(t,z,y);
        y->lchild = z->lchild;
        y->lchild->parent = y;
        y->color = z->color;

        // recycle z here , object pool;
    
    }
    
    if(y_origin_color == ZEUS_EVENT_TIMER_BLACK){
        return zeus_event_timer_rbtree_delete_fixup(t,x);
    }

    t->nil->lchild = t->nil->rchild = t->nil->parent = NULL;
    
    return ZEUS_OK;
}

zeus_status_t zeus_event_timer_rbtree_delete_fixup(zeus_event_timer_rbtree_t *t,\
                                                   zeus_event_timer_rbnode_t *x){

    zeus_event_timer_rbnode_t *w;

    while(x != t->root && x->color == ZEUS_EVENT_TIMER_BLACK){
        
        if(x == x->parent->lchild){
        
            w = x->parent->rchild;
            
            if(w->color == ZEUS_EVENT_TIMER_RED){
                w->color = ZEUS_EVENT_TIMER_BLACK;
                x->parent->color = ZEUS_EVENT_TIMER_RED;
                zeus_event_timer_rbtree_lrotate(t,x->parent);
                w = x->parent->rchild;
            }

            if(w->lchild->color == ZEUS_EVENT_TIMER_BLACK && w->rchild->color == ZEUS_EVENT_TIMER_BLACK){
                w->color = ZEUS_EVENT_TIMER_RED;
                x = x->parent;
            }else if(w->rchild->color == ZEUS_EVENT_TIMER_BLACK){
                w->lchild->color == ZEUS_EVENT_TIMER_BLACK;
                w->color = ZEUS_EVENT_TIMER_RED;
                zeus_event_timer_rbtree_rrotate(t,w);
                w = x->parent->rchild;
            }else{
                w->color = x->parent->color;
                x->parent->color = ZEUS_EVENT_TIMER_BLACK;
                w->rchild->color = ZEUS_EVENT_TIMER_BLACK;
                zeus_event_timer_rbtree_lrotate(t,x->parent);
                x = t->root;
            }

        }else{

            w = x->parent->lchild;
            
            if(w->color == ZEUS_EVENT_TIMER_RED){
                w->color = ZEUS_EVENT_TIMER_BLACK;
                x->parent->color = ZEUS_EVENT_TIMER_RED;
                zeus_event_timer_rbtree_rrotate(t,x->parent);
                w = x->parent->lchild;
            }

            if(w->lchild->color == ZEUS_EVENT_TIMER_BLACK && w->rchild->color == ZEUS_EVENT_TIMER_BLACK){
                w->color = ZEUS_EVENT_TIMER_RED;
                x = x->parent;
            }else if(w->lchild->color == ZEUS_EVENT_TIMER_BLACK){
                w->rchild->color = ZEUS_EVENT_TIMER_BLACK;
                w->color = ZEUS_EVENT_TIMER_RED;
                zeus_event_timer_rbtree_lrotate(t,w);
                w = x->parent->lchild;
            }else{
                w->color = x->parent->color;
                x->parent->color = ZEUS_EVENT_TIMER_BLACK;
                w->lchild->color = ZEUS_EVENT_TIMER_BLACK;
                zeus_event_timer_rbtree_rrotate(t,x->parent);
                x = t->root;
            }
        
        }
    
    }

    x->color = ZEUS_EVENT_TIMER_BLACK;

}

zeus_event_timer_rbnode_t *zeus_event_timer_rbtree_find_next(zeus_event_timer_rbtree_t *t,\
                                                             zeus_event_timer_rbnode_t *z){
    
    while(z->lchild != t->nil){
        z = z->lchild;
    }

    return z;
    
}
