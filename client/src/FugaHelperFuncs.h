#ifndef FUGAHELPER_FUNCS_H
#define FUGAHELPER_FUNCS_H

#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <QtGlobal>

std::vector<std::string> split(std::string s, std::string delim);
std::string replace(std::string str, std::string in, std::string out);
std::string merge(std::string array1, std::string array2);

std::string int2string(int i);
int string2int(const std::string s);
quint16 string2quint16(std::string s);
int char2int(char* s);
void showError(std::string in_msg);


#endif // FUGAHELPER_FUNCS_H
