#include "FugaServerSocket.h"
#include <sstream>
#include "FugaHelperFuncs.h"
#include "Fuga.h"

using namespace std;

// constructor
FugaServerSocket::FugaServerSocket(Fuga* in_Fuga)
    : FugaSocket(in_Fuga)
{
    connect(this,SIGNAL(sig_connected()),this,SLOT(slot_start_verify()));
    connect(this,SIGNAL(sig_received(std::string,std::vector<std::string>)),
            this,SLOT(slot_resolved(std::string,std::vector<std::string>)));
}

// ###################### verify connection ########################

// fetch udp connection data of other client
void FugaServerSocket::slot_start_verify() {
    connect(this, SIGNAL(sig_received(std::string,std::vector<std::string>)),
            this, SLOT(slot_verify(std::string,std::vector<std::string>)),Qt::UniqueConnection);

    // get signature
    stringstream ss("");
    m_msg2sign = rand();
    ss << "r_sverify-" << m_msg2sign << ";";
    send_direct(ss.str());
}

// receive answer to verification request
void FugaServerSocket::slot_verify(string in_type, vector<string> in_data) {
    if (in_type.compare("a_sverify")) return;
    cout << m_id << " | FugaServerSocket: Verify root" << endl;

    if (in_data.size() != 4) {
        showError("FugaServerSocket server sent invalid verify message!");
        return;
    }

    // verify root
    FugaCrypto* Crypto = m_Fuga->getContacts()->getCrypto();
    stringstream msg("");
    msg << m_msg2sign << "_" << in_data[1] << "_" << in_data[0];
    if (!Crypto->verify(m_name,in_data[2],msg.str(),in_data[3])) {
        showError("Failed to verify identity of root server!");
        return;
    }
    cout << m_id << " | FugaServerSocket: Root verification ACCEPTED." << endl;

    // connected successfully
    emit sig_connectionready();
}

// ####################### Resolving #########################

// send request to server to resolve username
void FugaServerSocket::doResolve(std::string in_name) {
    stringstream msg("");
    msg << "r_name2tcp-" << in_name << ";";
    send(msg.str().data());
}

// we have received an answer to a resolve request
void FugaServerSocket::slot_resolved(std::string in_type,std::vector<std::string> in_data) {

    // resolving failed?
    if (in_type == "a_name2tcp_failed") {
        emit sig_resolved(in_data[0], NULL, 0, "");
        return;
    }

    // is message we expect?
    if (in_type != "a_name2tcp") return;

    // extract data from in_data
    if (in_data.size() != 4) {
        // invalid message!
        cout << m_id << " | FugaServerSocket: Invalid message received! (" << in_data.size() << ")" << endl;
        return;
    }

    // emit signal
    string name = in_data[0];
    QHostAddress* ip = new QHostAddress(in_data[1].c_str());
    quint16 port = string2quint16(in_data[2]);
    string pubkey = in_data[3];
    emit sig_resolved(name, ip, port, pubkey);
}
