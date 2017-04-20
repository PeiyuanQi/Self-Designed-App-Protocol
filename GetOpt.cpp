/**********************************
 * Name: 			Peiyuan Qi
 * CaseNetworkID: 	pxq12
 * FileName: 		GetOpt.cpp
 * Created Data: 	20/04/2017
 * Description:
 *	implementation of class GetOpt
 * Refer: gnu.org/software/libc/manual/html_node/Getopt.html
 **********************************/

#include "GetOpt.h"

GetOpt::GetOpt(int count, char *value[], char *rule) {
	arg_count = count;
	arg_value = value;
	arg_rule = rule;
}

Config GetOpt::parse() {
	int opt;
	bool isSetR = false;

	Config cmd_config = Config();

	while ((opt = getopt(arg_count, arg_value, arg_rule)) != -1) {
		switch (opt) {
			case 'p':
				if (cmd_config.getMode() == unknown)
					cmd_config.setMode(packet_dump);
				else
					cmd_config.setMode(mode_error);
				break;
			case 's':
				if (cmd_config.getMode() == unknown)
					cmd_config.setMode(connection_summary);
				else
					cmd_config.setMode(mode_error);
				break;
			case 't':
				if (cmd_config.getMode() == unknown)
					cmd_config.setMode(round_trip_time);
				else
					cmd_config.setMode(mode_error);
				break;
			case 'r':
				cmd_config.setTraceFile(optarg);
				isSetR = true;
				break;
			default: /* '?' */
				fprintf(stderr, "Usage: %s [-p -s -t] [-r] trace-file-name\n",
				        arg_value[0]);
				exit(EXIT_FAILURE);
		}
	}

	if (!isSetR)
		cmd_config.setMode(file_error);

	/* Code to check GetOpt work well
	using namespace std;
	std::string astr = cmd_config.getTraceFile();
	std::cout << "cmd = " << cmd_config.getMode() << "; file = " << astr.c_str() << "; optind = " << optind
	          << std::endl;
	*/

	if (optind > arg_count) {
		fprintf(stderr, "Expected argument after options\n");
		exit(EXIT_FAILURE);
	}
	if (optind < arg_count) {
		fprintf(stderr, "non-option ARGV-elements: ");
		while (optind < arg_count) {
			fprintf(stderr, "%s\n", arg_value[optind++]);
		}
		exit(EXIT_FAILURE);
	}

	return cmd_config;
}


