#ifndef FUGACRYPTO_H
#define FUGACRYPTO_H

#include "Fuga.h"
#include <QtCrypto/QtCrypto>

class FugaCrypto {

    public:
        FugaCrypto(Fuga*);
        bool createKeys();
        std::string getPublic();
        std::string getSignature();

        std::string sign(std::string in_msg);
        bool verify(std::string, std::string);

    protected:
        Fuga* m_Fuga;
        QCA::PrivateKey m_privkey;
        QCA::PublicKey m_pubkey;

};

#endif // FUGACRYPTO_H
