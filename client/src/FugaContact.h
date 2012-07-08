#ifndef FUGACONTACT_H
#define FUGACONTACT_H

#include "Fuga.h"
#include "FuGaVideo.h"
#include "FuGaStreamer.h"
#include <QHostAddress>
#include <QTcpSocket>
#include <QThread>
#include <QTcpServer>
#include <string>
#include <vector>
#include <map>
#include <QSignalMapper>
#include <QSslSocket>

class FugaContact : public QObject {
    Q_OBJECT

    public:
        FugaContact(Fuga* in_Fuga, std::string in_name);
        std::string     name();
        bool            name(std::string in_name);
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
        FuGaVideo*      Video();
        FuGaStreamer*   Streamer();

        bool startStreaming();
        bool stopStreaming();
        bool isData();
        bool isConnected();
        bool isContact(std::string in_name);
        bool isVideoReady();
        void send(std::string in_msg);
        void addToBuffer(std::string in_msg);
        void sendBuffer();



    protected:
        Fuga*           m_Fuga;

        std::string     m_name;
        QHostAddress*	m_udp_ip;
        quint16			m_udp_port;
        quint16			m_img_width;
        quint16			m_img_height;

        QTcpSocket*		m_socket;
        quint16			m_tcp_port;
        QHostAddress*	m_tcp_ip;
        std::string		m_buffer;

        FuGaVideo*      m_Video;
        FuGaStreamer*   m_Streamer;
        QMutex*         m_mutex;

        void getInfos();

    signals:
        void sig_streaming();
        void sig_info();
        void sig_contact();
        void sig_data();
        void sig_received(std::string type, std::string data);

    public slots:
        void slot_infos(std::string type, std::string data);
        void slot_connected();
        void slot_receive();
};

#endif // FUGACONTACT_H
