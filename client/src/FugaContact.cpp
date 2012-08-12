#include "FugaContact.h"
#include "Fuga.h"
#include "FugaStreamer.h"
#include "FugaVideo.h"
#include "FugaHelperFuncs.h"

using namespace std;

// constructors
FugaContact::FugaContact(Fuga* in_Fuga, std::string in_name)
    : FugaSocket(in_Fuga)
{
    init();
    m_name = in_name;
    m_isaccepted = true;
    doResolve();
}
FugaContact::FugaContact(Fuga* in_Fuga, QSslSocket* in_socket)
    : FugaSocket(in_Fuga)
{
    init();
    m_socket = in_socket;
    cout << m_id << " | New connection" << endl;
    connectSocket();
    m_socket->startServerEncryption();
}

// init
void FugaContact::init() {
    m_isaccepted = false;
    m_udp_ip = NULL;
    m_udp_firstport = 0;
    m_Streamer = NULL;
    m_verified_active = false;
    m_verified_passive = false;
    m_pubkey = "";

    connect(this,SIGNAL(sig_connected()),this,SLOT(slot_start_hello()));
    connect(this,SIGNAL(sig_received(std::string,std::vector<std::string>)),
            this,SLOT(slot_gotRequest(std::string,std::vector<std::string>)));
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

// resolve name
void FugaContact::doResolve() {
    if (m_name == "" || isResolved()) return;
    FugaDns* Dns = m_Fuga->getContacts()->getDns(name2network(m_name));
    connect(Dns, SIGNAL(sig_resolved(std::string,QHostAddress*,quint16,std::string)),
            this, SLOT(slot_resolved(std::string,QHostAddress*,quint16,std::string)),Qt::UniqueConnection);
    Dns->doResolve(m_name);
}

// we have got the dns data
void FugaContact::slot_resolved(std::string in_name, QHostAddress* in_ip, quint16 in_port, std::string in_pubkey) {
    if (in_name.compare(m_name) != 0) return;
    cout << m_id << " | FugaContact: slot_resolved: " << m_name << endl;

    // disconnect
    FugaDns* Dns = m_Fuga->getContacts()->getDns(name2network(m_name));
    disconnect(Dns, SIGNAL(sig_resolved(std::string,QHostAddress*,quint16,std::string)),
            this, SLOT(slot_resolved(std::string,QHostAddress*,quint16,std::string)));

    // save infos
    m_tcp_ip = in_ip;
    m_tcp_port = in_port;
    m_pubkey = in_pubkey;

    // resolving successful?
    if (m_tcp_port == 0 || m_tcp_ip == NULL) {
        stringstream msg("");
        msg << "User '" << m_name << "' doesnt exist!" << endl;
        showError(msg.str());
        doDisconnect();
        m_Fuga->slot_mode_select();
    }

    if (isHello()) {
        doVerify();
    } else {
        doConnect();
    }
}

// is username resolved?
bool FugaContact::isResolved() {
    return (m_pubkey.empty()) ? false : true;
}

// fetch udp connection data of other client
void FugaContact::slot_start_hello() {

    // is it my turn to send r_hello?
    if (m_name == "") return;
    cout << m_id << " | FugaContact: Send hello" << endl;

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
       << Crypto->sign(msg.str()) << ";";
    send_direct(ss.str());
}

// receive answer to verification request
void FugaContact::slot_verify(string in_type, vector<string>) {

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
        if (m_name.compare(m_Fuga->getMe()->name()) == 0) {
            cout << "FugaContact: Connected with myself..." << endl;
            m_isaccepted = true;
            return;
        }

        if (!m_Fuga->getContacts()->registerContact(this)) {
            // another connection already exists!
            cout << m_id << " | FugaContact: destroying myself" << endl;

            // move buffer to other Contact
            FugaContact* other = m_Fuga->getContacts()->getContact(m_name);
            other->send(m_buffer);

            doDisconnect();
            return;
        }
    }

    cout << m_id << " | FugaContact: Contact accepted" << endl;
    m_isaccepted = true;
    emit sig_connectionready();
}

// is this connection to the other client accepted?
bool FugaContact::isAccepted() {
    return m_isaccepted;
}

// ############################# send data ########################

bool FugaContact::isConnectionReady() {
    return isAccepted();
}

// ########################## act as server #############################

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
        if (isResolved()) {
            doVerify();
        } else {
            doResolve();
        }
    }

    if (in_type == "r_verify") {
        stringstream ss("");
        FugaMe* Me = m_Fuga->getMe();
        if (in_data.size() != 2) {
            ss << "a_verify_failed-Invalid package;";
            send_direct(ss.str());
            return;
        }
        if (Me == NULL || m_pubkey.empty()) {
            ss << "a_verify_failed-Not ready yet;";
            send_direct(ss.str());
            return;
        }

        // verify
        FugaCrypto* Crypto = m_Fuga->getContacts()->getCrypto();
        stringstream msg("");
        msg << m_name << "_" << in_data[0] << "_" << m_Fuga->getMe()->name();
        if (!Crypto->verify(m_name,m_pubkey,msg.str(),in_data[1])) {
            ss << "a_verify_failed-Invalid signature;";
            send_direct(ss.str());
            return;
        }

        // verification ok
        ss << "a_verify_ok-;";
        send_direct(ss.str());
        cout << m_id << " | I HAVE ACCEPTED" << endl;
        m_verified_active = true;
        doAccept();
    }
}
