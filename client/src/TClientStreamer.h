#ifndef __TCLIENTSTREAMER_H__
#define __TCLIENTSTREAMER_H__

#include <QThread>

class TClientStreamer : public QThread {

	Q_OBJECT;

	public:
		TWebcamGrabber(QWidget *m_parent);

	protected:
		QWidget *m_parent;
		void run();

}

#endif // __TCLIENTSTREAMER_H__
