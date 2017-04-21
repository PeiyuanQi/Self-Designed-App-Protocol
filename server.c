/**********************************
 * Name: 			Peiyuan Qi
 * CaseNetworkID: 	pxq12
 * FileName: 		server.cpp
 * Created Data: 	20/04/2017
 * Description:
 *	implementation of server
 **********************************/

#include "server.h"

int usage(char *progname) {
	fprintf(stderr, "usage: %s port\n", progname);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
	struct sockaddr_in sin;
	struct protoent *protoinfo;
	int sd;

	//get cmd options
	if (argc != REQUIRED_ARGC)
		usage(argv[0]);

	/* determine protocol */
	if ((protoinfo = getprotobyname(PROTOCOL)) == NULL)
		errexit("cannot find protocol information for %s", PROTOCOL);

	/* setup endpoint info */
	memset((char *) &sin, 0x0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons ((u_short) atoi(argv[PORT_POS]));

	/* allocate a socket */
	sd = socket(PF_INET, SOCK_STREAM, protoinfo->p_proto);
	if (sd < 0)
		errexit("cannot create socket", NULL);

	/* bind the socket */
	if (bind(sd, (struct sockaddr *) &sin, sizeof(sin)) < 0)
		errexit("cannot bind to port %s", argv[PORT_POS]);

	/* listen for incoming connections */
	if (listen(sd, QUE_LIMIT) < 0)
		errexit("cannot listen on port %s\n", argv[PORT_POS]);

	//start working
	loop(sd);

	return EXIT_SUCCESS;
}

void loop(int sd) {
	int curIndex = 0;
	int ret, sd2;
	struct sockaddr addr;
	unsigned int addrlen;
	pktblt rpacket;
	char board[BOARD_SIZE][LINE_LIMIT];
	bool placeHold[BOARD_SIZE], status = true;

	//initialize board
	for (int i = 0; i < 10; ++i) {
		strcpy(board[i], "");
		placeHold[i] = false;
	}

	//do the loop
	do {
		/* accept a connection */
		sd2 = accept(sd, &addr, &addrlen);
		if (sd2 < 0)
			errexit("error accepting connection", NULL);

		memset(&rpacket, 0x0, sizeof(rpacket));
		ret = read(sd2, &rpacket, sizeof(rpacket));
		if (ret < 0){
			errexit("reading error", NULL);
		}
		if (rpacket.meta.instruction > 0) {
			switch (rpacket.meta.instruction) {
				case INST_CONNECT:
					fprintf(stdout, "C -> S: Connecting...\nS -> C: Connected!\n");
					break;
				case INST_SHUTDOWN:
					status = false;
					fprintf(stdout, "C -> S: Shutdown Server.\nS: Server is shutting down...\n");
					break;
				case INST_ADD:
					if (rpacket.meta.caplen <= LINE_LIMIT) {
						strcpy(board[curIndex], (char *) rpacket.data);
						placeHold[curIndex] = true;
						fprintf(stdout, "C -> S: Add Bulletin: %s.\nS: New bulletin added to position %d\n",
						        board[curIndex], curIndex);
						sendPkt(sd2, prepareMSGPkt("Add Success\n",curIndex));
						curIndex++;
					} else {
						fprintf(stdout, "C -> S: Bad Input\nS: Error Message\n");
						sendErrorPkt(sd2, bad_input);
						fprintf(stderr, "Error: Exceed input line limit\n");
					}
					break;
				case INST_GETALL:
					for (int i = 0; i < 10; ++i) {
						if (placeHold[i]) {
							sendPkt(sd2, prepareMSGPkt(board[i], i));
						}
					}
					break;
				default:
					break;
			}
		}

	} while (status);
}

void sendErrorPkt(int sd2, enum error_code errorCode) {
	pktblt epacket;

	memset(&epacket, 0x0, sizeof(epacket));
	switch (errorCode) {
		case bad_input:
			epacket.meta.instruction = INST_ERROR;
			strcpy((char *) epacket.data, "Error: Bad input, line limit exceed!\n");
			break;
		default:
			epacket.meta.instruction = INST_ERROR;
			strcpy((char *) epacket.data, "Error Unknown!\n");
			break;
	}
	sendPkt(sd2, epacket);
}

