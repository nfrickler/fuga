#ifndef __FUGACONTACTS_H__
#define __FUGACONTACTS_H__

#include "FugaDns.h"
#include "FugaContact.h"
#include "FugaSslServer.h"
#include "FugaCrypto.h"
#include "FugaNetDns.h"
#include <map>
#include <iostream>
#include <sstream>

class Fuga;
class FugaCrypto;

class FugaContacts : public QObject {
	Q_OBJECT

	public:
        FugaContacts(Fuga* in_Fuga);
        FugaContact* getContact(std::string in_name);
        bool isContact(std::string name);
        FugaDns* getDns();
        FugaNetDns* getNetDns();
        FugaCrypto* getCrypto();
        bool registerContact(FugaContact* in_Contact);
        void connectContact (FugaContact* in_Contact);

	protected:
        Fuga* m_Fuga;
        FugaCrypto* m_Crypto;
        FugaNetDns* m_NetDns;
        FugaDns* m_Dns;
        FugaSslServer* m_Server;
        std::map<std::string, FugaContact*> m_contacts;

        void startServer();

    signals:
        void sig_received(std::string in_name, std::string in_type, std::vector<std::string> in_data);
        void sig_connected(std::string in_name);
        void sig_disconnected(std::string in_name);

    public slots:
        void slot_con_received(std::string in_type, std::vector<std::string> in_data);
        void slot_con_connected();
        void slot_con_disconnected();

        void slot_addconnection(QSslSocket* in_socket);
};

#endif // __FUGACONTACTS_H__
