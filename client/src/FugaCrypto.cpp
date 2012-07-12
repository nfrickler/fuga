#include "FugaCrypto.h"
#include <QtCrypto/QtCrypto>

using namespace std;
std::string base64_decode(std::string const& encoded_string);

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

    QByteArray mysigned = m_privkey.signMessage(QCA::SecureArray(in_msg.c_str()),QCA::EMSA1_SHA1);
    return mysigned.toBase64().data();
}

// verify message
bool FugaCrypto::verify(string in_name, string in_pubkey, string in_msg, string in_sig) {

    // pubkey for this name already in database?
    QCA::PublicKey pubkey;
    if (isPubkeyOf(in_name)) {
        pubkey = getPubkeyOf(in_name);
    } else {
        pubkey = QCA::PublicKey::fromPEM(QByteArray(in_pubkey.c_str()));
        savePubkeyOf(in_name, pubkey);
    }

    cout << "in_sig: " << in_sig << endl;
    QByteArray signature = QByteArray::fromBase64(QByteArray(in_sig.c_str()));
    cout << "in_sig: " << signature.data() << endl;

    // verify
    if (pubkey.verifyMessage(QCA::SecureArray(in_msg.c_str()),
                                signature,
                                QCA::EMSA1_SHA1
                                )
    ) {
        cout << "IDENTITY VERIFIED" << endl;
    } else {
        cout << "FAILED TO VERIFY THE IDENTITY!!!!!!!!!"
             << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
    }
    return true;
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
