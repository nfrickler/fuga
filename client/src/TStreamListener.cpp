#include "TStreamListener.h"
#include <iostream>
#include <QDataStream>
#include <QNetworkInterface>

using namespace std;

/* constructor
 */
TStreamListener::TStreamListener (QHostAddress* ip, quint16 port, quint16 width, quint16 height) {

	// save input
	m_ip = ip;
	m_port = port;
	m_width = width;
	m_height = height;

	// init
	m_maxframenum = 60000;
	m_current_frame_num = 0;
	m_current_image_num = 0;
	m_qimage = NULL;
}

/* start listening socket
 */
void TStreamListener::startListening () {
	cout << "TStreamListener: Start Listening...." << endl;

	// start client
	m_udpSocket = new QUdpSocket(this);
	if (m_udpSocket == NULL) {
		cout << "Could not start Udp-Client!" << endl;
		return;
	}
	m_udpSocket->bind(*m_ip, m_port, QUdpSocket::ShareAddress);
	//m_udpSocket->joinMulticastGroup(*m_ip, QNetworkInterface::interfaceFromName("eth0"));
	connect(m_udpSocket, SIGNAL(readyRead()), this, SLOT(slot_processPendings()));
	cout << "TStreamListener: Udp is listening on ip " << m_ip->toString().toAscii().data() << " port " << m_port << endl;

	// start video writer
	m_writer = NULL;
	//m_writer = cvCreateVideoWriter("steamlistener.avi", CV_FOURCC('P','I','M','1'), 25, cvSize(m_width,m_height), 1);
	if (m_writer == NULL) {
		cout << "!!! ERROR: cvCreateVideoWriter" << endl;
	}

	// start time
	m_time = new QTime();
}

/* slot startListening
 */
void TStreamListener::slot_startListening () {
	startListening();
}

/* process pending datagrams
 */
void TStreamListener::slot_processPendings () {
	m_time->restart();

	// get all pending datagrams
	is_image = false;
	receiveFrame();

	// no image?
	if (!is_image) return;

	// push image to MyVideo
	emit newFrame(m_qimage);

	cout << "TStreamListener: current image: " << m_current_image_num << "(" << m_time->elapsed() << " ms)" << endl;
}

/* receive frame from network
 */
void TStreamListener::receiveFrame () {

	// read all pending datagrams
	QByteArray datagram = QByteArray();
	quint16 num_frame = 0;
	quint32 pos_data = 0;
	while (m_udpSocket->hasPendingDatagrams()) {

		// read datagram
		datagram.resize(m_udpSocket->pendingDatagramSize());
		if (datagram.size() <= 9) continue;
		m_udpSocket->readDatagram(datagram.data(), datagram.size());
		QDataStream ds (datagram);

		// get num_frame and pos_data
		ds >> num_frame >> pos_data;

		// skip old frames
		if (!isAcceptedFrame(num_frame)) continue;
		//cout << "receive: " << num_frame << "|" << pos_data << endl;

		// get checksum and is_full
		if (pos_data == 0) {
			ds >> m_checksumbuffer[num_frame];
			ds >> m_isfull_buffer[num_frame];
		}

		// save data
		if (!m_imagebuffer[num_frame]) m_imagebuffer[num_frame] = new QByteArray();
		if (m_imagebuffer[num_frame]->size() < (int) pos_data)
			m_imagebuffer[num_frame]->resize(pos_data);
		QByteArray mybuff = QByteArray();
		ds >> mybuff;
		m_imagebuffer[num_frame]->insert(pos_data, mybuff);

		// check for complete image
		if (pos_data == 0 && _puzzleImage()) return;
	}
}

/* puzzle image
 */
bool TStreamListener::_puzzleImage () {

	map<quint16, QByteArray*>::iterator curr,end;
	for (curr = m_imagebuffer.begin(), end = m_imagebuffer.end();  curr != end;  curr++) {

		// delete old crap
		if (!isAcceptedFrame(curr->first)) {
			cout << "TStreamListener: xxxxxxxxxxxxxxxxxxxxxxx num: " << curr->first << endl;
			m_checksumbuffer.erase(curr->first);
			m_imagebuffer.erase(curr->first);
			m_isfull_buffer.erase(curr->first);
			continue;
		}

		// received checksum and is_full?
		if (!m_checksumbuffer[curr->first]) {
			cout << "incomplete: " << m_checksumbuffer[curr->first] << endl;
			continue;
		}

		// check checksum
		quint16 curr_checksum = qChecksum(m_imagebuffer[curr->first]->data(), m_imagebuffer[curr->first]->size());
		if (curr_checksum != m_checksumbuffer[curr->first]) continue;

		// decompress data
		//*m_imagebuffer[curr->first] = qUncompress(*m_imagebuffer[curr->first]);

		// convert qbytearray to qimage
		is_image = true;
		m_qimage = new QImage();
		if (!m_qimage->loadFromData((uchar*) m_imagebuffer[curr->first]->data(), m_imagebuffer[curr->first]->size())) {
			cout << "TStreamListener: Error: could not load image from data!" << endl;
			is_image = false;
		}

		// write video
	//	if (m_writer != NULL) cvWriteFrame( m_writer, fIplImageHeader );
	//	cvReleaseImage(&fIplImageHeader);

		// remove from map
		m_imagebuffer.erase(curr->first);
		m_checksumbuffer.erase(curr->first);
		m_isfull_buffer.erase(curr->first);

		// return
		m_current_image_num = curr->first;
		cout << "TStreamListener: Received image (" << m_current_image_num << ") <<<------------" << endl;
		return true;
	}

	return false;
}

bool TStreamListener::isAcceptedFrame(quint16 frame) {
	quint16 tolerance = 2;
	quint16 tolerance_up = 20;
    bool result = false;

	// init
	if (m_current_frame_num == 0) {
		m_current_frame_num = frame;
		return true;
	}

    if (frame < m_current_frame_num) {
        if ((m_current_frame_num - frame) > (m_maxframenum - tolerance)) result = true;
        if ((m_current_frame_num - frame) < tolerance) result = true;
    } else {
        result = true;
        if ((frame - m_current_frame_num) > tolerance) {
			result = false;
            if ((frame - m_current_frame_num) > (m_maxframenum - tolerance)) result = true;
			if ((frame - m_current_frame_num) > tolerance_up) result = true;
        }
    }
    if (result) {
        if ((frame > m_current_frame_num && (m_current_frame_num - frame) < tolerance)
                || (frame < m_current_frame_num && (m_current_frame_num - frame) > (m_maxframenum - tolerance))) {
            m_current_frame_num = frame;
        }
	}

	if (!result)
		cout << "TStreamListener: Skip frame "
			 << frame << " (current: " << m_current_frame_num << ")"
			 << endl;
    return result;
}
