#include "FugaCrypto.h"
#include <QtCrypto/QtCrypto>

using namespace std;

// constructor
FugaCrypto::FugaCrypto(Fuga* in_Fuga) {
    m_Fuga = in_Fuga;

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
std::string FugaCrypto::getPubkey() {
    return m_pubkey.toPEM().toAscii().data();
}

// get signature
std::string FugaCrypto::getSignature() {
    return m_privkey.signature().data();
}

// sign message
std::string FugaCrypto::sign(std::string in_msg) {
    if(!m_privkey.canSign()) {
        cout << "FugaCrypto: Cannot sign!" << endl;
        return "";
    }
    return QCA::arrayToHex(m_privkey.signMessage(QCA::SecureArray(in_msg.c_str()),
                                                 QCA::EMSA1_SHA1)).toAscii().data();
}

// verify message
bool FugaCrypto::verify(string in_name, string in_pubkey, string in_msg, string in_sig) {

    // pubkey for this name already in database?
    QCA::PublicKey pubkey;
    if (isPubkeyOf(in_name)) {
        pubkey = getPubkeyOf(in_name);
    } else {
        pubkey = QCA::PublicKey::fromPEM(QString(in_pubkey.c_str()));
        savePubkeyOf(in_name, pubkey);
    }

    // verify
    return pubkey.verifyMessage(QCA::SecureArray(in_msg.c_str()),
                                QCA::hexToArray(QString(in_sig.c_str())),
                                QCA::EMSA1_SHA1);
}

// do we have saved the pubkey of?
bool FugaCrypto::isPubkeyOf(std::string) {
    // TODO
    return false;
}

// check if pubkey of other user already in save and return it
QCA::PublicKey FugaCrypto::getPubkeyOf(std::string in_name) {
    // TODO: search in locale file
    QCA::PublicKey pubkey = QCA::PublicKey::fromPEM(QString(""));
    return pubkey;
}

// save pubkey of other user
void FugaCrypto::savePubkeyOf(std::string in_name, QCA::PublicKey in_pubkey) {
    // TODO: save in locale file
    return;
}
