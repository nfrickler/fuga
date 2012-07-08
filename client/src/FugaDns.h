#ifndef FUGADNS_H
#define FUGADNS_H

#include "FugaContact.h"
#include "Fuga.h"
#include <QHostAddress>

class FugaDns : public FugaContact {

    public:
        FugaDns(Fuga* in_Fuga);

    signals:
        sig_resolved(std::string in_name, QHostAddress* in_ip, quint16 in_port);
};

#endif // FUGADNS_H
