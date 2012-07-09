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
        FugaContact(Fuga* in_Fuga, QTcpSocket* in_socket);
        std::string     name();
        QHostAddress*   udp_ip();
        bool            udp_ip(QHostAddress* in_ip);
        quint16			udp_port();
        bool            udp_port(quint16 in_port);
        quint16			img_width();
        bool            img_width(quint16 in_width);
        quint16			img_height();
        bool            img_height(quint16 in_height);
        QTcpSocket*		tcp_socket();
        bool    		tcp_socket(QTcpSocket in_socket);
        quint16			tcp_port();
        bool            tcp_port(quint16 in_port);
        QHostAddress*	tcp_ip();
        bool            tcp_ip(QHostAddress* in_ip);
        std::string		tcp_buffer();
        bool            tcp_buffer(std::string in_buffer);
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
        quint16			m_udp_port;
        quint16			m_img_width;
        quint16			m_img_height;

        std::map<std::string,std::string> m_data;

        QTcpSocket*		m_socket;
        quint16			m_tcp_port;
        QHostAddress*	m_tcp_ip;
        std::string		m_buffer;

        FugaVideo*      m_Video;
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
};

#endif // FUGACONTACT_H
