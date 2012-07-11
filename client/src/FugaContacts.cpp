#include "FugaContacts.h"
#include "Fuga.h"

using namespace std;

// constructor
FugaContacts::FugaContacts (Fuga* in_Fuga) {
    m_Fuga = in_Fuga;
    m_Dns = NULL;

    // start server
    startServer();

    // init crypto
    m_Crypto = new FugaCrypto(m_Fuga);

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
        connectContact(m_contacts[in_name]);
    }

    return m_contacts[in_name];
}

// connect Contact's signals to my slots
void FugaContacts::connectContact(FugaContact* in_Contact) {
    connect(in_Contact, SIGNAL(sig_accepted()),this,SLOT(slot_con_connected()));
    connect(in_Contact, SIGNAL(sig_disconnected()),this,SLOT(slot_con_disconnected()));
    connect(in_Contact,SIGNAL(sig_received(std::string,std::vector<std::string>)),
            this,SLOT(slot_con_received(std::string,std::vector<std::string>)));
}

// do we have contact to person xyz?
bool FugaContacts::isContact (string in_name) {
    map<string,FugaContact*>::const_iterator it = m_contacts.find(in_name);
    return it!=m_contacts.end();
}

// register new contact
bool FugaContacts::registerContact(FugaContact* in_Contact) {

    // user already exists?
    if (isContact(in_Contact->name())) {
        // Contact already exists!
        cout << "FugaContacts: Contact already exists => DELETE" << endl;
        return false;
    }

    // register
    m_contacts[in_Contact->name()] = in_Contact;

    return true;
}

// ########################### server ############################

// start own server
void FugaContacts::startServer() {
    m_Server = new FugaSslServer();
    quint16 myport = m_Fuga->getConfig()->getInt("tcp_port");
    m_Server->start("certs/server-cert.pem", "certs/server-key.enc", myport);
    connect(m_Server, SIGNAL(sig_newconnection(QSslSocket*)),
            this, SLOT(slot_addconnection(QSslSocket*)), Qt::UniqueConnection);
    cout << "FugaContacts: Started server on port " << m_Server->serverPort() << endl;
}

// add pending connection
void FugaContacts::slot_addconnection(QSslSocket* in_socket) {
    cout << "Someone connected to me using SSL!" << endl;
    FugaContact* newcontact = new FugaContact(m_Fuga, in_socket);
    connectContact(newcontact);
}

// ########################## dns ###########################

// get FugaDns object
FugaDns* FugaContacts::getDns() {
    if (m_Dns == NULL) m_Dns = new FugaDns(m_Fuga);
    return m_Dns;
}

// ####################### handle signals of Contacts ##################

void FugaContacts::slot_con_received(std::string in_type, std::vector<std::string> in_data) {
    FugaContact* sender = (FugaContact*) QObject::sender();
    if (!sender->isAccepted()) return;
    emit sig_received(sender->name(),in_type,in_data);
}
void FugaContacts::slot_con_connected() {
    cout << "FugaContacts: is connected" << endl;
    FugaContact* sender = (FugaContact*) QObject::sender();
    emit sig_connected(sender->name());
}
void FugaContacts::slot_con_disconnected() {
    FugaContact* sender = (FugaContact*) QObject::sender();
    emit sig_disconnected(sender->name());
    m_contacts.erase(sender->name());
}
