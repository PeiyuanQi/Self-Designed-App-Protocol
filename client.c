/**********************************
 * Name: 			Peiyuan Qi
 * CaseNetworkID: 	pxq12
 * FileName: 		client.cpp
 * Created Data: 	20/04/2017
 * Description:
 *	implementation of client
 **********************************/

#include "client.h"

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
	int argc;

	do {
		memset(args,0x0, INPUT_LIMIT * sizeof(char *));
		printf("client $ ");
		line = read_line();
		args = split_line(args, line, &argc);
		status = execute(args,argc);
		//free the memory for next loop
		free(line);
	} while (status);

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

char **split_line(char ** args, char *line, int *argc) {
	int pos = 0;
	//one line is no more than 1280 characters
	char **tokens = args;
	char *token;

	argc = 0;

	if (!tokens) {
		fprintf(stderr, "Error: Allocation error\n");
		exit(EXIT_FAILURE);
	}
	token = strtok(line, TOK_DELIM);
	while (token != NULL) {
		tokens[pos] = token;
		pos++;
		argc++;
		if (1280 <= pos) {
			fprintf(stderr, "client: Too long input.\n");
			exit(EXIT_FAILURE);
		}
		token = strtok(NULL, TOK_DELIM);
	}
	tokens[pos] = NULL;
	return tokens;
}

bool execute(char **args, int argc) {
	if (args[0] == NULL) {
		return true; //empty line
	} else{
		if (argc >= 1){
			fprintf(stdout,"%s %d",args[0],argc);
			return false;
		}
		else
		{
			fprintf(stdout, "Usage: enter \"help\" for instructions!\n");
			return true;
		}
	}
}