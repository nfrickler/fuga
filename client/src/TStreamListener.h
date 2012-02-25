#ifndef __TSTREAMPLAYER_H__
#define __TSTREAMPLAYER_H__

#include <QObject>
#include <QUdpSocket>
#include <qgl.h>
#include <map>
#include <QTime>

class TStreamListener : public QObject {
	Q_OBJECT

	public:
		TStreamListener(QHostAddress* ip, quint16 port, quint16 width, quint16 height);
		void startListening();

	protected:
		bool doRun;

		// frame handling
		QImage*  receiveFrame();
		QImage* _puzzleImage();
		bool isAcceptedFrame(quint16 frame);

		// connection
		QUdpSocket* m_udpSocket;
		QHostAddress* m_ip;
		quint16 m_port;
		quint16 m_width;
		quint16 m_height;
		quint16 m_maxframenum;

		// input buffer
		std::map<quint16, QByteArray*> m_imagebuffer;
		std::map<quint16, quint16> m_checksumbuffer;
		std::map<quint16, bool> m_isfull_buffer;
		quint16 m_current_image_num;
		quint16 m_current_frame_num;
		QTime* m_time;
		QByteArray m_datagram;

	signals:
		void newFrame(QImage* myimage);

	public slots:
		void slot_startListening();
		void slot_processPendings();

};

#endif // __TSTREAMPLAYER_H__
