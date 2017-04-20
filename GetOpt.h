/**********************************
 * Name: 			Peiyuan Qi
 * CaseNetworkID: 	pxq12
 * FileName: 		GetOpt.h
 * Created Data: 	20/04/2017
 * Description:
 *	class to parse input command, has method parse() return result
 * Refer: gnu.org/software/libc/manual/html_node/Getopt.html
 **********************************/

#ifndef P2_GETOPT_H
#define P2_GETOPT_H

#include <iostream>
#include <getopt.h>

#include "Config.h"

class GetOpt {
public:
	GetOpt(int count, char *value[], char *rule);

	Config parse();

private:
	char **arg_value;
	int arg_count;
	char *arg_rule;
};


#endif //P2_GETOPT_H
