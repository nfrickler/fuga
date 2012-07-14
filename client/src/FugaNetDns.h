#ifndef FUGANETDNS_H
#define FUGANETDNS_H

#include "FugaServerSocket.h"

class Fuga;

class FugaNetDns : public FugaServerSocket {
    Q_OBJECT

    public:
        FugaNetDns(Fuga* in_Fuga);

    signals:

    protected slots:

};

#endif // FUGANETDNS_H
