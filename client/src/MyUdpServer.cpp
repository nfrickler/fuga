#include "MyUdpServer.h"
#include <iostream>
#include <QTime>
#include <sstream>
#include <QDataStream>
#include <QNetworkInterface>
#include <QBuffer>
#include <QImage>

using namespace std;

/* constructor
 */
MyUdpServer::MyUdpServer (QHostAddress* ip, quint16 port, quint32 mtu, quint16 quality) {

	// save input
	m_ip = ip;
	m_port = port;
	m_mtu = mtu;
	m_quality = quality;

	// init
	srand(static_cast<int>(time(NULL)));
	m_image_number = 1 + rand() % (59999 - 1 + 1);
	timelag = 0;

	m_full_frame_rate = 100;
}

/* destructor
 */
MyUdpServer::~MyUdpServer () {

}

/* start udp-server
 */
void MyUdpServer::startServer () {

	// start udp-socket
	m_udpSocket = new QUdpSocket(0);
	if (!m_udpSocket) {
		cout << "Couldn't start UdpServer!" << endl;
		return;
	}
	QNetworkInterface myinterface = QNetworkInterface::interfaceFromName("eth0");
	cout << "Interface is valid: " << myinterface.isValid() << endl;

	m_udpSocket->bind(m_port, QUdpSocket::ShareAddress);
	//m_udpSocket->setMulticastInterface(QNetworkInterface::interfaceFromName("eth0"));
	m_udpSocket->moveToThread(this->thread());
	cout << "TWebcamServer: Sending on: " << m_ip->toString().toAscii().data() << " port: " << m_port << endl;
}

/* send frame over network
 */
void MyUdpServer::sendImage (QImage* myimage) {
	QTime* mytime = new QTime();
	mytime->start();
	cout << "MyUdpServer: size:" << myimage->byteCount() << " format:" << myimage->format() << endl;

	// return if no frame
	if (myimage == NULL) return;

	// get number of current image
	m_image_number++;
		if (m_image_number > 60000) m_image_number = 1;
	bool is_full = ((m_image_number % m_full_frame_rate != 0)) ? false : true;

	// convert vector<unsigned char> to QByteArray
	QByteArray* mybuffer = new QByteArray();
	QBuffer buf(mybuffer);

	// BUG: crashes here after some time!
	cout << " - before save" << endl;
	myimage->save(&buf, "JPEG");
	cout << " - after save" << endl;

	/*
	// compress data
	cout << "mybuffer before: " << mybuffer->size();
	*mybuffer = qCompress(*mybuffer, 4);
	mybuffer->squeeze();
	cout << " mybuffer after: " << mybuffer->size() << endl;
	*/

	// calc checksum
	quint16 curr_checksum = qChecksum(mybuffer->data(), mybuffer->size());

	// split in datagrams and send them
	quint32 curr_size = 0;
	quint32 curr_pos = 0;
	quint32 curr_flush = 1;
	quint32 buffersize = mybuffer->size();
	while(mybuffer->size() > 0) {
		QByteArray datagram = QByteArray();
		QDataStream ds(&datagram, QIODevice::WriteOnly);
		ds << m_image_number << curr_pos;

		// add checksum and is_full
		if (curr_pos == 0) ds << curr_checksum << is_full;

		// get next datagram
		curr_size = (mybuffer->size() > (int) m_mtu) ? m_mtu : mybuffer->size();
		curr_pos += curr_size;

		// add data
		QByteArray mybuf = QByteArray(mybuffer->mid(0, curr_size));
		ds << mybuf;
		*mybuffer = mybuffer->remove(0, curr_size);

		// send datagram
		int bytessent = m_udpSocket->writeDatagram(datagram.data(), datagram.size(), *m_ip, m_port);
		if (curr_pos > curr_flush * 20000) {
			curr_flush++;
			m_udpSocket->flush();
		}
		if (bytessent < 0) {
			cout << "TWebcamServer: Error - could not sent datagram (" << datagram.size() << " Bytes)" << endl;
			cout << "Error: " << m_udpSocket->errorString().toAscii().data() << endl;
			return;
		} else {
		//	cout << "TWebcamServer: Packet sent ----------->>> size: " << bytessent << endl;
		}
	}
	delete mybuffer;

	// everything done
	timelag+= mytime->elapsed() - 40;
	if (timelag < 0) timelag = 0;
	cout << "All data (" << m_image_number << "|" << mytime->elapsed() << " ms) sent. ------------> size: " << buffersize << endl;
}

// send slot
void MyUdpServer::sendNext(QImage* myimage) {
	if (timelag > 40) {
		myimage = NULL;
		delete myimage;
		cout << "MyUdpServer: xxxxxxxxxx Skipped frame (" << timelag << "ms) xxxxxxxxxxxxx" << endl;
		timelag-= 20;
		return;
	}
	sendImage(myimage);
	myimage = NULL;
	delete myimage;
}
