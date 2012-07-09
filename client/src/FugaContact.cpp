#include "FugaContact.h"
#include "Fuga.h"
#include "FugaStreamer.h"
#include "FugaVideo.h"

using namespace std;

FugaContact::FugaContact(Fuga* in_Fuga, std::string in_name)
    : m_Fuga(in_Fuga),
      m_name(in_name)
{
    m_socket = NULL;
    m_tcp_ip = NULL;
    m_tcp_port = 0;

    if (in_name != "root") resolve();
}

// ############################# accessors ########################

std::string FugaContact::name() {
    return (isFetched()) ? m_name : NULL;
}

QHostAddress* FugaContact::udp_ip() {
    return (isFetched()) ? m_udp_ip : NULL;
}

quint16 FugaContact::udp_port() {
    return (isFetched()) ? m_udp_port : 0;
}

quint16	FugaContact::img_width() {
    return (isFetched()) ? m_img_width : 0;
}

quint16	FugaContact::img_height() {
    return (isFetched()) ? m_img_height : 0;
}

QTcpSocket*	FugaContact::tcp_socket() {
    return (isFetched()) ? m_socket : NULL;
}

quint16	FugaContact::tcp_port() {
    return (isFetched()) ? m_tcp_port : 0;
}

QHostAddress* FugaContact::tcp_ip() {
    return (isFetched()) ? m_tcp_ip : 0;
}

std::string	FugaContact::tcp_buffer() {
    return (isFetched()) ? m_buffer : 0;
}

// ############################# streaming ########################

bool FugaContact::startStreaming() {

    // already exists?
    if (m_Streamer != NULL) {
        return m_Streamer;
    }
    m_mutex->unlock();

    // has enough data?
    if (!isFetched()) return NULL;

    // create new streamer
    m_Streamer = new FugaStreamer(
        m_udp_ip,
        m_Fuga->getConfig()->getInt("udp_port"),
        m_Fuga->getConfig()->getInt("img_width"),
        m_Fuga->getConfig()->getInt("img_height"),
        m_Fuga->getConfig()->getConfig("video_path")
    );

    // emit signal
    emit sig_streaming();

    return m_Streamer;
}

bool FugaContact::stopStreaming() {

    m_mutex->lock();
    if (m_Streamer != NULL) {
        delete m_Streamer;
        m_Streamer = NULL;
    }
    m_mutex->unlock();

    return true;
}

FugaVideo* FugaContact::Video() {

    // has enough data?
    if (!isFetched()) return NULL;

    // create new streamer
    FugaVideo* video = new FugaVideo(
        m_udp_ip,
        m_udp_port
    );

    return video;
}

// ############################# status ########################

bool FugaContact::isVideoReady() {
    // TODO
    return true;
}


// resolve username
void FugaContact::resolve() {
    if (isResolved()) return;
    FugaDns* Dns = m_Fuga->getContacts()->getDns();
    connect(Dns, SIGNAL(sig_resolved(std::string,QHostAddress*,quint16)),
            this, SLOT(slot_resolved(std::string,QHostAddress*,quint16)),Qt::UniqueConnection);
    Dns->resolve(m_name);
}

// is username resolved?
bool FugaContact::isResolved() {
    return (m_tcp_ip == NULL) ? false : true;
}

// connect to other client
void FugaContact::doConnect() {

    // do we have required data?
    if (!isResolved()) {
        resolve();
        return;
    }

    // start new socket
    m_socket = new QTcpSocket(0);
    m_socket->connectToHost(*m_tcp_ip, m_tcp_port);
    cout << "FugaContact: Start socket to: "
         << m_tcp_ip->toString().toAscii().data() << " : " << m_tcp_port << endl;

    // connect signals
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)),
        this, SLOT(slot_handleError(QAbstractSocket::SocketError)),Qt::UniqueConnection);
    connect(m_socket, SIGNAL(disconnected()), m_socket, SLOT(deleteLater()),Qt::UniqueConnection);
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(slot_received()),Qt::UniqueConnection);

    connect(m_socket, SIGNAL(connected()), this, SLOT(slot_connected()),Qt::UniqueConnection);

}

// are we connected to other client
bool FugaContact::isConnected() {
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        return true;
    }
    return false;
}

// fetch data of other client
void FugaContact::doFetch() {

}

// do we have fetched the data of other client?
bool FugaContact::isFetched() {

    return true;
}

// ############################# send data ########################

void FugaContact::send(string in_msg) {
    if (m_socket == NULL) {
        cout << "FugaContact: send failed: No socket: " << m_name << endl;
        addToBuffer(in_msg);
        doConnect();
        return;
    }
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
    cout << "FugaContact: received sth" << endl;

    // read from socket
    if (m_socket == NULL || !m_socket->bytesAvailable()) return;
    QByteArray* currbuffer = new QByteArray();
    currbuffer->append(m_socket->readAll());

    // is ready message?
    string inputstring = currbuffer->data();
    vector<string> msgs = split(inputstring, ";");

    cout << "FugaTcp: input (" << msgs.size() << "): " << inputstring << endl;

    // iterate over all messages
    for (vector<string>::iterator i = msgs.begin(); i != msgs.end(); ++i) {

        // split type and data
        vector<string> parts = split(*i, "-");
        if (parts.size() != 2) {
            // invalid message!
            cout << "FugaTcp: Invalid message received! (" << parts.size() << ")" << endl;
            continue;
        }
        string type = parts[0];
        string data = parts[1];

        emit sig_received(type, data);
    }
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

void FugaContact::slot_connected() {
    cout << "FugaContact: slot_connected to " << m_name << endl;
    sendBuffer();
    emit sig_connected();
}

// handle errors
void FugaContact::slot_handleError(QAbstractSocket::SocketError in_error) {
    cout << "FugaContact: Connection error:" << in_error << endl;
}

