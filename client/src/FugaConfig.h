#ifndef FUGACONFIG_H
#define FUGACONFIG_H

#include <QObject>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <QMutex>
#include "FugaHelperFuncs.h"

class FugaConfig : public QObject {

    public:
        FugaConfig();
		void setConfig(std::string name, std::string value);
		std::string getConfig(std::string name);
		int getInt(std::string name);
		bool getBool(std::string name);

    protected:
		void _readConfig();
		void _writeConfig();

        std::string m_path;
		std::map<std::string,std::string> m_config;
		QMutex* m_mutex;

};

#endif // FUGACONFIG_H
