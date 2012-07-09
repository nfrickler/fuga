#include "FugaContacts.h"
#include "Fuga.h"

using namespace std;

// constructor
FugaContacts::FugaContacts (Fuga* in_Fuga) {
    m_Fuga = in_Fuga;
	m_mutex = new QMutex();
    m_Dns = NULL;

    // start server
    startServer();

    // connect to root
    getDns();
}

// ################### contact handling #########################

// get Contact
FugaContact* FugaContacts::getContact(string in_name) {
    cout << "FugaContacts: Someone requested Contact " << in_name << endl;

    // create new contact
    if (!isContact(in_name)) {
        cout << "FugaContacts: Create new Contact" << endl;
        m_contacts[in_name] = new FugaContact(m_Fuga, in_name);
    }

    return m_contacts[in_name];
}

// do we have contact to person xyz?
bool FugaContacts::isContact (string in_name) {
    map<string,FugaContact*>::const_iterator it = m_contacts.find(in_name);
    return it!=m_contacts.end();
}

// ########################### server ############################

// start own server
bool FugaContacts::startServer() {

    // startup server
    m_Server = new QTcpServer();
    //m_Server = new QSslSocket();
    QHostAddress myip = QHostAddress(m_Fuga->getConfig()->getConfig("tcp_ip").c_str());
    quint16 myport = m_Fuga->getConfig()->getInt("tcp_port");
    if (!m_Server->listen(myip, myport)) {
        cout << "Could not start server!" << endl;
    }
    connect(m_Server, SIGNAL(newConnection()), this, SLOT(addPendingConnection()), Qt::UniqueConnection);
    cout << "FugaContacts: Started server on "
         << m_Server->serverAddress().toString().toAscii().data()
         << " : " << m_Server->serverPort() << endl;

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
    FugaContact* newcontact = new FugaContact(m_Fuga, socket);
    connect(newcontact,SIGNAL(sig_hereiam(FugaContact*,std::string)),
            this,SLOT(slot_add_hereiam(FugaContact*,std::string)), Qt::UniqueConnection);
}

// add FugaContact to list
void FugaContacts::slot_add_hereiam(FugaContact* in_Contact,std::string in_name) {
    if (!isContact(in_name)) m_contacts[in_name] = in_Contact;
    // TODO: if it already exists...!
}

// handle errors
void FugaContacts::slot_handleError(QAbstractSocket::SocketError in_error) {
    cout << "FugaContact: Connection error:" << in_error << endl;
}

// ########################## misc ###########################

// get FugaDns object
FugaDns* FugaContacts::getDns() {
    if (m_Dns == NULL) m_Dns = new FugaDns(m_Fuga);
    return m_Dns;
}

// ######################### handle m_waiting ##################
