
#include <stdio.h>

#include "../include/global.h"
#include "../include/connection_manager.h"

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
	/*Start Here*/
	sscanf(argv[1], "%" SCNu16, &CONTROL_PORT);
    init();

	return 0;
}
