#ifndef __FUGACONTACTS_H__
#define __FUGACONTACTS_H__

#include "FugaDns.h"
#include "FugaContact.h"
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
        QTcpServer* m_Server;
        std::map<std::string, FugaContact*> m_contacts;
		QMutex* m_mutex;

        bool startServer();

    public slots:
        void addPendingConnection();
        void slot_handleError(QAbstractSocket::SocketError in_error);
        void slot_add_hereiam(FugaContact*,std::string);
};

#endif // __FUGACONTACTS_H__
