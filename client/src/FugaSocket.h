#ifndef FUGASOCKET_H
#define FUGASOCKET_H

#include <QHostAddress>
#include <QSslSocket>

class Fuga;

class FugaSocket : public QObject {
    Q_OBJECT

    public:
        FugaSocket(Fuga* in_Fuga);

        virtual void doConnect();
        virtual void doDisconnect();
        std::string name();
        void send(std::string in_msg);

        bool isConnected();
        virtual bool isConnectionReady();

    protected:
        Fuga*           m_Fuga;
        QSslSocket*		m_socket;
        quint16			m_tcp_port;
        QHostAddress*	m_tcp_ip;
        std::string     m_name;
        std::string		m_buffer;
        std::string		m_buffer_direct;
        int             m_id;
        bool            m_connectionrequested;

        void connectSocket();
        void send_direct(std::string in_msg);
        void addToBuffer(std::string in_msg);
        void sendBuffer();
        void addToDirectBuffer(std::string in_msg);
        void sendDirectBuffer();

    signals:
        void sig_connected();
        void sig_disconnected();
        void sig_connectionready();

        void sig_received(std::string type, std::vector<std::string> data);

    protected slots:
        void slot_connected();
        void slot_connectionready();
        void slot_disconnected();
        void slot_handleError(QAbstractSocket::SocketError in_error);
        void slot_sslerror(const QList<QSslError> &);

        void slot_received();
        virtual void slot_gotRequest(std::string,std::vector<std::string>);
};

#endif // FUGASOCKET_H
