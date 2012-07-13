#include "Fuga.h"
#include <QMessageBox>
#include <dlfcn.h>

using namespace std;

// constructor
Fuga::Fuga() {

    // create main objects
    m_Config = new FugaConfig();
    m_Window = new FugaWindow(this);
    m_Contacts = new FugaContacts(this);
    m_Me = new FugaMe(this,"");

    m_selected = NULL;

    // start login process
    slot_mode_login();
}

// ########################## modes ###############################

// handle current mode
void Fuga::run() {

    // login mode?
    if (!isLogged()) {
        mode_login();
        return;
    }

    // select mode?
    if (!isSelected()) {
        mode_select();
        return;
    }

    mode_module();
}

// login process
void Fuga::mode_login () {
    cout << "Fuga: Enter login mode" << endl;

    // show login window
    m_Window->showLogin();
}

// select a module process
void Fuga::mode_select () {
    cout << "Fuga: Enter select mode" << endl;

    // show login window
    m_Window->showSelection();
}

// select a module process
void Fuga::mode_module () {
    cout << "Fuga: Enter module mode" << endl;

    // get videochat-object
    m_selected = new MFugaVideochat(getWindow(), this);
}

// are we logged in?
bool Fuga::isLogged() {
    return (m_loggedin) ? true : false;
}

// have we selected a module?
bool Fuga::isSelected() {
    return (m_selected == NULL) ? false : true;
}

// ########################## accessors ###############################

// get FugaWindow
FugaWindow* Fuga::getWindow() {
    return m_Window;
}

// get FugaConfig
FugaConfig* Fuga::getConfig() {
    return m_Config;
}

// get FugaContacts
FugaContacts* Fuga::getContacts() {
    return m_Contacts;
}

// get FugaMe
FugaMe* Fuga::getMe() {
    return m_Me;
}

// ########################## login ###############################

// back to login mode (aka logout)
void Fuga::slot_mode_login() {
    if (m_selected) delete m_selected;
    m_selected = NULL;
    m_loggedin = false;
    run();
}

// send login to server
void Fuga::slot_login(string in_name, string in_password) {
    disconnect(getWindow(), SIGNAL(sig_login(std::string,std::string)),
            this, SLOT(slot_login(std::string,std::string)));
    m_name = in_name;
    m_password = in_password;
    doLogin();
}

// send login to server
void Fuga::doLogin() {
    cout << "Fuga: Try to login..." << endl;
    m_Me = new FugaMe(this,m_name);
    FugaDns* Dns = getContacts()->getDns();
    connect(Dns, SIGNAL(sig_loggedin(int)),this,SLOT(slot_checklogin(int)),Qt::UniqueConnection);
    Dns->doLogin(m_name, m_password);
}

// get result of login request
void Fuga::slot_checklogin(int in_return) {
    cout << "Fuga: Got answer to login request: " << in_return << endl;

    switch (in_return) {
        case 0: {
            // success
            cout << "Fuga: Login success!" << endl;
            m_name = "";
            m_password = "";
            m_loggedin = true;
            break;
        }
        case 1: {
            // please confirm
            cout << "Fuga: Login confirm!" << endl;
            QMessageBox msgBox;
            msgBox.setText("Es existiert noch kein Nutzer mit diesem Namen. Möchtest du dich als neuer Nutzer unter diesem Namen registrieren?");
            msgBox.setInformativeText("Registrierung bestätigen");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            if (msgBox.exec() == QMessageBox::Yes) {
                cout << "Fuga: Confirming registration..." << endl;
                doLogin();
                return;
            }

            // login failed
            m_name = "";
            m_password = "";
            // no break
        }
        case 2: {
            showError("Login failed!");
        }
    }

    run();
}

// ########################## selection ###############################

// back to select mode
void Fuga::slot_mode_select() {
    if (m_selected) delete m_selected;
    m_selected = NULL;
    cout << "Fuga slot_mode_select ok" << endl;
    run();
}

// ########################## modules ###############################

void Fuga::slot_startModule(QString in_name) {
    cout << "Fuga: Module '" << in_name.toStdString() << "'has been selected" << endl;
    mode_module();
}
