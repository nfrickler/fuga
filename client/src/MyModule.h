#ifndef __MYMODULE_H__
#define __MYMODULE_H__

#include "TWebcamGrabber.h"
#include "MyUdpServer.h"
#include <QThread>
#include <QObject>
#include <QTimer>

class TWebcamGrabber;
class MyUdpServer;

class MyModule : public QObject {
	Q_OBJECT

	public:
		~MyModule();

		// video-server
		void startServer(QHostAddress* ip, quint16 port, quint32 mtu, quint16 quality,
					  quint16 img_width, quint16 img_height,
					  std::string videopath = "", bool is_videostreaming = false);
		void stopServer();

	protected:

		// video-server
		MyUdpServer* m_udpserver;
		TWebcamGrabber* m_tw_grabber;
		QThread* m_thread;
		QThread* m_thread2;
		QTimer* m_udpserver_timer;

};

#endif // __MYMODULE_H__
