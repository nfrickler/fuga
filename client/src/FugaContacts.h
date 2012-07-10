#ifndef __FUGACONTACTS_H__
#define __FUGACONTACTS_H__

#include "FugaDns.h"
#include "FugaContact.h"
#include "FugaSslServer.h"
#include <QMutex>
#include <map>
#include <iostream>
#include <sstream>
#include <QTcpServer>

class Fuga;

class FugaContacts : public QObject {
	Q_OBJECT

	public:
        FugaContacts(Fuga* in_Fuga);
        FugaContact* getContact(std::string in_name);
        bool isContact(std::string name);
        void handleError(QAbstractSocket::SocketError in_error);
        FugaDns* getDns();

	protected:
        Fuga* m_Fuga;
        FugaDns* m_Dns;
        FugaSslServer* m_Server;
        std::map<std::string, FugaContact*> m_contacts;
		QMutex* m_mutex;

        void startServer();

    public slots:
        void slot_addconnection(QSslSocket* in_socket);
        void slot_handleError(QAbstractSocket::SocketError in_error);
        void slot_add_hereiam(FugaContact*,std::string);
};

#endif // __FUGACONTACTS_H__
