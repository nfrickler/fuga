#ifndef FUGACONTACT_H
#define FUGACONTACT_H

#include "FugaSocket.h"
#include <map>

class Fuga;
class FugaDns;
class FugaVideo;
class FugaStreamer;

class FugaContact : public FugaSocket {
    Q_OBJECT

    public:
        FugaContact(Fuga* in_Fuga, std::string in_name);
        FugaContact(Fuga* in_Fuga, QSslSocket* in_socket);
        void init();

        FugaVideo*      Video();
        FugaStreamer*   Streamer();
        bool            startStreaming();
        bool            stopStreaming();

        void            doResolve();
        bool            isResolved();
        void            doHello();
        bool            isHello();
        void            doVerify();
        bool            isVerified();
        void            doAccept();
        bool            isAccepted();
        bool            isConnectionReady();

    protected:
        std::string     m_pubkey;
        FugaStreamer*   m_Streamer;
        bool            m_isaccepted;
        bool            m_verified_active;
        bool            m_verified_passive;
        QHostAddress*	m_udp_ip;
        quint16			m_udp_firstport;
        std::map<std::string,std::string> m_data;

    signals:
        void sig_streaming();
        void sig_resolved();
        void sig_hello();
        void sig_verified();

    public slots:
        void slot_resolved(std::string in_name, QHostAddress* in_ip, quint16 in_port, std::string in_pubkey);
        void slot_start_hello();
        void slot_hello(std::string in_type,std::vector<std::string> in_data);
        void slot_verify(std::string in_type,std::vector<std::string> in_data);

        void slot_gotRequest(std::string in_type,std::vector<std::string> in_data);
};

#endif // FUGACONTACT_H
