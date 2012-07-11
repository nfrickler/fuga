#include "FugaContact.h"
#include "Fuga.h"
#include "FugaStreamer.h"
#include "FugaVideo.h"

using namespace std;

FugaContact::FugaContact(Fuga* in_Fuga, std::string in_name)
    : m_Fuga(in_Fuga),
      m_name(in_name)
{
    m_isaccepted = true;
    m_socket = NULL;
    m_tcp_ip = NULL;
    m_tcp_port = 0;
    m_udp_ip = NULL;
    m_udp_firstport = 0;
    m_Streamer = NULL;

    if (in_name != "root") doResolve();
}

FugaContact::FugaContact(Fuga* in_Fuga, QSslSocket* in_socket)
    : m_Fuga(in_Fuga),
      m_socket(in_socket)
{
    m_isaccepted = false;
    m_tcp_ip = NULL;
    m_tcp_port = 0;
    m_udp_ip = NULL;
    m_udp_firstport = 0;
    m_Streamer = NULL;

    connectSocket();
    m_socket->startServerEncryption();
   // doFetch();
}

// connect signals to socket
void FugaContact::connectSocket() {
    if (m_socket == NULL) return;

    // startup
    connect(m_socket, SIGNAL(encrypted()), this, SLOT(slot_connected()),Qt::UniqueConnection);
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(slot_received()),Qt::UniqueConnection);

    // errors and disconnection
    connect(m_socket, SIGNAL(disconnected()), m_socket, SLOT(deleteLater()),Qt::UniqueConnection);
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
    if (!isAccepted() || !isFetched()) return false;

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
    if (!isAccepted() || !isFetched()) return NULL;

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
    if (in_name != m_name) return;
    cout << "FugaContact: slot_resolved " << m_name << endl;

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
    cout << "FugaContact: slot_connected to " << m_name << endl;
    emit sig_connected();
    doFetch();
}

// are we connected to other client
bool FugaContact::isConnected() {
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        return true;
    }
    return false;
}

// fetch udp connection data of other client
void FugaContact::doFetch() {
    connect(this, SIGNAL(sig_received(std::string,std::vector<std::string>)),
            this, SLOT(slot_fetched(std::string,std::vector<std::string>)),Qt::UniqueConnection);
    stringstream ss("");
    ss << "r_udpdata- ;";
    send_direct(ss.str());
}

// fetch udpdata from answer
void FugaContact::slot_fetched(string in_type, vector<string> in_data) {

    // udpdata success
    if (in_type == "a_udpdata_ok") {
        if (m_name == "") m_name = in_data[0];
        m_udp_ip = new QHostAddress(in_data[1].c_str());
        m_udp_firstport = string2quint16(in_data[2]);
        disconnect(this, SIGNAL(sig_received(std::string,std::vector<std::string>)),
                   this, SLOT(slot_fetched(std::string,std::vector<std::string>)));
        doAccept();
        return;
    }

    // udpdata failed
    if (in_type == "a_udpdata_failed") {
        showError("Could not fetch UDP data of contact!");
    }
}

// do we have fetched the udp data of other client?
bool FugaContact::isFetched() {
    return (m_udp_firstport == 0) ? false : true;
}

// is this connection to the other client accepted?
void FugaContact::doAccept() {

    if (!isAccepted()) {
        if (!m_Fuga->getContacts()->registerContact(this)) {
            // another connection already exists!
            cout << "FugaContact: destroying myself" << endl;

            // move buffer to other Contact
            FugaContact* other = m_Fuga->getContacts()->getContact(m_name);
            other->send(m_buffer);

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

// ############################# send data ########################

void FugaContact::send(string in_msg) {
    if (!isConnected()) {
        cout << "FugaContact: send failed: No socket: " << m_name << endl;
        addToBuffer(in_msg);
        doConnect();
        return;
    }
    if (!isAccepted()) {
        cout << "FugaContact: send failed; we are not accepted yet!" << endl;
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
    cout << "FugaContact: Received sth!" << endl;

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
        vector<string> parts = split(*i, "-");
        if (parts.size() != 2) {
            // invalid message!
            cout << "FugaContact: Invalid message received! (" << parts.size() << ")" << endl;
            continue;
        }
        string type = parts[0];
        string data = parts[1];

        // split data into vector
        vector<string> splitted = split(data, ",");

        cout << "FugaContact: SIG_RECEIVED " << type << endl;
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
    cout << "FugaContact: SSL errors" << endl;
    m_socket->ignoreSslErrors();
}

// ########################## client - me #############################

void FugaContact::slot_gotRequest(string in_type,vector<string> in_data) {

    if (in_type == "r_udpdata") {
        stringstream ss("");
        FugaMe* Me = m_Fuga->getMe();
        if (Me == NULL) {
            ss << "a_udpdata_failed-Not ready yet;";
            send_direct(ss.str());
            return;
        }
        QHostAddress udpip = QHostAddress(m_Fuga->getConfig()->getConfig("udp_ip").c_str());
        quint16 udpport = m_Fuga->getConfig()->getInt("udp_firstport");
        ss << "a_udpdata_ok-" << Me->name() << ","
           << udpip.toString().toAscii().data() << "," << udpport << ";";
        send_direct(ss.str());
    }
}
