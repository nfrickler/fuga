#include "ContactHandler.h"
#include <iostream>

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
		if (loadInfos && !isVideoReady(name)) m_supervisor->getTcp()->send_requestinfos(name);
		return;
	}
	m_mutex->lock();

	// create new contact
	m_contacts[name] = new contact;
	m_contacts[name]->name = name;
	m_contacts[name]->video = NULL;
	m_contacts[name]->udp_ip = NULL;
	m_contacts[name]->udp_port = 0;
	m_contacts[name]->img_width = 0;
	m_contacts[name]->img_height = 0;
	m_contacts[name]->video_started = false;
	m_contacts[name]->tcp_ip = NULL;
	m_contacts[name]->tcp_port = 0;
	m_contacts[name]->tcp_socket = NULL;
	m_contacts[name]->tcp_buffer = "";
	m_contacts[name]->tcp_buffer_send = "";

	m_mutex->unlock();
	if (loadInfos && !isVideoReady(name)) m_supervisor->getTcp()->send_requestinfos(name);
	cout << "ContactHandler: Added contact: " << name << endl;
}
void ContactHandler::removeContact(string name) {
	stopVideo(name);
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
		if (m_contacts[name]->video_started != false)
			m_contacts[newname]->video_started = m_contacts[name]->video_started;
		if (m_contacts[name]->video != NULL)
			m_contacts[newname]->video = m_contacts[name]->video;

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
	isVideoReady(name);
}
void ContactHandler::setUdpPort(string name, quint16 port) {
	if (!isName(name)) return;
	m_mutex->lock();
	m_contacts[name]->udp_port = port;
	m_mutex->unlock();
	isVideoReady(name);
}
void ContactHandler::setImg(string name, quint16 width, quint16 height) {
	if (!isName(name)) return;
	m_mutex->lock();
	m_contacts[name]->img_width = width;
	m_contacts[name]->img_height = height;
	m_mutex->unlock();
	isVideoReady(name);
}
void ContactHandler::setWidth(string name, quint16 width) {
	if (!isName(name)) return;
	m_mutex->lock();
	m_contacts[name]->img_width = width;
	m_mutex->unlock();
	isVideoReady(name);
}
void ContactHandler::setHeight(string name, quint16 height) {
	if (!isName(name)) return;
	m_mutex->lock();
	m_contacts[name]->img_height = height;
	m_mutex->unlock();
	isVideoReady(name);
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

// handle video
MyVideo* ContactHandler::startVideo(string name) {
	if (!isVideoReady(name)) return NULL;
	m_mutex->lock();

	if (m_contacts[name]->video == NULL) {
		m_contacts[name]->video = new MyVideo(	m_contacts[name]->udp_ip,
												m_contacts[name]->udp_port,
												m_contacts[name]->img_width,
												m_contacts[name]->img_height);
	}
	MyVideo* output = m_contacts[name]->video;

	m_mutex->unlock();
	return output;
}
void ContactHandler::stopVideo (string name) {
	m_mutex->lock();
	delete m_contacts[name]->video;
	m_contacts[name]->video = NULL;
	m_contacts[name]->video_started = false;
	m_mutex->unlock();
}
bool ContactHandler::isVideo (string name) {
	m_mutex->lock();
	bool output = m_contacts[name]->video_started;
	m_mutex->unlock();
	return output;
}

bool ContactHandler::isVideoReady (string name) {
	m_mutex->lock();
	bool output = false;
	if (m_contacts[name]
			&& m_contacts[name]->udp_ip != NULL
			&& m_contacts[name]->udp_port != 0
			&& m_contacts[name]->img_width != 0
			&& m_contacts[name]->img_height != 0
	) {
		output = true;
	}
	m_mutex->unlock();

	if (output && !isVideo(name)) {
		m_mutex->lock();
		m_contacts[name]->video_started = true;
		m_mutex->unlock();
		cout << "ContactHandler: Video ready for contact: " << name << endl;
	}
	if (output) emit videoReady(name);
	return output;
}

/* check, if contact name exists
 */
bool ContactHandler::isName (string name) {
	bool output = false;
	m_mutex->lock();
	if (m_contacts.count(name) == 1) output = true;
	m_mutex->unlock();
	//cout << "ContactHandler: XXXXX user: " << name << " = " << output << " XXXXX" << endl;
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
