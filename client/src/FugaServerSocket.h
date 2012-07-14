#ifndef FUGASERVERSOCKET_H
#define FUGASERVERSOCKET_H

#include "FugaSocket.h"

class Fuga;

class FugaServerSocket : public FugaSocket {
    Q_OBJECT

    public:
        FugaServerSocket(Fuga* in_Fuga);

        void doResolve(std::string in_name);

    protected:
        int m_msg2sign;

    signals:
        void sig_resolved(std::string,QHostAddress*,quint16,std::string);

    public slots:
        void slot_start_verify();
        void slot_verify(std::string in_type,std::vector<std::string> in_data);
        void slot_resolved(std::string in_type,std::vector<std::string> in_data);
};

#endif // FUGASERVERSOCKET_H
