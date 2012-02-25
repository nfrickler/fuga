#ifndef MYUDPSERVER_H
#define MYUDPSERVER_H

#include <QObject>
#include <QUdpSocket>
#include <QImage>

class MyUdpServer : public QObject {
	Q_OBJECT

	public:
		MyUdpServer(QHostAddress* ip, quint16 port, quint32 mtu, quint16 quality);
		~MyUdpServer();
		void startServer();
		void sendImage(QImage* myimage);

	public slots:
		void sendNext(QImage* myimage);

	protected:
		QUdpSocket * m_udpSocket;

		// current image
		quint16 m_image_number;
		int timelag;

		// connection data
		QHostAddress* m_ip;
		quint16 m_port;
		quint32 m_mtu;
		quint16 m_full_frame_rate;
		quint16 m_quality;

};

#endif // MYUDPSERVER_H
