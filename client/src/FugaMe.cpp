#include "FugaMe.h"
#include "FugaHelperFuncs.h"
#include <vector>

using namespace std;

FugaMe::FugaMe(Fuga* in_Fuga, string in_name, string in_password)
    : FugaContact(in_Fuga,in_name),
      m_Fuga(in_Fuga),
      m_name(in_name),
      m_password(in_password)
{

}

void FugaMe::name(std::string in_name) {
    m_name = in_name;
}

void FugaMe::password(std::string in_password) {
    m_password = in_password;
}

string FugaMe::name() {
    return (m_name.empty()) ? "" : m_name;
}

string FugaMe::password() {
    return (m_password.empty()) ? "" : m_password;
}

string FugaMe::network() {
    if (m_name.empty()) return "";
    vector<string> splitted = split(m_name,"%");
    if (splitted.size() != 2) return "";
    return splitted[1];
}
