/**********************************
 * Name: 			Peiyuan Qi
 * CaseNetworkID: 	pxq12
 * FileName: 		Config.h
 * Created Data: 	20/04/2017
 * Description:
 *	implementation of class Config
 **********************************/

#ifndef P2_CONFIG_H
#define P2_CONFIG_H

#include <string>

enum Mode {
	unknown, packet_dump, connection_summary, round_trip_time, mode_error, file_error
};

class Config {
public:
	Config();

	void setMode(Mode set);

	void setTraceFile(std::string f_name);

	Mode getMode();

	std::string getTraceFile();

private:
	Mode mode;
	std::string tracefile_name;
};


#endif //P2_CONFIG_H
