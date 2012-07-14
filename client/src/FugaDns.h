#ifndef FUGADNS_H
#define FUGADNS_H

#include "FugaServerSocket.h"

class Fuga;

class FugaDns : public FugaServerSocket {
    Q_OBJECT

    public:
        FugaDns(Fuga* in_Fuga);

        void doConnect();
        bool isConnectionReady();
        void doDisconnect();

        void doLogin();
        bool isLoggedin();

    protected:
        bool m_isloggedin;

    signals:
        void sig_knowhostnow();
        void sig_loggedin(int in_return);

    public slots:
        void slot_checklogin(std::string in_type,std::vector<std::string> in_data);
        void slot_start_reallyconnecting(std::string in_name, QHostAddress* in_ip, quint16 in_port,std::string in_pubkey);
};

#endif // FUGADNS_H
