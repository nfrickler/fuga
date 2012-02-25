#include "MyModule.h"
#include <iostream>
#include <QTimer>

using namespace std;

/* destructor
 */
MyModule::~MyModule () {
	stopServer();
}

/* start server
 */
void MyModule::startServer (QHostAddress* ip, quint16 port, quint32 mtu, quint16 quality,
							quint16 img_width, quint16 img_height,
							string videopath, bool is_videostreaming) {

	// is already running?
	if (m_udpserver != NULL) {
		cout << "MyModule: Udp-Server already running!" << endl;
		return;
	}

	// start new thread
	m_thread = new QThread();
	m_thread->start();
	m_thread2 = new QThread();
	m_thread2->start();

	// start webcam-grabber
	m_tw_grabber = new TWebcamGrabber(img_width, img_height, videopath, is_videostreaming);
	m_tw_grabber->moveToThread(m_thread);

	// start udp-server
	m_udpserver = new MyUdpServer(ip, port, mtu, quality);
	m_udpserver->moveToThread(m_thread2);
	m_udpserver->startServer();

	// start streaming
	QTimer* mytimer = new QTimer();
	connect(mytimer, SIGNAL(timeout()), m_tw_grabber, SLOT(getNextFrame()));
	connect(this, SIGNAL(startStreaming()), m_tw_grabber, SLOT(start()));
	connect(m_tw_grabber, SIGNAL(isNewFrame(QImage*)), m_udpserver, SLOT(sendNext(QImage*)));
	emit startStreaming();
	mytimer->start(40);
}

/* stop server
 */
void MyModule::stopServer () {
	m_udpserver_timer->stop();
	delete m_udpserver_timer;
	m_thread->exit();
	delete m_tw_grabber;
	delete m_udpserver;
	delete m_thread;
}
