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

	// start webcam-grabber
	m_tw_grabber = new TWebcamGrabber(img_width, img_height, videopath, is_videostreaming);
	m_tw_grabber->moveToThread(m_thread);

	// start streaming
	connect(this, SIGNAL(startStreaming()), m_tw_grabber, SLOT(start()));
	emit startStreaming();
}

/* stop server
 */
void MyModule::stopServer () {
	m_thread->exit();
	delete m_tw_grabber;
	delete m_thread;
}
