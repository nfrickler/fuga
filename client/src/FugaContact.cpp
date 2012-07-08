#include "FugaContact.h"

using namespace std;

FugaContact::FugaContact(Fuga* in_Fuga, std::string in_name) {

    // save
    m_Fuga = in_Fuga;

    // set to NULL
    m_socket = NULL;
}

// ############################# accessors ########################

std::string FugaContact::name() {
    return (isData()) ? m_name : NULL;
}

QHostAddress* FugaContact::udp_ip() {
    return (isData()) ? m_udp_ip : NULL;
}

quint16 FugaContact::udp_port() {
    return (isData()) ? m_udp_port : 0;
}

quint16	FugaContact::img_width() {
    return (isData()) ? m_img_width : 0;
}

quint16	FugaContact::img_height() {
    return (isData()) ? m_img_height : 0;
}

QTcpSocket*	FugaContact::tcp_socket() {
    return (isData()) ? m_socket : NULL;
}

quint16	FugaContact::tcp_port() {
    return (isData()) ? m_tcp_port : 0;
}

QHostAddress* FugaContact::tcp_ip() {
    return (isData()) ? m_tcp_ip : 0;
}

std::string	FugaContact::tcp_buffer() {
    return (isData()) ? m_buffer : 0;
}

// ############################# streaming ########################

bool FugaContact::startStreaming() {

    // already exists?
    m_mutex->lock();
    if (m_Streamer != NULL) {
        m_mutex->unlock();
        return m_Streamer;
    }
    m_mutex->unlock();

    // has enough data?
    if (!isData()) return NULL;

    // create new streamer
    m_mutex->lock();
    m_Streamer = new FuGaStreamer(
        m_udp_ip,
        m_Fuga->getConfig()->getInt("udp_port"),
        m_Fuga->getConfig()->getInt("img_width"),
        m_Fuga->getConfig()->getInt("img_height"),
        m_Fuga->getConfig()->getConfig("video_path")
    );
    m_mutex->unlock();

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

FuGaVideo* FugaContact::Video() {

    // has enough data?
    if (!isData()) return NULL;

    // create new streamer
    FuGaVideo* video = new FuGaVideo(
        m_udp_ip,
        m_udp_port
    );

    return video;
}

// ############################# status ########################

bool FugaContact::isConnected() {
    bool output = false;
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        output = true;
    }
    return output;
}

bool FugaContact::isVideoReady() {

    return true;
}

bool FugaContact::isData() {
    return (m_name.empty()) ? false : true;
}

// ############################# send data ########################

void FugaContact::send(string in_msg) {
    if (m_socket == NULL) {
        cout << "FugaContact: send failed: No socket: " << m_name << endl;
        addToBuffer(in_msg);
        getInfos();
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

// get infos for this user
void FugaContact::getInfos() {
    FugaContact* Root = m_Fuga->getContacts()->getContact("root");
    connect(Root, SIGNAL(sig_received(std::string,std::string)),
            this, SLOT(slot_infos(std::string,std::string)));
    stringstream ss("");
    ss << "r_getinfo-" << m_name << ";";
    Root->send(ss.str());
}

// ############################# receive data ########################

// slot to receive data
void FugaContact::slot_receive () {

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

void FugaContact::slot_infos(std::string in_type, std::string in_data) {
    if (in_type != "a_name2tcp") return;

    // disconnect
    FugaContact* Root = m_Fuga->getContacts()->getContact("root");
    disconnect(Root, SIGNAL(sig_received(std::string,std::string)),
            this, SLOT(slot_infos(std::string,std::string)));

    // split infos
    vector<string> configs = split(in_data, "|");
    for (vector<string>::iterator ii = configs.begin(); ii != configs.end(); ++ii) {
        vector<string> config_values = split(*ii, ":");
        if (config_values.size() != 2) continue;
    }

}

void FugaContact::slot_connected() {
    sendBuffer();
}

