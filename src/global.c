#include <stdio.h>
#include <netinet/in.h>
#include <sys/queue.h>
#include "../include/global.h"
#include "../include/time_manager.h"

void print_routing_table(){
    int i;
    int j;
    printf("------------routing table------------\n");
    for(i = 0;i != number_of_routers;++i){
        for(j = 0;j != number_of_routers;++j){
            printf("%d|",distance_vector[i][j]);
        }
        printf("\n");
    }
    printf("-------------------------------------\n");
}

void print_next_hop(){
    printf("------------next hop------------------\n");
    struct router *r;
    LIST_FOREACH(r,&router_list,next){
        printf("%d = %d\n",ntohs(r->id),ntohs(r->next_hop));
    }
    printf("---------------------------------------\n");
}

void print_time_connection(){
    struct time *t;
    printf("------------time connection-------------\n");
    LIST_FOREACH(t,&time_list,next){
        printf("%d is connected?:%d\n",t->index,t->isconnect);
    }
    printf("----------------------------------------\n");
}

void print_router_connection(){
    struct router *r;
    printf("------------router connection-------------\n");
    LIST_FOREACH(r,&router_list,next){
        printf("%d is connected?:%d\n",ntohs(r->id),r->connect);
    }
    printf("----------------------------------------\n");
}
