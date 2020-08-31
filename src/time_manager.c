#include <sys/queue.h>
#include <sys/time.h>
#include <netinet/in.h>

#include "../include/global.h"
#include "../include/time_manager.h"
#include "../include/router_handler.h"

void init_time(){
    LIST_INIT(&time_list);
    struct router *r;
    LIST_FOREACH(r,&router_list,next){
        struct time *t = (struct time*)malloc(sizeof(struct time));
        t->router_ip_addr = r->ip;
        t->index = r->index;
        gettimeofday(&t->begin_send_time,NULL);
        gettimeofday(&t->begin_expire_time,NULL);
        t->send_time.tv_sec = updates_periodic_interval;
        t->send_time.tv_usec = 0;
        t->expire_time.tv_sec = 3 * updates_periodic_interval;
        t->expire_time.tv_usec = 0;
        t->isconnect = r->connect;
        LIST_INSERT_HEAD(&time_list, t, next);
    }
}

struct timeval diff_time(struct timeval begin, struct timeval end){
    struct timeval diff;
    int microseconds = (end.tv_sec - begin.tv_sec) * 1000000 + ((int)end.tv_usec - (int)begin.tv_usec);
    if(microseconds > 0){
        diff.tv_usec = microseconds % 1000000;
        diff.tv_sec = microseconds / 1000000;
    }
    else{
        diff.tv_usec = -1;
        diff.tv_sec = -1;
    }
    return diff;
}

struct timeval total_time(struct timeval base,struct timeval interval){
    struct timeval total;
    int microseconds = (int)base.tv_usec + (int)interval.tv_usec;
    total.tv_usec =  microseconds % 1000000;
    total.tv_sec = base.tv_sec + interval.tv_sec + microseconds / 1000000;
    return total;
}

void update_time(int router_socket){
    struct timeval current;
    gettimeofday(&current,NULL);
    struct timeval expire;
    expire.tv_sec = 3 * updates_periodic_interval;
    expire.tv_usec = 0;
    struct timeval send;
    send.tv_sec = updates_periodic_interval;
    send.tv_usec = 0;
    struct time *t;
    print_time_connection();
    print_router_connection();
    LIST_FOREACH(t,&time_list,next){
        if(t->isconnect == TRUE){
            struct timeval diff_expire = diff_time(current,total_time(t->begin_expire_time,expire));
            struct timeval diff_send = diff_time(current,total_time(t->begin_send_time,send));
            t->expire_time.tv_sec = diff_expire.tv_sec;
            t->expire_time.tv_usec = diff_expire.tv_usec;
            t->send_time.tv_sec = diff_send.tv_sec;
            t->send_time.tv_usec = diff_send.tv_usec;
            if(diff_expire.tv_sec < 0 || diff_expire.tv_usec < 0){
                printf("Expired,router id = %d\n",t->index);
                t->isconnect = FALSE;
                struct router *r;
                LIST_FOREACH(r,&router_list,next){
                    if(r->index == t->index){
                        r->connect = FALSE;
                        r->cost = htons(UINT16_MAX);
                    }
                }
            }
            else if(diff_send.tv_sec < 0 || diff_send.tv_usec < 0){
                printf("Send timeout,router id = %d\n",t->index);
                t->begin_send_time = total_time(t->begin_send_time,send);
                t->send_time.tv_sec = updates_periodic_interval;
                t->send_time.tv_usec = 0;
                struct router *r;
                LIST_FOREACH(r,&router_list,next){
                    if(r->ip == t->router_ip_addr && r->connect == TRUE)
                        send_vector(router_socket,r);
                }
            }
        }
    }
}

struct timeval time_out(){
    struct time *t;
    struct timeval result;
    result.tv_sec = 1000000;
    int isinitial = 0;
    
    LIST_FOREACH(t,&time_list,next){
        if(t->isconnect == TRUE){
            if(isinitial == 0){
                result.tv_sec = t->expire_time.tv_sec;
                result.tv_usec = t->expire_time.tv_usec;
                struct timeval diff = diff_time(result,t->send_time);
                if(diff.tv_sec < 0 || diff.tv_usec < 0){
                    result.tv_sec = t->send_time.tv_sec;
                    result.tv_usec = t->send_time.tv_usec;
                }
                isinitial = 1;
            }
            else{
                struct timeval diff_expire = diff_time(result,t->expire_time);
                if(diff_expire.tv_sec < 0 || diff_expire.tv_usec < 0){
                    result.tv_sec = t->expire_time.tv_sec;
                    result.tv_usec = t->expire_time.tv_usec;
                }
                struct timeval diff_send = diff_time(result,t->send_time);
                if(diff_send.tv_sec < 0 || diff_send.tv_usec < 0){
                    result.tv_sec = t->send_time.tv_sec;
                    result.tv_usec = t->send_time.tv_usec;
                }            
            }        
        }
    }
    return result;
}
