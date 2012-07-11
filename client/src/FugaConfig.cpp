#include "FugaConfig.h"

using namespace std;

/* constructor
 */
FugaConfig::FugaConfig () {
	m_mutex = new QMutex();
	m_path = ".config";

    // read config from file
	_readConfig();

    // make sure, config-file exists
	_writeConfig();
}

/* set setting
 */
void FugaConfig::setConfig(string name, string value) {
	m_mutex->lock();

    // replace all : and ;  in name and value by _
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
string FugaConfig::getConfig(string name) {
	m_mutex->lock();

    // name exists?
	if (m_config.find(name) == m_config.end()) {
		m_mutex->unlock();
		return "";
	}

	string output = m_config[name];
	m_mutex->unlock();
	return output;
}

/* read settings from file
 */
void FugaConfig::_readConfig() {
    cout << "FugaConfig: reading config..." << endl;
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
        cout << "FugaConfig: " << parts[0] << " = " << parts[1] << endl;
	}

	// close file
	myfile.close();

	// set defaults
	if (m_config.find("udp_ip") == m_config.end()) m_config["udp_ip"] = "127.0.0.1";
    if (m_config.find("udp_firstport") == m_config.end()) m_config["udp_firstport"] = "7878";
	if (m_config.find("tcp_ip") == m_config.end()) m_config["tcp_ip"] = "127.0.0.1";
    if (m_config.find("tcp_port") == m_config.end()) m_config["tcp_port"] = "7979";
	if (m_config.find("root_ip") == m_config.end()) m_config["root_ip"] = "127.0.0.1";
	if (m_config.find("root_port") == m_config.end()) m_config["root_port"] = "7777";
    //if (m_config.find("video_path") == m_config.end()) m_config["video_path"] = "test.mp4";
}

/* write settings to file
 */
void FugaConfig::_writeConfig() {
    cout << "FugaConfig: writing config..." << endl;
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

/* get config-value as int
 * @param string: name of config
 */
int FugaConfig::getInt (string name) {
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
bool FugaConfig::getBool (string name) {
	if (getInt(name)) return true;
	return false;
}
