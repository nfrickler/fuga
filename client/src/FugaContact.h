#ifndef FUGACONTACT_H
#define FUGACONTACT_H

#include <QHostAddress>
#include <map>
#include <QSslSocket>

class Fuga;
class FugaDns;
class FugaVideo;
class FugaStreamer;

class FugaContact : public QObject {
    Q_OBJECT

    public:
        FugaContact(Fuga* in_Fuga, std::string in_name);
        FugaContact(Fuga* in_Fuga, QSslSocket* in_socket);
        FugaContact(Fuga* in_Fuga);
        void init(Fuga* in_Fuga);

        std::string     name();
        FugaVideo*      Video();
        FugaStreamer*   Streamer();
        bool            startStreaming();
        bool            stopStreaming();

        void            send(std::string in_msg);

        void            doResolve();
        bool            isResolved();
        void            doConnect();
        bool            isConnected();
        void            doHello();
        bool            isHello();
        void            doVerify();
        bool            isVerified();
        void            doAccept();
        bool            isAccepted();
        void            doDisconnect();

    protected:
        Fuga*           m_Fuga;
        FugaStreamer*   m_Streamer;
        bool            m_isaccepted;
        bool            m_verified_active;
        bool            m_verified_passive;

        std::string     m_name;
        QHostAddress*	m_udp_ip;
        quint16			m_udp_firstport;
        QSslSocket*		m_socket;
        quint16			m_tcp_port;
        QHostAddress*	m_tcp_ip;
        std::string		m_buffer;
        std::map<std::string,std::string> m_data;

        void send_direct(std::string in_msg);
        void addToBuffer(std::string in_msg);
        void sendBuffer();
        void getInfos();
        void connectSocket();

    signals:
        void sig_streaming();

        void sig_resolved();
        void sig_connected();
        void sig_hello();
        void sig_verified();
        void sig_accepted();
        void sig_double();

        void sig_received(std::string type, std::vector<std::string> data);
        void sig_disconnected();

    public slots:
        void slot_handleError(QAbstractSocket::SocketError in_error);
        void slot_sslerror(const QList<QSslError> &);

        void slot_resolved(std::string in_name, QHostAddress* in_ip, quint16 in_port);
        void slot_connected();
        void slot_hello(std::string in_type,std::vector<std::string> in_data);
        void slot_verify(std::string in_type,std::vector<std::string> in_data);
        void slot_accepted();
        void slot_disconnected();

        void slot_received();
        void slot_gotRequest(std::string in_type,std::vector<std::string> in_data);
};

#endif // FUGACONTACT_H
