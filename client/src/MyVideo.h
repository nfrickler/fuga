#ifndef __MYVIDEO_H__
#define __MYVIDEO_H__

#include <QWidget>
#include <QHostAddress>

#include <glib.h>
#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>

class MyVideo : public QWidget {
	Q_OBJECT

	public:
		MyVideo(QHostAddress* ip, quint16 port, quint16 width, quint16 height);
		~MyVideo();
		void start();
		void stop();

	protected:

		void init();
		GstElement* m_pipeline;
		GstElement* m_xvimagesink;

		// sizes
		int m_width;
		int m_height;

};

#endif // __MYVIDEO_H__
