#ifndef MYCONFIG_H
#define MYCONFIG_H

#include <QObject>
#include <map>
#include <string>
#include <vector>
#include <QMutex>

class MyConfig : public QObject {

	public:
		MyConfig();
		void setConfig(std::string name, std::string value);
		std::string getConfig(std::string name);
		int getInt(std::string name);
		bool getBool(std::string name);

protected:
		std::string m_path;
		void _readConfig();
		void _writeConfig();
		std::vector<std::string> split(std::string s, std::string delim);
		std::string replace(std::string str, std::string in, std::string out);
		std::map<std::string,std::string> m_config;
		QMutex* m_mutex;

};

#endif // MYCONFIG_H
