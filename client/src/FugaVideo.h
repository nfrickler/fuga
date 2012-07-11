#ifndef __FugaVideo_H__
#define __FugaVideo_H__

#include <QWidget>
#include <QHostAddress>
#include <QResizeEvent>

#include <glib.h>
#include <gst/gst.h>

class FugaVideo : public QWidget {
	Q_OBJECT

	public:
        FugaVideo(QHostAddress* in_ip, quint16 in_firstport);
		void resizeEvent(QResizeEvent *e);
        ~FugaVideo();
		void start();
		void stop();

	protected:

		void init();
		GstElement* m_pipeline;
        GstElement* m_xvimagesink;

        // connection
        QHostAddress* m_ip;
        quint16 m_firstport;
};

#endif // __FugaVideo_H__
