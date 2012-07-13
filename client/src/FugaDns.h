#ifndef FUGADNS_H
#define FUGADNS_H

#include "FugaSocket.h"

class Fuga;

class FugaDns : public FugaSocket {
    Q_OBJECT

    public:
        FugaDns(Fuga* in_Fuga);

        bool resolve(std::string in_name);

        bool isConnectionReady();

        bool isVerified();
        void doLogin(std::string in_name, std::string in_password);
        bool isLoggedin();

        void doDisconnect();

    protected:
        int m_msg2sign;
        bool m_isloggedin;

    signals:
        void sig_loggedin(int in_return);
        void sig_resolved(std::string,QHostAddress*,quint16,std::string);

    public slots:
        void slot_start_verify();
        void slot_verify(std::string in_type,std::vector<std::string> in_data);
        void slot_checklogin(std::string in_type,std::vector<std::string> in_data);

        void slot_doResolve(std::string in_type,std::vector<std::string> in_data);
};

#endif // FUGADNS_H
