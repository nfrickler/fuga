#ifndef __TWEBCAMGRABBER_H__
#define __TWEBCAMGRABBER_H__

#include <QObject>
#include <QImage>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappbuffer.h>

struct ProgramData {
	GMainLoop *loop;
	GstElement *sink;
};

class TWebcamGrabber : public QObject {
	Q_OBJECT

	public:
		TWebcamGrabber(quint16 img_width, quint16 img_height,
					   std::string videopath = "", bool is_videostreaming = false);
		~TWebcamGrabber();
		void doEmit(QImage* myimage);

	protected:
		quint16 m_width;
		quint16 m_height;
		std::string m_videopath;
		bool m_isvideostreaming;
		ProgramData* data;

	signals:
		void isNewFrame(QImage* myimage);

	public slots:
		void start();
		void getNextFrame();
		void stop();
};

#endif // __TWEBCAMGRABBER_H__
