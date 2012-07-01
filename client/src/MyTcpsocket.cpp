#include "MyTcpsocket.h"
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

/* constructor
 * @param Supervisor*: Supervisor-object
 */
MyTcpsocket::MyTcpsocket (Supervisor* mysupervisor) {

	// save input
	m_supervisor = mysupervisor;

	// startup server
	m_server = new QTcpServer();
	//m_server = new QSslSocket();
	QHostAddress myip = QHostAddress(m_supervisor->getConfig()->getConfig("tcp_ip").c_str());
	quint16 myport = m_supervisor->getConfig()->getInt("tcp_port");
	if (!m_server->listen(myip, myport)) {
		cout << "Could not start server!" << endl;
	}
	connect(m_server, SIGNAL(newConnection()), this, SLOT(addPendingContact()));
	cout << "Started local server on ip " << m_server->serverAddress().toString().toAscii().data() << " port: " << m_server->serverPort() << endl;

	// create signalmappers
	m_signalmapper = new QSignalMapper(this);
	connect(m_signalmapper, SIGNAL(mapped(QString)), this, SLOT(getAnswer(QString)));
	m_signalmapper_connect = new QSignalMapper(this);
	connect(m_signalmapper_connect, SIGNAL(mapped(QString)), this, SLOT(slot_connected(QString)));

	// add root to contacts
	m_rootname = "root";
	m_supervisor->getContactHandler()->addContact(m_rootname);
}

/* handle connection-errors
 */
void MyTcpsocket::handleError(QAbstractSocket::SocketError myerror) {
	cout << "MyTcpSocket: Connection error:" << myerror << endl;
}

/* ######################### contact handling ############################## */

/* add user to contact (passive by external request to server)
 */
void MyTcpsocket::addPendingContact () {
	cout << "Someone connected to me!" << endl;

	// get free random name (unknown_number)
	string newname_preffix = "unknown_";
	string chartable = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	string newname;
	quint32 counter = 0;
	while (1) {
		stringstream ss("");
		ss << newname_preffix.c_str() << counter;
		if (!m_supervisor->getContactHandler()->isName(ss.str())) {
			newname = ss.str();
			break;
		}
		counter++;
		if (counter > 1000) {
			cout << "MyTcpsocket: Too many new connections!" << endl;
			exit(1);
		}
	}

	// add as new contact
	m_supervisor->getContactHandler()->addContact(newname);
	setSocket(newname, m_server->nextPendingConnection());

	// request infos for new connection
	send_requestinfos(newname);
}

/* get tcp-socket to contact
 */
QTcpSocket* MyTcpsocket::getSocket(std::string name) {
	if (!m_supervisor->getContactHandler()->isName(name)) return NULL;
	cout << "MyTcpsocket: Try to get socket for " << name << "..." << endl;
	// create new, if not exists
	QTcpSocket* mysocket = m_supervisor->getContactHandler()->getTcpSocket(name);
	if (mysocket == NULL) {
		cout << "MyTcpsocket: No existing socket found!" << endl;

		// get connection data
		if (name.compare(m_rootname) == 0) {
			m_supervisor->getContactHandler()->setTcpIp(name, new QHostAddress(m_supervisor->getConfig()->getConfig("root_ip").c_str()));
			m_supervisor->getContactHandler()->setTcpPort(name, m_supervisor->getConfig()->getInt("root_port"));
		} else {
			// try to get connection-data
			if (m_supervisor->getContactHandler()->getTcpIp(name) == NULL
					|| m_supervisor->getContactHandler()->getTcpPort(name) == 0) {
				send_name2tcp(name);
				return NULL;
			}
		}

		// start new socket
		mysocket = new QTcpSocket(0);
		QHostAddress* tcpip = m_supervisor->getContactHandler()->getTcpIp(name);
		quint16 tcport = m_supervisor->getContactHandler()->getTcpPort(name);
		mysocket->connectToHost(*tcpip, tcport);
		cout << "MyTcpsocket: Connection details for " << name << ": ip: " << tcpip->toString().toAscii().data() << " port: " << tcport << endl;
		setSocket(name, mysocket);
	}

	return m_supervisor->getContactHandler()->getTcpSocket(name);
}

/* set tcp-socket to contact
 */
void MyTcpsocket::setSocket(std::string name, QTcpSocket* mysocket) {
	if (!m_supervisor->getContactHandler()->isName(name)) return;
	m_supervisor->getContactHandler()->setTcpSocket(name, mysocket);

	// connect signals
	connect(mysocket, SIGNAL(error(QAbstractSocket::SocketError)),
		m_supervisor->getTcp(), SLOT(handleError(QAbstractSocket::SocketError)));
	connect(mysocket, SIGNAL(disconnected()), mysocket, SLOT(deleteLater()));
	connect(mysocket, SIGNAL(readyRead()), m_signalmapper, SLOT (map()));
	m_signalmapper->setMapping(mysocket, QString(tr(name.c_str())));
	connect(mysocket, SIGNAL(connected()), m_signalmapper_connect, SLOT (map()));
	m_signalmapper_connect->setMapping(mysocket, QString(tr(name.c_str())));
}

/* ######################### receiving ##################################### */

/* receive answer from all connections
 */
void MyTcpsocket::getAnswer(QString inname) {
	cout << "Got answer!" << endl;
	string name = inname.toAscii().data();
	QTcpSocket* mysocket = m_supervisor->getContactHandler()->getTcpSocket(name);
	cout << "Got answer..." << endl;
	// read from socket
	if (mysocket == NULL || !mysocket->bytesAvailable()) return;
	QByteArray* currbuffer = new QByteArray();
	currbuffer->append(mysocket->readAll());

	// is ready message?
	string inputstring = currbuffer->data();
	vector<string> msgs = split(inputstring, ";");

	// add last (incomplete) message to buffer
	//	if (current->buffer == NULL) current->buffer = new QByteArray();
	//	current->buffer = new QByteArray(msgs.back().c_str());
	//	msgs.pop_back();
	cout << "MyTcpsocket: input (" << msgs.size() << "): " << inputstring << endl;

	// iterate over all messages
	for (vector<string>::iterator i = msgs.begin(); i != msgs.end(); ++i) {

		// split type and data
		vector<string> parts = split(*i, "-");
		if (parts.size() != 2) {
			// invalid message!
            cout << "MyTcpsocket: Invalid message received! (" << parts.size() << ")" << endl;
			continue;
		}
		string type = parts[0];
		string data = parts[1];

		// handle input
		if (type.compare("r_getinfo") == 0) {
			cout << "MyTcpsocket: Infos requested" << endl;

			// build info-package
			stringstream sss("");
			sss << "a_getinfo-";
			sss << "name:" << m_supervisor->getMe()->getName() << "|";
			sss << "udp_ip:" << m_supervisor->getConfig()->getConfig("udp_ip") << "|";
			sss << "udp_port:" << m_supervisor->getConfig()->getConfig("udp_port") << "|";
			sss << "img_width:" << m_supervisor->getConfig()->getConfig("img_width") << "|";
			sss << "img_height:" << m_supervisor->getConfig()->getConfig("img_height") << "|";
			sss << ";";

			sendTo(name, sss.str());
			break;

		// got infos
		} else if (type.compare("a_getinfo") == 0) {

			// split infos
			vector<string> configs = split(data, "|");
			if (!m_supervisor->getContactHandler()->isName(name)) break;
			for (vector<string>::iterator ii = configs.begin(); ii != configs.end(); ++ii) {
				vector<string> config_values = split(*ii, ":");
				if (config_values.size() != 2) continue;

				// got real name of contact?
				if (config_values[0].compare("name") == 0) {
					m_supervisor->getContactHandler()->changeName(name, config_values[1]);
					name = config_values[1];
				}

				// save infos in contactHandler
				if (config_values[0].compare("udp_ip") == 0)
					m_supervisor->getContactHandler()->setUdpIp(name, new QHostAddress(config_values[1].c_str()));
				if (config_values[0].compare("udp_port") == 0)
					m_supervisor->getContactHandler()->setUdpPort(name, string2int(config_values[1]));
				if (config_values[0].compare("img_width") == 0)
					m_supervisor->getContactHandler()->setWidth(name, string2int(config_values[1]));
				if (config_values[0].compare("img_height") == 0)
					m_supervisor->getContactHandler()->setHeight(name, string2int(config_values[1]));
			}

			cout << "MyTcpsocket: Added infos to contacthandler..." << endl;

		// login answers
		} else if (type.compare("a_login_ok") == 0) {

			// publish tcp-connection
			stringstream myss("");
			myss << "a_mytcp-" << m_supervisor->getConfig()->getConfig("tcp_ip")
			   << "," << m_supervisor->getConfig()->getConfig("tcp_port") << ";";
			sendTo(name, myss.str());

			m_supervisor->getMe()->finishLogin();
		} else if (type.compare("a_login_failed") == 0) {
			m_supervisor->getMe()->failLogin();

		} else if (type.compare("a_add_ok") == 0) {
				cout << " add okkk" << endl;
				m_supervisor->getMe()->finishRegist();

		} else if (type.compare("a_add_failed") == 0) {
				m_supervisor->getMe()->failRegist();

		} else if (type.compare("a_name2tcp") == 0) {
			vector<string> mydata = split(data, ",");
			if (mydata.size() == 3) {
				m_supervisor->getContactHandler()->setTcpIp(mydata[0], new QHostAddress(mydata[1].c_str()));
				m_supervisor->getContactHandler()->setTcpPort(mydata[0], string2int(mydata[2]));
			}
			if (mydata.size() > 0) send_buffer(mydata[0]);

		// misc
		} else if (type.compare("a_bad_request") == 0) {
			cout << "Someone accused me of sending a bad request???" << endl;
		} else {

			// split type and data
			vector<string> myparts = split(type, "_");
			if (myparts.size() < 2) {
				cout << "MyTcpsocket: Aliens contacted me! Received an unknown message!" << endl;
				continue;
			}
			emit newMsg(parts[1], type, data);
		}
	}
}

/* this slot is called, when connection is established
 */
void MyTcpsocket::slot_connected(QString inname) {
	send_buffer(inname.toAscii().data());
}

/* send buffered items
 */
void MyTcpsocket::send_buffer(string name) {
	cout << "Send buffer..." << endl;
	string buffer = m_supervisor->getContactHandler()->getTcpBufferSend(name);
	if (!buffer.empty()) sendTo(name, buffer);
}

/* ######################### sending ####################################### */

/* send message to user
 * @param string: name of user to get infos from
 * @param string: message to send
 */
void MyTcpsocket::sendTo(string name, string msg) {
	QTcpSocket* mysocket = getSocket(name);
	if (mysocket == NULL) {
		m_supervisor->getContactHandler()->addTcpBufferSend(name, msg);
		cout << "MyTcpsocket: sendTo failed: Unknown addressee or no socket: " << name << endl;
		return;
	}
	if (!mysocket->write(msg.c_str())) {
		cout << "MyTcpsocket: sendTo failed: " << mysocket->errorString().toAscii().data() << endl;
	}
	cout << "MyTcpSocket: sent to " << name << ": " << msg << endl;
}

/* request infos for user
 * @param char*: name of user to get infos from
 */
void MyTcpsocket::send_requestinfos (string name) {
	stringstream ss("");
	ss << "r_getinfo-" << name << ";";
	sendTo(name, ss.str());
}

/* verify login
 * @param string: name of user
 * @param string: password of user
 */
void MyTcpsocket::doLogin (string name, string password) {
	stringstream ss("");
	ss << "r_login-" << name << "," << password << ";";
	sendTo(m_rootname, ss.str());
}

void MyTcpsocket::add_usr (string name, string password) {
		cout << "tcpsocket: add_usr " << endl;
		stringstream ss("");
		ss << "r_add_usr-" << name << "," << password << ";";
		sendTo(m_rootname, ss.str());
}

/* request for tcp-connection details of other person
 * @param string: name of partner
 */
void MyTcpsocket::send_name2tcp (string name) {
	stringstream ss("");
	ss << "r_name2tcp-" << name << ";";
	sendTo(m_rootname, ss.str());
}

/* ######################### misc ########################################## */

/* join to char-arrays
 * @param string: first array to merge
 * @param string: next array to merge
 */
string MyTcpsocket::merge (string array1, string array2) {
	string s;
	ostringstream ss(s);
	ss << array1;
	ss << array2;
	return s;
}

/* split string by "delim"
 * @param string: string to split
 * @param char: delimiter
 */
vector<string> MyTcpsocket::split(string s, string delim) {
	vector<string> elems;
	stringstream ss(s);
	string item;
	char delimiter = *delim.data();
	while(std::getline(ss, item, delimiter)) {
		elems.push_back(item);
	}
	return elems;
}

/* convert string to int
 * @param string: string to convert
 */
int MyTcpsocket::string2int (string s) {
	stringstream ss(s);
	int i;
	if( (ss >> i).fail() ) {
		// conversion failed
		return 0;
	}
	return i;
}

/* convert string to int
 * @param string: string to convert
 */
int MyTcpsocket::char2int (char* s) {
	stringstream ss(s);
	int i;
	if( (ss >> i).fail() ) {
		// conversion failed
		return 0;
	}
	return i;
}

/* convert int to string
 * @param int: int to convert
 */
string MyTcpsocket::int2string (int i) {
	string s;
	stringstream ss(s);
	if( (ss << i).fail() ) {
		// conversion failed
		return "";
	}
	return s;
}
