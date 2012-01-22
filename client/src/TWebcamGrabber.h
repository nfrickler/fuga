#ifndef __TWEBCAMGRABBER_H__
#define __TWEBCAMGRABBER_H__

#include <QObject>
#include <opencv/cv.h>
#include <opencv/highgui.h>

class TWebcamGrabber : public QObject {
	Q_OBJECT

	public:
		TWebcamGrabber(quint16 img_width, quint16 img_height,
					   std::string videopath = "", bool is_videostreaming = false);
		~TWebcamGrabber();

	protected:
		void run();
		CvCapture * m_capture;
		CvVideoWriter* m_writer;
		quint16 m_width;
		quint16 m_height;
		std::string m_videopath;
		bool m_isvideostreaming;
		cv::VideoCapture m_cap;

	signals:
		void isNewFrame(IplImage* myimage);

	public slots:
		void nextFrame();

};

#endif // __TWEBCAMGRABBER_H__
