#include "MyConfig.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>

using namespace std;

/* constructor
 */
MyConfig::MyConfig () {
	m_mutex = new QMutex();
	m_path = ".config";

	// read settings
	_readConfig();

	// make sure, config-file exists
	_writeConfig();
}

/* set setting
 */
void MyConfig::setConfig(string name, string value) {
	m_mutex->lock();

	// replace all : and ; by _
	name = replace(name, ":", "_");
	name = replace(name, ";", "_");
	value = replace(value, ":", "_");
	value = replace(value, ";", "_");

	m_config[name] = value;
	_writeConfig();
	m_mutex->unlock();
}

/* get setting
 */
string MyConfig::getConfig(string name) {
	m_mutex->lock();
	if (m_config.find(name) == m_config.end()) {
		m_mutex->unlock();
		return "";
	}

	string output = m_config[name];
	m_mutex->unlock();
	return output;
}

// read/write
/* read settings from file (if available)
 */
void MyConfig::_readConfig() {
	cout << "MyConfig: reading config..." << endl;
	ifstream myfile;
	myfile.open(m_path.c_str());

	// read
	stringstream ss("");
	string buffer;
	while (!myfile.eof() && myfile.good()) {
		getline(myfile, buffer);
		ss << buffer;
	}
	string s = ss.str();

	// split into configs
	s = replace(s, "\n", "");
	s = replace(s, "\r", "");

	vector<string> s_split = split(s, ";");
	for (vector<string>::iterator i = s_split.begin(); i != s_split.end(); ++i) {

		// split type and data
		vector<string> parts = split(*i, ":");
		if (parts.size() != 2) continue;

		// save in obj-vars
		m_config[parts[0]] = parts[1];
	}

	// close file
	myfile.close();

	// set defaults
	if (m_config.find("udp_ip") == m_config.end()) m_config["udp_ip"] = "127.0.0.1";
	if (m_config.find("udp_port") == m_config.end()) m_config["udp_port"] = "7878";
	if (m_config.find("udp_mtu") == m_config.end()) m_config["udp_mtu"] = "500";
	if (m_config.find("udp_quality") == m_config.end()) m_config["udp_quality"] = "80";
	if (m_config.find("tcp_ip") == m_config.end()) m_config["tcp_ip"] = "127.0.0.1";
	if (m_config.find("tcp_port") == m_config.end()) m_config["tcp_port"] = "7979";
	if (m_config.find("img_width") == m_config.end()) m_config["img_width"] = "320";
	if (m_config.find("img_height") == m_config.end()) m_config["img_height"] = "240";
	if (m_config.find("root_ip") == m_config.end()) m_config["root_ip"] = "127.0.0.1";
	if (m_config.find("root_port") == m_config.end()) m_config["root_port"] = "7777";
}

/* write settings to file
 */
void MyConfig::_writeConfig() {
	cout << "MyConfig: writing config..." << endl;
	ofstream myfile;
	myfile.open (m_path.c_str());

	// sum config to string
	stringstream ss("");
	map<string, string>::iterator curr,end;
	for (curr = m_config.begin(), end = m_config.end();  curr != end;  curr++) {
		ss << curr->first << ":";
		ss << m_config[curr->first] << ";\n";
	}

	// write to file
	myfile << ss.str();

	// close file
	myfile.close();
}

/* split string by "delim"
 * @param string: string to split
 * @param char: delimiter
 */
vector<string> MyConfig::split(string s, string delim) {
	vector<string> elems;
	stringstream ss("");
	ss << s;
	string item;
	char delimiter = *delim.data();
	while(std::getline(ss, item, delimiter)) {
		elems.push_back(item);
	}
	return elems;
}

/* replace strings in string
 */
string MyConfig::replace(string str, string in, string out) {

	// find first
	unsigned int position = str.find(in);

	// replace all
	while ( position != string::npos ) {
		str.replace(position, 1, out);
		position = str.find(in, (position + 1));
	}

	return str;
}

/* get config-value as int
 * @param string: name of config
 */
int MyConfig::getInt (string name) {
	stringstream ss(getConfig(name));
	int i;
	if( (ss >> i).fail() ) {
		// conversion failed
		return 0;
	}
	return i;
}

/* get config-value as bool
 * @param string: name of config
 */
bool MyConfig::getBool (string name) {
	if (getInt(name)) return true;
	return false;
}
