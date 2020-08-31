#include <string.h>

#include "../include/global.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"

#define AUTHOR_STATEMENT "I, ndkamath, have read and understood the course academic integrity policy."

void author_response(int sock_index)
{
	uint16_t payload_len, response_len;
	char *cntrl_response_header, *cntrl_response_payload, *cntrl_response;

	payload_len = sizeof(AUTHOR_STATEMENT)-1; 
	cntrl_response_payload = (char *) malloc(payload_len);
	memcpy(cntrl_response_payload, AUTHOR_STATEMENT, payload_len);

	cntrl_response_header = create_response_header(sock_index, 0, 0, payload_len);

	response_len = CONTROL_RESPONSE_HEADER_SIZE+payload_len;
	cntrl_response = (char *) malloc(response_len);
	
	memcpy(cntrl_response, cntrl_response_header, CONTROL_RESPONSE_HEADER_SIZE);
	free(cntrl_response_header);
	
	memcpy(cntrl_response+CONTROL_RESPONSE_HEADER_SIZE, cntrl_response_payload, payload_len);
	free(cntrl_response_payload);

	sendALL(sock_index, cntrl_response, response_len);

	free(cntrl_response);
}
