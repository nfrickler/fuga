#ifndef __FUGACONTACTS_H__
#define __FUGACONTACTS_H__

#include "Fuga.h"
#include "FugaContact.h"
#include <QMutex>
#include <map>
#include <iostream>
#include <sstream>
#include <QSignalMapper>
#include <QTcpServer>

class Fuga;
class FugaContact;

class FugaContacts : public QObject {
	Q_OBJECT

	public:
        FugaContacts(Fuga* in_Fuga);
        FugaContact* getContact(std::string in_name);
        bool isContact(std::string name);

        void name2tcp(std::string in_name);
        void handleError(QAbstractSocket::SocketError in_error);


	protected:
        Fuga* m_Fuga;
        QTcpServer* m_Server;
        QSignalMapper* m_signalmapper;
        QSignalMapper* m_signalmapper_connect;
        std::map<std::string, FugaContact*> m_contacts;
        std::map<int, QTcpSocket*> m_pending;
		QMutex* m_mutex;

        bool _startServer();

    public slots:
        void addPendingConnection();
};

#endif // __FUGACONTACTS_H__
