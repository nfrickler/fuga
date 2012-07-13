#ifndef FUGA_H
#define FUGA_H

#include "FugaDns.h"
#include "FugaMe.h"
#include "FugaContacts.h"
#include "MFugaVideochat.h"

#include "FugaConfig.h"
#include "FugaWindow.h"
#include "FugaContacts.h"
#include <iostream>

class MFugaVideochat;
class FugaConfig;
class FugaWindow;
class FugaContacts;
class FugaModule;

class Fuga : public QObject {
    Q_OBJECT

    public:
        Fuga();
        void run();
        FugaMe* getMe();
        FugaConfig* getConfig();
        FugaWindow* getWindow();
        FugaContacts* getContacts();

        bool isLogged();
        bool isSelected();

    protected:
        FugaMe* m_Me;
        FugaConfig* m_Config;
        FugaWindow* m_Window;
        FugaContacts* m_Contacts;

        bool m_loggedin;
        MFugaVideochat* m_selected;

        void mode_login();
        void doLogin();

        void mode_select();
        void mode_module();

    public slots:
        void slot_login(std::string in_name, std::string in_password);
        void slot_checklogin(int in_return);
        void slot_startModule(QString in_name);

        void slot_mode_select();
        void slot_mode_login();
};

#endif // FUGA_H
