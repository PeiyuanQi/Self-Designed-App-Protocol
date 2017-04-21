/**********************************
 * Name: 			Peiyuan Qi
 * CaseNetworkID: 	pxq12
 * FileName: 		client.h
 * Created Data: 	20/04/2017
 * Description:
 *	implementation of client.h
 **********************************/

#ifndef P4_CLIENT_H
#define P4_CLIENT_H

#include "bulletin.h"

#define REQUIRED_ARGC 3
#define HOST_POS 1
#define PORT_POS 2
#define PROTOCOL "tcp"
#define INPUT_LIMIT 1280

#define TOK_DELIM " \t\r\n\a"

void clientLoop(int sd);

char *read_line(void);

char **split_line(char **args, char *line, int *argc);

bool execute(int sd, char **args, int *argc);

#endif //P4_CLIENT_H
