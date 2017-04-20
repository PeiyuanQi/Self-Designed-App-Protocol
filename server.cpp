/**********************************
 * Name: 			Peiyuan Qi
 * CaseNetworkID: 	pxq12
 * FileName: 		server.cpp
 * Created Data: 	20/04/2017
 * Description:
 *	implementation of server
 **********************************/

#include "server.h"

int usage (char *progname)
{
	fprintf (stderr,"Usage: %s port\n", progname);
	exit (EXIT_FAILURE);
}

int errexit (char *format, char *arg)
{
	fprintf (stderr,format,arg);
	fprintf (stderr,"\n");
	exit (EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	//get cmd options
	char parse_rule[] = "r:pst";
	GetOpt aGetOpt(argc, argv, parse_rule);
	Config aConfig = aGetOpt.parse();
	//this is main for shell, it will stay in the loop until exit.
	loop();
	return EXIT_SUCCESS;
}

/*
int main(int argc, char **argv) {
	//get cmd options
	char parse_rule[] = "r:pst";
	GetOpt aGetOpt(argc, argv, parse_rule);
	Config aConfig = aGetOpt.parse();

	switch (aConfig.getMode()) {
		case packet_dump:
			Packet_Dump(aConfig.getTraceFile());
			break;
		case connection_summary:
			Connection_Summary(aConfig.getTraceFile());
			break;
		case round_trip_time:
			Round_Trip_Time(aConfig.getTraceFile());
			break;
		case file_error:
			cerr << "Usage: [-r] filename is required!" << endl;
			exit(EXIT_FAILURE);
		case unknown:
			cerr << "Usage: [-r] filename [-pst] required!" << endl;
			exit(EXIT_FAILURE);
		case mode_error:
			cerr << "Only one mode can run each time!" << endl;
			exit(EXIT_FAILURE);
	}

	return 0;
}
 */