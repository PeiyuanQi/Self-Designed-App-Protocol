/**********************************
 * Name: 			Peiyuan Qi
 * CaseNetworkID: 	pxq12
 * FileName: 		server_multi.cpp
 * Created Data: 	25/04/2017
 * Description:
 *	implementation of server which accept multi client
 *	for establishing multi connection coding, I refer to :
 *	http://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
 **********************************/
#include "bulletin.h"
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <errno.h>

#define REQUIRED_ARGC 2
#define PORT_POS 1
#define QUE_LIMIT 1
#define PROTOCOL "tcp"
#define BUFLEN 1024
#define BOARD_SIZE 10
#define INCREMENT 1
#define MAX_CLIENTS 30
#define TRUE 1

void loop(int master_socket, int addrlen, struct sockaddr_in address);

void sendErrorPkt(int sd2, enum error_code errorCode);

int usage(char *progname) {
	fprintf(stderr, "usage: %s port\n", progname);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
	int opt = TRUE;
	int master_socket, addrlen;

	struct sockaddr_in address;
	struct protoent *protoinfo;

	//get cmd options
	if (argc != REQUIRED_ARGC)
		usage(argv[0]);

	/* determine protocol */
	if ((protoinfo = getprotobyname(PROTOCOL)) == NULL)
		errexit("cannot find protocol information for %s", PROTOCOL);

	//create a master socket
	if ((master_socket = socket(PF_INET, SOCK_STREAM, protoinfo->p_proto)) == 0) {
		errexit("Master-socket failed", NULL);
	}

	//set master socket to allow multiple connections ,
	if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0) {
		errexit("Set Socket OPT failed", NULL);
	}

	/* setup endpoint info */
	memset((char *) &address, 0x0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons ((u_short) atoi(argv[PORT_POS]));

	//bind the socket to localhost port 8888
	if (bind(master_socket, (struct sockaddr *) &address, sizeof(address)) < 0) {
		errexit("Bind Failed", NULL);
	}
	printf("Listen on port %d \n", atoi(argv[PORT_POS]));

	//try to specify maximum of 3 pending connections for the master socket
	if (listen(master_socket, 3) < 0) {
		errexit("Listen Failed", NULL);
	}

	//accept the incoming connection
	addrlen = sizeof(address);
	fprintf(stdout, "Waiting for connections ...\n");

	//start working
	loop(master_socket, addrlen, address);

	return EXIT_SUCCESS;
}

void loop(int master_socket, int addrlen, struct sockaddr_in address) {
	int curIndex = 0;
	int ret, tmpIndex, max_sd, sd, client_socket[MAX_CLIENTS], activity, new_socket;
	pktblt rpacket;
	char board[BOARD_SIZE][LINE_LIMIT];
	bool placeHold[BOARD_SIZE], status = true;

	//set of socket descriptors
	fd_set readfds;

	//initialise all client_socket[] to 0 so not checked
	for (int i = 0; i < MAX_CLIENTS; i++) {
		client_socket[i] = 0;
	}

	//initialize board
	for (int i = 0; i < BOARD_SIZE; ++i) {
		strcpy(board[i], "");
		placeHold[i] = false;
	}

	//do the loop
	do {

		//clear the socket set
		FD_ZERO(&readfds);

		//add master socket to set
		FD_SET(master_socket, &readfds);
		max_sd = master_socket;

		//add child sockets to set
		for (int i = 0; i < MAX_CLIENTS; i++) {
			//socket descriptor
			sd = client_socket[i];
			//if valid socket descriptor then add to monitor
			if (sd > 0)
				FD_SET(sd, &readfds);
			//highest file descriptor number
			if (sd > max_sd)
				max_sd = sd;
		}

		//wait for an activity on one of the sockets , timeout is NULL
		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
		if ((activity < 0) && (errno != EINTR)) {
			errexit("Select Failed", NULL);
		}

		//for a new income connection
		if (FD_ISSET(master_socket, &readfds)) {
			if ((new_socket = accept(master_socket, (struct sockaddr *) &address, (socklen_t *) &addrlen)) < 0) {
				errexit("Accept Failed", NULL);
			}

			//New Connection
			fprintf(stdout, "New Connection , SD: %d , IP: %s , Port: %d \n", new_socket,
			        inet_ntoa(address.sin_addr), ntohs(address.sin_port));
			fprintf(stdout, "C -> S: Connecting...\n");
			sendPkt(new_socket, preparePkt(INST_MSG, 0, 0, "Connection Established"));
			fprintf(stdout, "S -> C: Connected!\n");

			//add new socket to array of sockets
			for (int i = 0; i < MAX_CLIENTS; i++) {
				//if position is empty
				if (client_socket[i] == 0) {
					client_socket[i] = new_socket;
					fprintf(stdout, "Adding to list of sockets as %d\n", i);
					break;
				}
			}
		}

		//else its some IO operation on some other socket
		for (int i = 0; i < MAX_CLIENTS; i++) {
			if (!status) break;

			sd = client_socket[i];

			if (FD_ISSET(sd, &readfds)) {
				//one of sockets is active
				memset(&rpacket, 0x0, sizeof(rpacket));
				ret = read(sd, &rpacket, sizeof(rpacket));
				if (ret < 0) {
					errexit("reading error", NULL);
				}
				if (rpacket.meta.instruction > 0) {
					switch (rpacket.meta.instruction) {
						case INST_CONNECT:
							fprintf(stdout, "C -> S: Connecting...\n");
							sendPkt(sd, preparePkt(INST_MSG, 0, 0, "Connection Established"));
							fprintf(stdout, "S -> C: Connected!\n");
							break;
						case INST_SHUTDOWN:
							status = false;
							fprintf(stdout, "C -> S: Shutdown Server.\nS: Server is shutting down...\n");
							for (int k = 0; k < MAX_CLIENTS; ++k) {
								if (client_socket[k] != 0) close(client_socket[k]);
							}
							break;
						case INST_ADD:
							if (curIndex >= BOARD_SIZE) {
								curIndex--;
								for (int j = 0; j < BOARD_SIZE - INCREMENT; ++j) {
									strcpy(board[j], board[j + INCREMENT]);
									placeHold[j] = true;
								}
								placeHold[BOARD_SIZE - INCREMENT] = false;
							}
							if (rpacket.meta.caplen <= LINE_LIMIT) {
								strcpy(board[curIndex], (char *) rpacket.data);
								placeHold[curIndex] = true;
								fprintf(stdout, "C -> S: Add Bulletin: %s\nS: New bulletin added to position %d\n",
								        board[curIndex], curIndex);
								sendPkt(sd, prepareMSGPkt("Add Success", curIndex));
								curIndex++;
							} else {
								fprintf(stdout, "C -> S: Bad Input\nS -> C: Error Message\n");
								sendErrorPkt(sd, bad_input);
								fprintf(stderr, "Error: Exceed input line limit\n");
							}
							break;
						case INST_GETALL:
							fprintf(stdout, "C -> S: getall\n");
							for (int j = 0; j < BOARD_SIZE; ++j) {
								if (placeHold[j]) {
									sendPkt(sd, prepareMSGPkt(board[j], j));
									fprintf(stdout, "S -> C: board[%d] content\n", j);
								}
							}
							sendPkt(sd, preparePkt(INST_ENDTRANS, 0, 0, ""));
							fprintf(stdout, "S -> C: End Transmission\n");
							break;
						case INST_DELETE:
							if (curIndex == 0) {
								fprintf(stderr, "delete error\n");
								sendErrorPkt(sd, delete_empty);
							} else {
								curIndex--;
							}
							tmpIndex = rpacket.meta.optBltIndex;
							fprintf(stdout, "C -> S: delete %d\n", tmpIndex);
							if (!placeHold[tmpIndex]) {
								fprintf(stderr, "%d is empty!\n", tmpIndex);
								sendErrorPkt(sd, delete_empty);
								fprintf(stdout, "S -> C: Error Message\n");
							} else {
								placeHold[tmpIndex] = false;
								strcpy(board[tmpIndex], "");
								for (int j = tmpIndex; j < BOARD_SIZE; ++j) {
									int nextBulletin = j + INCREMENT;
									if ((nextBulletin) < BOARD_SIZE) {
										if (placeHold[nextBulletin]) {
											strcpy(board[j], board[nextBulletin]);
											placeHold[j] = true;
										} else {
											strcpy(board[j], "");
											placeHold[j] = false;
										}
									} else {
										strcpy(board[j], "");
										placeHold[j] = false;
									}
								}
								fprintf(stdout, "Board %d deleted\nS -> C: Delete Suceess\n", tmpIndex);
								sendPkt(sd, prepareMSGPkt("Delete Success", tmpIndex));
							}
							break;
						case INST_CLEAR:
							fprintf(stdout, "C -> S: clearall\n");
							curIndex = 0;
							for (int j = 0; j < BOARD_SIZE; ++j) {
								placeHold[j] = false;
								strcpy(board[j], "");
							}
							fprintf(stdout, "S: board cleared\n");
							sendPkt(sd, preparePkt(INST_MSG, 0, 0, "Board Cleared"));
							break;
						case INST_UPDATE:
							tmpIndex = rpacket.meta.optBltIndex;
							fprintf(stdout, "C -> S: update %d\n", tmpIndex);
							if (!placeHold[tmpIndex]) {
								fprintf(stderr, "%d is empty!\n", tmpIndex);
								sendErrorPkt(sd, update_empty);
								fprintf(stdout, "S -> C: Error Message\n");
							} else {
								strcpy(board[tmpIndex], (char *) rpacket.data);
								fprintf(stdout, "S: board %d updated\n", tmpIndex);
								sendPkt(sd, prepareMSGPkt("updated", tmpIndex));
							}
							break;
						case INST_EXIT:
							fprintf(stdout, "C -> S: Client Exit\nS: Wait for other connection...\n");
							//Close the socket and mark as 0 in list for reuse
							close(sd);
							client_socket[i] = 0;
							break;
						default:
							break;
					}
				}
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

