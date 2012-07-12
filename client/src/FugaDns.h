#ifndef FUGADNS_H
#define FUGADNS_H

#include "FugaContact.h"
#include <QHostAddress>
#include <map>

class FugaDns : public FugaContact {
    Q_OBJECT

    public:
        FugaDns(Fuga* in_Fuga);
        bool resolve(std::string in_name);
        void login(std::string in_name, std::string in_password);
        void send(std::string in_msg);

        void doVerify();
        bool isVerified();

    protected:
        void doConnect();
        int m_msg2sign;

    signals:
        void sig_resolved(std::string in_name, QHostAddress* in_ip, quint16 in_port);
        void sig_loggedin(int in_return);

    public slots:
        void slot_connected();
        void slot_verify(std::string in_type,std::vector<std::string> in_data);
        void slot_doResolve(std::string in_type,std::vector<std::string> in_data);
        void slot_resolved(std::string in_name, QHostAddress* in_ip, quint16 in_port);
        void slot_checklogin(std::string in_type,std::vector<std::string> in_data);
        void slot_disconnected();

};

#endif // FUGADNS_H
