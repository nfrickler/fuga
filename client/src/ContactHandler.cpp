#include "ContactHandler.h"
#include <iostream>
#include <sstream>

struct contact;
using namespace std;

/* constructor
 */
ContactHandler::ContactHandler (Supervisor* mysupervisor) {
	m_supervisor = mysupervisor;
	m_mutex = new QMutex();
}

// add/remove contacts
void ContactHandler::addContact(string name, bool loadInfos) {
	if (isName(name)) {
		cout << "ContactHandler: contact already exists: " << name << endl;
		if (loadInfos && !hasData(name)) m_supervisor->getTcp()->send_requestinfos(name);
		return;
	}
	m_mutex->lock();

	// create new contact
	m_contacts[name] = new contact;
	m_contacts[name]->name = name;
	m_contacts[name]->video = NULL;
	m_contacts[name]->streamer = NULL;
	m_contacts[name]->udp_ip = NULL;
	m_contacts[name]->udp_port = 0;
	m_contacts[name]->img_width = 0;
	m_contacts[name]->img_height = 0;
	m_contacts[name]->tcp_ip = NULL;
	m_contacts[name]->tcp_port = 0;
	m_contacts[name]->tcp_socket = NULL;
	m_contacts[name]->tcp_buffer = "";
	m_contacts[name]->tcp_buffer_send = "";

	m_mutex->unlock();
	if (loadInfos && !hasData(name)) m_supervisor->getTcp()->send_requestinfos(name);
	cout << "ContactHandler: Added contact: " << name << endl;
}
void ContactHandler::removeContact(string name) {
	rmVideo(name);
	rmStreamer(name);
	m_mutex->lock();
	m_contacts.erase(name);
	m_mutex->unlock();
}
bool ContactHandler::changeName(string name, string newname) {
	if (!isName(name)) return false;
	if (name.compare(newname) == 0) return true;

	// newname already exists?
	if (isName(newname)) {
		m_mutex->lock();

		if (m_contacts[name]->udp_ip != NULL)
			m_contacts[newname]->udp_ip = m_contacts[name]->udp_ip;
		if (m_contacts[name]->udp_port != 0)
			m_contacts[newname]->udp_port = m_contacts[name]->udp_port;
		if (m_contacts[name]->img_width != 0)
			m_contacts[newname]->img_width = m_contacts[name]->img_width;
		if (m_contacts[name]->img_height != 0)
			m_contacts[newname]->img_height = m_contacts[name]->img_height;
		if (m_contacts[name]->tcp_ip != NULL)
			m_contacts[newname]->tcp_ip = m_contacts[name]->tcp_ip;
		if (m_contacts[name]->tcp_port != 0)
			m_contacts[newname]->tcp_port = m_contacts[name]->tcp_port;
		if (m_contacts[name]->tcp_buffer.compare("") != 0)
			m_contacts[newname]->tcp_buffer = m_contacts[name]->tcp_buffer;
		if (m_contacts[name]->tcp_buffer_send.compare("") != 0)
			m_contacts[newname]->tcp_buffer_send = m_contacts[name]->tcp_buffer_send;
		if (m_contacts[name]->tcp_socket != NULL)
			m_contacts[newname]->tcp_socket = m_contacts[name]->tcp_socket;
		if (m_contacts[name]->video != NULL)
			m_contacts[newname]->video = m_contacts[name]->video;
		if (m_contacts[name]->streamer != NULL)
			m_contacts[newname]->streamer = m_contacts[name]->streamer;

		/* two tcp-connections to same user?
			Keep connection, where the user first in alphabetical order
			has the smallest port-number
		*/
		/*
		cout << "Socket of " << name << ": " << m_contacts[name]->tcp_socket << endl;
		cout << "Socketdata: local: " << m_contacts[name]->tcp_socket->localPort() << " peer: " << m_contacts[name]->tcp_socket->peerPort() << endl;
		cout << "Socket of " << newname << ": " << m_contacts[newname]->tcp_socket << endl;
		cout << "Socketdata: local: " << m_contacts[newname]->tcp_socket->localPort() << " peer: " << m_contacts[newname]->tcp_socket->peerPort() << endl;
*/
		if (m_contacts[name]->tcp_socket != NULL) {
			if (m_contacts[newname]->tcp_socket != NULL) {
				if (newname < m_supervisor->getMe()->getName()) {
					// other is first
					if (m_contacts[name]->tcp_socket->peerPort() < m_contacts[newname]->tcp_socket->peerPort()) {
						m_contacts[newname]->tcp_socket = m_contacts[name]->tcp_socket;
					}
				} else {
					// I'm first
					if (m_contacts[name]->tcp_socket->localPort() < m_contacts[newname]->tcp_socket->localPort()) {
						m_contacts[newname]->tcp_socket = m_contacts[name]->tcp_socket;
					}
				}
			} else {
				m_contacts[newname]->tcp_socket = m_contacts[name]->tcp_socket;
			}
		}

		m_contacts.erase(name);
		m_mutex->unlock();
		cout << "ContactHandler: Newname already exists: " << newname << endl;
		return false;
	}

	// copy & delete
	addContact(newname, false);
	m_mutex->lock();
	m_contacts[newname] = m_contacts[name];
	m_contacts.erase(name);
	m_contacts[newname]->name = newname;
	m_mutex->unlock();

	hasData(newname);
	cout << "ContactHandler: Changed name from: " << name << " to " << newname << endl;
cout << "Socket of user: " << m_contacts[newname]->tcp_socket << endl;
	return true;
}

// setter
void ContactHandler::setUdpIp(string name, QHostAddress* ip) {
	if (!isName(name)) return;
	m_mutex->lock();
	m_contacts[name]->udp_ip = ip;
	m_mutex->unlock();
	hasData(name);
}
void ContactHandler::setUdpPort(string name, quint16 port) {
	if (!isName(name)) return;
	m_mutex->lock();
	m_contacts[name]->udp_port = port;
	m_mutex->unlock();
	hasData(name);
}
void ContactHandler::setImg(string name, quint16 width, quint16 height) {
	if (!isName(name)) return;
	m_mutex->lock();
	m_contacts[name]->img_width = width;
	m_contacts[name]->img_height = height;
	m_mutex->unlock();
	hasData(name);
}
void ContactHandler::setWidth(string name, quint16 width) {
	if (!isName(name)) return;
	m_mutex->lock();
	m_contacts[name]->img_width = width;
	m_mutex->unlock();
	hasData(name);
}
void ContactHandler::setHeight(string name, quint16 height) {
	if (!isName(name)) return;
	m_mutex->lock();
	m_contacts[name]->img_height = height;
	m_mutex->unlock();
	hasData(name);
}
void ContactHandler::setTcpIp(string name, QHostAddress* ip) {
	if (!isName(name)) return;
	m_mutex->lock();
	m_contacts[name]->tcp_ip = ip;
	m_mutex->unlock();
}
void ContactHandler::setTcpPort(string name, quint16 port) {
	if (!isName(name)) return;
	m_mutex->lock();
	m_contacts[name]->tcp_port = port;
	m_mutex->unlock();
}
void ContactHandler::setTcpSocket(string name, QTcpSocket* mysocket) {
	if (!isName(name)) return;
	m_mutex->lock();
	m_contacts[name]->tcp_socket = mysocket;
	m_mutex->unlock();
}
void ContactHandler::addTcpBufferSend(string name, string buffer) {
	if (!isName(name)) return;
	m_mutex->lock();
	stringstream ss(m_contacts[name]->tcp_buffer_send);
	ss << buffer;
	m_contacts[name]->tcp_buffer_send = ss.str();
	m_mutex->unlock();
}

// getter
QHostAddress* ContactHandler::getUdpIp(string name) {
	if (!isName(name)) return NULL;
	m_mutex->lock();
	QHostAddress* output = m_contacts[name]->udp_ip;
	m_mutex->unlock();
	return output;
}
quint16 ContactHandler::getUdpPort(string name) {
	if (!isName(name)) return 0;
	m_mutex->lock();
	quint16 output = m_contacts[name]->udp_port;
	m_mutex->unlock();
	return output;
}
quint16 ContactHandler::getImgWidth(string name) {
	if (!isName(name)) return 0;
	m_mutex->lock();
	quint16 output = m_contacts[name]->img_width;
	m_mutex->unlock();
	return output;
}
quint16 ContactHandler::getImgHeight(string name) {
	if (!isName(name)) return 0;
	m_mutex->lock();
	quint16 output = m_contacts[name]->img_height;
	m_mutex->unlock();
	return output;
}
QHostAddress* ContactHandler::getTcpIp(string name) {
	if (!isName(name)) return NULL;
	m_mutex->lock();
	QHostAddress* output = m_contacts[name]->tcp_ip;
	m_mutex->unlock();
	return output;
}
quint16 ContactHandler::getTcpPort(string name) {
	if (!isName(name)) return 0;
	m_mutex->lock();
	quint16 output = m_contacts[name]->tcp_port;
	m_mutex->unlock();
	return output;
}
QTcpSocket* ContactHandler::getTcpSocket(string name) {
	if (!isName(name) || !isTcpContact(name)) return NULL;
	m_mutex->lock();

	// return socket
	QTcpSocket* output = m_contacts[name]->tcp_socket;
	m_mutex->unlock();
	return output;
}
string ContactHandler::getTcpBufferSend(std::string name) {
	if (!isName(name)) return "";
	m_mutex->lock();
	string output = m_contacts[name]->tcp_buffer_send;
	m_contacts[name]->tcp_buffer_send = "";
	m_mutex->unlock();
	return output;
}

/* has data of contact
  */
bool ContactHandler::hasData (string name) {
	m_mutex->lock();
	if (!m_contacts[name]
			|| m_contacts[name]->udp_ip == NULL
			|| m_contacts[name]->udp_port == 0
			|| m_contacts[name]->img_width == 0
			|| m_contacts[name]->img_height == 0
	) {
		m_mutex->unlock();
		return false;
	}
	m_mutex->unlock();

	// emit signal
	emit s_isData(name);

	return true;
}

/* get streamer object of person
  */
FuGaStreamer* ContactHandler::getStreamer (string name) {
	if (!isName(name)) return NULL;

	// already exists?
	m_mutex->lock();
	if (m_contacts[name]->streamer != NULL) {
		m_mutex->unlock();
		return m_contacts[name]->streamer;
	}
	m_mutex->unlock();

	// has enough data?
	if (!hasData(name)) return NULL;

	// create new streamer
	m_mutex->lock();
	m_contacts[name]->streamer = new FuGaStreamer(
		m_contacts[name]->udp_ip,
		m_supervisor->getConfig()->getInt("udp_port"),
		m_supervisor->getConfig()->getInt("img_width"),
		m_supervisor->getConfig()->getInt("img_height"),
		m_supervisor->getConfig()->getConfig("video_path")
	);
	m_mutex->unlock();

	// emit signal
	emit s_isStreaming(name);

	return m_contacts[name]->streamer;
}

/* delete streamer object of person
  */
void ContactHandler::rmStreamer (string name) {
	if (!isName(name)) return;

	m_mutex->lock();
	if (m_contacts[name]->streamer != NULL) {
		delete m_contacts[name]->streamer;
		m_contacts[name]->streamer = NULL;
	}
	m_mutex->unlock();
}

/* get video object of person
  */
FuGaVideo* ContactHandler::getVideo (string name) {
	if (!isName(name)) return NULL;

	// already exists?
	m_mutex->lock();
	if (m_contacts[name]->video != NULL) {
		m_mutex->unlock();
		return m_contacts[name]->video;
	}
	m_mutex->unlock();

	// has enough data?
	if (!hasData(name)) return NULL;

	// create new streamer
	m_mutex->lock();
	m_contacts[name]->video = new FuGaVideo(
		m_contacts[name]->udp_ip,
		m_contacts[name]->udp_port
	);
	m_mutex->unlock();

	return m_contacts[name]->video;
}

/* delete video object of person
  */
void ContactHandler::rmVideo (string name) {
	if (!isName(name)) return;

	m_mutex->lock();
	if (m_contacts[name]->video != NULL) {
		delete m_contacts[name]->video;
		m_contacts[name]->video = NULL;
	}
	m_mutex->unlock();
}

/* check, if contact name exists
 */
bool ContactHandler::isName (string name) {
	bool output = false;
	m_mutex->lock();
	if (m_contacts.count(name) == 1) output = true;
	m_mutex->unlock();
	return output;
}

/* return true, if connection established
 */
bool ContactHandler::isTcpContact (string name, bool socketOnly) {
	if (!isName(name)) return false;
	bool output = false;
	m_mutex->lock();
	if (m_contacts[name]->tcp_socket != NULL) {
		if (socketOnly) {
			output = true;
		} else {
			if (m_contacts[name]->tcp_socket->state() == QAbstractSocket::ConnectedState)
				output = true;
		}
	}
	m_mutex->unlock();

	return output;
}
