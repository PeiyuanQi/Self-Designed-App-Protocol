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
	fprintf(stderr, "Usage: %s port\n", progname);
	exit(EXIT_FAILURE);
}

int errexit(char *format, char *arg) {
	fprintf(stderr, format, arg);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

void loop(int sd2);

int main(int argc, char **argv) {
	struct sockaddr_in sin;
	struct sockaddr addr;
	struct protoent *protoinfo;
	unsigned int addrlen;
	int sd, sd2;

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
	sd = socket(PF_INET, SOCK_STREAM, protoinfo->p_proto);    //  would be SOCK_DGRAM for UDP
	if (sd < 0)
		errexit("cannot create socket", NULL);

	/* bind the socket */
	if (bind(sd, (struct sockaddr *) &sin, sizeof(sin)) < 0)
		errexit("cannot bind to port %s", argv[PORT_POS]);

	/* listen for incoming connections */
	if (listen(sd, QUE_LIMIT) < 0)
		errexit("cannot listen on port %s\n", argv[PORT_POS]);

	/* accept a connection */
	sd2 = accept(sd, &addr, &addrlen);
	if (sd2 < 0)
		errexit("error accepting connection", NULL);

	//start working
	loop(sd2);

	/* write message to the connection
	if (write (sd2,argv [MSG_POS],strlen (argv [MSG_POS])) < 0)
		errexit ("error writing message: %s", argv [MSG_POS]);
*/

	return EXIT_SUCCESS;
}

void loop(int sd2) {
	int status = 1;
	int ret;
	pktblt rpacket;
	std::string board[BOARD_SIZE];

	//initialize board
	for (int i = 0; i < 10; ++i) {
		board[i] = "";
	}

	//do the loop
	do {
		memset(&rpacket, 0x0, sizeof(rpacket));
		ret = read(sd2, &rpacket, sizeof(rpacket));
		if (ret < 0)
			errexit("reading error", NULL);
		fprintf(stdout, "%s\n",rpacket.data);
	} while (status);
}
