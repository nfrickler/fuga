#include "FugaDns.h"
#include "Fuga.h"
#include "FugaHelperFuncs.h"
#include <sstream>

using namespace std;

FugaDns::FugaDns(Fuga* in_Fuga)
    : FugaSocket(in_Fuga)
{
    m_name = "root";
    m_isloggedin = false;
    m_tcp_ip = new QHostAddress(m_Fuga->getConfig()->getConfig("root_ip").c_str());
    m_tcp_port = m_Fuga->getConfig()->getInt("root_port");
    connect(this,SIGNAL(sig_connected()),this,SLOT(slot_start_verify()));
    connect(this,SIGNAL(sig_received(std::string,std::vector<std::string>)),
            this,SLOT(slot_doResolve(std::string,std::vector<std::string>)));
}

// ####################### resolving #########################

// send request to server to resolve username
bool FugaDns::resolve(std::string in_name) {
    stringstream msg("");
    msg << "r_name2tcp-" << in_name << ";";
    send(msg.str().data());
    return true;
}

// we have received an answer to a resolve request
void FugaDns::slot_doResolve (std::string in_type,std::vector<std::string> in_data) {

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
        cout << m_id << " | FugaDns: Invalid message received! (" << in_data.size() << ")" << endl;
        return;
    }

    // emit signal
    string name = in_data[0];
    QHostAddress* ip = new QHostAddress(in_data[1].c_str());
    quint16 port = string2quint16(in_data[2]);
    string pubkey = in_data[3];
    emit sig_resolved(name, ip, port, pubkey);
}

// ########################## login ##############################

// send login request to server
void FugaDns::doLogin(std::string in_name, std::string in_password) {
    connect(this, SIGNAL(sig_received(std::string,std::vector<std::string>)),
            this, SLOT(slot_checklogin(std::string,std::vector<std::string>)));
    QHostAddress myip = QHostAddress(m_Fuga->getConfig()->getConfig("tcp_ip").c_str());
    quint16 myport = m_Fuga->getConfig()->getInt("tcp_port");
    stringstream ss("");
    ss << "r_login-" << in_name << ","
       << in_password << ","
       << myip.toString().toStdString() << "," << myport << ","
       << m_Fuga->getContacts()->getCrypto()->getPubkey() <<";";
    send_direct(ss.str());
}

// check result of login request
void FugaDns::slot_checklogin(std::string in_type,std::vector<std::string> in_data) {
    cout << m_id << " | FugaDns: Got answer to login: " << in_type << endl;

    // successful login?
    if (in_type == "a_login_ok") {
        m_isloggedin = true;
        emit sig_loggedin(0);
        disconnect(this, SIGNAL(sig_received(std::string,std::vector<std::string>)),
                   this, SLOT(slot_checklogin(std::string,std::vector<std::string>)));
        sendBuffer();
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

// are we logged in?
bool FugaDns::isLoggedin() {
    return m_isloggedin;
}

// Ready to send?
bool FugaDns::isConnectionReady() {
    return isLoggedin();
}

// ###################### connect to root ########################

// fetch udp connection data of other client
void FugaDns::slot_start_verify() {
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
    cout << m_id << " | FugaDns: Verify root" << endl;

    if (in_data.size() != 4) {
        showError("FugaDns server sent invalid verify message!");
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
    cout << m_id << " | FugaDns: Root verification ACCEPTED." << endl;

    // connected successfully
    emit sig_connectionready();
}

// do disconnect
void FugaDns::doDisconnect() {
    showError("Connection to root server has been disconnected!");
    emit sig_disconnected();
}
