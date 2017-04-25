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
	int ret, sd2, tmpIndex;
	struct sockaddr addr;
	unsigned int addrlen;
	pktblt rpacket;
	char board[BOARD_SIZE][LINE_LIMIT];
	bool placeHold[BOARD_SIZE], status = true;

	//initialize board
	for (int i = 0; i < BOARD_SIZE; ++i) {
		strcpy(board[i], "");
		placeHold[i] = false;
	}

	//accept a new income connection
	sd2 = accept(sd, &addr, &addrlen);

	//do the loop
	do {
		/* accept a connection */

		if (sd2 < 0)
			errexit("error accepting connection", NULL);

		memset(&rpacket, 0x0, sizeof(rpacket));
		ret = read(sd2, &rpacket, sizeof(rpacket));
		if (ret < 0) {
			errexit("reading error", NULL);
		}
		if (rpacket.meta.instruction > 0) {
			switch (rpacket.meta.instruction) {
				case INST_CONNECT:
					fprintf(stdout, "C -> S: Connecting...\n");
					sendPkt(sd2, preparePkt(INST_MSG, 0, 0, "Connection Established"));
					fprintf(stdout, "S -> C: Connected!\n");
					break;
				case INST_SHUTDOWN:
					status = false;
					fprintf(stdout, "C -> S: Shutdown Server.\nS: Server is shutting down...\n");
					break;
				case INST_ADD:
					if (curIndex >= BOARD_SIZE) {
						curIndex--;
						for (int i = 0; i < BOARD_SIZE - INCREMENT; ++i) {
							strcpy(board[i], board[i + INCREMENT]);
							placeHold[i] = true;
						}
						placeHold[BOARD_SIZE - INCREMENT] = false;
					}
					if (rpacket.meta.caplen <= LINE_LIMIT) {
						strcpy(board[curIndex], (char *) rpacket.data);
						placeHold[curIndex] = true;
						fprintf(stdout, "C -> S: Add Bulletin: %s\nS: New bulletin added to position %d\n",
						        board[curIndex], curIndex);
						sendPkt(sd2, prepareMSGPkt("Add Success", curIndex));
						curIndex++;
					} else {
						fprintf(stdout, "C -> S: Bad Input\nS -> C: Error Message\n");
						sendErrorPkt(sd2, bad_input);
						fprintf(stderr, "Error: Exceed input line limit\n");
					}
					break;
				case INST_GETALL:
					fprintf(stdout, "C -> S: getall\n");
					for (int i = 0; i < BOARD_SIZE; ++i) {
						if (placeHold[i]) {
							sendPkt(sd2, prepareMSGPkt(board[i], i));
							fprintf(stdout, "S -> C: board[%d] content\n", i);
						}
					}
					sendPkt(sd2, preparePkt(INST_ENDTRANS, 0, 0, ""));
					fprintf(stdout, "S -> C: End Transmission\n");
					break;
				case INST_DELETE:
					if (curIndex == 0) {
						fprintf(stderr, "delete error\n");
						sendErrorPkt(sd2, delete_empty);
					}
					tmpIndex = rpacket.meta.optBltIndex;
					fprintf(stdout, "C -> S: delete %d\n", tmpIndex);
					if (!placeHold[tmpIndex]) {
						fprintf(stderr, "%d is empty!\n", tmpIndex);
						sendErrorPkt(sd2, delete_empty);
						fprintf(stdout, "S -> C: Error Message\n");
					} else {
						curIndex--;
						placeHold[tmpIndex] = false;
						strcpy(board[tmpIndex], "");
						for (int i = tmpIndex; i < BOARD_SIZE; ++i) {
							int nextBulletin = i + INCREMENT;
							if ((nextBulletin) < BOARD_SIZE) {
								if (placeHold[nextBulletin]) {
									strcpy(board[i], board[nextBulletin]);
									placeHold[i] = true;
								} else {
									strcpy(board[i], "");
									placeHold[i] = false;
								}
							} else {
								strcpy(board[i], "");
								placeHold[i] = false;
							}
						}
						fprintf(stdout, "Board %d deleted\nS -> C: Delete Suceess\n", tmpIndex);
						sendPkt(sd2, prepareMSGPkt("Delete Success", tmpIndex));
					}
					break;
				case INST_CLEAR:
					fprintf(stdout, "C -> S: clearall\n");
					curIndex = 0;
					for (int i = 0; i < BOARD_SIZE; ++i) {
						placeHold[i] = false;
						strcpy(board[i], "");
					}
					fprintf(stdout, "S: board cleared\n");
					sendPkt(sd2, preparePkt(INST_MSG, 0, 0, "Board Cleared"));
					break;
				case INST_UPDATE:
					tmpIndex = rpacket.meta.optBltIndex;
					fprintf(stdout, "C -> S: update %d\n", tmpIndex);
					if (!placeHold[tmpIndex]) {
						fprintf(stderr, "%d is empty!\n", tmpIndex);
						sendErrorPkt(sd2, update_empty);
						fprintf(stdout, "S -> C: Error Message\n");
					} else {
						strcpy(board[tmpIndex], (char *) rpacket.data);
						fprintf(stdout, "S: board %d updated\n", tmpIndex);
						sendPkt(sd2, prepareMSGPkt("updated", tmpIndex));
					}
					break;
				case INST_EXIT:
					fprintf(stdout, "C -> S: Client Exit\nS: Wait for next connection...\n");
					sd2 = accept(sd, &addr, &addrlen);
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
			strcpy((char *) epacket.data, "Error: Bad input, line limit exceed!");
			break;
		case delete_empty:
			epacket.meta.instruction = INST_ERROR;
			strcpy((char *) epacket.data, "Error: Try to delete an empty bulletin or empty board!");
			break;
		case update_empty:
			epacket.meta.instruction = INST_ERROR;
			strcpy((char *) epacket.data, "Error: Try to update an empty bulletin!");
			break;
		default:
			epacket.meta.instruction = INST_ERROR;
			strcpy((char *) epacket.data, "Error Unknown!");
			break;
	}
	sendPkt(sd2, epacket);
}

