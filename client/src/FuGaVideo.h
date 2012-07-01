#ifndef __FUGAVIDEO_H__
#define __FUGAVIDEO_H__

#include <QWidget>
#include <QHostAddress>
#include <QResizeEvent>

#include <glib.h>
#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>

class FuGaVideo : public QWidget {
	Q_OBJECT

	public:
		FuGaVideo(QHostAddress* in_address, quint16 in_port);
		void resizeEvent(QResizeEvent *e);
		~FuGaVideo();
		void start();
		void stop();

	protected:

		void init();
		GstElement* m_pipeline;
		GstElement* m_xvimagesink;

		// sizes
		QHostAddress* m_address;
		quint16 m_port;

};

#endif // __FUGAVIDEO_H__
