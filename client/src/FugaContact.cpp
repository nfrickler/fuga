#include "FugaContact.h"
#include "Fuga.h"
#include "FugaStreamer.h"
#include "FugaVideo.h"

using namespace std;

// constructors
FugaContact::FugaContact(Fuga* in_Fuga) {
    init(in_Fuga);
}
FugaContact::FugaContact(Fuga* in_Fuga, std::string in_name) {
    init(in_Fuga);
    m_name = in_name;
    m_isaccepted = true;
    doResolve();
}
FugaContact::FugaContact(Fuga* in_Fuga, QSslSocket* in_socket) {
    init(in_Fuga);
    m_socket = in_socket;
    connectSocket();
    m_socket->startServerEncryption();
}

// init
void FugaContact::init(Fuga* in_Fuga) {
    m_name = "";
    m_Fuga = in_Fuga;
    m_socket = NULL;
    m_isaccepted = false;
    m_tcp_ip = NULL;
    m_tcp_port = 0;
    m_udp_ip = NULL;
    m_udp_firstport = 0;
    m_Streamer = NULL;
    m_verified_active = false;
    m_verified_passive = false;
}

// connect signals to socket
void FugaContact::connectSocket() {
    if (m_socket == NULL) return;

    // startup
    connect(m_socket, SIGNAL(encrypted()), this, SLOT(slot_connected()),Qt::UniqueConnection);
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(slot_received()),Qt::UniqueConnection);

    // errors and disconnection
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(slot_disconnected()),Qt::UniqueConnection);
    connect(this, SIGNAL(sig_disconnected()), m_socket, SLOT(deleteLater()),Qt::UniqueConnection);
    connect(m_socket, SIGNAL(sslErrors(const QList<QSslError> &)),
            this, SLOT(slot_sslerror(const QList<QSslError> &)));
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)),
        this, SLOT(slot_handleError(QAbstractSocket::SocketError)),Qt::UniqueConnection);

    // receive requests
    connect(this, SIGNAL(sig_received(std::string,std::vector<std::string>)),
            this, SLOT(slot_gotRequest(std::string,std::vector<std::string>)),Qt::UniqueConnection);
}

// ############################# accessors ########################

std::string FugaContact::name() {
    return m_name;
}

// ############################# streaming ########################

// start streaming
bool FugaContact::startStreaming() {

    // object accepted?
    if (!isAccepted() || !isHello()) return false;

    // create new streamer
    if (m_Streamer == NULL) {
        m_Streamer = new FugaStreamer(
            new QHostAddress(m_Fuga->getConfig()->getConfig("udp_ip").c_str()),
            m_Fuga->getConfig()->getInt("udp_firstport"),
            m_Fuga->getConfig()->getConfig("video_path")
        );
    }
    m_Streamer->start();

    // emit signal
    emit sig_streaming();

    return true;
}

// stop streaming
bool FugaContact::stopStreaming() {
    if (m_Streamer == NULL) return true;
    delete m_Streamer;
    m_Streamer = NULL;
    return true;
}

// get video object
FugaVideo* FugaContact::Video() {

    // object accepted?
    if (!isAccepted() || !isHello()) return NULL;

    // create new streamer
    FugaVideo* video = new FugaVideo(
        m_udp_ip,
        m_udp_firstport
    );

    return video;
}

// ############################# status ########################

// resolve username
void FugaContact::doResolve() {
    if (isResolved()) return;
    FugaDns* Dns = m_Fuga->getContacts()->getDns();
    connect(Dns, SIGNAL(sig_resolved(std::string,QHostAddress*,quint16)),
            this, SLOT(slot_resolved(std::string,QHostAddress*,quint16)),Qt::UniqueConnection);
    Dns->resolve(m_name);
}

// we have got the dns data
void FugaContact::slot_resolved(std::string in_name, QHostAddress* in_ip, quint16 in_port) {
    if (in_name.compare(m_name) != 0) return;
    cout << "FugaContact: slot_resolved: " << m_name << endl;

    // disconnect
    FugaContact* Dns = m_Fuga->getContacts()->getDns();
    disconnect(Dns, SIGNAL(sig_resolved(std::string,QHostAddress*,quint16)),
            this, SLOT(slot_resolved(std::string,QHostAddress*,quint16)));

    // save infos
    m_tcp_ip = in_ip;
    m_tcp_port = in_port;

    doConnect();
}

// is username resolved?
bool FugaContact::isResolved() {
    return (m_tcp_ip == NULL) ? false : true;
}

// connect to other client
void FugaContact::doConnect() {

    // username already resolved?
    if (!isResolved()) {
        doResolve();
        return;
    }

    // start new socket
    cout << "FugaContact: Start socket to: "
         << m_tcp_ip->toString().toAscii().data() << " : " << m_tcp_port << endl;
    m_socket = new QSslSocket(0);
    connectSocket();
    m_socket->connectToHostEncrypted(m_tcp_ip->toString(), m_tcp_port);
}

// we are connected
void FugaContact::slot_connected() {
    cout << "FugaContact: connection established" << endl;
    disconnect(m_socket, SIGNAL(encrypted()),
               this, SLOT(slot_connected()));
    emit sig_connected();
    doHello();
}

// are we connected to other client
bool FugaContact::isConnected() {
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        return true;
    }
    return false;
}

// fetch udp connection data of other client
void FugaContact::doHello() {

    // is it my turn to send r_hello?
    if (m_name == "") return;
    cout << "FugaContact: Send hello" << endl;

    // send r_hello
    connect(this, SIGNAL(sig_received(std::string,std::vector<std::string>)),
            this, SLOT(slot_hello(std::string,std::vector<std::string>)),Qt::UniqueConnection);
    stringstream ss("");
    QHostAddress udpip = QHostAddress(m_Fuga->getConfig()->getConfig("udp_ip").c_str());
    quint16 udpport = m_Fuga->getConfig()->getInt("udp_firstport");
    ss << "r_hello-" << m_Fuga->getMe()->name() << ","
       << udpip.toString().toAscii().data() << "," << udpport << ";";
    send_direct(ss.str());
}

// receive answer to hello request
void FugaContact::slot_hello(string in_type, vector<string> in_data) {

    // hello success
    if (in_type == "a_hello_ok") {
        if (m_name.empty()) m_name = in_data[0];
        m_udp_ip = new QHostAddress(in_data[1].c_str());
        m_udp_firstport = string2quint16(in_data[2]);
        disconnect(this, SIGNAL(sig_received(std::string,std::vector<std::string>)),
                   this, SLOT(slot_hello(std::string,std::vector<std::string>)));
        doVerify();
        return;
    }

    // hello failed
    if (in_type == "a_hello_failed") {
        showError("Hello failed!");
    }
}

// do we have fetched the udp data of other client?
bool FugaContact::isHello() {
    return (m_udp_firstport == 0) ? false : true;
}

// fetch udp connection data of other client
void FugaContact::doVerify() {
    connect(this, SIGNAL(sig_received(std::string,std::vector<std::string>)),
            this, SLOT(slot_verify(std::string,std::vector<std::string>)),Qt::UniqueConnection);

    // get signature
    stringstream ss("");
    FugaCrypto* Crypto = m_Fuga->getContacts()->getCrypto();
    std::string name = m_Fuga->getMe()->name();
    int timestamp = QDateTime::currentDateTime().toTime_t();

    stringstream msg("");
    msg << name << "_" << timestamp << "_" << m_name;

    ss << "r_verify-" << timestamp << ","
       << Crypto->getPubkey() << ","
       << Crypto->sign(msg.str()) << ";";
    send_direct(ss.str());
}

// receive answer to verification request
void FugaContact::slot_verify(string in_type, vector<string> in_data) {

    // hello success
    if (in_type == "a_verify_ok") {
        disconnect(this, SIGNAL(sig_received(std::string,std::vector<std::string>)),
                   this, SLOT(slot_verify(std::string,std::vector<std::string>)));
        cout << "ACCEPTED BY OTHER" << endl;
        m_verified_passive = true;
        doAccept();
        return;
    }

    // hello failed
    if (in_type == "a_verify_failed") {
        showError("Verification failed!");
    }
}

// do we have fetched the udp data of other client?
bool FugaContact::isVerified() {
    return (m_verified_active && m_verified_passive) ? true : false;
}

// is this connection to the other client accepted?
void FugaContact::doAccept() {

    // verified?
    if (!isVerified()) return;

    if (!isAccepted()) {

        // connected to myself?
        cout << "names: " << m_name << " and " << m_Fuga->getMe()->name() << endl;
        if (m_name.compare(m_Fuga->getMe()->name()) == 0) {
            cout << "FugaContact: Connected with myself..." << endl;
            m_isaccepted = true;
            return;
        }

        if (!m_Fuga->getContacts()->registerContact(this)) {
            // another connection already exists!
            cout << "FugaContact: destroying myself" << endl;

            // move buffer to other Contact
            FugaContact* other = m_Fuga->getContacts()->getContact(m_name);
            other->send(m_buffer);

            m_socket->disconnectFromHost();
            delete this;
            return;
        }
    }

    cout << "FugaContact: Contact accepted" << endl;
    m_isaccepted = true;
    emit sig_accepted();
}

// we are accepted!
void FugaContact::slot_accepted() {
    sendBuffer();
}

// is this connection to the other client accepted?
bool FugaContact::isAccepted() {
    return m_isaccepted;
}

// do disconnect
void FugaContact::doDisconnect() {
    slot_disconnected();
}

// other client disconnected
void FugaContact::slot_disconnected() {
    stopStreaming();
    emit sig_disconnected();
}

// ############################# send data ########################

void FugaContact::send(string in_msg) {
    if (!isConnected()) {
        cout << "FugaContact: send failed: No socket to " << m_name << endl;
        addToBuffer(in_msg);
        doConnect();
        return;
    }
    if (!isAccepted()) {
        cout << "FugaContact: send failed; wait until accepted!" << endl;
        addToBuffer(in_msg);
        return;
    }

    send_direct(in_msg);
}

// actually send message
void FugaContact::send_direct(string in_msg) {
    if (!m_socket->write(in_msg.c_str())) {
        cout << "FugaContact: send failed: " << m_socket->errorString().toAscii().data() << endl;
    }
    cout << "FugaContact: sent to " << m_name << ": " << in_msg << endl;
}

void FugaContact::addToBuffer(std::string in_msg) {
    stringstream ss("");
    ss << m_buffer;
    ss << in_msg;
    m_buffer = ss.str();
}

void FugaContact::sendBuffer() {
    cout << "FugaContact: Sending buffer..." << endl;
    if (!m_buffer.empty()) send(m_buffer);
}

// ############################# receive data ########################

// slot to receive waiting data
void FugaContact::slot_received () {

    // read from socket
    if (m_socket == NULL || !m_socket->bytesAvailable()) return;
    QByteArray* currbuffer = new QByteArray();
    currbuffer->append(m_socket->readAll());

    // is ready message?
    string inputstring = currbuffer->data();
    vector<string> msgs = split(inputstring, ";");

    cout << "FugaContact: input (" << msgs.size() << "): " << inputstring << endl;

    // iterate over all messages
    for (vector<string>::iterator i = msgs.begin(); i != msgs.end(); ++i) {

        // split type and data
        string myinput = *i;
        int splitat = myinput.find("-");
        string type = myinput.substr(0,splitat);
        string data = myinput.substr((splitat+1));

        // split data into vector
        vector<string> splitted = split(data, ",");

        emit sig_received(type, splitted);
    }
}

// handle errors
void FugaContact::slot_handleError(QAbstractSocket::SocketError in_error) {
    cout << "FugaContact: Connection error:" << in_error
         << " string: " << m_socket->errorString().toAscii().data() << endl;
}

// handle ssl errors
void FugaContact::slot_sslerror(const QList<QSslError> &) {
    cout << "FugaContact: SSL error" << endl;
    m_socket->ignoreSslErrors();
}

// ########################## client - me #############################

void FugaContact::slot_gotRequest(string in_type,vector<string> in_data) {

    if (in_type == "r_hello") {
        stringstream ss("");
        FugaMe* Me = m_Fuga->getMe();
        if (in_data.size() != 3) {
            ss << "a_hello_failed-Invalid package;";
            send_direct(ss.str());
            return;
        }
        if (Me == NULL) {
            ss << "a_hello_failed-Not ready yet;";
            send_direct(ss.str());
            return;
        }

        // save data
        if (m_name.empty()) m_name = in_data[0];
        m_udp_ip = new QHostAddress(in_data[1].c_str());
        m_udp_firstport = string2quint16(in_data[2]);

        // send a_hello_ok
        QHostAddress udpip = QHostAddress(m_Fuga->getConfig()->getConfig("udp_ip").c_str());
        quint16 udpport = m_Fuga->getConfig()->getInt("udp_firstport");
        ss << "a_hello_ok-" << Me->name() << ","
           << udpip.toString().toAscii().data() << "," << udpport << ";";
        send_direct(ss.str());
        doVerify();
    }

    if (in_type == "r_verify") {
        stringstream ss("");
        FugaMe* Me = m_Fuga->getMe();
        if (in_data.size() != 3) {
            ss << "a_verify_failed-Invalid package;";
            send_direct(ss.str());
            return;
        }
        if (Me == NULL) {
            ss << "a_verify_failed-Not ready yet;";
            send_direct(ss.str());
            return;
        }

        // verify
        FugaCrypto* Crypto = m_Fuga->getContacts()->getCrypto();
        stringstream msg("");
        msg << m_name << "_" << in_data[0] << "_" << m_Fuga->getMe()->name();
        if (!Crypto->verify(m_name,in_data[1],msg.str(),in_data[2])) {
            ss << "a_verify_failed-Invalid signature;";
            send_direct(ss.str());
            return;
        }

        // verification ok
        ss << "a_verify_ok-;";
        send_direct(ss.str());
        cout << "I HAVE ACCEPTED" << endl;
        m_verified_active = true;
        doAccept();
    }
}
