#ifndef __FugaVideo_H__
#define __FugaVideo_H__

#include <QWidget>
#include <QHostAddress>
#include <QResizeEvent>

#include <glib.h>
#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>

class FugaVideo : public QWidget {
	Q_OBJECT

	public:
        FugaVideo(QHostAddress* in_address, quint16 in_port);
		void resizeEvent(QResizeEvent *e);
        ~FugaVideo();
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

#endif // __FugaVideo_H__
