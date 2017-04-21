/**********************************
 * Name: 			Peiyuan Qi
 * CaseNetworkID: 	pxq12
 * FileName: 		bulletin.h
 * Created Data: 	20/04/2017
 * Description:
 *	declaration of app protocol
 **********************************/

#ifndef P4_BULLETIN_H
#define P4_BULLETIN_H

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>

#define MAX_DATA_SIZE 1024
#define LINE_LIMIT 1024

#define INST_CONNECT 1
#define INST_SHUTDOWN 2
#define INST_ADD 3
#define INST_GETALL 4
#define INST_DELETE 5
#define INST_CLEAR 6
#define INST_UPDATE 7
#define INST_ERROR 8
#define INST_MSG 9

enum error_code {
	bad_input
};

typedef struct pktheader_t {
	unsigned short instruction, caplen;
	unsigned short optBltIndex, optional;
} pktheader;

typedef struct pktblt_t {
	pktheader meta;
	unsigned char data[MAX_DATA_SIZE];
} pktblt;

int errexit(char *format, char *arg);

pktblt preparePkt(char msg[], int index);

void sendPkt(int sd2, pktblt aPkt);


#endif //P4_BULLETIN_H
