#include "FugaContacts.h"

using namespace std;

/* constructor
 */
FugaContacts::FugaContacts (Fuga* in_Fuga) {
    m_Fuga = in_Fuga;
	m_mutex = new QMutex();

    // start server
    _startServer();

    // connect to root
    // TODO
}

// get Contact
FugaContact* FugaContacts::getContact(string in_name) {
    cout << "FugaContacts: Someone requested Contact " << in_name << endl;

    // create new contact
    if (!isContact(in_name)) m_contacts[in_name] = new FugaContact(m_Fuga, in_name);

    return m_contacts[in_name];
}

bool FugaContacts::isContact (string in_name) {
    map<string,FugaContact*>::const_iterator it = m_contacts.find(in_name);
    return it!=m_contacts.end();
}

// start own server
bool FugaContacts::_startServer() {

    // startup server
    m_Server = new QTcpServer();
    //m_Server = new QSslSocket();
    QHostAddress myip = QHostAddress(m_Fuga->getConfig()->getConfig("tcp_ip").c_str());
    quint16 myport = m_Fuga->getConfig()->getInt("tcp_port");
    if (!m_Server->listen(myip, myport)) {
        cout << "Could not start server!" << endl;
    }
    connect(m_Server, SIGNAL(newConnection()), this, SLOT(addPendingConnection()));
    cout << "FugaContacts: Started server on "
         << m_Server->serverAddress().toString().toAscii().data()
         << " : " << m_Server->serverPort() << endl;

    // create signalmappers
   // m_signalmapper = new QSignalMapper(this);
   // connect(m_signalmapper, SIGNAL(mapped(QString)), this, SLOT(getAnswer(QString)));
   // m_signalmapper_connect = new QSignalMapper(this);
   // connect(m_signalmapper_connect, SIGNAL(mapped(QString)), this, SLOT(slot_mode_login(QString)));

    return true;
}

// handle connection errors
void FugaContacts::handleError(QAbstractSocket::SocketError in_error) {
    cout << "FugaTcp: Connection error:" << in_error << endl;
}

// add pending connection
void FugaContacts::addPendingConnection () {
    cout << "Someone connected to me!" << endl;

    // add pending connection
    QTcpSocket* socket = m_Server->nextPendingConnection();
    m_pending[m_pending.size()] = m_Server->nextPendingConnection();

    // connect signals
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
        this, SLOT(handleError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
    connect(socket, SIGNAL(readyRead()), m_signalmapper, SLOT (map()));
 //   m_signalmapper->setMapping(socket, QString(tr(name.c_str())));
    connect(socket, SIGNAL(connected()), m_signalmapper_connect, SLOT (map()));
 //   m_signalmapper_connect->setMapping(mysocket, QString(tr(name.c_str())));

}

void FugaContacts::name2tcp (string in_name) {
    stringstream ss("");
    ss << "r_name2tcp-" << in_name << ";";
    string hello = ss.str();
    string name = "root";
    getContact(name)->send(hello);
}


