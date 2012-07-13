#include "FugaSocket.h"
#include "FugaHelperFuncs.h"

using namespace std;

// constructor
FugaSocket::FugaSocket(Fuga* in_Fuga) {

    m_Fuga = in_Fuga;
    m_name = "";
    m_socket = NULL;
    m_tcp_ip = NULL;
    m_tcp_port = 0;
    m_buffer = "";
    m_id = rand() % 100;

}

// connect signals to socket
void FugaSocket::connectSocket() {
    if (m_socket == NULL) return;

    // startup
    connect(m_socket, SIGNAL(encrypted()), this, SLOT(slot_connected()),Qt::UniqueConnection);
    connect(this,SIGNAL(sig_connectionready()),this,SLOT(slot_connectionready()));
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

std::string FugaSocket::name() {
    return m_name;
}

// ########################### handle connection #########################

// connect
void FugaSocket::doConnect() {
    if (isConnected()) return;

    // username already resolved?
    if (m_tcp_ip == NULL || m_tcp_port == 0) {
        cout << m_id << " | FugaSocket: Cannot connect as we have no ip/port" << endl;
        return;
    }

    // start new socket
    cout << m_id << " | FugaSocket: Start socket to: "
         << m_tcp_ip->toString().toAscii().data() << " : " << m_tcp_port << endl;
    m_socket = new QSslSocket(0);
    connectSocket();
    m_socket->connectToHostEncrypted(m_tcp_ip->toString(), m_tcp_port);
}

// connection established
void FugaSocket::slot_connected() {
    cout << m_id << " | FugaSocket: Connection established" << endl;
    disconnect(m_socket, SIGNAL(encrypted()),this, SLOT(slot_connected()));
    emit sig_connected();
}

// connection established?
bool FugaSocket::isConnected() {
    return (m_socket && m_socket->state() == QAbstractSocket::ConnectedState)
        ? true : false;
}

// connection is ready for normal sending
void FugaSocket::slot_connectionready() {
    cout << m_id << " | FugaSocket: Connection is ready." << endl;
    sendBuffer();
}

// do disconnect
void FugaSocket::doDisconnect() {
    if (m_socket != NULL) m_socket->disconnectFromHost();
    emit sig_disconnected();
}

// other part disconnected
void FugaSocket::slot_disconnected() {
    doDisconnect();
}

// handle errors
void FugaSocket::slot_handleError(QAbstractSocket::SocketError in_error) {
    cout << m_id << " | FugaSocket: Connection error:" << in_error
         << " : " << m_socket->errorString().toAscii().data() << endl;
   // emit sig_disconnected();
}

// handle ssl errors
void FugaSocket::slot_sslerror(const QList<QSslError> &) {
    cout << m_id << " | FugaSocket: SSL error" << endl;
    m_socket->ignoreSslErrors();
}

// ############################## sending #########################

// send
void FugaSocket::send(string in_msg) {
    if (!isConnected()) {
        cout << m_id << " | FugaSocket: send failed: No socket to " << m_name << endl;
        addToBuffer(in_msg);
        doConnect();
        return;
    }
    if (!isConnectionReady()) {
        cout << m_id << " | FugaSocket: send failed; connection not ready!" << endl;
        addToBuffer(in_msg);
        return;
    }

    send_direct(in_msg);
}

// connection ready to send?
bool FugaSocket::isConnectionReady() {
    return true;
}

// actually send message
void FugaSocket::send_direct(string in_msg) {
    if (!m_socket->write(in_msg.c_str())) {
        cout << m_id << " | FugaSocket: send failed: " << m_socket->errorString().toAscii().data() << endl;
    }
    cout << m_id << " | FugaSocket: sent to " << m_name << ": " << in_msg << endl;
}

void FugaSocket::addToBuffer(std::string in_msg) {
    stringstream ss("");
    ss << m_buffer;
    ss << in_msg;
    m_buffer = ss.str();
}

void FugaSocket::sendBuffer() {
    cout << m_id << " | FugaSocket: Sending buffer..." << endl;
    if (!m_buffer.empty()) send(m_buffer);
}

// ############################# receive data ########################

// slot to receive waiting data
void FugaSocket::slot_received () {

    // read from socket
    if (m_socket == NULL || !m_socket->bytesAvailable()) return;
    QByteArray* currbuffer = new QByteArray();
    currbuffer->append(m_socket->readAll());

    // is ready message?
    string inputstring = currbuffer->data();
    vector<string> msgs = split(inputstring, ";");

    cout << m_id << " | FugaSocket: input (" << msgs.size() << "): " << inputstring << endl;

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

// handle requests
void FugaSocket::slot_gotRequest(std::string in_type,std::vector<std::string> in_data) {
    return;
}
