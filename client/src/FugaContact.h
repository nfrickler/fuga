#ifndef FUGACONTACT_H
#define FUGACONTACT_H

#include <QHostAddress>
#include <QTcpSocket>
#include <QTcpServer>
#include <map>
#include <QSignalMapper>
#include <QSslSocket>
#include <QMutex>

class Fuga;
class FugaDns;
class FugaVideo;
class FugaStreamer;

class FugaContact : public QObject {
    Q_OBJECT

    public:
        FugaContact(Fuga* in_Fuga, std::string in_name);
        FugaContact(Fuga* in_Fuga, QSslSocket* in_socket);
        std::string     name();
        FugaVideo*      Video();
        FugaStreamer*   Streamer();

        bool startStreaming();
        bool stopStreaming();
        bool isContact(std::string in_name);
        bool isVideoReady();
        void send(std::string in_msg);
        void addToBuffer(std::string in_msg);
        void sendBuffer();

        void resolve();
        bool isResolved();
        void doConnect();
        bool isConnected();
        void doFetch();
        bool isFetched();



    protected:
        Fuga*           m_Fuga;

        std::string     m_name;
        QHostAddress*	m_udp_ip;
        quint16			m_udp_firstport;

        std::map<std::string,std::string> m_data;

        QSslSocket*		m_socket;
        quint16			m_tcp_port;
        QHostAddress*	m_tcp_ip;
        std::string		m_buffer;

        FugaStreamer*   m_Streamer;

        void getInfos();
        void connectSocket();

    signals:
        void sig_streaming();
        void sig_info();
        void sig_connected();
        void sig_fetched();
        void sig_hereiam(FugaContact* in_Contact, std::string in_name);
        void sig_received(std::string type, std::vector<std::string> data);

    public slots:
        void slot_resolved(std::string in_name, QHostAddress* in_ip, quint16 in_port);
        void slot_handleError(QAbstractSocket::SocketError in_error);
        void slot_connected();
        void slot_received();
        void slot_doAnswer(std::string in_type,std::vector<std::string> in_data);
        void slot_fetch(std::string in_type,std::vector<std::string> in_data);
        void slot_sslerror(const QList<QSslError> &);
};

#endif // FUGACONTACT_H
