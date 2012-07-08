#include "Fuga.h"

using namespace std;

/* constructor
 */
Fuga::Fuga() {

    // create main objects
    m_Config = new FugaConfig();
    m_Auth = new FugaAuth(this);
    m_Window = new FugaWindow(this);
    m_Contacts = new FugaContacts(this);

    // start login process
    _mode_login();
}

/* login process
 */
void Fuga::_mode_login () {
    cout << "Fuga: Enter login mode" << endl;

    // check, if logged in already
    if (m_Auth->isLogged()) {
        _mode_select();
        return;
    }

    // show login window
    m_Window->showLogin(m_Auth);
}

/* select a module process
 */
void Fuga::_mode_select () {
    cout << "Fuga: Enter select mode" << endl;

    // show login window
    m_Window->showSelection();
}

/* select a module process
 */
void Fuga::_mode_module () {
    cout << "Fuga: Enter module mode" << endl;

    // create module object
    // TODO

    // bind signal to fall back to selection process
    // TODO

    // run module
    // TODO

    // get videochat-object
    new ModVideochat(getWindow(), this);
}

/* get FugaWindow
 */
FugaWindow* Fuga::getWindow() {
    return m_Window;
}

/* get FugaAuth
 */
FugaAuth* Fuga::getAuth() {
    return m_Auth;
}

/* get FugaConfig
 */
FugaConfig* Fuga::getConfig() {
    return m_Config;
}

/* get FugaContacts
 */
FugaContacts* Fuga::getContacts() {
    return m_Contacts;
}

void Fuga::slot_mode_login() {
    _mode_login();
}

void Fuga::slot_mode_select() {
    _mode_select();
}

void Fuga::slot_mode_module() {
    _mode_module();
}

