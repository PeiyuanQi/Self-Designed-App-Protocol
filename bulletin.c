/**********************************
 * Name: 			Peiyuan Qi
 * CaseNetworkID: 	pxq12
 * FileName: 		bulletin.h
 * Created Data: 	20/04/2017
 * Description:
 *	implementation of app protocol functions
 **********************************/

#include "bulletin.h"



int errexit(char *format, char *arg) {
	fprintf(stderr, format, arg);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

pktblt preparePkt(char msg[], int index) {
	pktblt aPkt;
	aPkt.meta.instruction = INST_MSG;
	char data[LINE_LIMIT];

	sprintf(data, "Board %d: %s", index, msg);
	if (strlen(data) <= LINE_LIMIT) {
		aPkt.meta.caplen = strlen(data);
		strcpy((char *) aPkt.data, data);
	} else {
		fprintf(stderr, "Error: Message is too long!\n");
	}
	return aPkt;
}

void sendPkt(int sd2, pktblt aPkt) {
	if ((write(sd2, &aPkt, sizeof(aPkt))) < 0) {
		errexit("error writing message: %s", (char *) aPkt.data);
	}
}