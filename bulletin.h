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

#define MAX_DATA_SIZE 1024

typedef struct pktheader {
	unsigned short instruction, caplen;
	unsigned short optBltIndex, optional;
};

struct pktblt {
	pktheader meta;
	unsigned char data[MAX_DATA_SIZE];
};

#endif //P4_BULLETIN_H
