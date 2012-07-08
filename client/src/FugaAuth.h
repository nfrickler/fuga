#ifndef FUGAAUTH_H
#define FUGAAUTH_H

#include "Fuga.h"
#include <iostream>

class Fuga;

class FugaAuth : public QObject {
    Q_OBJECT

    public:
        FugaAuth(Fuga* in_Fuga);
        bool isLogged();

    protected:
        Fuga* m_Fuga;

    public slots:
        void slot_login (std::string in_name, std::string in_password);
};

#endif // FUGAAUTH_H
