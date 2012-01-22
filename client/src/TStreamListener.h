#ifndef __TSTREAMPLAYER_H__
#define __TSTREAMPLAYER_H__

#include <QObject>
#include <QUdpSocket>
#include <opencv/cv.h>
#include <opencv/highgui.h>
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
		void receiveFrame();
		bool isAcceptedFrame(quint16 frame);
		bool _puzzleImage();
		QByteArray* applyPatch(QByteArray* inarray1, QByteArray* patch);

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
		bool is_image;
		quint16 m_current_image_num;
		quint16 m_current_frame_num;
		QImage* m_qimage;
		QTime* m_time;

		CvVideoWriter* m_writer;

	signals:
		void newFrame(QImage* myimage);

	public slots:
		void slot_startListening();
		void slot_processPendings();

};

#endif // __TSTREAMPLAYER_H__
