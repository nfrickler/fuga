#ifndef __FugaStreamer_H__
#define __FugaStreamer_H__

#include <QObject>
#include <QHostAddress>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappbuffer.h>

class FugaStreamer : public QObject {
	Q_OBJECT

	public:
        FugaStreamer(QHostAddress* in_address, quint16 in_port, quint16 in_width,
					 quint16 in_height, std::string in_path);
        ~FugaStreamer();

	protected:
		QHostAddress* m_address;
		quint16 m_port;
		quint16 m_width;
		quint16 m_height;
		std::string m_path;

		GstElement* m_pipeline;

	public slots:
		void start();
		void stop();
};

#endif // __FugaStreamer_H__
