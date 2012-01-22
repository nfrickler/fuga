#include "TWebcamGrabber.h"
#include <iostream>
#include <QTime>

using namespace std;

/* constructor
 */
TWebcamGrabber::TWebcamGrabber (quint16 img_width, quint16 img_height,
								string videopath, bool is_videostreaming) {

	// save input
	m_capture = NULL;
	m_width = img_width;
	m_height = img_height;
	m_videopath = videopath;
	m_isvideostreaming = is_videostreaming;

	// start capturing
	int count = (m_isvideostreaming) ? 1 : 0;
	m_cap = cv::VideoCapture();

	while (m_capture == NULL && !m_cap.isOpened()) {
		cout << "Looking for webcam .... #################" << endl;
		if (count == 0) {
			m_cap.open(0);
			usleep(500);

			if(!m_cap.isOpened())
				cout << "TWebcamGrabber: Could not access webcam!" << endl;
		//	m_capture = cvCaptureFromCAM(-1);
		}
		if (count == 1) {
			m_capture = cvCaptureFromFile(m_videopath.c_str());
			if (m_capture != NULL) m_isvideostreaming = true;
		}
		if (count >= 2) { return; }
		count++;
	}
	cout << "TWebcamGrabber: Webcam/Video found :-)" << endl;

	// start video writer
	m_writer = NULL;
/*
	m_writer = cvCreateVideoWriter("mywebcam.avi", CV_FOURCC('P','I','M','1'), 25, cvSize(img_width,img_height), 1);
	if (m_writer == NULL) {
		cout << "!!! ERROR: cvCreateVideoWriter" << endl;
	}
*/
}

/* destructor
 */
TWebcamGrabber::~TWebcamGrabber () {
	cvReleaseCapture(&m_capture);
	cvReleaseVideoWriter(&m_writer);
	m_cap.release();
}

/* send next frame
 */
void TWebcamGrabber::nextFrame() {
	QTime* mytime = new QTime();
	mytime->start();
	IplImage* myimage;

	// get next frame from webcam
	if (m_cap.isOpened()) {
		cv::Mat frame;
		m_cap >> frame;
		// resize image
		myimage = cvCreateImage(cvSize(frame.cols, frame.rows),
			IPL_DEPTH_8U, frame.channels()
		);
		myimage->imageData = (char*) frame.data;
	}

	// get next frame from video
	else if (m_capture != NULL) {
		myimage = cvQueryFrame(m_capture);
	}

	// get next frame failed
	else {
		cout << "TWebcamGrabber: ERROR: NO VIDEO DEVICE!" << endl;
		return;
	}

	// have image?
	if (myimage == NULL) {
		cout << "TWebcamGrabber: No image" << endl;
		return;
	}

	// resize image
	IplImage* smallimage = cvCreateImage(cvSize(m_width, m_height),
		IPL_DEPTH_8U, 3
	);
	cvResize(myimage, smallimage);

	// emit signal on success
	if (smallimage == NULL) return;
	emit isNewFrame(smallimage);

	// write video
	//if (m_writer != NULL) cvWriteFrame(m_writer, myimage);

	cout << "TWebcamGrabber: Got frame in " << mytime->elapsed() << "ms" << endl;
	return;
}
