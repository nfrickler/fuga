#include "FugaCrypto.h"
#include <QtCrypto/QtCrypto>

using namespace std;

// constructor
FugaCrypto::FugaCrypto(Fuga* in_Fuga) {
    m_Fuga = in_Fuga;
    QCA::Initializer init = QCA::Initializer();

    // load keys
    QFile* myfile = new QFile("fugakey.priv");
    if (myfile->exists()) {
        m_privkey = QCA::PrivateKey::fromPEMFile(myfile->fileName(),"mypassphrase");
    } else {
        createKeys();
    }
    m_pubkey = m_privkey.toPublicKey();
}

// create new keys
bool FugaCrypto::createKeys () {
    m_privkey = QCA::KeyGenerator().createRSA(1024);
    m_privkey.toPEMFile("fugakey.priv","mypassphrase");
}

// get public key
std::string FugaCrypto::getPublic() {
    return m_pubkey.toPEM().toAscii().data();
}

// get signature
std::string FugaCrypto::getSignature() {
    return m_privkey.signature().data();
}

// sign message
std::string FugaCrypto::sign(std::string in_msg) {
    return m_privkey.signMessage(QCA::MemoryRegion(in_msg.c_str()),QCA::EMSA1_SHA1).data();
}

// verify message
bool FugaCrypto::verify(std::string in_msg, std::string sig) {
    return m_pubkey.verifyMessage(QCA::MemoryRegion(in_msg.c_str()),
                                  QByteArray(sig.c_str()),
                                  QCA::EMSA1_SHA1);
}

