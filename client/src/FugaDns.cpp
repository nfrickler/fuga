#include "FugaDns.h"
#include "Fuga.h"
#include "FugaHelperFuncs.h"
#include <sstream>

using namespace std;

// constructor
FugaDns::FugaDns(Fuga* in_Fuga)
    : FugaServerSocket(in_Fuga)
{
    m_name = "mydns";
    m_isloggedin = false;
}

// ###################### connect ########################

// we cannot connect yet as we don't know to what server
void FugaDns::doConnect() {
    // get our network
    m_name = m_Fuga->getMe()->network();

    // netresolve network
    connect(m_Fuga->getContacts()->getNetDns(),SIGNAL(sig_resolved(std::string,QHostAddress*,quint16,std::string)),
            this,SLOT(slot_start_reallyconnecting(std::string,QHostAddress*,quint16,std::string)));
    m_Fuga->getContacts()->getNetDns()->doResolve(m_name);
}

void FugaDns::slot_start_reallyconnecting(std::string in_name, QHostAddress* in_ip, quint16 in_port, std::string in_pubkey) {
    if (in_name.compare(m_name) != 0) return;

    disconnect(m_Fuga->getContacts()->getNetDns(),SIGNAL(sig_resolved(std::string,QHostAddress*,quint16,std::string)),
            this,SLOT(slot_start_reallyconnecting(std::string,QHostAddress*,quint16,std::string)));

    // save data
    m_tcp_ip = in_ip;
    m_tcp_port = in_port;

    FugaSocket::doConnect();
}

// do disconnect
void FugaDns::doDisconnect() {
    showError("Connection to root server has been disconnected!");
    emit sig_disconnected();
}

// ########################## login ##############################

// send login request to server
void FugaDns::doLogin() {
    cout << m_id << " | FugaDns: do login..." << endl;
    connect(this, SIGNAL(sig_received(std::string,std::vector<std::string>)),
            this, SLOT(slot_checklogin(std::string,std::vector<std::string>)));
    FugaMe* Me = m_Fuga->getMe();
    QHostAddress myip = QHostAddress(m_Fuga->getConfig()->getConfig("tcp_ip").c_str());
    quint16 myport = m_Fuga->getConfig()->getInt("tcp_port");
    stringstream ss("");
    ss << "r_login-" << Me->name() << ","
       << Me->password() << ","
       << myip.toString().toStdString() << "," << myport << ","
       << m_Fuga->getContacts()->getCrypto()->getPubkey() <<";";
    send_direct(ss.str());
}

// check result of login request
void FugaDns::slot_checklogin(std::string in_type,std::vector<std::string>) {
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
