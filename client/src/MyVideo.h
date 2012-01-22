#ifndef __MYVIDEO_H__
#define __MYVIDEO_H__

#include "TStreamListener.h"
#include <QImage>
#include <QThread>
#include <qgl.h>
#include <QGraphicsView>

class TStreamListener;

class MyVideo : public QGLWidget {
	Q_OBJECT

	public:
		MyVideo(QHostAddress* ip, quint16 port, quint16 width, quint16 height);
		~MyVideo();

		// set/get frame
		void setFrame(IplImage *myimage);
		IplImage* getFrame();

	protected:

		// listening
		QThread* m_thread;
		TStreamListener* m_ts_listener;

		// gl
	//	void initializeGL();
		virtual void paintGL();
		virtual void resizeGL(int width, int height);

		// sizes
		int m_width;
		int m_height;

		// current
		QImage * m_image;
		int timelag;

	signals:
		void signal_startListening();

	public slots:
		void updateWidget(QImage* myimage);
		void doResize(int width, int height);

};

class TestGraphicsView : public QGraphicsView {
	Q_OBJECT

	public:
		TestGraphicsView();

	protected:
		virtual void setupViewport(QWidget *viewport);
		void resizeEvent ( QResizeEvent * event );

	signals:
		void is_resized(int width, int height);
};

#endif // __MYVIDEO_H__
