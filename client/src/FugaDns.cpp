#include "FugaDns.h"
#include "Fuga.h"
#include "FugaHelperFuncs.h"
#include <sstream>

using namespace std;

FugaDns::FugaDns(Fuga* in_Fuga)
    : FugaContact(in_Fuga)
{
    m_Fuga = in_Fuga;
    m_name = "root";

    // connect
    connect(this,SIGNAL(sig_disconnected()),this,SLOT(slot_disconnected()));
}

// send request to server to resolve username
bool FugaDns::resolve(std::string in_name) {
    stringstream msg("");
    msg << "r_name2tcp-" << in_name << ";";
    send(msg.str().data());
    return true;
}

// we have received an answer to a resolve request
void FugaDns::slot_doResolve (std::string in_type,std::vector<std::string> in_data) {

    // is message we expect?
    if (in_type != "a_name2tcp") return;

    // extract data from in_data
    if (in_data.size() != 3) {
        // invalid message!
        cout << "FugaDns: Invalid message received! (" << in_data.size() << ")" << endl;
        return;
    }

    // emit signal
    string name = in_data[0];
    QHostAddress* ip = new QHostAddress(in_data[1].c_str());
    quint16 port = string2quint16(in_data[2]);
    emit sig_resolved(name, ip, port);
}

void FugaDns::slot_resolved(std::string in_name, QHostAddress* in_ip, quint16 in_port) {
    return;
}

// send login request to server
void FugaDns::login(std::string in_name, std::string in_password) {
    connect(this, SIGNAL(sig_received(std::string,std::vector<std::string>)),
            this, SLOT(slot_checklogin(std::string,std::vector<std::string>)));
    QHostAddress myip = QHostAddress(m_Fuga->getConfig()->getConfig("tcp_ip").c_str());
    quint16 myport = m_Fuga->getConfig()->getInt("tcp_port");
    stringstream ss("");
    ss << "r_login-" << in_name << ","
       << in_password << ","
       << myip.toString().toStdString() << "," << myport << ","
       << ";";
    send(ss.str());
}

// check result of login request
void FugaDns::slot_checklogin(std::string in_type,std::vector<std::string> in_data) {
    cout << "FugaDns: Got answer to login: " << in_type << endl;

    // successful login?
    if (in_type == "a_login_ok") {
        emit sig_loggedin(0);
        disconnect(this, SIGNAL(sig_received(std::string,std::vector<std::string>)),
                   this, SLOT(slot_checklogin(std::string,std::vector<std::string>)));
    }

    // confirm?
    if (in_type == "a_login_confirm") {
        emit sig_loggedin(1);
    }

    // fail?
    if (in_type == "a_login_failed") {
        emit sig_loggedin(2);
    }
}

void FugaDns::send(string in_msg) {
    if (!isConnected()) {
        cout << "FugaContact: send failed: No socket to " << m_name << endl;
        addToBuffer(in_msg);
        doConnect();
        return;
    }
    send_direct(in_msg);
}

// ###################### connect to root ########################

// connect to root server
void FugaDns::doConnect() {

    // get config
    m_tcp_ip = new QHostAddress(m_Fuga->getConfig()->getConfig("root_ip").c_str());
    m_tcp_port = m_Fuga->getConfig()->getInt("root_port");

    // connect receive to resolve
    connect(this, SIGNAL(sig_received(std::string,std::vector<std::string>)),
            this, SLOT(slot_doResolve(std::string,std::vector<std::string>)));

    // connect
    FugaContact::doConnect();
}

// we are connected
void FugaDns::slot_connected() {
    cout << "FugaDns: slot_connected to root" << endl;
    doVerify();
}

// fetch udp connection data of other client
void FugaDns::doVerify() {
    connect(this, SIGNAL(sig_received(std::string,std::vector<std::string>)),
            this, SLOT(slot_verify(std::string,std::vector<std::string>)),Qt::UniqueConnection);

    // get signature
    stringstream ss("");
    m_msg2sign = rand();
    ss << "r_sverify-" << m_msg2sign << ";";
    send_direct(ss.str());
}

// receive answer to verification request
void FugaDns::slot_verify(string in_type, vector<string> in_data) {
    if (in_type.compare("a_sverify")) return;
    cout << "FugaDns: Verify root" << endl;

    if (in_data.size() != 4) {
        showError("FugaDns server sent invalid verify message!");
        return;
    }

    // verify root
    FugaCrypto* Crypto = m_Fuga->getContacts()->getCrypto();
    stringstream msg("");
    msg << m_msg2sign << "_" << in_data[1] << "_" << in_data[0];
    cout << "Verify 3" << endl;
    if (!Crypto->verify(m_name,in_data[2],msg.str(),in_data[3])) {
        showError("Failed to verify identity of root server!");
        return;
    }
    cout << "FugaDns: Root verification ACCEPTED." << endl;

    // connected successfully
    sendBuffer();
    emit sig_connected();
    emit sig_accepted();
}

void FugaDns::slot_disconnected() {
    showError("Connection to root server has been disconnected.");
    m_Fuga->slot_mode_login();
}
