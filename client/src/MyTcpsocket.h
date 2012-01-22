#ifndef __MYTCPSOCKET_H__
#define __MYTCPSOCKET_H__

#include "Supervisor.h"
#include <QThread>
#include <QTcpSocket>
#include <QTcpServer>
#include <string>
#include <vector>
#include <map>
#include <QSignalMapper>
#include <QSslSocket>

class Supervisor;

class MyTcpsocket : public QObject {
	Q_OBJECT

	public:
		MyTcpsocket(Supervisor* m_supervisor);
		void send_requestinfos(std::string name);
		void send_name2tcp(std::string name);
		void sendTo(std::string name, std::string msg);
		void add_usr(std::string name, std::string msg);

		// connection handling
		void setSocket(std::string name, QTcpSocket* mysocket);
		QTcpSocket* getSocket(std::string name);

		// sending messages
		void doLogin(std::string name, std::string password);

	protected:
		Supervisor* m_supervisor;
		QTcpServer* m_server;
		std::string m_rootname;
		QSignalMapper* m_signalmapper;
		QSignalMapper* m_signalmapper_connect;

		// misc
		int string2int(const std::string s);
		int char2int(char* s);
		std::string int2string(int i);
		std::string merge(std::string array1, std::string array2);
		std::vector<std::string> split(std::string s, std::string delim);
		void send_buffer(std::string name);

	signals:
		void newMsg(std::string type, std::string fulltype, std::string msg);

	public slots:
		void addPendingContact();
		void getAnswer(QString name);
		void slot_connected(QString name);
		void handleError(QAbstractSocket::SocketError);

};

#endif // __MYTCPSOCKET_H__
