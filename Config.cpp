/**********************************
 * Name: 			Peiyuan Qi
 * CaseNetworkID: 	pxq12
 * FileName: 		Config.cpp
 * Created Data: 	20/04/2017
 * Description:
 *	implementation of class Config
 **********************************/

#include "Config.h"

Config::Config() {
	mode = unknown;
	tracefile_name = "";
}

void Config::setMode(Mode set) {
	mode = set;
}

void Config::setTraceFile(std::string f_name) {
	tracefile_name = f_name;
}

Mode Config::getMode() {
	return mode;
}

std::string Config::getTraceFile() {
	return tracefile_name;
}