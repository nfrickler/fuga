#ifndef MYUDPSERVER_H
#define MYUDPSERVER_H

#include <QObject>
#include <QUdpSocket>
#include <opencv/cv.h>
#include <opencv/highgui.h>

class MyUdpServer : public QObject {
	Q_OBJECT

	public:
		MyUdpServer(QHostAddress* ip, quint16 port, quint32 mtu, quint16 quality);
		~MyUdpServer();
		void startServer();
		void sendImage(IplImage* myimage);

	public slots:
		void sendNext(IplImage* myimage);

	protected:
		QUdpSocket * m_udpSocket;
		QByteArray* m_last_full_image;
		QByteArray* getDiff(QByteArray* inarray1, QByteArray* inarray2);

		// current image
		quint16 m_image_number;
		QByteArray * mybuffer;
		IplImage* fIplImageHeader;
		int timelag;

		// connection data
		QHostAddress* m_ip;
		quint16 m_port;
		quint32 m_mtu;
		quint16 m_full_frame_rate;
		quint16 m_quality;

};

#endif // MYUDPSERVER_H
