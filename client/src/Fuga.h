#ifndef FUGA_H
#define FUGA_H

#include "FugaConfig.h"
#include "FugaWindow.h"
#include "FugaAuth.h"
#include "ModVideochat.h"
#include "FugaContacts.h"
#include <iostream>

class FugaConfig;
class FugaWindow;
class FugaAuth;
class ModVideochat;
class FugaContacts;

class Fuga : public QObject {
    Q_OBJECT

    public:
        Fuga();
        FugaConfig* getConfig();
        FugaWindow* getWindow();
        FugaAuth* getAuth();
        FugaContacts* getContacts();

    protected:
        FugaConfig* m_Config;
        FugaWindow* m_Window;
        FugaAuth* m_Auth;
        FugaContacts* m_Contacts;

        void _mode_login();
        void _mode_select();
        void _mode_module();

    public slots:
        void slot_mode_login();
        void slot_mode_select();
        void slot_mode_module();
};

#endif // FUGA_H
