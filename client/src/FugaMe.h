#ifndef FUGAME_H
#define FUGAME_H

#include "FugaContact.h"

class Fuga;

class FugaMe : public FugaContact {
    Q_OBJECT

    public:
        FugaMe(Fuga* in_Fuga, std::string in_name, std::string in_password);
        std::string name();
        std::string password();
        std::string network();
        void name(std::string in_name);
        void password(std::string in_password);

    protected:
        Fuga* m_Fuga;
        std::string m_name;
        std::string m_password;
};

#endif // FUGAME_H
