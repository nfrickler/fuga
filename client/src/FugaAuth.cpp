#include "FugaAuth.h"

using namespace std;

FugaAuth::FugaAuth(Fuga* in_Fuga) {
    m_Fuga = in_Fuga;
}

/* send login to server
 */
void FugaAuth::slot_login (string in_name, string in_password) {
    stringstream ss("");
    ss << "r_login-" << in_name << "," << in_password << ";";
    cout << "FugaAuth: send login request: " << ss.str() << endl;
    m_Fuga->getContacts()->getContact("root")->send(ss.str());
}

/* is logged in?
 */
bool FugaAuth::isLogged() {
    return false;
}
