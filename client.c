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

int errexit(char *format, char *arg) {
	fprintf(stderr, format, arg);
	fprintf(stderr, "\n");
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

	memcpy(spacket.data, "hello", 6);

	if (write(sd, &spacket, sizeof(spacket)) < 0)
		errexit("error writing message: %s", (char *) spacket.data);

	/* close & exit */
	close(sd);
	exit(0);
}