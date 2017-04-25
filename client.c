/**********************************
 * Name: 			Peiyuan Qi
 * CaseNetworkID: 	pxq12
 * FileName: 		client.cpp
 * Created Data: 	20/04/2017
 * Description:
 *	implementation of client
 **********************************/

#include "client.h"

bool checkFD(int sd, pktblt *tmpPkt);

int usage(char *progname) {
	fprintf(stderr, "usage: %s host port\n", progname);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
	struct sockaddr_in sin;
	struct hostent *hinfo;
	struct protoent *protoinfo;
	int sd;
	pktblt spacket;

	if (argc != REQUIRED_ARGC)
		usage(argv[0]);

	/* lookup the hostname */
	hinfo = gethostbyname(argv[HOST_POS]);
	if (hinfo == NULL)
		errexit("cannot find name: %s", argv[HOST_POS]);

	/* set endpoint information */
	memset((char *) &sin, 0x0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons (atoi(argv[PORT_POS]));
	memcpy((char *) &sin.sin_addr, hinfo->h_addr, hinfo->h_length);

	if ((protoinfo = getprotobyname(PROTOCOL)) == NULL)
		errexit("cannot find protocol information for %s", PROTOCOL);

	/* allocate a socket */
	sd = socket(PF_INET, SOCK_STREAM, protoinfo->p_proto);
	if (sd < 0)
		errexit("cannot create socket", NULL);

	/* connect the socket */
	fprintf(stdout, "Connecting...\n");
	if (connect(sd, (struct sockaddr *) &sin, sizeof(sin)) < 0)
		errexit("cannot connect", NULL);

	clientLoop(sd);

	if (write(sd, &spacket, sizeof(spacket)) < 0)
		errexit("error writing message: %s", (char *) spacket.data);

	/* close & exit */
	close(sd);
	exit(0);
}

void clientLoop(int sd) {
	bool status = true;
	char *line;
	char **args = malloc(INPUT_LIMIT * sizeof(char *));
	int *argc = malloc(sizeof(int));
	pktblt tmpopkt;
	int rec;

	//build connection
	sendPkt(sd, preparePkt(INST_CONNECT, 0, 0, ""));
	memset(&tmpopkt, 0x0, sizeof(tmpopkt));
	rec = read(sd, &tmpopkt, sizeof(tmpopkt));
	if (rec < 0) {
		errexit("reading error", NULL);
	}
	if (tmpopkt.meta.instruction == INST_MSG) {
		fprintf(stdout, "%s\n", (char *) tmpopkt.data);
	} else {
		fprintf(stdout, "Connection time out\n");
		return;
	}

	//begin loop
	do {
		memset(args, 0x0, INPUT_LIMIT * sizeof(char *));
		printf("client $ ");
		line = read_line();
		args = split_line(args, line, argc);
		status = execute(sd, args, argc);
		//free the memory for next loop
		free(line);
	} while (status);

	free(argc);
	free(args);
}

char *read_line(void) {
	//refer: Tutorial - Write a Shell in C / Writing Your Own Shell
	char *line = NULL;
	size_t linesize = 0; // have getline allocate a buffer for us
	if (getline(&line, &linesize, stdin) < 0) {
		fprintf(stderr, "Error Input\n");
		exit(EXIT_FAILURE);
	}
	return line;
}

char **split_line(char **args, char *line, int *argc) {
	int pos = 0;
	//one line is no more than 1280 characters
	char **tokens = args;
	char *token;

	(*argc) = 0;

	if (!tokens) {
		fprintf(stderr, "Error: Allocation error\n");
		exit(EXIT_FAILURE);
	}
	token = strtok(line, TOK_DELIM);
	while (token != NULL) {
		tokens[pos] = token;
		pos++;
		(*argc)++;
		if (INPUT_LIMIT <= pos) {
			fprintf(stderr, "client: Too long input.\n");
			exit(EXIT_FAILURE);
		}
		token = strtok(NULL, TOK_DELIM);
	}
	tokens[pos] = NULL;
	return tokens;
}

bool checkFD(int sd, pktblt *tmpPkt) {
	int ret;
	memset(tmpPkt, 0x0, sizeof((*tmpPkt)));
	ret = read(sd, tmpPkt, sizeof((*tmpPkt)));
	if (ret < 0) {
		errexit("Reading Error", NULL);
	}
	if ((*tmpPkt).meta.instruction == INST_MSG) {
		fprintf(stdout, "%s\n", (char *) (*tmpPkt).data);
		return true;
	}
	if ((*tmpPkt).meta.instruction == INST_ERROR) {
		fprintf(stderr, "%s\n", (char *) (*tmpPkt).data);
		return true;
	} else {
		fprintf(stderr, "Lost Connection\n");
		return false;
	}
}

bool execute(int sd, char **args, int *argc) {
	pktblt tmpPkt;
	int ret;

	if (args[0] == NULL) {
		return true; //empty line
	} else {
		if ((*argc) >= SING_ARG_LIMT) {
			if (strcmp(args[0], "exit") == 0) {
				//if it is an exit command
				if (*argc > 1) {
					fprintf(stdout, "Usage: exit\n");
					return true;
				}
				sendPkt(sd, preparePkt(INST_EXIT, 0, 0, ""));
				fprintf(stdout, "exiting...\n");
				return false;
			} else if (strcmp(args[0], "shutdown") == 0) {
				//if it is a shutdown command
				if (*argc > SING_ARG_LIMT) {
					fprintf(stdout, "Usage: shutdown\n");
					return true;
				}
				fprintf(stdout, "shutting down server...\n");
				sendPkt(sd, preparePkt(INST_SHUTDOWN, 0, 0, ""));
				fprintf(stdout, "exiting...\n");
				return false;
			} else if (strcmp(args[0], "add") == 0) {
				//if it is an add command
				char tmpMsg[LINE_LIMIT];
				memset(tmpMsg, 0x0, LINE_LIMIT);
				if (*argc > SING_ARG_LIMT) {
					for (int i = INDEX_POS; i < ((*argc) - 1); ++i) {
						if (strlen(tmpMsg) + strlen(args[i]) + 1 < LINE_LIMIT) {
							strcat(tmpMsg, args[i]);
							strcat(tmpMsg, " ");
						} else {
							fprintf(stderr, "Input too long\n");
							return true;
						}
					}
				} else {
					fprintf(stdout, "Usage: add [message]\n");
					return true;
				}
				if (strlen(tmpMsg) + strlen(args[(*argc) - BESIDE_DIS]) < LINE_LIMIT) {
					strcat(tmpMsg, args[(*argc) - BESIDE_DIS]);
				} else {
					fprintf(stderr, "Input too long\n");
					return true;
				}
				sendPkt(sd, preparePkt(INST_ADD, 0, 0, tmpMsg));
				return checkFD(sd, &tmpPkt);
			} else if (strcmp(args[0], "getall") == 0) {
				//if it getall command
				if (*argc > SING_ARG_LIMT) {
					fprintf(stdout, "Usage: getall\n");
					return true;
				}
				sendPkt(sd, preparePkt(INST_GETALL, 0, 0, ""));
				memset(&tmpPkt, 0x0, sizeof(tmpPkt));
				ret = read(sd, &tmpPkt, sizeof(tmpPkt));
				if (ret < 0) {
					errexit("reading error", NULL);
				}
				while (tmpPkt.meta.instruction != INST_ENDTRANS) {
					if (tmpPkt.meta.instruction == INST_MSG) fprintf(stdout, "%s\n", (char *) tmpPkt.data);
					memset(&tmpPkt, 0x0, sizeof(tmpPkt));
					ret = read(sd, &tmpPkt, sizeof(tmpPkt));
					if (ret < 0) {
						errexit("Reading error, might lost connection", NULL);
					}
				}
				return true;
			} else if (strcmp(args[0], "delete") == 0) {
				//if it is delete command
				if (*argc != DELETE_ARGC) {
					fprintf(stdout, "Usage: delete [index]\n");
					return true;
				}
				if (strlen(args[INDEX_POS]) == INDEX_DIGIT_NO && args[INDEX_POS][DIGIT_POS] >= '0' &&
				    args[INDEX_POS][DIGIT_POS] <= '9') {
					sendPkt(sd, preparePkt(INST_DELETE, atoi(args[INDEX_POS]), 0, ""));
					return checkFD(sd, &tmpPkt);
				} else {
					fprintf(stdout, "Usage: delete [index]\n");
					return true;
				}
			} else if (strcmp(args[0], "clearall") == 0) {
				//if it is clearall command
				if (*argc > SING_ARG_LIMT) {
					fprintf(stdout, "Usage: clearall\n");
					return true;
				} else {
					sendPkt(sd, preparePkt(INST_CLEAR, 0, 0, ""));
					return checkFD(sd, &tmpPkt);
				}
			} else if (strcmp(args[0], "update") == 0) {
				//if it is a update command
				char tmpMsg[LINE_LIMIT];
				int tmpIndex = 0;
				if (*argc < UPDATE_ARGC) {
					fprintf(stdout, "Usage: update [index] [mesaage]\n");
					return true;
				}
				if (strlen(args[INDEX_POS]) != INDEX_DIGIT_NO || args[INDEX_POS][DIGIT_POS] < '0' ||
				    args[INDEX_POS][DIGIT_POS] > '9') {
					fprintf(stdout, "Usage: update [index] [mesaage]\n");
					return true;
				}
				tmpIndex = atoi(args[INDEX_POS]);
				//form msg string
				memset(tmpMsg, 0x0, LINE_LIMIT);
				for (int i = UPDATE_MSG_POS; i < ((*argc) - 1); ++i) {
					if (strlen(tmpMsg) + strlen(args[i]) + 1 < LINE_LIMIT) {
						strcat(tmpMsg, args[i]);
						strcat(tmpMsg, " ");
					} else {
						fprintf(stderr, "Input too long\n");
						return true;
					}
				}
				//add last tolken to string
				if (strlen(tmpMsg) + strlen(args[(*argc) - BESIDE_DIS]) < LINE_LIMIT) {
					strcat(tmpMsg, args[(*argc) - BESIDE_DIS]);
				} else {
					fprintf(stderr, "Input too long\n");
					return true;
				}
				//send packet
				sendPkt(sd, preparePkt(INST_UPDATE, tmpIndex, 0, tmpMsg));
				//get feed back
				return checkFD(sd, &tmpPkt);
			} else if (strcmp(args[0], "help") == 0) {
				fprintf(stdout,
				        "help:\n\texit\n\tshutdown\n\tadd [message]\n\tgetall\n\tdelete [index]\n\tclearall\n\tupdate [index] [message]\n");
				return true;
			} else {
				fprintf(stdout, "Usage: enter \"help\" for instructions!\n");
				return true;
			}
		} else {
			fprintf(stdout, "Usage: enter \"help\" for instructions!\n");
			return true;
		}
	}
	return true;
}