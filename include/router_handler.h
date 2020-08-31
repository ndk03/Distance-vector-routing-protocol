#ifndef _ROUTING_HANDLER_H_
#define _ROUTING_HANDLER_H_

#define UPDATE_FIELD 2
#define SOURCE_ROUTER_PORT 2
#define SOURCE_IP_ADDR 4
#define UPDATE_HEADER 8
#define UPDATE_ENTRY_LENGTH 12

void recv_update(int router_socket);
void send_vector(int router_socket, struct router *r);

#endif
