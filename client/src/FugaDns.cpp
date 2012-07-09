#include "FugaDns.h"
#include "Fuga.h"
#include "FugaHelperFuncs.h"

using namespace std;

FugaDns::FugaDns(Fuga* in_Fuga)
    : FugaContact(in_Fuga, "root")
{
    m_Fuga = in_Fuga;

    // connect
    doConnect();
}

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
