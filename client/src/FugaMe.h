#ifndef FUGAME_H
#define FUGAME_H

#include "FugaContact.h"

class Fuga;

class FugaMe : public FugaContact {
    Q_OBJECT

    public:
        FugaMe(Fuga* in_Fuga, std::string in_name);
        std::string name();
        bool name(std::string in_name);

    protected:
        Fuga* m_Fuga;
        std::string m_name;
};

#endif // FUGAME_H
