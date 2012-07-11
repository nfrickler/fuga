#ifndef __FUGASTREAMER_H__
#define __FUGASTREAMER_H__

#include <QObject>
#include <QHostAddress>

#include <gst/gst.h>

class FugaStreamer : public QObject {
	Q_OBJECT

	public:
        FugaStreamer(QHostAddress* in_ip, quint16 in_firstport, std::string in_path);
        ~FugaStreamer();

        GstElement* getAudiohandler();
        GstElement* getVideohandler();

	protected:
        QHostAddress* m_ip;
        quint16 m_firstport;
		std::string m_path;

		GstElement* m_pipeline;
        GstElement* m_videohandler;
        GstElement* m_audiohandler;

	public slots:
		void start();
		void stop();
};

#endif // __FUGASTREAMER_H__
