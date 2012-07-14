#include "FugaNetDns.h"
#include "Fuga.h"

using namespace std;

// constructor
FugaNetDns::FugaNetDns(Fuga* in_Fuga)
    : FugaServerSocket(in_Fuga)
{
    cout << m_id << " | FugaNetDns: new FugaNetDns object" << endl;
    m_name = "root";
    m_tcp_ip = new QHostAddress(m_Fuga->getConfig()->getConfig("root_ip").c_str());
    m_tcp_port = m_Fuga->getConfig()->getInt("root_port");
}
