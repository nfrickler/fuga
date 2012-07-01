#ifndef __CONTACTHANDLER_H__
#define __CONTACTHANDLER_H__

#include "Supervisor.h"
#include "FuGaStreamer.h"
#include "FuGaVideo.h"
#include <QMutex>
#include <map>
#include <QTcpSocket>
#include <QByteArray>
#include <QHostAddress>

class MyVideo;
class Supervisor;

struct contact {
	std::string		name;

	QHostAddress*	udp_ip;
	quint16			udp_port;
	quint16			img_width;
	quint16			img_height;

	QTcpSocket*		tcp_socket;
	quint16			tcp_port;
	QHostAddress*	tcp_ip;
	std::string		tcp_buffer;
	std::string		tcp_buffer_send;

	FuGaStreamer* streamer;
	FuGaVideo* video;
};

class ContactHandler : public QObject {
	Q_OBJECT

	public:
		ContactHandler(Supervisor* mysupervisor);

		// add/remove contacts
		void addContact(std::string name, bool loadInfos = false);
		void removeContact(std::string name);
		bool isVideoReady(std::string name);
		bool isTcpContact(std::string name, bool socketOnly = false);
		bool changeName(std::string name, std::string newname);
		bool isName(std::string name);
		bool hasData(std::string name);

		// setter
		void setUdpIp(std::string name, QHostAddress* ip);
		void setUdpPort(std::string name, quint16 port);
		void setImg(std::string name, quint16 width, quint16 height);
		void setWidth(std::string name, quint16 width);
		void setHeight(std::string name, quint16 height);
		void setTcpIp(std::string name, QHostAddress* ip);
		void setTcpPort(std::string name, quint16 port);
		void setTcpSocket(std::string name, QTcpSocket* mysocket);
		void addTcpBufferSend(std::string name, std::string buffer);

		// getter
		QHostAddress* getUdpIp(std::string name);
		quint16 getUdpPort(std::string name);
		quint16 getImgWidth(std::string name);
		quint16 getImgHeight(std::string name);
		QHostAddress* getTcpIp(std::string name);
		quint16 getTcpPort(std::string name);
		QTcpSocket* getTcpSocket(std::string name);
		std::string getTcpBufferSend(std::string name);

		// streamer and video handling
		FuGaStreamer* getStreamer(std::string name);
		void rmStreamer(std::string name);
		FuGaVideo* getVideo(std::string name);
		void rmVideo(std::string name);

	protected:
		Supervisor* m_supervisor;
		std::map<std::string, contact*> m_contacts;
		QMutex* m_mutex;

	signals:
		void s_isStreaming(std::string name);
		void s_isData(std::string name);
		void s_isContact(std::string name);
};

#endif // __CONTACTHANDLER_H__
