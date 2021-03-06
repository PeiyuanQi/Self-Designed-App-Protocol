/**********************************
 * Name: 			Peiyuan Qi
 * CaseNetworkID: 	pxq12
 * FileName: 		server.h
 * Created Data: 	20/04/2017
 * Description:
 *	implementation of server.h
 **********************************/

#ifndef P4_SERVER_H
#define P4_SERVER_H

#include "bulletin.h"

#define REQUIRED_ARGC 2
#define PORT_POS 1
#define QUE_LIMIT 1
#define PROTOCOL "tcp"
#define BUFLEN 1024
#define BOARD_SIZE 10
#define INCREMENT 1

void loop(int sd);

void sendErrorPkt(int sd2, enum error_code errorCode);

#endif //P4_SERVER_H
