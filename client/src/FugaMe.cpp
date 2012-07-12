#include "FugaMe.h"

using namespace std;

FugaMe::FugaMe(Fuga* in_Fuga, string in_name)
    : FugaContact(in_Fuga,in_name),
      m_Fuga(in_Fuga),
      m_name(in_name)
{

}

string FugaMe::name() {
    return (m_name.empty()) ? "" : m_name;
}
